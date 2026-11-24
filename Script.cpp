#include <io.h>
#include <algorithm>

#include "Script.h"
#include "Core.h"
#include "Constants.h"
#include "D2Ptrs.h"
#include "JSUnit.h"
#include "Helpers.h"
#include "ScriptEngine.h"
#include "D2BS.h"
#include "HangReport.h"

using namespace std;

static volatile LONG lastScriptId = 0;

// Up to this long for a replaced script's thread to end: Script::Stop's normal budget.
static const DWORD kReplaceWaitMs = 500;

Script::Script(const wchar_t* file, ScriptState state, uint argc, JSAutoStructuredCloneBuffer** argv)
    : context(NULL), globalObject(NULL), scriptObject(NULL), script(NULL), runtime(NULL), me(NULL), execCount(0), isAborted(false), isPaused(false), isReallyPaused(false), scriptState(state),
      threadHandle(INVALID_HANDLE_VALUE), threadId(0), id((DWORD)InterlockedIncrement(&lastScriptId)), argc(argc), argv(argv), startFailed(false),
      predecessor(NULL) {
    // Everything that can throw runs before the event and the lock exist: a throwing constructor runs no destructor.
    if (scriptState == Command && wcslen(file) < 1) {
        fileName = wstring(L"Command Line");
    } else {
        if (_waccess(file, 0) != 0) {
            DEBUG_LOG(file);

            throw std::exception("File not found");
        }

        wchar_t* tmpName = _wcsdup(file);
        if (!tmpName)
            throw std::exception("Could not dup filename");

        _wcslwr_s(tmpName, wcslen(file) + 1);
        fileName = wstring(tmpName);
        replace(fileName.begin(), fileName.end(), L'/', L'\\');
        free(tmpName);
    }

    LastGC = 0;
    hasActiveCX = false;
    eventSignal = CreateEvent(nullptr, true, false, nullptr);
    if (!eventSignal)
        throw std::exception("Could not create the script's event");
    InitializeCriticalSection(&lock);
}

Script::~Script(void) {
    isAborted = true;
    hasActiveCX = false;
    // JS_SetPendingException(context, JSVAL_NULL);
    if (context && runtime && JS_IsInRequest(runtime))
        JS_EndRequest(context);

    EnterCriticalSection(&lock);
    //    JS_SetRuntimeThread(rt);
    if (context)
        JS_DestroyContext(context);
    // JS_ClearRuntimeThread(rt);
    if (runtime)
        JS_DestroyRuntime(runtime);

    context = NULL;
    scriptObject = NULL;
    globalObject = NULL;
    script = NULL;
    FreeArgs();
    CloseHandle(eventSignal);
    includes.clear();
    if (threadHandle != INVALID_HANDLE_VALUE)
        CloseHandle(threadHandle);
    if (predecessor)
        CloseHandle(predecessor);
    LeaveCriticalSection(&lock);
    DeleteCriticalSection(&lock);
}

void Script::FreeArgs(void) {
    FreeCloneBuffers(argv, argc);
    argv = NULL;
    argc = 0;
}

int Script::GetExecutionCount(void) {
    return execCount;
}

DWORD Script::GetThreadId(void) {
    return (threadHandle == INVALID_HANDLE_VALUE ? -1 : threadId);
}

void Script::RunCommand(const wchar_t* command) {
    // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;
    // rcs->script = this;
    // rcs->command = _wcsdup(command);

    if (isAborted) { // this should never happen -bob
        // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;

        // rcs->script = this;
        // rcs->command = _wcsdup(L"delay(1000000);");

        Log(L"Console Aborted HELP!");
        // HANDLE hwnd = CreateThread(NULL, 0, RunCommandThread, (void*) rcs, 0, NULL);
    }

    Event* evt = new Event;
    evt->owner = this;
    evt->argc = argc;
    evt->name = _strdup("Command");
    evt->arg1 = _wcsdup(command);
    EnterCriticalSection(&Vars.cEventSection);
    evt->owner->EventList.push_front(evt);
    LeaveCriticalSection(&Vars.cEventSection);
    evt->owner->TriggerOperationCallback();
    SetEvent(evt->owner->eventSignal);
}

