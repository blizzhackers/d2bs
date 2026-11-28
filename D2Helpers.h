#ifndef D2HELPERS_H
#define D2HELPERS_H

#include "D2Structs.h"
#include "D2Ptrs.h"
#include "js32.h"

#define DEBUG_LOG(MESSAGE) Log(L"%s (%s, %d)", MESSAGE, __FILE__, __LINE__)

enum DistanceType { Euclidean, Chebyshev, Manhattan };

enum ClientGameState { ClientStateNull, ClientStateMenu, ClientStateInGame, ClientStateBusy };

void Log(wchar_t* szFormat, ...);
void LogNoFormat(const wchar_t* szString);

ClientGameState ClientState(void);
bool GameReady(void);
bool WaitForGameReady(void);

// The wait of one native call that reads the game under the game lock. Ready() waits like WaitForGameReady, with one
// GameReadyTimeout deadline for the whole call. It reads readiness without the lock, so the game can turn busy before
// the lock is taken: a lookup that finds nothing asks RetryIfBusy() while it still holds the lock, and on true releases
// it and calls Ready() again. Construct it before taking the lock: it records whether this thread may wait, which reads
// false for every thread under the lock.
class GameWait {
  public:
    GameWait();
    // False out of game, when the deadline passes, or at once on a busy game for a thread running blocking listeners
    // or holding the game lock. Once it has returned true it returns true at once until RetryIfBusy() does, so a
    // native's first lookup runs on the state it finds, as a WaitForGameReady caller's does.
    bool Ready(void);
    // Under the game lock, after a lookup found nothing: true when the game is busy there and this call may wait.
    bool RetryIfBusy(void);

  private:
    bool mayWait;
    bool ready;
    bool retry;
    DWORD start;
};

// The player's area, 0 when the game is not ready; the caller holds the game lock, which keeps the room links live.
DWORD GetPlayerAreaLocked(void);
bool PlayerActLoaded(void);

// DllMain allocates the per-thread blocking-listener slot before any thread starts and releases it at detach.
bool AllocBlockingListenerSlot(void);
void FreeBlockingListenerSlot(void);

// An active scope (a blocker event) marks the current thread, while it lives, as running that event's listeners, so
// WaitForGameReady and GameWait fail at once on a busy game, also after the event's sender stopped waiting; the
// scope restores the previous mark when it ends.
class BlockingListenerScope {
  public:
    explicit BlockingListenerScope(bool active);
    ~BlockingListenerScope();

  private:
    bool previous;
    BlockingListenerScope(const BlockingListenerScope&);
    BlockingListenerScope& operator=(const BlockingListenerScope&);
};

void SendMouseClick(int x, int y, int clicktype);
void SendKeyPress(uint type, uint key, uint ext);
POINT CalculateTextLen(const char* szwText, int Font);
POINT CalculateTextLen(const wchar_t* szwText, int Font);
void WorldToScreen(POINT* pPos);
void ScreenToWorld(POINT* ptPos);
POINT ScreenToAutomap(int x, int y);
void AutomapToScreen(POINT* pPos);
// The level of the loaded act with that number, its rooms built; NULL when it is not in that act, the game is not
// ready, or its rooms could not be built. The pointer is live only while the caller holds the game lock from before
// the call until its last use, since the game thread frees an act's levels under that lock.
Level* GetLevel(DWORD dwLevelNo);
class AutoCriticalRoom;
// GetLevel after wait.Ready(), again after each miss while the game is busy under the lock. A level found comes with
// the game lock, taken before the lookup, held in cRoom until the caller deletes it; NULL comes with cRoom NULL.
Level* WaitForLevel(DWORD dwLevelNo, GameWait& wait, AutoCriticalRoom*& cRoom);
void D2CLIENT_Interact(UnitAny* pUnit, DWORD dwMoveType);
void myDrawText(const wchar_t* szwText, int x, int y, int color, int font);
void myDrawCenterText(const wchar_t* szwText, int x, int y, int color, int font, int div);
// UnitAny* FindItemByPosition(DWORD x, DWORD y, DWORD Location);
BYTE CalcPercent(DWORD dwVal, DWORD dwMaxVal, BYTE iMin = NULL);
DWORD GetTileLevelNo(Room2* lpRoom2, DWORD dwTileNo);
// The caller holds the game lock.
UnitAny* GetMercUnit(UnitAny* pUnit);

BOOL ClickNPCMenu(DWORD NPCClassId, DWORD MenuId);
int GetItemLocation(UnitAny* pItem);
// void	SelectInventoryItem(DWORD x, DWORD y, DWORD dwLocation);

int GetSkill(WORD wSkillId);
BOOL SetSkill(JSContext* cx, WORD wSkillId, BOOL bLeft, DWORD dwItemId = 0xFFFFFFFF);
char* GetSkillByID(WORD id);
WORD GetSkillByName(char* szSkillName);

const char* GetUnitName(UnitAny* pUnit, char* szBuf, size_t bufSize);
void GetItemCode(UnitAny* pUnit, char* szBuf);

