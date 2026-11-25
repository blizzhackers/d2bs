#include "JSScript.h"
#include "Script.h"
#include "ScriptEngine.h"
#include "D2BS.h"
#include "Helpers.h"

EMPTY_CTOR(script)

// A Script object holds its script's id, not the Script: the script may be deleted while the object lives on.
// 0 for a foreign or prototype this. script_class has no trace or finalize hook, so the engine never reads it.
static DWORD ScriptIdOf(JSContext* cx, JSObject* obj) {
    return (DWORD)(UINT_PTR)JS_GetInstancePrivate(cx, obj, &script_class, NULL);
}
static void* ScriptPrivate(DWORD id) {
    return (void*)(UINT_PTR)id;
}

JSAPI_PROP(script_getProperty) {
    DWORD scriptId = ScriptIdOf(cx, obj);
    if (!scriptId)
        return JS_TRUE;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    int which = JSVAL_TO_INT(ID);
    std::wstring name;

    ScriptEngine::LockScriptList("script.getProperty");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    if (!script) {
        ScriptEngine::UnLockScriptList("script.getProperty");
        // An ended script reads as not running with an empty name (callers use string methods on name right after
        // iterating); its other members have no value. Set explicitly: the property keeps its slot, so leaving vp
        // alone would return the last value read while the script lived.
        if (which == SCRIPT_RUNNING)
            vp.setBoolean(false);
        else if (which == SCRIPT_FILENAME)
            return StoreString(vp, JS_GetEmptyString(JS_GetRuntime(cx))) ? JS_TRUE : JS_FALSE;
        else
            vp.setUndefined();
        return JS_TRUE;
    }
    switch (which) {
    case SCRIPT_FILENAME:
        name = script->GetShortFilename();
        break;
    case SCRIPT_GAMETYPE:
        vp.setBoolean(script->GetState() == InGame ? false : true);
        break;
    case SCRIPT_RUNNING:
        vp.setBoolean(script->IsRunning());
        break;
    case SCRIPT_THREADID:
        vp.setInt32(script->GetThreadId());
        break;
    case SCRIPT_MEMORY: {
        JSRuntime* rt = script->GetRuntime();
        vp.setInt32(rt ? JS_GetGCParameter(rt, JSGC_BYTES) : 0);
        break;
    }
    default:
        break;
    }
    ScriptEngine::UnLockScriptList("script.getProperty");

    // Interned after the lock is free: it allocates, and the GC may run.
    if (which == SCRIPT_FILENAME && !StoreString(vp, JS_InternUCString(cx, name.c_str())))
        return JS_FALSE;
    return JS_TRUE;
}

JSAPI_FUNC(script_getNext) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    DWORD scriptId = ScriptIdOf(cx, self);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    ScriptEngine::LockScriptList("script.getNext");
    for (ScriptMap::iterator it = ScriptEngine::scripts.begin(); scriptId && it != ScriptEngine::scripts.end(); it++) {
        if (it->second->GetId() == scriptId) {
            it++;
            if (it != ScriptEngine::scripts.end()) {
                JS_SetPrivate(cx, self, ScriptPrivate(it->second->GetId()));
                JS_SET_RVAL(cx, vp, JSVAL_TRUE);
            }
            break;
        }
    }
    ScriptEngine::UnLockScriptList("script.getNext");
    return JS_TRUE;
}

JSAPI_FUNC(script_stop) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    DWORD scriptId = ScriptIdOf(cx, self);
    ScriptEngine::LockScriptList("script.stop");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    // A not-started copy counts too: a reload's new copy is listed while its thread waits for the old copy and
    // compiles, and Run honours a stop set before it starts.
    bool notStarted = script && !script->hasActiveCX && script->GetExecutionCount() == 0 && script->GetState() != Command;
    bool wait = script && (script->IsRunning() || notStarted) && script->SignalStop() && script->GetThreadId() != GetCurrentThreadId();
    ScriptEngine::UnLockScriptList("script.stop");
    // Waits like Script::Stop (up to 500 ms) for the other script's context to end, finding it again each time: it
    // may be deleted whenever the list lock is free.
    for (int i = 0; wait && i < 50; i++) {
        Sleep(10);
        ScriptEngine::LockScriptList("script.stop");
        script = ScriptEngine::FindScriptById(scriptId);
        wait = script && script->hasActiveCX;
        ScriptEngine::UnLockScriptList("script.stop");
    }
    return JS_TRUE;
}

JSAPI_FUNC(script_pause) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    DWORD scriptId = ScriptIdOf(cx, self);
    ScriptEngine::LockScriptList("script.pause");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    if (script && script->IsRunning())
        script->Pause();
    ScriptEngine::UnLockScriptList("script.pause");
    return JS_TRUE;
}

JSAPI_FUNC(script_resume) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    DWORD scriptId = ScriptIdOf(cx, self);
    ScriptEngine::LockScriptList("script.resume");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    if (script && script->IsPaused())
        script->Resume();
    ScriptEngine::UnLockScriptList("script.resume");
    return JS_TRUE;
}

