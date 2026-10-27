#pragma once

#include "Offset.h"
#include "D2Intercepts.h"
#include "D2Handlers.h"
#include "D2BS.h"

PatchHook Patches[] = {
    {PatchCall, GetDllOffset("D2Client.dll", 0x7C89D), (DWORD)GameInput_Intercept, 5},          // Updated 1.14d //0047C89D-BASE
    {PatchJmp, GetDllOffset("D2Client.dll", 0x53B30), (DWORD)GameDraw_Intercept, 5},            // Updated 1.14d //00453B30-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x5F802), (DWORD)GamePacketReceived_Intercept, 5}, // Updated 1.14d //0045F802-BASE
    {PatchJmp, GetDllOffset("D2Client.dll", 0x12AE5A), (DWORD)GamePacketSent_Interception, 5},  // Updated 1.14d //0052AE5A-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x62D72), (DWORD)GetSelectedUnit_Intercept, 5},    // Updated 1.14d //00462D72-BASE
    {PatchJmp, GetDllOffset("D2Client.dll", 0x5E4D5), (DWORD)PlayerAssignment_Intercept, 5},    // Updated 1.14d //0045E4D5-BASE
    {PatchBytes, GetDllOffset("D2Client.dll", 0x68770), (DWORD)0xc3, 1},                        // Updated 1.14d //00468770-BASE
    {PatchBytes, GetDllOffset("D2CLIENT.DLL", 0x4EBFB), (BYTE)0x75, 1},                         // Congrats Screen //0044EBFB-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x4EBEF), (DWORD)CongratsScreen_Intercept, 5},     // Updated 1.14d //0044EBEF-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x547B6), (DWORD)GameActChange_Intercept, 5},      // Updated 1.14d //004547B6-BASE
    {PatchJmp, GetDllOffset("D2Client.dll", 0x567EB), (DWORD)GameActChange2_Intercept, 5},      // Updated 1.14d //004567EB-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x4F57C), (DWORD)GameLeave_Intercept, 5},          // Updated 1.14d //0044F57C-BASE
    {PatchCall, GetDllOffset("D2Client.dll", 0x61704), (DWORD)GameAttack_Intercept, 5},         // Updated 1.14d //00461704-BASE
    //	{PatchCall,	GetDllOffset("D2Client.dll", 0xA7364),	(DWORD)AddUnit_Intercept,				5},
    //	{PatchCall,	GetDllOffset("D2Client.dll", 0xA6F25),	(DWORD)RemoveUnit_Interept,			9},

    // this patch needs a constant in ChatPacketRecv_Interception updated also
    {PatchCall, GetDllOffset("BNCLIENT.DLL", 0x121B20), (DWORD)ChatPacketRecv_Interception, 7}, // Updated 1.14d //00521B20-BASE
    // Match the incoming Battle.net filter in the June 2020 1.6.4U DLL for a controlled comparison.
    {PatchCall, GetDllOffset("BNCLIENT.DLL", 0x11C7AF), (DWORD)BnetPacketRecv_Interception, 5}, // 1.14d //0051C7AF-BASE

    {PatchCall, GetDllOffset("D2Multi.dll", 0x48813), (DWORD)Whisper_Intercept, 7},      // Updated 1.14d //00448813-BASE
    {PatchCall, GetDllOffset("D2Multi.dll", 0x42A61), (DWORD)ChannelInput_Intercept, 5}, // Updated 1.14d //00442A61-BASE

    //{PatchCall,	GetDllOffset("D2Multi.dll", 0x14A9A),	(DWORD)ChannelWhisper_Intercept,		5},//1.13d //not used
    //{PatchJmp,	GetDllOffset("D2Multi.dll", 0x14BE0),	(DWORD)ChannelChat_Intercept,			6},//1.13d
    //{PatchJmp,	GetDllOffset("D2Multi.dll", 0x14850),	(DWORD)ChannelEmote_Intercept,			6},//1.13d

    {PatchCall, GetDllOffset("D2Win.dll", 0xF9A0D), (DWORD)GameDrawOOG_Intercept, 5}, // Updated 1.14d //004F9A0D-BASE

    {PatchCall, GetDllOffset("D2CMP.dll", 0x2091E5), (DWORD)GameCrashFix_Intercept, 10},      // 1.14d //006091E5-BASE
    {PatchJmp, GetDllOffset("D2Common.dll", 0x270783), (DWORD)LvlSubPlaceLoop_Intercept, 17}, // NEW 1.14d //00670783-BASE
    {PatchJmp, GetDllOffset("D2Common.dll", 0x270724), (DWORD)LvlSubPickLoop_Intercept, 10},  // NEW 1.14d //00670724-BASE

    // The sprite cache, shared by Storm's read thread and the threads that ask for sprites: see SpriteCacheChargeRead_Intercept.
    {PatchCall, GetDllOffset("D2CMP.dll", 0x1FF6EF), (DWORD)SpriteCachePinRead_Intercept, 10},    // NEW 1.14d //005FF6EF-BASE
    {PatchBytes, GetDllOffset("D2CMP.dll", 0x1FF726), (BYTE)0x90, 4},                             // NEW 1.14d //005FF726-BASE
    {PatchCall, GetDllOffset("D2CMP.dll", 0x1FF73A), (DWORD)SpriteCacheChargeRead_Intercept, 5},  // NEW 1.14d //005FF73A-BASE
    {PatchCall, GetDllOffset("D2CMP.dll", 0x20002E), (DWORD)SpriteCacheChargeIssue_Intercept, 5}, // NEW 1.14d //0060002E-BASE
    {PatchCall, GetDllOffset("D2CMP.dll", 0x1FFFB2), (DWORD)SpriteCacheAllocRead_Intercept, 5},   // NEW 1.14d //005FFFB2-BASE
    {PatchJmp, GetDllOffset("D2CMP.dll", 0x1FF4A0), (DWORD)SpriteCacheEvict_Intercept, 5},        // NEW 1.14d //005FF4A0-BASE
    // The cache's unlink (0x5FF1B0) leaves a pinned list head in place, yet touching a sprite (0x5FF5B0) appends its
    // entry to the touched list regardless, linking it twice; a sprite being decoded is pinned, and can be its list's head.
    // Without the check a pinned head unlinks like any other pinned entry; eviction skips pinned entries on its own.
    {PatchBytes, GetDllOffset("D2CMP.dll", 0x1FF1EB), (BYTE)0x90, 10}, // NEW 1.14d //005FF1EB-BASE
    {PatchBytes, GetDllOffset("D2CMP.dll", 0x1FF238), (BYTE)0x90, 6},  // NEW 1.14d //005FF238-BASE

    {PatchCall, GetDllOffset("D2MCPClient.dll", 0x4B1AE), (DWORD)RealmPacketRecv_Interception, 6},

    {PatchCall, GetDllOffset("Fog.dll", 0x8CC4), (DWORD)LogMessageBoxA_Intercept, 6}, // Updated 1.14d //00408CC4-BASE
    {PatchJmp, GetDllOffset("Fog.dll", 0x82E0), (DWORD)FogCrash_Intercept, 6},        // NEW 1.14d //004082E0-BASE
    //{PatchJmp,  GetDllOffset("Fog.dll", 0x8900),		(DWORD)FogException/*D2GAME_exit0*/,		6}, //NEW 1.14d //00408900-BASE
    {PatchJmp, GetDllOffset("Fog.dll", 0x1790), (DWORD)ErrorReportLaunch, 6}, // NEW 1.14d //00401790-BASE

    // Storm's read thread paces background reads to 256 KB/s, and leaving a game cancels every queued sprite read one
    // by one, each waiting for the thread's next paced pass: hundreds queued froze the exit for tens of seconds.
    // je -> jmp skips the pacing, as Storm's own rate 0 does; an empty queue still waits 250 ms.
    {PatchBytes, GetDllOffset("Storm.dll", 0x1A588), (BYTE)0xEB, 1}, // NEW 1.14d //0041A588-BASE
};

