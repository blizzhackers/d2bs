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
    : execCount(0), scriptState(state), context(NULL), hadContext(false), script(NULL), runtime(NULL), me(NULL), argc(argc), argv(argv), globalObject(NULL),
      scriptObject(NULL), isPaused(false), isReallyPaused(false), isAborted(false), runBegun(false), threadHandle(INVALID_HANDLE_VALUE),
      id((DWORD)InterlockedIncrement(&lastScriptId)), startFailed(false), predecessor(NULL), refs(1), engineRefReleased(0), endReported(0), threadId(0) {
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
    InitializeCriticalSection(&listenersLock);
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
    // A Script that never had a context gets no destroy-callback drain: events queued to it (a message sent before
    // its Run failed, a console command after its start failed, a DisposeMe) are freed here. Only those kinds can
    // reach it, since it never ran JS (no listener, hook or timer), and their clear paths free plain memory: clone
    // buffers need no runtime. One that had a context leaves a late event unfreed: its roots died with the runtime.
    while (!hadContext && !EventList.empty()) {
        EnterCriticalSection(&Vars.cEventSection);
        Event* evt = EventList.back();
        EventList.pop_back();
        LeaveCriticalSection(&Vars.cEventSection);
        ExecScriptEvent(evt, true);
    }

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
    DeleteCriticalSection(&listenersLock);
}

void Script::FreeArgs(void) {
    FreeCloneBuffers(argv, argc);
    argv = NULL;
    argc = 0;
}

void Script::AddRef(void) {
    InterlockedIncrement(&refs);
}

// Reaches zero only after DisposeScript released the engine's reference, which it does only for a never-started Script.
void Script::Release(void) {
    if (InterlockedDecrement(&refs) == 0)
        delete this;
}

int Script::GetExecutionCount(void) {
    return execCount;
}

DWORD Script::GetThreadId(void) {
    return (threadHandle == INVALID_HANDLE_VALUE ? -1 : threadId);
}

