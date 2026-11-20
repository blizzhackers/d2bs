#pragma once

#include <windows.h>

// The hang report: a watchdog thread that, when the game window stops responding, writes what the game thread (and,
// for the first hangs of a process, the other threads) is doing to the d2bs log and the last packets to the logs
// folder. It changes no game state: it pauses one thread at a time for the moment its registers and stack are copied, and
// the whole process only for the instant a dump's snapshot is taken.

enum HangStopMode { HangStopJoin, HangStopUnload, HangStopProcessExit };

// Startup's last step: creates the watchdog unless HangReportSeconds is 0. It arms once the engine has started and the
// game thread has ticked.
void HangStart(void);
// Shutdown's and DllMain's first step; ProcessExit only sets flags, as Windows has already ended every other thread.
void HangStop(HangStopMode mode);
// Game thread only, once per timer tick.
void HangTick(HWND hwnd);
// D2Thread's startup is done (the engine is ready): the watch arms at its next poll after the game thread has ticked. Its
// lines go to the profile's log once a profile is set, and to d2bs--<date>.log before that.
void HangEngineStarted(void);
// A crash or halt is being reported. The watch checks this at each step of its work: a report, follow-up, packet log or
// dump not yet started is skipped, a report under way stops at its next step, and a hang dump being written cancels
// itself at dbghelp's next cancel check.
void HangQuiesce(void);
// Names the calling thread in hang reports; the name is copied.
void HangNameThread(const wchar_t* name);
void HangForgetThread(void);
