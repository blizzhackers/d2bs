#include <algorithm>
#include <new>
#include <vector>
#include <assert.h>
#include <io.h>
#include <limits.h>

#include "ScriptEngine.h"
#include "Core.h"
#include "JSGlobalFuncs.h"
#include "JSGlobalClasses.h"
#include "JSUnit.h"
#include "Constants.h"
#include "D2BS.h"
#include "Console.h"
#include "D2Ptrs.h"
#include "Events.h"
#include "Helpers.h"

using namespace std;
Script* ScriptEngine::console = NULL;
ScriptMap ScriptEngine::scripts = ScriptMap();
EngineState ScriptEngine::state = Stopped;
CRITICAL_SECTION ScriptEngine::scriptListLock = {0};
static bool scriptListLockReady = false;

namespace {
struct DelayedTimer {
    int id;
    Script* owner;
    HANDLE timer;
    AutoRoot* callback;
    AutoRoot* args; // array of the extra arguments, or NULL
    bool repeat;
    bool queued; // a "timer" event for it waits in the owner's EventList
    bool cancelled;
};

// Guards timers, lastTimerId and every DelayedTimer field but the AutoRoots, which only the owner's thread touches.
// Taken before Vars.cEventSection, never inside it. A record leaves the map before its owner's thread frees it, and the
// timer's completion routine is passed the id, not the record, so a late one finds nothing.
std::map<int, DelayedTimer*> timers;
CRITICAL_SECTION timerLock;
bool timerLockReady = false;
int lastTimerId = 0;
} // namespace
// internal ForEachScript helpers
bool __fastcall DisposeScript(Script* script, void*, uint);
bool __fastcall DisposeCachedScript(Script* script, void*, uint);
bool __fastcall StopScript(Script* script, void* argv, uint argc);

Script* ScriptEngine::CompileFile(const wchar_t* file, ScriptState state, uint argc, JSAutoStructuredCloneBuffer** argv, bool recompile) {
    if (GetState() != Running)
        return NULL;

    // Checked before construction: without unwind semantics a throwing constructor leaks the Script itself.
    if (_waccess(file, 0) != 0) {
        DEBUG_LOG(file);
        Print(L"File not found");
        return NULL;
    }

    // Built before the list is touched, so a failure leaves a running instance alone; GetFilename() is the one map key
    // (lower case, backslashes), the form DisposeScript and IsListed compare against.
    Script* script = NULL;
    try {
        script = new Script(file, state, argc, argv);
    } catch (std::exception& e) {
        wchar_t* what = AnsiToUnicode(e.what());
        if (what) {
            Print(L"%s", what);
            delete[] what;
        }
        return NULL;
    }

    // One instance per file, swapped in one locked step: the running one leaves the map, the new one takes its key,
    // and the old one's own thread disposes it when it ends. The new copy's thread waits for that (Script::Run), so
    // the caller never stalls here.
    HANDLE oldThread = NULL;
    LockScriptList("CompileFile");
    if (ScriptEngine::GetState() != Running) {
        UnLockScriptList("CompileFile");
        // The caller still owns argv when this returns NULL.
        script->argv = NULL;
        script->argc = 0;
        delete script;
        return NULL;
    }
    ScriptMap::iterator it = scripts.find(script->GetFilename());
    if (it != scripts.end()) {
        Script* old = it->second;
        scripts.erase(it);
        // A self-load never waits (its predecessor is the loader); DuplicateThreadHandle leaves NULL for an unstarted copy.
        if (old->threadId != GetCurrentThreadId() && !old->DuplicateThreadHandle(&oldThread))
            oldThread = NULL;
        old->SignalStop();
    }
    script->predecessor = oldThread;
    scripts[script->GetFilename()] = script;
    UnLockScriptList("CompileFile");
    return script;
}

void ScriptEngine::RunCommand(const wchar_t* command) {
    if (GetState() != Running)
        return;
    try {
        console->RunCommand(command);
    } catch (std::exception e) {
        wchar_t* what = AnsiToUnicode(e.what());
        Print(what);
        delete[] what;
    }
}

void ScriptEngine::DisposeScript(Script* script, bool fromPass) {
    LockScriptList("DisposeScript");
    // Only this Script's own entry: a replacement of the same file may already hold the name.
    for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) {
        if (it->second == script) {
            scripts.erase(it);
            break;
        }
    }

    if (GetCurrentThreadId() == script->threadId) {
        UnLockScriptList("DisposeScript");
        // Unlisted, it gains no new reference, so the count only falls; each holder releases after its own entry's
        // callback in a ForEachScript pass. The delete stays on this thread because the runtime must die here. Bounded:
        // a holder whose pass runs JS that waits for this script (join()) would otherwise hang both threads.
        for (DWORD start = GetTickCount(); InterlockedCompareExchange(&script->refs, 1, 1) != 1; Sleep(1)) {
            if (GetTickCount() - start > 10000) {
                // Unlisted, nothing else would stop it or take its hooks off screen, so both happen here. A DisposeMe
                // met while main() runs is retried by ScriptThread's own call once main() unwinds (never for the
                // console); otherwise this is a one-off leak.
                script->SignalStop();
                Genhook::Clean(script);
                if (script->hasActiveCX && script->GetState() != Command)
                    Log(L"Script %s still in use after 10 s; deletion deferred to its thread's exit", script->GetShortFilename());
                else
                    Log(L"Script %s still in use after 10 s; left undeleted", script->GetShortFilename());
                return;
            }
        }
        delete script;
    } else if (script->startFailed) {
        UnLockScriptList("DisposeScript");
        // No thread and no engine state: the last holder deletes it. A pass only unlists it, since the loader still
        // reads it after a pass's release could have freed it. Released once, for a loader that calls twice.
        if (!fromPass && InterlockedExchange(&script->engineRefReleased, 1) == 0)
            script->Release();
    } else {
        // Queued before the list lock is free: once unlisted, the script's own thread deletes it as soon as it can
        // take the lock, and only its own thread may destroy its runtime.
        Event* evt = new Event;
        evt->owner = script;
        evt->name = "DisposeMe";
        script->FireEvent(evt);
        UnLockScriptList("DisposeScript");
    }
}