InventoryLayout* GetLayoutFromTable(DWORD dwTable);
// The unit tables a unit of that type lives in for scripts: getUnit walks them and D2CLIENT_FindUnit looks up there.
UnitHashTable* UnitTablesFor(DWORD dwType);
UnitAny* D2CLIENT_FindUnit(DWORD dwId, DWORD dwType);
UnitAny* FindMissileOwner(UnitAny* pMissile);

POINT GetScreenSize();
int D2GetScreenSizeX();
int D2GetScreenSizeY();

CellFile* LoadCellFile(const wchar_t* lpszPath, DWORD bMPQ = 3);

AutomapLayer* InitAutomapLayer(DWORD levelno);
DWORD __fastcall D2CLIENT_InitAutomapLayer_STUB(DWORD nLayerNo);
void myDrawAutomapCell(CellFile* cellfile, int xpos, int ypos, BYTE col);
DWORD ReadFile(HANDLE hFile, void* buf, DWORD len);
void* memcpy2(void* dest, const void* src, size_t count);
HANDLE OpenFileRead(const char* filename);
HANDLE OpenFileRead(const wchar_t* filename);
BYTE* AllocReadFile(const wchar_t* filename, DWORD* pSize);
CellFile* LoadBmpCellFile(BYTE* buf1, int width, int height);
CellFile* LoadBmpCellFile(const wchar_t* filename);
CellFile* myInitCellFile(CellFile* cf);

// D2COMMON_AddRoomData for pRoom2, which keeps its preset list free of the copies the game's room build adds again to a
// list that outlived the room's last unload. Call it holding the game lock.
void LoadRoomData(Act* pAct, Room2* pRoom2, Room1* pNear);

static inline UnitAny* GetPlayerUnit() {
    return D2CLIENT_GetPlayerUnit();
}
static inline void AddRoomData(Room2* room) {
    LoadRoomData(room->pLevel->pMisc->pAct, room, room->pRoom1);
}
static inline void RemoveRoomData(Room2* room) {
    D2COMMON_RemoveRoomData(room->pLevel->pMisc->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, room->pRoom1);
}
static inline char* __stdcall GetLevelName(const Level* level) {
    return D2COMMON_GetLevelText(level->dwLevelNo)->szName;
}
static inline char* __stdcall GetLevelIdName(DWORD level) {
    return D2COMMON_GetLevelText(level)->szName;
}

DWORD __fastcall D2CLIENT_GetUnitName_STUB(DWORD UnitAny);
DWORD __fastcall D2CLIENT_GetUIVar_STUB(DWORD varno);
DWORD __fastcall D2CLIENT_LoadUIImage_ASM(char* Path);
void __fastcall D2CLIENT_SetSelectedUnit_STUB(DWORD UnitAny);
void __fastcall D2CLIENT_Interact_ASM(DWORD Struct);
DWORD __fastcall D2CLIENT_ClickParty_ASM(DWORD RosterUnit, DWORD Mode);
void __fastcall D2CLIENT_ClickShopItem_ASM(DWORD x, DWORD y, DWORD BuyOrSell);
void __fastcall D2CLIENT_ShopAction_ASM(DWORD pTable, DWORD pItem, DWORD pNPC, DWORD _1, DWORD pTable2 /* Could be also the ItemCost?*/, DWORD dwMode, DWORD _2,
                                        DWORD _3);
void __fastcall D2CLIENT_ClickItemRight_ASM(DWORD x, DWORD y, DWORD Location, DWORD pItem, DWORD pItemPath);
void __fastcall D2CLIENT_ClickBelt(DWORD x, DWORD y, Inventory* pInventoryData);
void __fastcall D2CLIENT_ClickBeltRight_ASM(DWORD pInventory, DWORD pPlayer, DWORD HoldShift, DWORD dwPotPos);
void __fastcall D2CLIENT_GetItemDesc_ASM(DWORD pUnit, wchar_t* pBuffer);
void __fastcall D2COMMON_DisplayOverheadMsg_ASM(DWORD pUnit);
void __fastcall D2CLIENT_MercItemAction_ASM(DWORD bPacketType, DWORD dwSlot);
DWORD __fastcall D2CLIENT_LoadUIImage_ASM(char* lpszPath);
void __fastcall D2CLIENT_PlaySound(DWORD dwSoundId);
// DWORD __fastcall D2CLIENT_TestPvpFlag_STUB(DWORD planum1, DWORD planum2, DWORD flagmask);
void __fastcall D2GFX_DrawRectFrame_STUB(RECT* rect);
DWORD __cdecl D2CLIENT_GetMinionCount(UnitAny* pUnit, DWORD dwType);
void __fastcall D2CLIENT_HostilePartyUnit(RosterUnit* pUnit, DWORD dwButton);
void __stdcall D2CLIENT_TakeWaypoint(DWORD dwWaypointId, DWORD dwArea);
DWORD __fastcall D2CLIENT_SendGamePacket_ASM(DWORD dwLen, BYTE* bPacket);
void __stdcall D2CLIENT_LeftClickItem(DWORD Location, UnitAny* pPlayer, Inventory* pInventory, int x, int y, DWORD dwClickType, InventoryLayout* pLayout);

double GetDistance(long x1, long y1, long x2, long y2, DistanceType type = Euclidean);
bool IsScrollingText();
void ReadProcessBYTES(HANDLE hProcess, DWORD lpAddress, void* buf, int len);
#endif