void Script::RunCommand(const wchar_t* command) {
    if (isAborted) { // this should never happen -bob
        Log(L"Console Aborted HELP!");
    }

    Event* evt = new Event;
    evt->owner = this;
    evt->argc = argc;
    evt->name = "Command";
    evt->arg1 = _wcsdup(command);
    // Out of memory: drop the command rather than queue a NULL line the console would read.
    if (!evt->arg1) {
        delete evt;
        return;
    }
    EnterCriticalSection(&Vars.cEventSection);
    EventList.push_front(evt);
    LeaveCriticalSection(&Vars.cEventSection);
    // Only `this` after the push: the console may run and free evt as soon as the event lock is free.
    TriggerOperationCallback();
    SetEvent(eventSignal);
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
        hadContext = true;

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
        runBegun = true;
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
        runBegun = true;
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
    runBegun = true;
    if (isAborted) {
        // Stopped before main(): a stop that found hasActiveCX clear left this run's end line to here.
        ReportEnded();
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

    // Out of memory before the script runs: it does not start, as when its arguments can't be read. Removing a root
    // that was never added does nothing.
    if (!JS_AddValueRoot(GetContext(), &main) || !JS_AddValueRoot(GetContext(), &dummy)) {
        JS_RemoveValueRoot(GetContext(), &main);
        JS_RemoveValueRoot(GetContext(), &dummy);
        JS_ReportPendingException(context);
        if (scriptState != Command) {
            JS_RemoveScriptRoot(context, &script);
            script = NULL;
        }
        JS_EndRequest(GetContext());
        hasActiveCX = false;
        return false;
    }
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
    // Before hasActiveCX clears: after a natural end, a caller that waits for running to read false and then prints
    // lands after this line.
    ReportEnded();
    // Nothing runs on this context any more: running reads false and no event queues for a cached, ended script.
    hasActiveCX = false;
    // On the owner thread, which the callback roots need: an ended cached copy may never be destroyed, and its timers
    // can no longer fire.
    ScriptEngine::DestroyTimers(this);

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
    // Only a run that has published hasActiveCX reports here. Read after the barrier above, pairing with Run's: a run
    // that has not yet sees the stop and reports itself, and one that fails before then logs "failed to start".
    if (hasActiveCX)
        ReportEnded();
    // trigger call back so script ends
    TriggerOperationCallback();
    SetEvent(eventSignal);
    return true;
}

bool Script::ClaimEndReport(void) {
    return InterlockedExchange(&endReported, 1) == 0;
}

// Any thread. The run's first end, main() returning or a stop, logs the line, and later ones stay silent. Skipped
// once D2BS shuts down: nothing flushes the print queue then, and its lock is deleted soon after.
void Script::ReportEnded(void) {
    if (GetState() == Command || !Vars.bActive || !ClaimEndReport())
        return;
    const wchar_t* displayName = fileName.c_str() + wcslen(Vars.szScriptPath) + 1;
    Print(L"Script %s ended", displayName);
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

// With the script list lock held. A copy that has not started takes the message without a listener check, since its
// top level has not added any yet; a started one, running or paused, needs a scriptmsg listener. An ended or
// stopping copy takes none, nor one whose thread could not start: nothing would run it before the script is destroyed.
bool Script::AcceptsSentMessage(void) {
    if (isAborted || startFailed)
        return false;
    if (!runBegun)
        return true;
    return context && hasActiveCX && IsListenerRegistered("scriptmsg");
}

bool Script::IsAborted() {
    return isAborted;
}

namespace {
// Deletes each listener's root. Called outside listenersLock: removing a root is an engine call.
void FreeListeners(FunctionList& list) {
    for (FunctionList::iterator it = list.begin(); it != list.end(); it++) {
        (*it)->Release();
        delete *it;
    }
    list.clear();
}
} // namespace

bool Script::IsListenerRegistered(const char* evtName) {
    if (evtName[0] == '\0')
        return false;
    std::string key(evtName);
    EnterCriticalSection(&listenersLock);
    FunctionMap::const_iterator entry = functions.find(key);
    bool registered = entry != functions.end() && !entry->second.empty();
    LeaveCriticalSection(&listenersLock);
    return registered;
}

bool Script::RegisterEvent(const char* evtName, jsval evtFunc) {
    // Once per function and event, as in a browser: a second copy would run again on every event and keep its root
    // after the one removeEventListener() takes out.
    if (!JSVAL_IS_FUNCTION(context, evtFunc) || evtName[0] == '\0' || IsRegisteredEvent(evtName, evtFunc))
        return true;
    AutoRoot* root = new AutoRoot(context, evtFunc);
    if (!root->ok()) {
        delete root;
        return false;
    }
    std::string key(evtName);
    EnterCriticalSection(&listenersLock);
    functions[key].push_back(root);
    LeaveCriticalSection(&listenersLock);
    return true;
}

bool Script::IsRegisteredEvent(const char* evtName, jsval evtFunc) {
    if (evtName[0] == '\0')
        return false;
    FunctionMap::iterator entry = functions.find(evtName);
    if (entry == functions.end())
        return false;
    for (FunctionList::iterator it = entry->second.begin(); it != entry->second.end(); it++)
        if (*(*it)->value() == evtFunc)
            return true;
    return false;
}

void Script::UnregisterEvent(const char* evtName, jsval evtFunc) {
    if (evtName[0] == '\0')
        return;
    std::string key(evtName);
    AutoRoot* func = NULL;
    EnterCriticalSection(&listenersLock);
    FunctionMap::iterator entry = functions.find(key);
    if (entry != functions.end()) {
        for (FunctionList::iterator it = entry->second.begin(); it != entry->second.end(); it++) {
            if (*(*it)->value() == evtFunc) {
                func = *it;
                entry->second.erase(it);
                break;
            }
        }
        // A key exists only while it has listeners: IsListenerRegistered and the event hooks rely on it.
        if (entry->second.empty())
            functions.erase(entry);
    }
    LeaveCriticalSection(&listenersLock);
    delete func;
}

void Script::ClearEvent(const char* evtName) {
    FunctionList cleared;
    std::string key(evtName);
    EnterCriticalSection(&listenersLock);
    FunctionMap::iterator entry = functions.find(key);
    if (entry != functions.end()) {
        cleared.swap(entry->second);
        functions.erase(entry);
    }
    LeaveCriticalSection(&listenersLock);
    FreeListeners(cleared);
}

void Script::ClearAllEvents(void) {
    FunctionMap cleared;
    EnterCriticalSection(&listenersLock);
    cleared.swap(functions);
    LeaveCriticalSection(&listenersLock);
    for (FunctionMap::iterator it = cleared.begin(); it != cleared.end(); it++)
        FreeListeners(it->second);
}
void Script::FireEvent(Event* evt) {
    // Only `this` once the event is queued: the script may run and free it as soon as the event lock is free. Every
    // caller passes its own event (evt->owner == this) and keeps this Script alive across the call.
    EnterCriticalSection(&Vars.cEventSection);
    EventList.push_front(evt);
    LeaveCriticalSection(&Vars.cEventSection);

    if (IsRunning())
        TriggerOperationCallback();
    SetEvent(eventSignal);
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
            // A stop that already logged this run's "ended" line keeps this one quiet.
            if (script->ClaimEndReport()) {
                // Hex escapes, split so the colour digit isn't read as hex: a universal-character-name raises C4428.
                Print(L"\xFF" L"c2D2BS\xFF" L"c0 :: Script %s failed to start", script->GetShortFilename());
            }
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