bool ScriptEngine::IsListed(Script* script) {
    bool listed = false;
    LockScriptList("IsListed");
    for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) {
        if (it->second == script) {
            listed = true;
            break;
        }
    }
    UnLockScriptList("IsListed");
    return listed;
}

Script* ScriptEngine::FindScriptById(DWORD id) {
    if (id == 0)
        return NULL;
    for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++)
        if (it->second->GetId() == id)
            return it->second;
    return NULL;
}
void ScriptEngine::LockScriptList(char* loc) {
    EnterCriticalSection(&scriptListLock);
    // Log(loc);
}
void ScriptEngine::UnLockScriptList(char* loc) {
    // Log(loc);
    LeaveCriticalSection(&scriptListLock);
}
unsigned int ScriptEngine::GetCount(bool active, bool unexecuted) {
    if (GetState() != Running)
        return 0;
    LockScriptList("getCount");
    int count = scripts.size();
    for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) {
        if (!active && it->second->IsRunning() && !it->second->IsAborted())
            count--;
        if (!unexecuted && it->second->GetExecutionCount() == 0 && !it->second->IsRunning())
            count--;
    }
    assert(count >= 0);
    UnLockScriptList("getCount");
    return count;
}

BOOL ScriptEngine::Startup(void) {
    if (GetState() == Stopped) {
        // Initialized once, before the state leaves Stopped: hooks installed at DLL load enumerate scripts as soon as
        // it does, and a later Startup must not re-initialize a lock a tearing-down script may still hold.
        if (!scriptListLockReady) {
            InitializeCriticalSection(&scriptListLock);
            scriptListLockReady = true;
        }
        state = Starting;
        // Kept across a Shutdown: scripts still tearing down on their own threads may take it.
        if (!timerLockReady) {
            InitializeCriticalSection(&timerLock);
            timerLockReady = true;
        }
        LockScriptList("startup - enter");
        if (wcslen(Vars.szConsole) > 0) {
            wchar_t file[_MAX_FNAME + _MAX_PATH];
            swprintf_s(file, _countof(file), L"%s\\%s", Vars.szScriptPath, Vars.szConsole);
            console = new Script(file, Command);
        } else {
            console = new Script(L"", Command);
        }
        scripts[L"console"] = console;
        console->BeginThread(ScriptThread);
        state = Running;
        UnLockScriptList("startup - leave");
    }
    return TRUE;
}

void ScriptEngine::Shutdown(void) {
    if (GetState() == Running) {
        // bring the engine down properly
        LockScriptList("Shutdown");
        state = Stopping;
        StopAll(true);
        console->Stop(true, true);

        // clear all scripts now that they're stopped
        ForEachScript(::DisposeScript, NULL, 0);
        // The pass only unlists a never-started Script; the console has no loader to release it, so this call does.
        if (console->startFailed)
            DisposeScript(console);

        if (!scripts.empty())
            scripts.clear();

        UnLockScriptList("shutdown");
        state = Stopped;
    }
}

void ScriptEngine::StopAll(bool forceStop) {
    if (GetState() != Running)
        return;

    ForEachScript(StopScript, &forceStop, 1);
}

void ScriptEngine::UpdateConsole() {
    console->UpdatePlayerGid();
}
void ScriptEngine::FlushCache(void) {
    if (GetState() != Running)
        return;

    static bool isFlushing = false;

    if (isFlushing || Vars.bDisableCache)
        return;

    EnterCriticalSection(&Vars.cFlushCacheSection);

    isFlushing = true;

    ForEachScript(DisposeCachedScript, NULL, 0);

    isFlushing = false;

    LeaveCriticalSection(&Vars.cFlushCacheSection);
}

bool ScriptEngine::ForEachScript(ScriptCallback callback, void* argv, uint argc) {
    // Before the first Startup the list lock does not exist yet, and the packet, copydata and input hooks are live.
    if (callback == NULL || GetState() == Stopped)
        return false;
    bool block = false;

    LockScriptList("forEachScript");
    // A reference per entry keeps it alive through its callback; the caller's own Script takes none: only its own
    // thread deletes it, and a callback that runs its listeners here (a packet its native sent) would wait on itself.
    DWORD self = GetCurrentThreadId();
    std::vector<std::pair<Script*, bool> > list;
    for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) {
        bool held = it->second->threadId != self;
        if (held)
            it->second->AddRef();
        list.push_back(std::make_pair(it->second, held));
    }
    UnLockScriptList("forEachScript");

    for (size_t i = 0; i < list.size(); i++) {
        if (callback(list[i].first, argv, argc))
            block = true;
        if (list[i].second)
            list[i].first->Release();
    }

    return block;
}

