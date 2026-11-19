#pragma once
#ifndef __SCRIPTENGINE_H__
#define __SCRIPTENGINE_H__

#include <list>
#include <map>
#include <string>

#include "js32.h"
#include "AutoRoot.h"
#include "Script.h"

typedef std::map<std::wstring, Script*> ScriptMap;

typedef bool(__fastcall* ScriptCallback)(Script*, void*, uint);

enum EngineState { Starting, Running, Paused, Stopping, Stopped };

class ScriptEngine {
    ScriptEngine(void){};
    virtual ~ScriptEngine(void) = 0;
    ScriptEngine(const ScriptEngine&);
    ScriptEngine& operator=(const ScriptEngine&);
    static JSRuntime* runtime;
    static JSContext* context;
    static Script* console;
    static EngineState state;
    static CRITICAL_SECTION scriptListLock;

  public:
    friend class Script;
    static ScriptMap scripts;

    static CRITICAL_SECTION lock;
    static BOOL Startup(void);
    static void Shutdown(void);
    static EngineState GetState(void) {
        return state;
    }
    // For the hang report, which reads the lock's owner without taking it.
    static CRITICAL_SECTION* ListLockForReport(void) {
        return &scriptListLock;
    }

    static void FlushCache(void);

    static Script* CompileFile(const wchar_t* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL, bool recompile = false);
    static void RunCommand(const wchar_t* command);
    static void DisposeScript(Script* script);

    static void LockScriptList(char* loc);
    static void UnLockScriptList(char* loc);

    static bool ForEachScript(ScriptCallback callback, void* argv, uint argc);
    static unsigned int GetCount(bool active = true, bool unexecuted = false);

    static JSRuntime* GetRuntime(void) {
        return runtime;
    }
    static JSContext* GetGlobalContext(void) {
        return context;
    }

    static void StopAll(bool forceStop = false);
    static void ExecEventAsync(char* evtName, AutoRoot** argv, uint argc);
    static void InitClass(JSContext* context, JSObject* globalObject, JSClass* classp, JSFunctionSpec* methods, JSPropertySpec* props, JSFunctionSpec* s_methods,
                          JSPropertySpec* s_props);
    static void DefineConstant(JSContext* context, JSObject* globalObject, const char* name, int value);
    static void UpdateConsole();
    // setTimeout / setInterval. A timer belongs to the script that set it, whose thread alone runs its callback and
    // frees it; any thread may cancel it by id. Returns the id, 0 when no timer could be set.
    static int AddTimer(Script* owner, JSContext* cx, jsval callback, uint argc, jsval* argv, int delayMs, bool repeat);
    static void ClearTimer(int id);
    static int ClearTimers(Script* owner);
    static void RunTimer(Event* evt, bool clearList);
    static void DestroyTimers(Script* owner);
    JSGCCallback gcCallback(JSRuntime* rt, JSGCStatus status);
};

// these ForEachScript helpers are exposed in case they can be of use somewhere
bool __fastcall StopIngameScript(Script* script, void*, uint);
bool __fastcall ExecEventOnScript(Script* script, void* argv, uint argc);
struct EventHelper {
    char* evtName;
    AutoRoot** argv;
    uint argc;
    bool executed;
};
JSBool operationCallback(JSContext* cx);
JSBool contextCallback(JSContext* cx, uint contextOp);
// gcCallback(JSContext* cx, JSGCStatus status);
void reportError(JSContext* cx, const char* message, JSErrorReport* report);
bool ExecScriptEvent(Event* evt, bool clearList);
// A script must end when stopped, or when it is an in-game script and its game is over. The operation callback,
// delay() and SetSkill's event loop all stop running a script's events on this test.
bool ScriptMustEnd(Script* script);
void CALLBACK EventTimerProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue);
#endif