bool Script::BeginThread(LPTHREAD_START_ROUTINE ThreadFunc) {
    EnterCriticalSection(&lock);
    DWORD dwExitCode = STILL_ACTIVE;

    if (GetExitCodeThread(threadHandle, &dwExitCode) && dwExitCode == STILL_ACTIVE) {
        LeaveCriticalSection(&lock);
        return false;
    }

    threadHandle = CreateThread(0, 0, ThreadFunc, this, 0, &threadId);
    if (threadHandle) {
        LeaveCriticalSection(&lock);
        return true;
    }

    threadHandle = INVALID_HANDLE_VALUE;
    startFailed = true;
    LeaveCriticalSection(&lock);
    return false;
}

bool Script::Run(void) {
    // A reload starts its new copy once the replaced one has ended, or after this bound: waited here on the new copy's
    // thread, so load() returns at once and the replacement is listed but not yet running meanwhile.
    if (predecessor) {
        WaitForSingleObject(predecessor, kReplaceWaitMs);
        CloseHandle(predecessor);
        predecessor = NULL;
    }

    bool inRequest = false;
    try {
        runtime = JS_NewRuntime(Vars.dwMemUsage, JS_NO_HELPER_THREADS);
        if (!runtime)
            throw std::exception("Couldn't create the runtime");
        JS_SetNativeStackQuota(runtime, (size_t)50000);
        // JS_SetRuntimeThread(runtime);
        JS_SetContextCallback(runtime, contextCallback);

        context = JS_NewContext(runtime, 0x2000);
        if (!context)
            throw std::exception("Couldn't create the context");

        JS_SetErrorReporter(context, reportError);
        JS_SetOperationCallback(context, operationCallback);
        JS_SetOptions(context, JSOPTION_STRICT | JSOPTION_VAROBJFIX);
        JS_SetVersion(context, JSVERSION_LATEST);
        //

        JS_SetContextPrivate(context, this);

        JS_BeginRequest(context);
        inRequest = true;

        globalObject = JS_GetGlobalObject(context);
        jsval meVal = JSVAL_VOID;
        if (JS_GetProperty(GetContext(), globalObject, "me", &meVal) != JS_FALSE) {
            JSObject* meObject = JSVAL_TO_OBJECT(meVal);
            me = (myUnit*)JS_GetPrivate(GetContext(), meObject);
        }

        if (scriptState == Command) {
            if (wcslen(Vars.szConsole) > 0) {
                script = JS_CompileFile(context, globalObject, fileName);
            } else {
                char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
                script = JS_CompileScript(context, globalObject, cmd, strlen(cmd), "Command Line", 1);
            }
        } else
            script = JS_CompileFile(context, globalObject, fileName);

        if (!script) {
            JS_ReportPendingException(context);
            throw std::exception("Couldn't compile the script");
        }
        // Rooted until JS_ExecuteScript holds it: reading the load() arguments below can run the GC, which scans the
        // stack but not this member. The console's root lasts as long as the console.
        if (!JS_AddNamedScriptRoot(context, &script, scriptState == Command ? "console script" : "script")) {
            script = NULL;
            throw std::exception("Couldn't root the script");
        }

        JS_EndRequest(context);
        inRequest = false;
    } catch (std::exception& e) {
        // Leaves no context or runtime behind: ScriptThread disposes this Script next (the console stays listed), and ~Script skips NULL members.
        Log(L"Script %s: %hs", GetShortFilename(), e.what());
        // Cleared under the list lock before the context that owns it is destroyed: UpdatePlayerGid writes through it
        // under that lock.
        ScriptEngine::LockScriptList("Run failure");
        me = NULL;
        ScriptEngine::UnLockScriptList("Run failure");
        if (context) {
            if (inRequest)
                JS_EndRequest(context);
            JS_DestroyContext(context);
            context = NULL;
        }
        if (runtime) {
            JSRuntime* rt = runtime;
            // Unpublished under the list lock first: script.memory reads the runtime while it holds that lock.
            ScriptEngine::LockScriptList("Run failure");
            runtime = NULL;
            ScriptEngine::UnLockScriptList("Run failure");
            JS_DestroyRuntime(rt);
        }
        globalObject = NULL;
        script = NULL;
        return false;
    }
    // only let the script run if it's not already running
    if (IsRunning()) {
        if (scriptState != Command) {
            JS_RemoveScriptRoot(context, &script);
            script = NULL;
        }
        return true;
    }
    // A stop or replacement that landed while the thread was starting stands: each Script runs at most once, so
    // clearing the flag here would only lose it. ScriptThread then treats the copy as an ended one: disposed with
    // DisableCache or once unlisted, otherwise kept in the cache. Published before the flag is read, pairing with
    // SignalStop's store-then-read: a stop either is seen here or finds hasActiveCX set and interrupts the runtime.
    hasActiveCX = true;
    MemoryBarrier();
    if (isAborted) {
        hasActiveCX = false;
        if (scriptState != Command) {
            JS_RemoveScriptRoot(context, &script);
            script = NULL;
        }
        return true;
    }

    jsval main = INT_TO_JSVAL(1), dummy = INT_TO_JSVAL(1);
    JS_BeginRequest(GetContext());

    // The arguments passed from load() are rooted before the first is read: reading a later one can run the GC.
    JS::AutoValueVector argvalue(context);
    bool haveArgs = argvalue.resize(argc);
    for (uint i = 0; haveArgs && i < argc; i++)
        haveArgs = argv[i]->read(context, &argvalue[i]);

    if (!haveArgs) {
        // The arguments can't be held, so the script never starts, as with a compile failure.
        JS_ReportPendingException(context); // a failed resize has already reported its out-of-memory error
        if (scriptState != Command) {
            JS_RemoveScriptRoot(context, &script);
            script = NULL;
        }
        JS_EndRequest(GetContext());
        hasActiveCX = false;
        return false;
    }

    JS_AddValueRoot(GetContext(), &main);
    JS_AddValueRoot(GetContext(), &dummy);
    JSBool executed = JS_ExecuteScript(GetContext(), globalObject, script, &dummy);
    if (scriptState != Command) {
        JS_RemoveScriptRoot(context, &script);
        script = NULL;
    }
    if (executed != JS_FALSE && JS_GetProperty(GetContext(), globalObject, "main", &main) != JS_FALSE &&
        JSVAL_IS_FUNCTION(GetContext(), main)) {
        JS_CallFunctionValue(GetContext(), globalObject, main, this->argc, argvalue.begin(), &dummy);
    }
    JS_RemoveValueRoot(GetContext(), &main);
    JS_RemoveValueRoot(GetContext(), &dummy);

    // The buffers are freed on this thread once main() returns: main's values are copies read into argvalue.
    FreeArgs();

    JS_EndRequest(GetContext());
    // Nothing runs on this context any more: running reads false and no event queues for a cached, ended script.
    hasActiveCX = false;

    execCount++;
    // Stop();
    return true;
}