// Fixes to the game's own bytes that stay when D2BS unloads: restoring them under a running game would leave the server
// step's last run time on a clock the original step can't catch up with.
PatchHook Permanent[] = {
    // The single-player server step (0x52FC20) masks timeGetTime to 31 bits, then runs a game frame once now - last
    // reaches 40 ms by a signed compare: past each multiple of 2^31 ms of Windows uptime (24.8 days), now stays below
    // last and the game never steps again. An all-ones mask keeps the full clock. jge -> jae lets a last taken on the
    // masked clock, as when D2BS is injected into a game in progress, cost one frame and a resync instead.
    {PatchBytes, GetDllOffset("D2Game.dll", 0x12FC4B), (BYTE)0xFF, 1}, // NEW 1.14d //0052FC4B-BASE
    {PatchBytes, GetDllOffset("D2Game.dll", 0x12FC64), (BYTE)0x73, 1}, // NEW 1.14d //0052FC64-BASE
};

PatchHook Conditional[] = {
    {PatchJmp, GetDllOffset("BNCLIENT.DLL", 0x12366C), (DWORD)ClassicSTUB, 5, &Vars.bUseRawCDKey}, // Updated 1.14d //0052366C-BASE
    {PatchJmp, GetDllOffset("BNCLIENT.DLL", 0x123958), (DWORD)LodSTUB, 5, &Vars.bUseRawCDKey},     // Updated 1.14d //00523958-BASE
    {PatchCall, GetDllOffset("D2CLIENT.DLL", 0x4EF28), (DWORD)FailToJoin, 6, &Vars.bReduceFTJ},    // FTJ Reducer
    {PatchCall, GetDllOffset("D2Gfx.DLL", 0xF5623), (DWORD)Multi, 6, &Vars.bMulti},
    {PatchCall, GetDllOffset("D2Gfx.DLL", 0xF5831), (DWORD)Windowname, 6, &Vars.bMulti},
    {PatchCall, GetDllOffset("BNCLIENT.DLL", 0x11944E), (DWORD)CacheFix, 6, &Vars.bCacheFix},
    {PatchCall, GetDllOffset("BNCLIENT.DLL", 0x119434), (DWORD)CacheFix, 6, &Vars.bCacheFix},
    {PatchBytes, GetDllOffset("D2CLIENT.DLL", 0x51C31), (WORD)0x9090, 2, &Vars.bSleepy}, // Sleep
    {PatchBytes, GetDllOffset("D2Win.DLL", 0xFA66F), (BYTE)0xEB, 1, &Vars.bSleepy}       // OOG Sleep
};