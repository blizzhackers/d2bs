#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <list>

#include "js32.h"
#include "JSGlobalClasses.h"
#include "AutoRoot.h"
#include "JSUnit.h"
#include "Events.h"
enum ScriptState { InGame, OutOfGame, Command };

class Script;

typedef std::map<std::wstring, bool> IncludeList;
typedef std::list<AutoRoot*> FunctionList;
typedef std::map<std::string, FunctionList> FunctionMap;
typedef std::list<Script*> ScriptList;

struct Event {
    Event() : done(NULL), refs(1), freeArgs(NULL){};
    Script* owner;
    JSObject* object;
    FunctionList functions;
    JSAutoStructuredCloneBuffer** argv;
    uint argc;
    char* name;
    void* arg1;
    void* arg2;
    void* arg3;
    void* arg4;
    void* arg5;
    // A blocker event (one whose listeners' verdict another thread waits for) has two owners, the waiting thread and
    // the script that runs it, and whichever lets go last frees it: a wait that times out must not free an event the
    // script still holds, and only the event's own completion may end its wait.
    HANDLE done;
    volatile long refs;
    void (*freeArgs)(Event* evt);
};

// Fires a blocker event at its owner and waits up to timeoutMs for the verdict its listeners leave in *(DWORD*)arg4;
// false (not blocked) when they take longer. evt->freeArgs must free the event's arguments. The game thread lets go
// of the game lock while it waits, so a listener can call natives that take it (clickMap(), me.cancel(), ...), and
// so the waiting thread must hold no lock that such a listener can need.
bool AwaitBlocker(Event* evt, DWORD timeoutMs);
// AwaitBlocker in two steps, for a caller that fires under a lock and waits after leaving it.
void FireBlocker(Event* evt);
bool WaitForBlocker(Event* evt, DWORD timeoutMs);
// The owning script's side, for a blocker event it ran or dropped: records the verdict and ends the wait.
void CompleteBlocker(Event* evt, bool block);
void ReleaseBlocker(Event* evt);

class Script {
  private:
    std::wstring fileName;
    int execCount;
    ScriptState scriptState;
    JSContext* context;
    JSScript* script;
    JSRuntime* runtime;
    myUnit* me;
    uint argc;
    JSAutoStructuredCloneBuffer** argv;

    JSObject *globalObject, *scriptObject;
    bool isLocked, isPaused, isReallyPaused, isAborted;

    IncludeList includes, inProgress;

    HANDLE threadHandle;

    CRITICAL_SECTION lock;

    // Script(const char* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
    Script(const wchar_t* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
    Script(const Script&);
    Script& operator=(const Script&);
    ~Script(void);

  public:
    DWORD threadId;
    friend class ScriptEngine;
    FunctionMap functions;
    void Run(void);
    void Join(void);
    void Pause(void);
    void Resume(void);
    bool IsPaused(void);
    bool BeginThread(LPTHREAD_START_ROUTINE ThreadFunc);
    void RunCommand(const wchar_t* command);
    inline void SetPauseState(bool reallyPaused) {
        isReallyPaused = reallyPaused;
    }
    inline bool IsReallyPaused(void) {
        return isReallyPaused;
    }
    void Stop(bool force = false, bool reallyForce = false);

    inline const wchar_t* GetFilename(void) {
        return fileName.c_str();
    }
    const wchar_t* GetShortFilename(void);
    inline JSContext* GetContext(void) {
        return context;
    }
    inline JSRuntime* GetRuntime(void) {
        return runtime;
    }
    inline JSObject* GetGlobalObject(void) {
        return globalObject;
    }
    inline JSObject* GetScriptObject(void) {
        return scriptObject;
    }
    inline ScriptState GetState(void) {
        return scriptState;
    }
    inline void TriggerOperationCallback(void) {
        if (hasActiveCX)
            JS_TriggerOperationCallback(runtime);
    }
    int GetExecutionCount(void);
    DWORD GetThreadId(void);
    DWORD LastGC;
    bool hasActiveCX; // hack to get away from JS_IsRunning
    HANDLE eventSignal;

    // UGLY HACK to fix up the player gid on game join for cached scripts/oog scripts
    void UpdatePlayerGid(void);
    // Hack. Include from console needs to run on the RunCommandThread / cx.
    //		 a better solution may be to keep a list of threadId / cx and have a GetCurrentThreadCX()
    inline void SetContext(JSContext* cx) {
        context = cx;
    }
    bool IsRunning(void);
    bool IsAborted(void);
    void Lock() {
        EnterCriticalSection(&lock);
    } // needed for events walking function list
    void Unlock() {
        LeaveCriticalSection(&lock);
    }
    bool IsIncluded(const wchar_t* file);
    bool Include(const wchar_t* file);

    bool IsListenerRegistered(const char* evtName);
    void RegisterEvent(const char* evtName, jsval evtFunc);
    bool IsRegisteredEvent(const char* evtName, jsval evtFunc);
    void UnregisterEvent(const char* evtName, jsval evtFunc);
    void ClearEvent(const char* evtName);
    void ClearAllEvents(void);
    void FireEvent(Event*);
    std::list<Event*> EventList;
};

struct RUNCOMMANDSTRUCT {
    Script* script;
    const wchar_t* command;
};

DWORD WINAPI RunCommandThread(void* data);
DWORD WINAPI ScriptThread(void* data);
DWORD WINAPI FuncThread(void* data);
DWORD WINAPI EventThread(LPVOID lpParam);
bool callEventFunction(JSContext* cx, Event* evt);