bool ScriptEngine::InitClass(JSContext* context, JSObject* globalObject, JSClass* classp, JSFunctionSpec* methods, JSPropertySpec* props, JSFunctionSpec* s_methods,
                             JSPropertySpec* s_props) {
    return JS_InitClass(context, globalObject, NULL, classp, classp->construct, 0, props, methods, s_props, s_methods) != NULL;
}

bool ScriptEngine::DefineConstant(JSContext* context, JSObject* globalObject, const char* name, int value) {
    return JS_DefineProperty(context, globalObject, name, INT_TO_JSVAL(value), NULL, NULL, JSPROP_PERMANENT_VAR) != JS_FALSE;
}

// internal ForEachScript helper functions
bool __fastcall DisposeScript(Script* script, void*, uint) {
    ScriptEngine::DisposeScript(script, true);
    return true;
}

// The console is not a cached script: ScriptEngine::console and its map key keep pointing at it after a flush.
bool __fastcall DisposeCachedScript(Script* script, void*, uint) {
    if (script->GetState() != Command)
        ScriptEngine::DisposeScript(script, true);
    return true;
}

bool __fastcall StopScript(Script* script, void* argv, uint argc) {
    script->TriggerOperationCallback();
    if (script->GetState() != Command)
        script->Stop(*(bool*)(argv), ScriptEngine::GetState() == Stopping);
    return true;
}

bool __fastcall StopIngameScript(Script* script, void*, uint) {
    if (script->GetState() == InGame)
        script->Stop(true);
    return true;
}

bool ScriptMustEnd(Script* script) {
    return script->IsAborted() || (script->GetState() == InGame && ClientState() == ClientStateMenu);
}

JSBool operationCallback(JSContext* cx) {
    Script* script = (Script*)JS_GetContextPrivate(cx);
    static int callBackCount = 0;
    callBackCount++;

    // moved this to delay. operation callback isnt a good spot for this anymore
    // event heavy stuff like shop bot would trigger gc alot while normal playing would not.
    /*if (callBackCount % 30 == 0){
    JS_GC(JS_GetRuntime(cx));
    callBackCount = 0;
    }*/

    HWND debug_wnd = NULL; // FindWindow(NULL, "JavaZone");
    if (debug_wnd && D2GFX_GetHwnd() != debug_wnd) {
        JS_BeginRequest(cx);
        JS::Value err;
        if (JS_CallFunctionName(cx, JS_GetGlobalObject(cx), "Error", 0, NULL, &err)) {
            JS::Value stack;
            if (JS_GetProperty(cx, JSVAL_TO_OBJECT(err), "stack", &stack)) {
                char str[1024];
                int l = 0;
                COPYDATASTRUCT aCopy = {164344, 0, str};
                if (JSVAL_IS_STRING(stack)) {
                    if (l = JS_EncodeStringToBuffer(cx, JSVAL_TO_STRING(stack), str, 1024)) {
                        str[l] = 0;
                        aCopy.cbData = l + 1;
                        SendMessage(debug_wnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy);
                        // Print("%s", str);
                    }
                } else if (JSVAL_IS_OBJECT(stack)) {
                    if (JS_CallFunctionName(cx, JSVAL_TO_OBJECT(stack), "ToString", 0, NULL, &err)) {

                        if (l = JS_EncodeStringToBuffer(cx, JSVAL_TO_STRING(err), str, 1024)) {
                            str[l] = 0;
                            aCopy.cbData = l + 1;
                            SendMessage(debug_wnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy);
                            // Print("%s", str);
                        }
                    }
                }
            }
        }
        JS_EndRequest(cx);
    }
    bool pause = script->IsPaused();

    if (pause)
        script->SetPauseState(true);
    while (script->IsPaused()) {
        Sleep(50);
        JS_MaybeGC(cx);
    }
    if (pause)
        script->SetPauseState(false);

    if (!ScriptMustEnd(script)) {
        // A pause that lands mid-drain leaves the rest queued for the drain after resume, as in delay().
        while (script->EventList.size() > 0 && !ScriptMustEnd(script) && !script->IsPaused()) {
            EnterCriticalSection(&Vars.cEventSection);
            Event* evt = script->EventList.back();
            script->EventList.pop_back();
            LeaveCriticalSection(&Vars.cEventSection);
            ExecScriptEvent(evt, false);
        }
        return !ScriptMustEnd(script);
    } else {
        return false;
    }
}

