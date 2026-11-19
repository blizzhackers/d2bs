#pragma once

#include <windows.h>

// The hang report: a watchdog thread that, when the game window stops responding, writes what every thread is doing
// to the d2bs log and the last packets to the logs folder. It only reads; the game keeps running untouched.

enum HangStopMode { HangStopJoin, HangStopUnload, HangStopProcessExit };

// Startup's last step: creates the watchdog unless HangReportSeconds is 0. It arms once the engine has started and the
// game thread has ticked.
void HangStart(void);
// Shutdown's and DllMain's first step; ProcessExit only sets flags, as Windows has already ended every other thread.
void HangStop(HangStopMode mode);
// Game thread only, once per timer tick.
void HangTick(HWND hwnd);
// D2Thread's startup is done and the profile is known, so the watch's lines reach the profile's log: it arms now.
void HangEngineStarted(void);
// A crash or halt is being reported: no hang report starts or continues after this.
void HangQuiesce(void);
// Names the calling thread in hang reports; the name is copied.
void HangNameThread(const wchar_t* name);
void HangForgetThread(void);