void Script::UpdatePlayerGid(void) {
    UnitAny* player = D2CLIENT_GetPlayerUnit();
    // Under the list lock: Run's failure path clears `me` under it before the context that owns it is destroyed. `me`
    // is NULL until the script's context exists, and again after a failed start.
    ScriptEngine::LockScriptList("UpdatePlayerGid");
    if (me)
        me->dwUnitId = player ? player->dwUnitId : 0;
    ScriptEngine::UnLockScriptList("UpdatePlayerGid");
}

void Script::Pause(void) {
    if (!IsAborted() && !IsPaused())
        isPaused = true;
    TriggerOperationCallback();
    SetEvent(eventSignal);
}

bool Script::DuplicateThreadHandle(HANDLE* out) {
    *out = NULL;
    HANDLE hThread = threadHandle;
    if (hThread == INVALID_HANDLE_VALUE || hThread == NULL)
        return true;
    return !!DuplicateHandle(GetCurrentProcess(), hThread, GetCurrentProcess(), out, SYNCHRONIZE, FALSE, 0);
}

void Script::Resume(void) {
    if (!IsAborted() && IsPaused())
        isPaused = false;
    TriggerOperationCallback();
    SetEvent(eventSignal);
}

bool Script::IsPaused(void) {
    return isPaused;
}

const wchar_t* Script::GetShortFilename() {
    if (wcscmp(fileName.c_str(), L"Command Line") == 0)
        return fileName.c_str();
    else
        return (fileName.c_str() + wcslen(Vars.szScriptPath) + 1);
}

