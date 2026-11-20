#ifndef D2INTERCEPTS_H
#define D2INTERCEPTS_H

#include "D2Structs.h"

void RealmPacketRecv_Interception();
void GameDraw_Intercept(void);
void GameInput_Intercept();
void GamePacketReceived_Intercept();
void GamePacketSent_Interception();
UnitAny* GetSelectedUnit_Intercept(void);
void Whisper_Intercept();
void GameAttack_Intercept();
void PlayerAssignment_Intercept();
void GameCrashFix_Intercept();
void LvlSubPlaceLoop_Intercept();
void LvlSubPickLoop_Intercept();
void SpriteCachePinRead_Intercept();
void __fastcall SpriteCacheChargeRead_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD dwSize, DWORD);
SpriteCacheEntry* __fastcall SpriteCacheEvict_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD bUntouchedFirst);
void __fastcall SpriteCacheChargeIssue_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD dwSize, DWORD bUntouchedFirst);
SpriteCacheEntry* __fastcall SpriteCacheAllocRead_Intercept(SpriteCache* pCache, DWORD dwSize, DWORD dwPayload, DWORD bUntouched);
void StormStreamUpdate_Intercept();
void GameDrawOOG_Intercept(void);
void CongratsScreen_Intercept(void);
void GameActChange_Intercept(void);
void GameActChange2_Intercept(void);
void EndNpcSpeech_Intercept();
void FreeSpeechPanel_Intercept();
void FreeNpcMessageList_Intercept();
void GameLeave_Intercept(void);
void ChannelInput_Intercept(void);
void ChatPacketRecv_Interception();
void BnetPacketRecv_Interception();
DWORD WINAPI RevisionTempPath_Intercept(DWORD bufferLength, LPSTR buffer);
void AddUnit_Intercept(UnitAny* lpUnit);
void RemoveUnit_Intercept(UnitAny* lpUnit);
int WINAPI LogMessageBoxA_Intercept(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
DWORD __fastcall FogCrash_Intercept(BYTE* crashInfo, DWORD);
// The logs-folder file stem of a crash or hang report: <logs><prefix><profile>-<time>-<pid>.
bool CrashReportStem(wchar_t* stem, size_t len, const wchar_t* prefix);
// Deletes the oldest logs-folder files matching pattern until at most keep remain, passing over up to 8 it can't delete.
void PruneCrashFiles(const wchar_t* pattern, DWORD keep);
// The free space a crash or hang dump needs in the logs folder before it is written: one value, so the two agree.
const ULONGLONG kDumpMinFreeBytes = 2ULL << 30;

#endif