JSBool contextCallback(JSContext* cx, uint contextOp) {
    if (contextOp == JSCONTEXT_NEW) {
        JS_BeginRequest(cx);

        JS_SetErrorReporter(cx, reportError);
        JS_SetOperationCallback(cx, operationCallback);

        JS_SetVersion(cx, JSVERSION_LATEST);
        JS_SetOptions(cx, JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_VAROBJFIX | JSOPTION_ASMJS | JSOPTION_STRICT);

        // JS_SetGCZeal(cx, 2, 1);
        JSObject* globalObject = JS_NewGlobalObject(cx, &global_obj, NULL);
        if (!globalObject) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        JS_SetGCParameter(JS_GetRuntime(cx), JSGC_MODE, 2);

        // Every JS_FALSE return ends the request first: the engine then destroys cx without calling back for JSCONTEXT_DESTROY.
        if (JS_InitStandardClasses(cx, globalObject) == JS_FALSE || JS_DefineFunctions(cx, globalObject, global_funcs) == JS_FALSE) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }

        // Classes come before the `me` private exists, so a failure here has nothing to free.
        for (JSClassSpec* entry = global_classes; entry->classp != NULL; entry++) {
            if (!ScriptEngine::InitClass(cx, globalObject, entry->classp, entry->methods, entry->properties, entry->static_methods,
                                         entry->static_properties)) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
        }

        myUnit* lpUnit = new (std::nothrow) myUnit;
        if (!lpUnit) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        memset(lpUnit, NULL, sizeof(myUnit));

        UnitAny* player = D2CLIENT_GetPlayerUnit();
        lpUnit->dwMode = (DWORD)-1;
        lpUnit->dwClassId = (DWORD)-1;
        lpUnit->dwType = UNIT_PLAYER;
        lpUnit->dwUnitId = player ? player->dwUnitId : NULL;
        lpUnit->_dwPrivateType = PRIVATE_UNIT;

        JSObject* meObject = BuildObject(cx, &unit_class, unit_methods, me_props, lpUnit);
        // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
        if (!meObject) {
            delete lpUnit;
            JS_EndRequest(cx);
            return JS_FALSE;
        }

        if (JS_DefineProperty(cx, globalObject, "me", OBJECT_TO_JSVAL(meObject), NULL, NULL, JSPROP_PERMANENT_VAR) == JS_FALSE) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }

#define DEFCONST(vp) ScriptEngine::DefineConstant(cx, globalObject, #vp, vp)
        if (!DEFCONST(FILE_READ) || !DEFCONST(FILE_WRITE) || !DEFCONST(FILE_APPEND)) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
#undef DEFCONST

        JS_EndRequest(cx);
    }
    if (contextOp == JSCONTEXT_DESTROY) {
        Script* script = (Script*)JS_GetContextPrivate(cx);
        // A context made for no Script has no private and nothing to clean up.
        if (!script)
            return JS_TRUE;
        script->hasActiveCX = false;
        // First, so no other thread can queue a timer event behind the drain below.
        ScriptEngine::DestroyTimers(script);
        // Also before the drain: a click passes its owner check and queues its event in one globalSection hold, so once
        // the hooks are unlisted every click event for this script is already queued.
        Genhook::Clean(script);
        while (script->EventList.size() > 0) {
            EnterCriticalSection(&Vars.cEventSection);
            Event* evt = script->EventList.back();
            script->EventList.pop_back();
            LeaveCriticalSection(&Vars.cEventSection);
            ExecScriptEvent(evt, true); // clean list and pop events
        }

        script->ClearAllEvents();
    }
    return JS_TRUE;
}

// JSGCCallback gcCallback(JSRuntime *rt, JSGCStatus status)
//{
//	//Script* script = (Script*)JS_GetContextPrivate(cx);
//	DWORD cur =GetCurrentThreadId() ;
//	//if (cur != script->GetThreadId())
//	//	return JS_FALSE;
//	if(status == JSGC_BEGIN)
//	{
//
//		//Script* script = (Script*)JS_GetContextPrivate(cx);
//		//Print("Entering GC %s",script->GetShortFilename());
////		EnterCriticalSection(&ScriptEngine::lock);
//
//#ifdef DEBUG
//		Log("*** ENTERING GC ***");
//#ifdef lord2800_INFO
//		Print("*** ENTERING GC ***");
//#endif
//#endif
//	}
//	else if(status == JSGC_END)
//	{
//	//	Script* script = (Script*)JS_GetContextPrivate(cx);
//	//	Print("Leaving GC %s",script->GetShortFilename());
//#ifdef DEBUG
//		Log("*** LEAVING GC ***");
//#ifdef lord2800_INFO
//		Print("*** LEAVING GC ***");
//#endif
//#endif
////		LeaveCriticalSection(&ScriptEngine::lock);
//	}
//	//return JS_TRUE;
//}

void reportError(JSContext* cx, const char* message, JSErrorReport* report) {
    bool warn = JSREPORT_IS_WARNING(report->flags);
    bool isStrict = JSREPORT_IS_STRICT(report->flags);
    const char* type = (warn ? "Warning" : "Error");
    const char* strict = (isStrict ? "Strict " : "");
    wchar_t* filename = report->filename ? AnsiToUnicode(report->filename) : _wcsdup(L"<unknown>");
    wchar_t* displayName = filename;
    if (_wcsicmp(L"Command Line", filename) != 0 && _wcsicmp(L"<unknown>", filename) != 0)
        displayName = filename + wcslen(Vars.szPath);

    Log(L"[%hs%hs] Code(%d) File(%s:%d) %hs\nLine: %hs", strict, type, report->errorNumber, filename, report->lineno, message, report->linebuf);
    Print(L"[\u00FFc%d%hs%hs\u00FFc0 (%d)] File(%s:%d) %hs", (warn ? 9 : 1), strict, type, report->errorNumber, displayName, report->lineno, message);

    if (filename[0] == L'<')
        free(filename);
    else
        delete[] filename;

    if (Vars.bQuitOnError && !JSREPORT_IS_WARNING(report->flags) && ClientState() == ClientStateInGame)
        D2CLIENT_ExitGame();
    else
        Console::ShowBuffer();
}

