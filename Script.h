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
    JSAutoStructuredCloneBuffer** argv;
    uint argc;
    const char* name; // a string literal: never copied or freed
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
// so the waiting thread must hold no other lock that such a listener can need. Any other thread that holds the game
// lock does not wait at all: the listeners run afterwards, and the verdict is false.
bool AwaitBlocker(Event* evt, DWORD timeoutMs);
// AwaitBlocker in two steps, for a caller that fires under a lock and waits after leaving it.
void FireBlocker(Event* evt);
bool WaitForBlocker(Event* evt, DWORD timeoutMs);
// The owning script's side, for a blocker event it ran or dropped: records the verdict and ends the wait.
void CompleteBlocker(Event* evt, bool block);
void ReleaseBlocker(Event* evt);
// Deletes each of the first count buffers and then the array, which must come from new[]; NULL buffers does nothing.
void FreeCloneBuffers(JSAutoStructuredCloneBuffer** buffers, uint count);

class Script {
  private:
    std::wstring fileName;
    int execCount;
    ScriptState scriptState;
    JSContext* context;
    // Set once Run has created the context: ~Script then frees no queued event itself, since a late one's roots and
    // buffers belong to the runtime that is already gone.
    bool hadContext;
    JSScript* script;
    JSRuntime* runtime;
    myUnit* me;
    uint argc;
    JSAutoStructuredCloneBuffer** argv;

    JSObject *globalObject, *scriptObject;
    bool isLocked, isPaused, isReallyPaused, isAborted;
    // Set by Run once the copy has started, failed or found a stop; read by script.send under the script list lock.
    volatile bool runBegun;

    IncludeList includes, inProgress;

    HANDLE threadHandle;
    DWORD id;
    bool startFailed; // BeginThread could not create the thread
    // The replaced copy's thread, waited for at the top of Run so the loader never stalls; NULL when none.
    HANDLE predecessor;
    // One reference belongs to the engine from construction; ScriptEngine::ForEachScript takes one more per entry it
    // calls back on. A started Script is deleted only by its own thread once the others are released (its runtime must
    // die there). A timeout of DisposeScript's bound during main() is retried at thread exit; the Script is left
    // undeleted only when no retry follows (the console) or the retry also times out. A never-started one goes with
    // the last release.
    volatile LONG refs;
    volatile LONG engineRefReleased; // 1 once DisposeScript has released the engine's reference
    // 1 once the run's end is reported, as "ended" (ReportEnded) or "failed to start" (ReportStartFailed): the first
    // report wins, so a run logs at most one end line. A Script runs at most once, so it is never reset.
    volatile LONG endReported;
    // Any thread: true only for the run's first end report, which then prints its line.
    bool ClaimEndReport(void);
    void ReportEnded(void);
    void AddRef(void);
    void Release(void);

    CRITICAL_SECTION lock;
    // Guards the structure of `functions` (keys and lists), never the listeners' calls: the event hooks read it from
    // other threads. Held only for map operations: no lock but the CRT heap's, no JS call, root change or wait inside it.
    CRITICAL_SECTION listenersLock;

    // Script(const char* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
    Script(const wchar_t* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
    Script(const Script&);
    Script& operator=(const Script&);
    ~Script(void);
    void FreeArgs(void);

  public:
    DWORD threadId;
    friend class ScriptEngine;
    FunctionMap functions;
    bool Run(void);
    // Starts at 1, one per Script; 0 means no script. It would repeat only after 2^32 scripts in one process, so a Script
    // object names one script for good in practice.
    inline DWORD GetId(void) {
        return id;
    }
    // A handle to wait on the script's thread; *out is NULL when no thread was started. False only when the handle
    // could not be copied. Takes no lock: the caller holds the list lock, which keeps the Script and its handle alive.
    bool DuplicateThreadHandle(HANDLE* out);
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
    // Sets the stop flags and wakes the script without taking its lock or waiting; false when already stopped.
    bool SignalStop(void);
    // For a Run that returned false: prints "failed to start" unless a stop already logged this run's "ended" line.
    void ReportStartFailed(void);
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
    inline void SetContext(JSContext* cx) {
        context = cx;
    }
    bool IsRunning(void);
    bool AcceptsSentMessage(void);
    bool IsAborted(void);
    bool IsIncluded(const wchar_t* file);
    bool Include(const wchar_t* file);

    // Any thread. True while at least one listener is registered for evtName; takes only listenersLock.
    bool IsListenerRegistered(const char* evtName);
    // False only when the listener could not be rooted (out of memory, maybe unreported); nothing is added then. A
    // non-function, an empty name or a function already listening for evtName adds nothing and returns true.
    bool RegisterEvent(const char* evtName, jsval evtFunc);
    // The registry's mutators and IsRegisteredEvent run on the script's own thread, or at teardown after it has ended,
    // so the owner reads `functions` without the lock.
    bool IsRegisteredEvent(const char* evtName, jsval evtFunc);
    void UnregisterEvent(const char* evtName, jsval evtFunc);
    void ClearEvent(const char* evtName);
    void ClearAllEvents(void);
    void FireEvent(Event*);
    std::list<Event*> EventList;
};

DWORD WINAPI ScriptThread(void* data);
DWORD WINAPI FuncThread(void* data);
DWORD WINAPI EventThread(LPVOID lpParam);
bool callEventFunction(JSContext* cx, Event* evt);
