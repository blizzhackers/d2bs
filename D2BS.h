#ifndef __D2BS_H__
#define __D2BS_H__

#pragma once

#ifndef XP_WIN
#define XP_WIN
#endif

// build\Build-D2BS.ps1 defines the version from VERSION and the commit (d2bs_build.h); builds without it say so.
#ifndef D2BS_VERSION
#define D2BS_VERSION L"local build"
#endif
#ifndef D2BS_SHORT_VERSION
#define D2BS_SHORT_VERSION D2BS_VERSION
#endif

#include <windows.h>
#include <vector>
#include <queue>

#include "ScreenHook.h"

// struct Variables;

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

#define PRIVATE_UNIT 1
#define PRIVATE_ITEM 3
// Bit 0 clear: natives that read any object's private data and test it against PRIVATE_UNIT turn a Party or Room away.
#define PRIVATE_PARTY 4
#define PRIVATE_ROOM 6

struct Private {
    DWORD dwPrivateType;
};

struct Module {
    union {
        HMODULE hModule;
        DWORD dwBaseAddress;
    };
    DWORD _1;
    wchar_t szPath[MAX_PATH];
};

struct Variables {
    int nChickenHP;
    int nChickenMP;
    DWORD dwInjectTime;
    DWORD dwGameTime;
    BOOL bDontCatchNextMsg;
    BOOL bClickAction;
    BOOL bNeedShutdown;
    BOOL bUseGamePrint;
    BOOL bShutdownFromDllMain;
    BOOL bChangedAct;
    BOOL bGameLoopEntered;
    DWORD dwGameThreadId;
    DWORD dwLocale;

    DWORD dwMaxGameTime;
    DWORD dwGameTimeout;
    BOOL bTakeScreenshot;
    BOOL bQuitOnError;
    BOOL bQuitOnHostile;
    BOOL bStartAtMenu;
    BOOL bActive;
    BOOL bBlockKeys;
    BOOL bBlockMouse;
    BOOL bDisableCache;
    BOOL bUseProfileScript;
    BOOL bLoadedWithCGuard;
    BOOL bLogConsole;
    BOOL bEnableUnsupported;
    BOOL bForwardMessageBox;
    BOOL bHaltDumps;
    DWORD dwHangReportMs; // 0 = no hang reports
    BOOL bUseRawCDKey;
    BOOL bQuitting;
    BOOL bCacheFix;
    BOOL bMulti;
    BOOL bSleepy;
    BOOL bReduceFTJ;
    int dwMemUsage;
    int dwConsoleFont;
    Module* pModule;
    HMODULE hModule;
    HWND hHandle;

    wchar_t szPath[_MAX_PATH];
    wchar_t szLogPath[_MAX_PATH];
    wchar_t szScriptPath[_MAX_PATH];
    wchar_t szProfile[256];
    wchar_t szStarter[_MAX_FNAME];
    wchar_t szConsole[_MAX_FNAME];
    wchar_t szDefault[_MAX_FNAME];
    char szHosts[256];
    char szClassic[30];
    char szLod[30];
    wchar_t szTitle[256];
    wchar_t szCommandLine[256];

    WNDPROC oldWNDPROC;
    HHOOK hMouseHook;
    HHOOK hKeybHook;

    UINT_PTR uTimer;
    long SectionCount;
    // Odd from a game's first getParty() or Room until the game is left, even otherwise, and a new value for every game,
    // so a Party or Room object never finds its player or room in a later game. dwMenuSession is its value as the menu
    // last showed it: a game
    // opens only while the two are equal. Both are read and written only with cGameLoopSection held (JSParty.cpp).
    DWORD dwGameSession;
    DWORD dwMenuSession;

    std::queue<std::wstring> qPrintBuffer;
    std::map<unsigned __int32, CellFile*> mCachedCellFiles;
    std::vector<std::pair<DWORD, DWORD>> vUnitList;
    // std::list<Event*> EventList;
    CRITICAL_SECTION cEventSection;
    CRITICAL_SECTION cRoomSection;
    CRITICAL_SECTION cMiscSection;
    CRITICAL_SECTION cScreenhookSection;
    CRITICAL_SECTION cPrintSection;
    CRITICAL_SECTION cTextHookSection;
    CRITICAL_SECTION cImageHookSection;
    CRITICAL_SECTION cBoxHookSection;
    CRITICAL_SECTION cFrameHookSection;
    CRITICAL_SECTION cLineHookSection;
    CRITICAL_SECTION cFlushCacheSection;
    CRITICAL_SECTION cConsoleSection;
    CRITICAL_SECTION cGameLoopSection;
    CRITICAL_SECTION cUnitListSection;
    CRITICAL_SECTION cFileSection;

    DWORD dwSelectedUnitId;
    DWORD dwSelectedUnitType;
    POINT pMouseCoords;
};

extern Variables Vars;

BOOL Startup(void);
void Shutdown(void);

#endif