bool Script::SignalStop(void) {
    if (isAborted)
        return false;
    isAborted = true;
    // Pairs with Run's publish of hasActiveCX: the flag is visible before hasActiveCX is read below.
    MemoryBarrier();
    isPaused = false;
    isReallyPaused = false;
    if (GetState() != Command) {
        const wchar_t* displayName = fileName.c_str() + wcslen(Vars.szScriptPath) + 1;
        Print(L"Script %s ended", displayName);
    }
    // trigger call back so script ends
    TriggerOperationCallback();
    SetEvent(eventSignal);
    return true;
}

void Script::Stop(bool force, bool reallyForce) {
    // if we've already stopped, just return
    if (isAborted)
        return;
    EnterCriticalSection(&lock);
    SignalStop();
    // normal wait: 500ms, forced wait: 300ms, really forced wait: 100ms
    int maxCount = (force ? (reallyForce ? 10 : 30) : 50);
    if (GetCurrentThreadId() != GetThreadId()) {
        for (int i = 0; hasActiveCX == true; i++) {
            // if we pass the time frame, just ignore the wait because the thread will end forcefully anyway
            if (i >= maxCount)
                break;
            Sleep(10);
        }
    }
    LeaveCriticalSection(&lock);
}

bool Script::IsIncluded(const wchar_t* file) {
    uint count = 0;
    wchar_t* fname = _wcsdup(file);
    if (!fname)
        return false;

    _wcslwr_s(fname, wcslen(fname) + 1);
    StringReplace(fname, '/', '\\', wcslen(fname));
    count = includes.count(wstring(fname));
    free(fname);

    return !!count;
}

bool Script::Include(const wchar_t* file) {
    wchar_t* fname = _wcsdup(file);
    if (!fname)
        return false;
    // since includes will happen on the same thread, locking here is acceptable
    EnterCriticalSection(&lock);
    _wcslwr_s(fname, wcslen(fname) + 1);
    StringReplace(fname, L'/', L'\\', wcslen(fname));

    // don't invoke the string ctor more than once...
    wstring currentFileName = wstring(fname);
    // ignore already included, 'in-progress' includes, and self-inclusion
    if (!!includes.count(fname) || !!inProgress.count(fname) || (currentFileName.compare(fileName.c_str()) == 0)) {
        LeaveCriticalSection(&lock);
        free(fname);
        return true;
    }
    bool rval = false;

    JSContext* cx = GetContext();

    JS_BeginRequest(cx);

    JSScript* script = JS_CompileFile(cx, GetGlobalObject(), fname);
    if (script) {
        jsval dummy;
        inProgress[fname] = true;
        rval = !!JS_ExecuteScript(cx, GetGlobalObject(), script, &dummy);
        if (rval)
            includes[fname] = true;
        else
            JS_ReportPendingException(cx);
        inProgress.erase(fname);
        // JS_RemoveRoot(&scriptObj);
    } else
        JS_ReportPendingException(cx);

    JS_EndRequest(cx);
    // JS_RemoveScriptRoot(cx, &script);
    LeaveCriticalSection(&lock);
    free(fname);
    return rval;
}

bool Script::IsRunning(void) {
    return context && !(IsAborted() || IsPaused() || !hasActiveCX);
}

bool Script::IsAborted() {
    return isAborted;
}

bool Script::IsListenerRegistered(const char* evtName) {
    return strlen(evtName) > 0 && functions.count(evtName) > 0;
}

void Script::RegisterEvent(const char* evtName, jsval evtFunc) {
    EnterCriticalSection(&lock);
    // Once per function and event, as in a browser: a second copy would run again on every event and keep its root
    // after the one removeEventListener() takes out.
    if (JSVAL_IS_FUNCTION(context, evtFunc) && strlen(evtName) > 0 && !IsRegisteredEvent(evtName, evtFunc)) {
        AutoRoot* root = new AutoRoot(context, evtFunc);
        functions[evtName].push_back(root);
    }
    LeaveCriticalSection(&lock);
}

bool Script::IsRegisteredEvent(const char* evtName, jsval evtFunc) {
    // nothing can be registered under an empty name
    if (strlen(evtName) < 1)
        return false;

    // if there are no events registered under that name at all, then obviously there
    // can't be a specific one registered under that name
    if (functions.count(evtName) < 1)
        return false;

    for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++)
        if (*(*it)->value() == evtFunc)
            return true;

    return false;
}