// The step after each listener or timer callback call: true when the next one may run. Every dispatch runs inside a
// native that returns true (delay(), the operation callback, a sending native), so a thrown error is reported here or
// it stays pending and resurfaces later. Nothing pending means the call was ended (a stop, or an error scripts cannot
// catch), so nothing more of this event runs.
static bool ListenerCallContinues(JSContext* cx, JSBool ok) {
    if (ok)
        return true;
    if (!JS_IsExceptionPending(cx))
        return false;
    JS_ReportPendingException(cx);
    return true;
}

// Calls each function of fns; with checkRegistry, one that owner no longer holds for evtName (removed during this
// dispatch) is skipped. A call's result counts toward *block only when the call returned normally. False once the
// script is ending or a call was ended.
static bool CallListenerValues(JSContext* cx, Script* owner, const char* evtName, bool checkRegistry, JS::AutoValueVector& fns, uint argc,
                               jsval* argv, bool* block) {
    for (size_t i = 0; i < fns.length(); i++) {
        if (ScriptMustEnd(owner))
            return false;
        if (checkRegistry && !owner->IsRegisteredEvent(evtName, fns[i]))
            continue;
        jsval rval = JSVAL_VOID;
        JSBool ok = JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), fns[i], argc, argv, &rval);
        if (!ListenerCallContinues(cx, ok))
            return false;
        if (ok && block && JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval))
            *block = true;
    }
    return true;
}

// Copies evtName's listeners of owner and calls them; one added during the calls waits for the next event and one
// removed is skipped, as in a browser. The lookup must not create the name: an empty entry would keep the game
// waiting on this script for an event it no longer listens to.
static bool CallListeners(JSContext* cx, Script* owner, const char* evtName, uint argc, jsval* argv, bool* block) {
    FunctionMap::iterator found = owner->functions.find(evtName);
    if (found == owner->functions.end())
        return true;
    JS::AutoValueVector fns(cx);
    for (FunctionList::iterator it = found->second.begin(); it != found->second.end(); it++) {
        if (!fns.append(*(*it)->value())) {
            JS_ReportPendingException(cx); // the failed append has already reported its out-of-memory error
            return true;
        }
    }
    return CallListenerValues(cx, owner, evtName, true, fns, argc, argv, block);
}