JSAPI_FUNC(script_send) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    jsval* argv = JS_ARGV(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    DWORD scriptId = ScriptIdOf(cx, self);

    ScriptEngine::LockScriptList("script.send");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    bool running = script && script->IsRunning();
    ScriptEngine::UnLockScriptList("script.send");
    if (!running)
        return JS_TRUE;

    // Cloned with the script list unlocked: cloning an object can run its getters.
    JSAutoStructuredCloneBuffer** buffers = argc ? new JSAutoStructuredCloneBuffer*[argc] : NULL;
    for (uint i = 0; i < argc; i++) {
        buffers[i] = new JSAutoStructuredCloneBuffer;
        if (!buffers[i]->write(cx, argv[i])) {
            FreeCloneBuffers(buffers, i + 1);
            return JS_FALSE;
        }
    }

    // Found again: a getter may have ended the script while the list was unlocked.
    ScriptEngine::LockScriptList("script.send");
    script = ScriptEngine::FindScriptById(scriptId);
    if (!script || !script->IsRunning()) {
        ScriptEngine::UnLockScriptList("script.send");
        FreeCloneBuffers(buffers, argc);
        return JS_TRUE;
    }
    Event* evt = new Event;
    evt->owner = script;
    evt->argc = argc;
    evt->name = _strdup("scriptmsg");
    evt->arg1 = new DWORD(argc);
    evt->argv = buffers;

    EnterCriticalSection(&Vars.cEventSection);
    script->EventList.push_front(evt);
    LeaveCriticalSection(&Vars.cEventSection);
    // The receiver may run and free evt once the event lock is free: only `script` (alive under the list lock) is used
    // after, and the signal wakes a receiver waiting in delay().
    script->TriggerOperationCallback();
    SetEvent(script->eventSignal);
    ScriptEngine::UnLockScriptList("script.send");

    return JS_TRUE;
}

JSAPI_FUNC(script_join) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    DWORD scriptId = ScriptIdOf(cx, self);
    HANDLE hThread = NULL;
    bool copied = true;
    // Joining its own thread would wait forever, and stop() cannot end the wait. Decided by id before the lookup: a
    // replaced copy still running is unlisted, and an ended cached script's thread id may belong to another thread.
    Script* caller = (Script*)JS_GetContextPrivate(cx);
    if (scriptId && caller && caller->GetId() == scriptId)
        THROW_ERROR(cx, "A script can't join itself");
    ScriptEngine::LockScriptList("script.join");
    Script* script = ScriptEngine::FindScriptById(scriptId);
    if (script)
        copied = script->DuplicateThreadHandle(&hThread);
    ScriptEngine::UnLockScriptList("script.join");
    if (!copied)
        THROW_ERROR(cx, "Couldn't wait for the script");
    // Waited on a copy of the handle, after the list lock is free: the script's thread takes that lock to end.
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    return JS_TRUE;
}

JSAPI_FUNC(my_getScript) {
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    jsval* argv = JS_ARGV(cx, vp);
    DWORD scriptId = 0;
    if (argc == 1 && JSVAL_IS_BOOLEAN(argv[0]) && JSVAL_TO_BOOLEAN(argv[0]) == JS_TRUE) {
        // The caller's own Script lives at least as long as its thread is in this call.
        Script* self = (Script*)JS_GetContextPrivate(cx);
        if (self)
            scriptId = self->GetId();
    } else {
        bool byTid = argc == 1 && JSVAL_IS_INT(argv[0]);
        bool byName = argc == 1 && JSVAL_IS_STRING(argv[0]);
        DWORD tid = byTid ? (DWORD)JSVAL_TO_INT(argv[0]) : 0;
        wchar_t* name = NULL;
        if (byName) {
            const jschar* chars = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(argv[0]));
            if (!chars)
                return JS_FALSE;
            name = _wcsdup(chars);
            if (!name) {
                JS_ReportOutOfMemory(cx);
                return JS_FALSE;
            }
            StringReplace(name, L'/', L'\\', wcslen(name));
        }
        // Any other argument (none, false, a fraction, ...) picks the first script, as before.
        ScriptEngine::LockScriptList("getScript");
        for (ScriptMap::iterator it = ScriptEngine::scripts.begin(); it != ScriptEngine::scripts.end(); it++) {
            Script* script = it->second;
            if ((byTid && script->GetThreadId() != tid) || (byName && _wcsicmp(script->GetShortFilename(), name) != 0))
                continue;
            scriptId = script->GetId();
            break;
        }
        ScriptEngine::UnLockScriptList("getScript");
        free(name);
    }
    if (!scriptId)
        return JS_TRUE;

    JSObject* res = BuildObject(cx, &script_class, script_methods, script_props, ScriptPrivate(scriptId));
    if (!res)
        THROW_ERROR(cx, "Failed to build the script object");
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
    return JS_TRUE;
}
JSAPI_FUNC(my_getScripts) {

    DWORD dwArrayCount = NULL;

    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    JS_BeginRequest(cx);
    if (!pReturnArray || !JS_AddRoot(cx, &pReturnArray)) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }
    ScriptEngine::LockScriptList("getScripts");

    for (ScriptMap::iterator it = ScriptEngine::scripts.begin(); it != ScriptEngine::scripts.end(); it++) {
        JSObject* res = BuildObject(cx, &script_class, script_methods, script_props, ScriptPrivate(it->second->GetId()));
        if (!res) {
            ScriptEngine::UnLockScriptList("getScripts");
            JS_RemoveRoot(cx, &pReturnArray);
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        // Defined, not set: an inherited index setter would run script code while the script list is locked.
        if (!JS_DefineElement(cx, pReturnArray, dwArrayCount, OBJECT_TO_JSVAL(res), NULL, NULL, JSPROP_ENUMERATE)) {
            ScriptEngine::UnLockScriptList("getScripts");
            JS_RemoveRoot(cx, &pReturnArray);
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        dwArrayCount++;
    }

    ScriptEngine::UnLockScriptList("getScripts");
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
    JS_RemoveRoot(cx, &pReturnArray);
    JS_EndRequest(cx);
    return JS_TRUE;
}