void Script::UnregisterEvent(const char* evtName, jsval evtFunc) {
    if (strlen(evtName) < 1)
        return;

    EnterCriticalSection(&lock);
    AutoRoot* func = NULL;
    for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++) {
        if (*(*it)->value() == evtFunc) {
            func = *it;
            break;
        }
    }
    functions[evtName].remove(func);
    // func->Release();
    delete func;

    // Remove event completely if there are no listeners for it.
    if (functions.count(evtName) > 0 && functions[evtName].size() == 0)
        functions.erase(evtName);

    LeaveCriticalSection(&lock);
}

void Script::ClearEvent(const char* evtName) {
    EnterCriticalSection(&lock);
    for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++) {
        AutoRoot* func = *it;
        func->Release();
        delete func;
    }
    functions[evtName].clear();
    LeaveCriticalSection(&lock);
}

void Script::ClearAllEvents(void) {
    EnterCriticalSection(&lock);
    for (FunctionMap::iterator it = functions.begin(); it != functions.end(); it++)
        ClearEvent(it->first.c_str());
    functions.clear();
    LeaveCriticalSection(&lock);
}
void Script::FireEvent(Event* evt) {
    // EnterCriticalSection(&ScriptEngine::lock);
    EnterCriticalSection(&Vars.cEventSection);
    evt->owner->EventList.push_front(evt);
    LeaveCriticalSection(&Vars.cEventSection);

    if (evt->owner && evt->owner->IsRunning()) {
        evt->owner->TriggerOperationCallback();
    }
    SetEvent(eventSignal);
    // LeaveCriticalSection(&ScriptEngine::lock);
}

#ifdef DEBUG
typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // must be 0x1000
    LPCWSTR szName;    // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCWSTR szThreadName) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
    }
}
#endif

DWORD WINAPI RunCommandThread(void* data) {
    RUNCOMMANDSTRUCT* rcs = (RUNCOMMANDSTRUCT*)data;
    JSContext* cx = rcs->script->GetContext();
    JS_BeginRequest(cx);
    jsval rval;
    JS_AddNamedValueRoot(cx, &rval, "Cmd line rtl");
    if (JS_EvaluateUCScript(cx, JS_GetGlobalObject(cx), rcs->command, wcslen(rcs->command), "Command Line", 0, &rval)) {
        if (!JSVAL_IS_NULL(rval) && !JSVAL_IS_VOID(rval)) {
            JS_ConvertValue(cx, rval, JSTYPE_STRING, &rval);
            const wchar_t* text = JS_GetStringCharsZ(cx, JS_ValueToString(cx, rval));
            Print(L"%s", text);
        }
    }
    JS_RemoveRoot(cx, &rval);
    JS_EndRequest(cx);
    JS_TriggerOperationCallback(JS_GetRuntime(cx));
    return 0;
}

void FreeCloneBuffers(JSAutoStructuredCloneBuffer** buffers, uint count) {
    if (!buffers)
        return;
    for (uint i = 0; i < count; i++)
        delete buffers[i];
    delete[] buffers;
}

DWORD WINAPI ScriptThread(void* data) {
    Script* script = (Script*)data;
    if (script) {
#ifdef DEBUG
        SetThreadName(0xFFFFFFFF, script->GetShortFilename());
#endif
        HangNameThread(script->GetShortFilename());
        bool started = script->Run();
        HangForgetThread();
        if (!started) {
            // Hex escapes, split so the colour digit isn't read as hex: a universal-character-name raises C4428 here.
            Print(L"\xFF" L"c2D2BS\xFF" L"c0 :: Script %s failed to start", script->GetShortFilename());
            // The console stays: ScriptEngine::console and the map's "console" key keep pointing at it.
            if (script->GetState() != Command)
                ScriptEngine::DisposeScript(script);
        } else if ((Vars.bDisableCache || !ScriptEngine::IsListed(script)) && script->GetState() != Command)
            // A console whose main() returned stays listed, not running, like a failed one: ScriptEngine::console keeps
            // pointing at it.
            ScriptEngine::DisposeScript(script);
    }
    return 0;
}