bool ExecScriptEvent(Event* evt, bool clearList) {
    JSContext* cx;

    if (!clearList)
        cx = evt->owner->GetContext();
    const char* evtName = evt->name;
    if (strcmp(evtName, "itemaction") == 0) {
        if (!clearList) {
            DWORD* gid = (DWORD*)evt->arg1;
            char* code = (char*)evt->arg2;
            DWORD* mode = (DWORD*)evt->arg3;
            bool* global = (bool*)evt->arg4;

            JS_BeginRequest(cx);
            // An event whose arguments can't be made (out of memory, already reported) reaches no listener.
            JS::AutoValueVector argv(cx);
            if (argv.resize(4)) {
                argv[0] = JS_NumberValue(*gid);
                argv[1] = JS_NumberValue(*mode);
                argv[3] = (BOOLEAN_TO_JSVAL(*global));
                JSString* str = JS_NewStringCopyZ(cx, code);
                if (str) {
                    argv[2] = STRING_TO_JSVAL(str);
                    CallListeners(cx, evt->owner, evtName, 4, argv.begin(), NULL);
                }
            }
            JS_EndRequest(cx);
        }
        delete evt->arg1;
        free(evt->arg2);
        delete evt->arg3;
        delete evt->arg4;
        delete evt;
        return true;
    }
    if (strcmp(evtName, "gameevent") == 0) {
        if (!clearList) {
            JS_BeginRequest(cx);
            // Rooted before the strings are made: making the second can run the GC.
            JS::AutoValueVector argv(cx);
            if (argv.resize(5)) {
                argv[0] = JS_NumberValue(*(BYTE*)evt->arg1);
                argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
                argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);
                JSString* str1 = JS_NewStringCopyZ(cx, (char*)evt->arg4);
                if (str1)
                    argv[3] = STRING_TO_JSVAL(str1);
                JSString* str2 = str1 ? JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg5) : NULL;
                if (str2) {
                    argv[4] = STRING_TO_JSVAL(str2);
                    CallListeners(cx, evt->owner, evtName, 5, argv.begin(), NULL);
                }
            }
            JS_EndRequest(cx);
        }
        delete evt->arg1;
        delete evt->arg2;
        delete evt->arg3;
        free(evt->arg4);
        free(evt->arg5);
        delete evt;
        return true;
    }
    if (strcmp(evtName, "copydata") == 0) {
        if (!clearList) {
            JS_BeginRequest(cx);
            JS::AutoValueVector argv(cx);
            if (argv.resize(2)) {
                argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
                JSString* str = JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2);
                if (str) {
                    argv[1] = STRING_TO_JSVAL(str);
                    CallListeners(cx, evt->owner, evtName, 2, argv.begin(), NULL);
                }
            }
            JS_EndRequest(cx);
        }
        delete evt->arg1;
        free(evt->arg2);
        delete evt;
        return true;
    }
    if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
        strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
        bool block = false;
        if (!clearList) {
            JS_BeginRequest(cx);
            // Rooted before the strings are made: making the second can run the GC.
            JS::AutoValueVector argv(cx);
            if (argv.resize(2)) {
                JSString* str1 = JS_NewStringCopyZ(cx, (char*)evt->arg1);
                if (str1)
                    argv[0] = STRING_TO_JSVAL(str1);
                JSString* str2 = str1 ? JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2) : NULL;
                if (str2) {
                    argv[1] = STRING_TO_JSVAL(str2);
                    BlockingListenerScope blocking(evt->done != NULL);
                    CallListeners(cx, evt->owner, evtName, 2, argv.begin(), &block);
                }
            }
            JS_EndRequest(cx);
        }
        if (strcmp(evtName, "chatmsgblocker") == 0 || strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
            CompleteBlocker(evt, block);
        } else {
            free(evt->arg1);
            free(evt->arg2);
            delete evt;
        }
        return true;
    }
    if (strcmp(evtName, "mousemove") == 0) {
        if (!clearList) {
            JS_BeginRequest(cx);
            JS::AutoValueVector argv(cx);
            if (argv.resize(2)) {
                argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
                argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
                CallListeners(cx, evt->owner, evtName, 2, argv.begin(), NULL);
            }
            JS_EndRequest(cx);
        }
        delete evt->arg1;
        delete evt->arg2;
        delete evt;
        return true;
    }
    if (strcmp(evtName, "mouseclick") == 0) {
        if (!clearList) {
            JS_BeginRequest(cx);
            JS::AutoValueVector argv(cx);
            if (argv.resize(4)) {
                argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
                argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
                argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);
                argv[3] = JS_NumberValue(*(DWORD*)evt->arg4);

                CallListeners(cx, evt->owner, evtName, 4, argv.begin(), NULL);
            }
            JS_EndRequest(cx);
        }
        delete evt->arg1;
        delete evt->arg2;
        delete evt->arg3;
        delete evt->arg4;
        delete evt;
        return true;
    }
    if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydown") == 0 || strcmp(evtName, "keyupblocker") == 0 ||
        strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "memana") == 0 || strcmp(evtName, "melife") == 0 ||
        strcmp(evtName, "playerassign") == 0) {
        bool block = false;
        if (!clearList) {
            JS_BeginRequest(cx);
            JS::AutoValueVector argv(cx);
            if (argv.resize(1)) {
                argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
                BlockingListenerScope blocking(evt->done != NULL);
                CallListeners(cx, evt->owner, evtName, 1, argv.begin(), &block);
            }
            JS_EndRequest(cx);
        }
        if (strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keyupblocker") == 0) {
            CompleteBlocker(evt, block);
        } else {
            delete evt->arg1;
            delete evt;
        }
        return true;
    }
    if (strcmp(evtName, "ScreenHookClick") == 0) {
        bool block = false;
        if (!clearList) {
            JS_BeginRequest(cx);
            // The handler is the hook's at this moment: a hook removed since the click, or whose handler was cleared,
            // calls nothing and does not block.
            JS::AutoValueVector fns(cx);
            JS::AutoValueVector argv(cx);
            if (fns.resize(1) && argv.resize(3)) {
                fns[0] = Genhook::GetClickHandler(evt->owner, *(ULONGLONG*)evt->arg5);
                if (JSVAL_IS_FUNCTION(cx, fns[0])) {
                    argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
                    argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
                    argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);

                    BlockingListenerScope blocking(evt->done != NULL);
                    CallListenerValues(cx, evt->owner, "", false, fns, 3, argv.begin(), &block);
                }
            }

            JS_EndRequest(cx);
        }
        CompleteBlocker(evt, block);

        return true;
    }
    if (strcmp(evtName, "Command") == 0) {
        if (!clearList) {
            wchar_t* cmd = (wchar_t*)evt->arg1;
            std::wstring test;

            test.append(L"try{ ");
            test.append(cmd);
            test.append(L" } catch (error){print(error)}");
            JS_BeginRequest(cx);
            jsval rval;

            if (JS_EvaluateUCScript(cx, JS_GetGlobalObject(cx), test.data(), test.length(), "Command Line", 0, &rval)) {
                if (!JSVAL_IS_NULL(rval) && !JSVAL_IS_VOID(rval)) {
                    // A value that can't become a string (no toString, or one that throws) is reported, not printed.
                    JSString* str = JS_ValueToString(cx, rval);
                    const wchar_t* text = str ? JS_GetStringCharsZ(cx, str) : NULL;
                    if (text)
                        Print(L"%s", text);
                    else
                        JS_ReportPendingException(cx);
                }
            }
            JS_EndRequest(cx);
        }
        free(evt->arg1);
        delete evt;
        return true;
    }
    if (strcmp(evtName, "scriptmsg") == 0) {
        if (!clearList) {
            DWORD* argc = (DWORD*)evt->arg1;
            JS_BeginRequest(cx);
            // Rooted before the first is read: reading a later one can run the GC. A message whose values can't all
            // be held or read is reported and reaches no listener.
            JS::AutoValueVector argv(cx);
            bool haveArgs = argv.resize(*argc);
            for (uint i = 0; haveArgs && i < *argc; i++)
                haveArgs = evt->argv[i]->read(cx, &argv[i]);
            if (haveArgs)
                CallListeners(cx, evt->owner, evtName, *argc, argv.begin(), NULL);
            else
                JS_ReportPendingException(cx); // a failed resize has already reported its out-of-memory error
            JS_EndRequest(cx);
        }
        for (uint i = 0; i < evt->argc; i++) {
            evt->argv[i]->clear();
            delete evt->argv[i];
        }
        delete[] evt->argv;
        delete evt->arg1;
        delete evt;
        return true;
    }
    if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
        bool block = false;
        if (!clearList) {
            BYTE* help = (BYTE*)evt->arg1;
            DWORD* size = (DWORD*)evt->arg2;
            //  DWORD* argc = (DWORD*)1;
            JS_BeginRequest(cx);

            JSObject* arr = JS_NewUint8Array(cx, *size);
            // JSObject* arr = JS_NewArrayObject(cx, 0, NULL);

            // An event whose arguments can't be made skips its listeners and is not blocked.
            if (arr && JS_AddRoot(cx, &arr)) {
                bool filled = true;
                for (uint i = 0; i < *size; i++) {
                    jsval jsarr = UINT_TO_JSVAL(help[i]);
                    if (!JS_SetElement(cx, arr, i, &jsarr)) {
                        filled = false;
                        break;
                    }
                }
                if (filled) {
                    jsval argv = OBJECT_TO_JSVAL(arr);
                    // evt->argv[0]->read(cx, &argv[0]);
                    // JS_AddValueRoot(cx, &argv[0]);

                    BlockingListenerScope blocking(evt->done != NULL);
                    CallListeners(cx, evt->owner, evtName, 1, &argv, &block);
                }
                JS_RemoveRoot(cx, &arr);
            }
            JS_EndRequest(cx);
        }
        CompleteBlocker(evt, block);

        return true;
    }
    if (strcmp(evtName, "timer") == 0) {
        ScriptEngine::RunTimer(evt, clearList);
        return true;
    }
    if (strcmp(evtName, "DisposeMe") == 0) {
        // Drained while its script is being destroyed, the request is already being carried out.
        if (!clearList)
            ScriptEngine::DisposeScript(evt->owner);
        delete evt;
        return true;
    }

    // A blocker event that no branch runs must still end its wait and drop its reference.
    if (evt->freeArgs)
        CompleteBlocker(evt, false);
    return true;
}
namespace {
// With timerLock held.
void QueueTimerEvent(DelayedTimer* t) {
    t->queued = true;
    Event* evt = new Event;
    evt->owner = t->owner;
    evt->argc = 0;
    evt->name = "timer";
    evt->arg1 = new int(t->id);
    t->owner->FireEvent(evt);
}

// On the owner's thread, after the record has left the map.
void FreeTimer(DelayedTimer* t) {
    CancelWaitableTimer(t->timer);
    CloseHandle(t->timer);
    delete t->callback;
    delete t->args;
    delete t;
}

// A failed root may not have reported, and the native's JS_FALSE with nothing pending would end the script without a
// line. An engine out-of-memory report leaves nothing pending, so after one this can print the line a second time.
void ReportRootFailure(JSContext* cx) {
    if (!JS_IsExceptionPending(cx))
        JS_ReportOutOfMemory(cx);
}
} // namespace

bool ScriptEngine::AddTimer(Script* owner, JSContext* cx, jsval callback, uint argc, jsval* argv, int delayMs, bool repeat, int* id) {
    *id = 0;
    const char* name = repeat ? "setInterval" : "setTimeout";
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!timer) {
        DWORD err = GetLastError();
        JS_ReportError(cx, "%s: could not create a timer (error %lu)", name, err);
        return false;
    }
    DelayedTimer* t = new DelayedTimer;
    t->owner = owner;
    t->timer = timer;
    t->args = NULL;
    t->callback = new AutoRoot(cx, callback);
    if (!t->callback->ok()) {
        FreeTimer(t);
        ReportRootFailure(cx);
        return false;
    }
    if (argc > 0) {
        JSObject* args = JS_NewArrayObject(cx, (int)argc, argv);
        if (!args) {
            FreeTimer(t);
            return false;
        }
        t->args = new AutoRoot(cx, OBJECT_TO_JSVAL(args));
        if (!t->args->ok()) {
            FreeTimer(t);
            ReportRootFailure(cx);
            return false;
        }
    }
    t->repeat = repeat;
    t->queued = false;
    t->cancelled = false;

    LARGE_INTEGER due;
    due.QuadPart = -(LONGLONG)delayMs * 10000; // relative, in 100 ns units
    // A period of 0 makes a one-shot timer, so an interval repeats at least every millisecond.
    LONG period = repeat ? (delayMs > 0 ? delayMs : 1) : 0;
    EnterCriticalSection(&timerLock);
    // Ids run 1..INT_MAX and wrap to 1, skipping live ones, so clearInterval can name every timer and no record is
    // replaced; the map can never hold INT_MAX timers, so the loop ends.
    int newId;
    do {
        newId = lastTimerId < INT_MAX ? lastTimerId + 1 : 1;
        lastTimerId = newId;
    } while (timers.find(newId) != timers.end());
    t->id = newId;
    timers[newId] = t;
    // The completion routine runs on this thread, the owner's, while it waits alertably (delay()).
    BOOL set = SetWaitableTimer(timer, &due, period, EventTimerProc, (LPVOID)(INT_PTR)newId, FALSE);
    DWORD err = set ? 0 : GetLastError();
    if (!set)
        timers.erase(newId);
    LeaveCriticalSection(&timerLock);
    if (!set) {
        FreeTimer(t);
        JS_ReportError(cx, "%s: could not set a timer (error %lu)", name, err);
        return false;
    }
    *id = newId;
    return true;
}

void ScriptEngine::ClearTimer(int id) {
    EnterCriticalSection(&timerLock);
    std::map<int, DelayedTimer*>::iterator it = timers.find(id);
    if (it != timers.end() && !it->second->cancelled) {
        DelayedTimer* t = it->second;
        t->cancelled = true;
        CancelWaitableTimer(t->timer);
        // The owner frees it, through the timer event already queued or this one.
        if (!t->queued)
            QueueTimerEvent(t);
    }
    LeaveCriticalSection(&timerLock);
}

// Cancels like ClearTimer rather than freeing, so a callback may call it, its own record included.
int ScriptEngine::ClearTimers(Script* owner) {
    int count = 0;
    EnterCriticalSection(&timerLock);
    for (std::map<int, DelayedTimer*>::iterator it = timers.begin(); it != timers.end(); ++it) {
        DelayedTimer* t = it->second;
        if (t->owner != owner || t->cancelled)
            continue;
        t->cancelled = true;
        CancelWaitableTimer(t->timer);
        if (!t->queued)
            QueueTimerEvent(t);
        count++;
    }
    LeaveCriticalSection(&timerLock);
    return count;
}

void ScriptEngine::RunTimer(Event* evt, bool clearList) {
    int id = *(int*)evt->arg1;
    DelayedTimer* t = NULL;
    bool run = false, done = false;
    EnterCriticalSection(&timerLock);
    std::map<int, DelayedTimer*>::iterator it = timers.find(id);
    if (it != timers.end() && it->second->owner == evt->owner) {
        t = it->second;
        run = !t->cancelled && !clearList;
        done = !run || !t->repeat;
        if (done)
            timers.erase(it);
    }
    LeaveCriticalSection(&timerLock);

    // Only this thread frees a record, so t stays valid through the callback, even if it clears its own timer. An
    // interval stays queued meanwhile: one that fires again while its callback runs (in a delay()) is not re-entered.
    if (run) {
        JSContext* cx = evt->owner->GetContext();
        JS_BeginRequest(cx);
        uint32_t argc = 0;
        jsval* argv = NULL;
        // A stored argument that can't be read fails the call: the callback is skipped, and an error left pending is
        // reported.
        JSBool ok = JS_TRUE;
        if (t->args) {
            JSObject* args = JSVAL_TO_OBJECT(*t->args->value());
            ok = JS_GetArrayLength(cx, args, &argc);
            if (ok && argc > 0) {
                argv = new jsval[argc];
                for (uint32_t i = 0; ok && i < argc; i++)
                    ok = JS_GetElement(cx, args, i, &argv[i]);
            }
        }
        if (ok) {
            jsval rval;
            ok = JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *t->callback->value(), argc, argv, &rval);
        }
        // A throw is reported and an interval stays scheduled; a call ended with nothing pending reports nothing (a
        // stop then ends the drain on ScriptMustEnd).
        ListenerCallContinues(cx, ok);
        delete[] argv;
        JS_EndRequest(cx);
    }
    if (t && !done) {
        EnterCriticalSection(&timerLock);
        t->queued = false;
        // Cleared while its callback ran: no event was queued for it, so it goes now.
        if (t->cancelled) {
            timers.erase(id);
            done = true;
        }
        LeaveCriticalSection(&timerLock);
    }
    if (done && t)
        FreeTimer(t);
    delete (int*)evt->arg1;
    delete evt;
}

void ScriptEngine::DestroyTimers(Script* owner) {
    if (!timerLockReady)
        return;
    std::vector<DelayedTimer*> mine;
    EnterCriticalSection(&timerLock);
    for (std::map<int, DelayedTimer*>::iterator it = timers.begin(); it != timers.end();) {
        if (it->second->owner == owner) {
            mine.push_back(it->second);
            timers.erase(it++);
        } else
            ++it;
    }
    LeaveCriticalSection(&timerLock);
    for (size_t i = 0; i < mine.size(); i++)
        FreeTimer(mine[i]);
}

void CALLBACK EventTimerProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue) {
    int id = (int)(INT_PTR)lpArg;
    EnterCriticalSection(&timerLock);
    std::map<int, DelayedTimer*>::iterator it = timers.find(id);
    // One queued firing is enough: a script running behind gets one call, not a backlog.
    if (it != timers.end() && !it->second->cancelled && !it->second->queued)
        QueueTimerEvent(it->second);
    LeaveCriticalSection(&timerLock);
}