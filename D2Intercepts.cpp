#include "D2Handlers.h"
#include "D2Ptrs.h"
#include "D2BS.h"
#include "Helpers.h"
#include <shlwapi.h>
#include "RevisionTempPath.h"

DWORD WINAPI RevisionTempPath_Intercept(DWORD bufferLength, LPSTR buffer) {
    DWORD length = GetRevisionTempPath(bufferLength, buffer);
    if (!length) {
        DWORD error = GetLastError();
        Log(L"Battle.net revision temp directory unavailable (Windows error %u)", error);
        SetLastError(error);
    }
    return length;
}

// Preserve the legacy DLL's full ECX comparison, including its treatment of the upper bits.
// The payload size is passed on the stack; fastcall removes that extra argument on return.
static __declspec(noinline) DWORD __fastcall BnetPacketAllowed(DWORD command, BYTE*, DWORD) {
    return command != 0x4C;
}

// Reproduces the June 2020 DLL's native filter at Game.exe + 0x11C7AF. The original caller
// supplies ECX (command), EDX (payload) and one stack argument (size). Command 0x4C is
// dispatched as 0; all other commands reach the original dispatcher with their registers intact.
void __declspec(naked) BnetPacketRecv_Interception() {
    __asm {
        pushad
        push eax
        call BnetPacketAllowed
        cmp eax, 0
        popad
        jne Dispatch
        xor ecx, ecx
        mov eax, [esp + 4]
    Dispatch:
        jmp BNCLIENT_DispatchPacket_I
        ret
    }
}

void __declspec(naked) RealmPacketRecv_Interception() {
    __asm {
		test esi, esi
		jz block
		pushad

		call RealmPacketRecv
		cmp eax, 0

		popad
		je Block
		call esi
Block:
		ret
    }
}

void __declspec(naked) GamePacketReceived_Intercept() {
    __asm {
		pushad;

		call GamePacketReceived;
		test eax, eax;

		popad;
		jnz OldCode;

		mov edx, 0;

	OldCode:
		mov eax, [esp + 4];
		push eax;
		call D2NET_ReceivePacket_I;

		ret 4;
    }
}

void __declspec(naked) GamePacketSent_Interception() {
    __asm {
		pushf
		pushad;

		mov ecx, [esp + 0x22 + 4 + 0xC];
		mov edx, [esp + 0x22 + 4 + 0x4];
		call GamePacketSent;
		test eax, eax;

		popad;
		jnz OldCode;

		mov[esp + 0x4 + 4 + 2], 0;

	OldCode:
		popf
			jnz good
			xor eax, eax
			pop ebp
			ret 0Ch
			good :
		jmp D2CLIENT_SendPacket_II
    }
}

void GameDraw_Intercept(void) {
    GameDraw();
}

void __declspec(naked) GameInput_Intercept() {
    __asm {
		pushad
		mov ecx, ebx
		call GameInput
		cmp eax, -1
		popad
		je BlockIt
		call D2CLIENT_InputCall_I
		ret

BlockIt:
		xor eax,eax
		ret
    }
}

UnitAny* GetSelectedUnit_Intercept(void) {
    if (Vars.bClickAction) {
        if (Vars.dwSelectedUnitId) {
            UnitAny* pUnit = D2CLIENT_FindUnit(Vars.dwSelectedUnitId, Vars.dwSelectedUnitType);

            return pUnit;
        }

        return NULL;
    }

    return D2CLIENT_GetSelectedUnit();
}

void __declspec(naked) Whisper_Intercept() {
    __asm
    {
		mov [esp-674h-4+4], edi
		pop edi
		sub esp, 678h
		pushad
		mov ecx, ebx
		mov edx, [ebp+8]
		call WhisperHandler
		popad
          // jmp D2MULTI_WhisperIntercept_Jump
		jmp edi
    }
}

VOID __declspec(naked) ChatPacketRecv_Interception() {
    __asm {
		mov edx, [ebp + 8]
		mov ecx, ebx
		pushad
		sub ecx, 4
		add edx, 4

		call ChatPacketRecv
		test eax, eax
		popad
 
		je Block
		call edi
Block:
		ret
    }
}

void __declspec(naked) GameAttack_Intercept() {
    __asm {
		pushad
		mov ecx, esi // attack struct
		call GameAttack
		cmp eax, -1
		popad
		je OldCode

		call D2CLIENT_GetSelectedUnit
		
		cmp eax, 0
		je OldCode

		mov [esp+0x08+0x4+0x4], 1 // bool unit

OldCode:
		mov eax, [p_D2CLIENT_MouseY]
		mov eax, [eax]
		retn
    }
}

void __declspec(naked) PlayerAssignment_Intercept() {
    __asm
    {
		FNOP
		CALL D2CLIENT_AssignPlayer_I
		MOV ECX, EAX
		CALL GamePlayerAssignment
		RETN
    }
}

void __declspec(naked) GameCrashFix_Intercept() {
    __asm {
		CMP ECX, 0
		JE Skip
		MOV DWORD PTR DS:[ECX+0x10],EDX
Skip:
		MOV DWORD PTR DS:[EAX+0xC],0
		RETN
    }
}

// The outdoor level generators walk the LvlSub rows of one type until a row of another type, with no end check:
// 0x670750 places a type's rows, 0x6706A0 rolls which of them a level gets. A walk of the table's last type, 12 (Act
// 5's barricades, or a mod's level SubType 12), reads the uninitialized dword past the table, a new leftover each game
// (the table is reloaded per game); a 12 there makes a garbage row, which 0x670750 hands to the DS1 loader and crashes
// on. These loop tails stop at the end of the table and walk the rows inside it as before.
void __declspec(naked) LvlSubPlaceLoop_Intercept() {
    __asm {
		add ebx, 0x15C // the next row
		mov ecx, [p_D2COMMON_LvlSubTxtCount]
		mov eax, [ecx]
		imul eax, eax, 0x15C
		mov ecx, [p_D2COMMON_LvlSubTxt]
		add eax, [ecx]
		cmp ebx, eax
		jae Done
		mov eax, [ebx]
		cmp eax, [esi+0x18] // the type being walked
		jne Done
		jmp D2COMMON_LvlSubPlaceLoop_I
Done:
		jmp D2COMMON_LvlSubPlaceLoopDone_I
    }
}

void __declspec(naked) LvlSubPickLoop_Intercept() {
    __asm {
		add ebx, 0x15C // the next row
		mov ecx, [p_D2COMMON_LvlSubTxtCount]
		mov eax, [ecx]
		imul eax, eax, 0x15C
		mov ecx, [p_D2COMMON_LvlSubTxt]
		add eax, [ecx]
		cmp ebx, eax
		jae Done
		cmp [ebx], edx // edx = the type being walked
		jne Done
		jmp D2COMMON_LvlSubPickLoop_I
Done:
		jmp D2COMMON_LvlSubPickLoopDone_I
    }
}

// The sprite cache is shared by Storm's read thread and the threads that ask for sprites. The game's eviction (0x5FF4A0)
// picks and unlinks its victim without the cache's lock, and a read thread that evicts can release an entry another
// thread is releasing (closing its file request twice halts, 657) or free a sprite the game thread is using. So the
// read thread only charges its read and pins the sprite it decodes, both under the lock; only the threads that ask for
// sprites evict, and eviction does its list work under the lock.
void __fastcall SpriteCacheChargeRead_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD dwSize, DWORD) {
    EnterCriticalSection(&pCache->Lock);
    pEntry->dwSize += dwSize;
    pCache->dwBytes += dwSize;
    pEntry->dwFlags &= ~2; // the decode's pin; the game's own clear (0x5FF726) runs without the lock and is patched out
    LeaveCriticalSection(&pCache->Lock);
}

// The decode's pin (flag 2) shares its word with the list flag the game thread rewrites, under the lock, when it uses a
// sprite; set without the lock, either write could undo the other and leave an entry flagged for the wrong list.
static void __fastcall SpriteCachePinForRead(SpriteCacheEntry* pEntry) {
    SpriteCache* pCache = p_D2CMP_SpriteCache;
    EnterCriticalSection(&pCache->Lock);
    pEntry->dwFlags |= 2;
    LeaveCriticalSection(&pCache->Lock);
}

void __declspec(naked) SpriteCachePinRead_Intercept() {
    __asm {
		mov edi, [ebx+0x94] // the sprite's cache entry
		push eax            // live at 0x5FF6F9, where ecx and edx are written before they are read
		mov ecx, edi
		call SpriteCachePinForRead
		pop eax
		add dword ptr [esp], 5 // return to 0x5FF6F9, an instruction start whether or not the patch is in place
		ret
    }
}

static SpriteCacheEntry* SpriteCachePopOldest(SpriteCache* pCache, DWORD bUntouched) {
    SpriteCacheEntry* pEntry;
    __asm {
		push bUntouched
		mov edx, pCache
		call D2CMP_SpriteCachePopOldest_I
		mov pEntry, eax
    }
    return pEntry;
}

static void SpriteCacheUnlink(SpriteCache* pCache, SpriteCacheEntry* pEntry) {
    __asm {
		mov eax, pEntry
		mov ecx, pCache
		call D2CMP_SpriteCacheUnlink_I
    }
}

// 0x5FF4A0 with the lock around its list work, never taking pExcept or, for a trim, the touched list's tail. Releasing
// stays outside the lock, as in the game: releasing a sprite whose read is still queued waits for the read thread, which
// takes the lock to charge a read.
static SpriteCacheEntry* SpriteCacheEvict(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD bUntouchedFirst, SpriteCacheEntry* pExcept, bool bKeepLastUsed) {
    EnterCriticalSection(&pCache->Lock);
    if (!pCache->dwCount)
        pEntry = NULL;
    else if (pEntry) {
        if (pEntry->dwFlags & 2)
            pEntry = NULL;
        else
            SpriteCacheUnlink(pCache, pEntry);
    } else {
        // the game's pick: the touched list's head, or the untouched one's when asked for and older
        DWORD bUntouched = 0;
        if (pCache->pUntouchedHead) {
            if (!pCache->pTouchedHead)
                bUntouched = bUntouchedFirst;
            else if (bUntouchedFirst)
                bUntouched = pCache->pUntouchedHead->dwTouched < pCache->pTouchedHead->dwTouched;
        }
        SpriteCacheEntry* pHead = bUntouched ? pCache->pUntouchedHead : pCache->pTouchedHead;
        SpriteCacheEntry* pKeep = bKeepLastUsed ? pCache->pTouchedTail : NULL;
        bool bCanGo = pHead && !(pHead->dwFlags & 2) && pHead != pExcept && pHead != pKeep;
        pEntry = bCanGo ? SpriteCachePopOldest(pCache, bUntouched) : NULL;
    }
    LeaveCriticalSection(&pCache->Lock);
    if (!pEntry)
        return NULL;

    if (pEntry->pPayload) {
        pCache->pfnRelease(pEntry->pPayload);
        pEntry->pPayload = NULL;
    }
    if (pEntry->pBuffer) {
        FOG_FreeMemory(pCache->pPool, pEntry->pBuffer, "D2BS", __LINE__, 0);
        pEntry->pBuffer = NULL;
    }

    EnterCriticalSection(&pCache->Lock);
    // the size leaves the total only after the release, which waits for a queued read: that read's charge is in it
    pCache->dwBytes -= pEntry->dwSize;
    if (pCache->pFreeTail) {
        pCache->pFreeTail->pNext = pEntry;
        pEntry->pPrev = pCache->pFreeTail;
    } else
        pCache->pFreeHead = pEntry;
    pCache->pFreeTail = pEntry;
    pCache->dwCount--;
    LeaveCriticalSection(&pCache->Lock);
    return pEntry;
}

SpriteCacheEntry* __fastcall SpriteCacheEvict_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD bUntouchedFirst) {
    return SpriteCacheEvict(pCache, pEntry, bUntouchedFirst, NULL, false);
}

// Evicts the oldest entries until the cache's bytes plus dwRoom fit its budget, stopping at an oldest entry that can't
// go: pinned, pExcept, or the touched list's tail. Trims run on the threads that ask for sprites, and the lookup 0x6001F0
// queues reads after touching the sprite it hands to the draw, which leaves that sprite the touched tail until the draw.
static void SpriteCacheTrim(SpriteCache* pCache, DWORD bUntouchedFirst, SpriteCacheEntry* pExcept, DWORD dwRoom) {
    for (;;) {
        EnterCriticalSection(&pCache->Lock);
        bool bOver = pCache->dwBytes + dwRoom > pCache->dwBudget;
        LeaveCriticalSection(&pCache->Lock);
        if (!bOver || !SpriteCacheEvict(pCache, NULL, bUntouchedFirst, pExcept, true))
            return;
    }
}

// 0x5FF630 right after a sprite's read is queued: charge it, then trim. The game pins the sprite (flag 2) to keep its
// trim off it, but the read's completion clears that flag, so a read finishing mid-trim could evict the sprite being
// charged (Halt 2631); the sprite is excluded by address instead, and the flag belongs to the completion alone.
void __fastcall SpriteCacheChargeIssue_Intercept(SpriteCache* pCache, SpriteCacheEntry* pEntry, DWORD dwSize, DWORD bUntouchedFirst) {
    EnterCriticalSection(&pCache->Lock);
    pEntry->dwSize += dwSize;
    pCache->dwBytes += dwSize;
    LeaveCriticalSection(&pCache->Lock);
    SpriteCacheTrim(pCache, bUntouchedFirst, pEntry, 0);
}

// 0x5FFC40 called from 0x5FFFA0: the entry for a sprite about to be read. Finished reads are charged without a trim, and
// an allocation that doesn't fit refuses, rather than evicts, while the oldest list head was used or allocated in the
// last 15 s (0x5FFCBD), leaving the sprite undrawn; so the cache first makes room, least recently used first across both
// lists.
SpriteCacheEntry* __fastcall SpriteCacheAllocRead_Intercept(SpriteCache* pCache, DWORD dwSize, DWORD dwPayload, DWORD bUntouched) {
    SpriteCacheTrim(pCache, 1, NULL, dwSize);
    return D2CMP_SpriteCacheAlloc(pCache, dwSize, dwPayload, bUntouched);
}

static void StormStreamRefill(StormReadRequest* pRequest) {
    __asm {
		mov esi, pRequest // the refill takes its request in esi; the compiler saves esi for a function whose asm uses it
		call STORM_StreamRefill_I
    }
}

// Storm's read thread (0x41A550) lets go of its lock (0x779068) once it has picked a request, reads, then checks the
// request's sound buffer and refills or stops it. Ending a sound stream (0x41A990, for example on the game's sound
// thread) clears the buffer under that lock, so either use could act on a buffer the request no longer has; the refill
// reads it again under the lock without a check (0x4157D1). The check and both uses run under the lock here: the lock
// is recursive, and the refill already takes it and calls DirectSound under it.
static void __fastcall StormStreamUpdate(StormReadRequest* pRequest) {
    typedef HRESULT(__stdcall * SoundBufferStop_t)(void* pBuffer);
    EnterCriticalSection(p_STORM_ReadLock);
    if (pRequest->pBuffer) {
        if (pRequest->dwRead)
            StormStreamRefill(pRequest);
        else if (pRequest->dwReadSize) {
            // nothing was read (the end of the file, or no place to read into); IDirectSoundBuffer::Stop is the buffer's method at +48h
            SoundBufferStop_t pStop = (SoundBufferStop_t)(*(void***)pRequest->pBuffer)[0x48 / 4];
            pStop(pRequest->pBuffer);
            if (pRequest->pChannel)
                pRequest->pChannel->dwState = 4;
        }
    }
    LeaveCriticalSection(p_STORM_ReadLock);
}

void __declspec(naked) StormStreamUpdate_Intercept() {
    __asm {
		mov ecx, esi // the read thread's current request; eax, ecx and edx are written before they are read from 0x41A736 on
		call StormStreamUpdate
		add dword ptr [esp], 0x0C // return to 0x41A736, an instruction start whether or not the patch is in place
		ret
    }
}

void GameDrawOOG_Intercept(void) {
    GameDrawOOG();
}

void __declspec(naked) CongratsScreen_Intercept(void) {
    __asm
    {
		call D2CLIENT_CongratsScreen_I
		pushad
		call SetMaxDiff
		popad
		retn
    }
}

void __declspec(naked) GameActChange_Intercept(void) {
    __asm
    {
		POP EAX
		PUSH ESI
		XOR ESI, ESI
		CMP [Vars.bChangedAct], 0
		MOV [Vars.bChangedAct], 0
		JMP EAX
    }
}

void __declspec(naked) GameActChange2_Intercept(void) {
    __asm
    {
		MOV ESP, EBP
		POP EBP
		MOV [Vars.bChangedAct], 1
		retn
    }
}

// An NPC's quest speech or gossip leaves the speech panel a callback that plays the NPC's next message from its message
// list when the speech ends (0x4B1830 reads the list without a NULL check), and only the callback clears itself. Closing
// the NPC mid-speech (0x4B3C20, which me.cancel() reaches) and leaving the game end the speech and set the list to NULL
// without running it, and the next speech to end, an object's for one, would run it with no list (access violation at
// 0x661406), in the same game or a later one. The callback goes with the list here, as when its chain ends (0x4B6B6B).
static void ClearNpcSpeechCallback() {
    DWORD dwCallback = *p_D2CLIENT_SpeechDoneCallback;
    if (dwCallback != D2CLIENT_NpcQuestSpeechDone_I && dwCallback != D2CLIENT_NpcGossipSpeechDone_I)
        return; // an object's speech callback (0x4A27B0) doesn't read the list
    *p_D2CLIENT_SpeechDoneCallback = 0;
    *p_D2CLIENT_SpeechDoneCallbackArg = 0;
    *p_D2CLIENT_NpcChainActive = 0;
}

// Both take the place of a call (0x4B3CC2, 0x456E09) whose caller writes eax, ecx and edx before reading them.
void __declspec(naked) EndNpcSpeech_Intercept() {
    __asm {
		call D2CLIENT_EndNpcSpeech_I
		jmp ClearNpcSpeechCallback
    }
}

void __declspec(naked) FreeSpeechPanel_Intercept() {
    __asm {
		call D2CLIENT_FreeSpeechPanel_I
		jmp ClearNpcSpeechCallback
    }
}

void __declspec(naked) GameLeave_Intercept(void) {
    __asm
    {
		call GameLeave
		jmp D2CLIENT_GameLeave_I
    }
}

void __declspec(naked) ChannelInput_Intercept(void) {
    __asm {
		push ecx
		mov ecx, esi

		call ChannelInput

		test eax, eax
		pop ecx

		jz SkipInput
		call D2MULTI_ChannelInput_I

SkipInput:
		ret
    }
}

void __declspec(naked) AddUnit_Intercept(UnitAny* lpUnit) {
    __asm
    {
		call [D2CLIENT_GameAddUnit_I]
		pushad
		push esi
		call AddUnit
		popad
		retn
    }
}

void __declspec(naked) RemoveUnit_Intercept(UnitAny* lpUnit) {
    __asm {
		pushad
		push dword ptr ds:[esi+edx*4]
		call RemoveUnit
		popad
		mov eax,dword ptr ds:[ecx+0xE4]
		mov DWORD PTR ds:[esi+edx*4], eax
		retn
    }
}

VOID __declspec(naked) __fastcall ClassicSTUB() {
    *p_BNCLIENT_ClassicKey = Vars.szClassic;
    __asm {
		
		jmp BNCLIENT_DClass;
    }
}

VOID __declspec(naked) __fastcall LodSTUB() {
    *p_BNCLIENT_XPacKey = Vars.szLod;
    __asm {
		
		jmp BNCLIENT_DLod;
    }
}

void __declspec(naked) FailToJoin() {
    __asm
    {
		cmp esi, 4000;
		ret;
    }
}

int EraseCacheFiles() {
    CHAR path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, path);

    CHAR szSearch[MAX_PATH];
    memset(szSearch, 0x00, MAX_PATH);

    strcpy_s(szSearch, path);
    PathAppendA(szSearch, "\\*.dat");

    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind = FindFirstFileA(szSearch, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char FilePath[MAX_PATH];
            memset(FilePath, 0x00, MAX_PATH);

            strcpy_s(FilePath, path);
            PathAppendA(FilePath, FindFileData.cFileName);
            DeleteFileA(FilePath);

        } while (FindNextFileA(hFind, &FindFileData));

        FindClose(hFind);
    }

    return 0;
}

HMODULE __stdcall Multi(LPSTR Class, LPSTR Window) {
    return 0;
}

HANDLE __stdcall Windowname(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
                            HINSTANCE hInstance, LPVOID lpParam) {
    WCHAR szWindowName[200] = L"D2";
    WCHAR szClassName[200] = L"CNAME";

    if (wcslen(Vars.szTitle) > 1)
        wcscpy_s(szWindowName, _countof(szWindowName), Vars.szTitle);

    WCHAR* wClassName = AnsiToUnicode(lpClassName);
    wcscpy_s(szClassName, _countof(szClassName), wClassName);
    delete[] wClassName;

    return CreateWindowExW(dwExStyle, szClassName, szWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HANDLE __stdcall CacheFix(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                          DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    EraseCacheFiles();
    CHAR path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, path);

    if (Vars.bCacheFix) {
        char szTitle[128];
        GetWindowText(D2GFX_GetHwnd(), szTitle, 128);
        CHAR Def[100] = "";

        if (strlen(szTitle) > 1) {
            sprintf_s(Def, "\\bncache%d.dat", szTitle);
            strcat_s(path, Def);
        } else {
            srand(GetTickCount());
            sprintf_s(Def, "\\bncache%d.dat", rand() % 0x2000);

            strcat_s(path, Def);
        }
    } else
        strcat_s(path, "\\bncache.dat");

    return CreateFileA(path, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

WINUSERAPI
int WINAPI MessageBoxA(__in_opt HWND hWnd, __in_opt LPCSTR lpText, __in_opt LPCSTR lpCaption, __in UINT uType);

int WINAPI LogMessageBoxA_Intercept(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    GetStackWalk();
    char* dllAddrs;
    Log(L"Error message box, caption: \"%hs\", message:\n%hs\n%hs", lpCaption, lpText, dllAddrs = DllLoadAddrStrs());
    free(dllAddrs);
    return MessageBoxA(hWnd, lpText, lpCaption, uType);
}

#include <DbgHelp.h>
LONG WINAPI MyUnhandledExceptionFilter(_In_ struct _EXCEPTION_POINTERS* ExceptionInfo) {
    // NOT WORKING ONE, WORKING ONE IS IN Helpers.cpp
    MessageBox(NULL, "QWE", "QWE", MB_OK);
    HANDLE hFile = INVALID_HANDLE_VALUE;
    for (int i = 0; hFile == INVALID_HANDLE_VALUE; ++i) {
        char fname[100];
        sprintf_s(fname, "Crash%03d.dump", i);
        hFile = CreateFile(fname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    DWORD ProcessId = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS /*READ_CONTROL | PROCESS_VM_READ*/, TRUE, ProcessId);
    MINIDUMP_EXCEPTION_INFORMATION ExceptionParam;
    ExceptionParam.ThreadId = GetCurrentThreadId();
    ExceptionParam.ExceptionPointers = ExceptionInfo;
    ExceptionParam.ClientPointers = TRUE;
    MiniDumpWriteDump(hProcess, ProcessId, hFile, MiniDumpNormal, &ExceptionParam, NULL, NULL);
    CloseHandle(hFile);
    exit(0);
    return EXCEPTION_EXECUTE_HANDLER;
}

//  FogException(6, (int)&Default, a3, "Unrecoverable internal error %08x", a2);
void FogException() {
    __try {
        RaiseException(1, // exception code
                       0, // continuable exception
                       0, NULL);
    } __except (UnhandledExceptionFilter(GetExceptionInformation())) {
        exit(0);
        exit(0);
    }
}

namespace {
// Marks the exception stream of a Halt's dump: a Halt raises no exception of its own.
const DWORD kHaltDumpCode = 0xE0D20001;
// Heap, stacks and the game's data: what tracing a Halt back to its unit or item needs; images come from the binaries.
const MINIDUMP_TYPE kCrashDumpType =
    (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithFullMemoryInfo |
                    MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules | MiniDumpIgnoreInaccessibleMemory);
const DWORD kCrashDumpsKept = 5;        // a dump is a few hundred MB, and every profile writes to the same logs folder
const DWORD kPacketLogsKept = 50;
const ULONGLONG kDumpMinFreeBytes = 2ULL << 30;
const DWORD kDumpTimeoutMs = 60000;

// The longest suffix a report adds to its stem.
const wchar_t kLongestSuffix[] = L".packets.txt";

// <logs>crash-<profile>-<yyyymmdd-hhmmss>-<pid>, one stem per report. False when the logs path leaves no room for it:
// the CRT's _s functions end the process on overflow, so every path here is built truncating and checked.
bool CrashReportStem(wchar_t* stem, size_t len) {
    wchar_t profile[48];
    size_t n = 0;
    for (const wchar_t* p = Vars.szProfile; *p && n < _countof(profile) - 1; p++)
        profile[n++] = (iswalnum(*p) || *p == L'-' || *p == L'_') ? *p : L'_';
    profile[n] = L'\0';
    SYSTEMTIME t;
    GetLocalTime(&t);
    return _snwprintf_s(stem, len - (_countof(kLongestSuffix) - 1), _TRUNCATE, L"%scrash-%s-%04u%02u%02u-%02u%02u%02u-%u", Vars.szLogPath,
                        n ? profile : L"noprofile", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, GetCurrentProcessId()) >= 0;
}

// Deletes the oldest files matching pattern in the logs folder until at most keep remain. A file it can't delete (one
// open in a debugger) is passed over for the next oldest, up to kSkipMax of them; no allocations of its own, as in the
// rest of the report.
void PruneCrashFiles(const wchar_t* pattern, DWORD keep) {
    const int kSkipMax = 8;
    wchar_t skipped[kSkipMax][MAX_PATH];
    int skippedCount = 0;
    wchar_t search[MAX_PATH], oldest[MAX_PATH], candidate[MAX_PATH];
    if (_snwprintf_s(search, _countof(search), _TRUNCATE, L"%s%s", Vars.szLogPath, pattern) < 0)
        return;
    for (;;) {
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(search, &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            return;
        DWORD count = 0;
        FILETIME oldestTime = {0xFFFFFFFF, 0xFFFFFFFF};
        oldest[0] = L'\0';
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            count++;
            if (CompareFileTime(&fd.ftLastWriteTime, &oldestTime) < 0 &&
                _snwprintf_s(candidate, _countof(candidate), _TRUNCATE, L"%s%s", Vars.szLogPath, fd.cFileName) >= 0) {
                bool skip = false;
                for (int i = 0; i < skippedCount && !skip; i++)
                    skip = _wcsicmp(skipped[i], candidate) == 0;
                if (!skip) {
                    oldestTime = fd.ftLastWriteTime;
                    wcscpy_s(oldest, candidate);
                }
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
        if (count <= keep || !oldest[0])
            return;
        if (!DeleteFileW(oldest)) {
            if (skippedCount == kSkipMax)
                return;
            wcscpy_s(skipped[skippedCount++], oldest);
        }
    }
}

// Process snapshots (Windows 8.1, ProcessSnapShot.h and minidumpapiset.h), which this toolset's headers predate.
const DWORD kSnapshotFlags = 0x00000001 /* PSS_CAPTURE_VA_CLONE */ | 0x00000004 /* PSS_CAPTURE_HANDLES */ |
                             0x00000008 /* PSS_CAPTURE_HANDLE_NAME_INFORMATION */ | 0x00000010 /* PSS_CAPTURE_HANDLE_BASIC_INFORMATION */ |
                             0x00000020 /* PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION */ | 0x00000040 /* PSS_CAPTURE_HANDLE_TRACE */ |
                             0x00000080 /* PSS_CAPTURE_THREADS */ | 0x00000100 /* PSS_CAPTURE_THREAD_CONTEXT */ |
                             0x00000200 /* PSS_CAPTURE_THREAD_CONTEXT_EXTENDED */ | 0x04000000 /* PSS_CREATE_BREAKAWAY_OPTIONAL */ |
                             0x08000000 /* PSS_CREATE_BREAKAWAY */ | 0x20000000 /* PSS_CREATE_USE_VM_ALLOCATIONS */ |
                             0x80000000 /* PSS_CREATE_RELEASE_SECTION */;
const int kIsProcessSnapshotCallback = 16;
typedef DWORD(WINAPI* PssCaptureSnapshotFn)(HANDLE process, DWORD flags, DWORD threadContextFlags, HANDLE* snapshot);
typedef DWORD(WINAPI* PssFreeSnapshotFn)(HANDLE process, HANDLE snapshot);

// The callback parameter is non-NULL when dbghelp was given a snapshot.
BOOL CALLBACK DumpCallback(PVOID fromSnapshot, PMINIDUMP_CALLBACK_INPUT input, PMINIDUMP_CALLBACK_OUTPUT output) {
    if (input && output) {
        if ((int)input->CallbackType == kIsProcessSnapshotCallback && fromSnapshot)
            output->Status = S_FALSE;
        else if (input->CallbackType == ReadMemoryFailureCallback)
            output->Status = S_OK; // without a snapshot: leave out memory that changed mid-dump instead of failing
    }
    return TRUE;
}

struct DumpJob {
    const wchar_t* path;
    MINIDUMP_EXCEPTION_INFORMATION info;
    BOOL written;
    BOOL fromSnapshot;
    DWORD error; // GetLastError when not written; dbghelp reports an HRESULT there
};

DWORD WINAPI WriteDumpThread(LPVOID param) {
    DumpJob* job = (DumpJob*)param;
    HANDLE hFile = CreateFileW(job->path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        job->error = GetLastError();
        return 0;
    }
    // dbghelp reads a snapshot (a copy-on-write clone of the process) the way it reads another process. Dumping the
    // live process from inside it fails with ERROR_PARTIAL_COPY: dbghelp's own allocations change what it is reading.
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    PssCaptureSnapshotFn capture = (PssCaptureSnapshotFn)GetProcAddress(kernel32, "PssCaptureSnapshot");
    PssFreeSnapshotFn release = (PssFreeSnapshotFn)GetProcAddress(kernel32, "PssFreeSnapshot");
    HANDLE snapshot = NULL;
    if (!capture || !release || capture(GetCurrentProcess(), kSnapshotFlags, CONTEXT_ALL, &snapshot) != ERROR_SUCCESS)
        snapshot = NULL;
    job->fromSnapshot = snapshot != NULL;
    MINIDUMP_CALLBACK_INFORMATION callback = {DumpCallback, snapshot};
    job->written = MiniDumpWriteDump(snapshot ? snapshot : GetCurrentProcess(), GetCurrentProcessId(), hFile, kCrashDumpType, &job->info, NULL, &callback);
    if (!job->written)
        job->error = GetLastError();
    if (snapshot)
        release(GetCurrentProcess(), snapshot);
    CloseHandle(hFile);
    return 0;
}

// 0 no report yet, 1 one is being written, 2 written. A second thread that crashes meanwhile waits instead of ending
// the process under the first one's dump; a fault on the writing thread itself returns at once, as it would wait on
// itself.
volatile LONG g_crashReportState = 0;
volatile DWORD g_crashReportThread = 0;

struct CrashReportDone {
    ~CrashReportDone() {
        InterlockedExchange(&g_crashReportState, 2);
    }
};

// On x86 RtlCaptureContext takes the context from its caller's EBP frame, so this function keeps one.
#pragma optimize("y", off)
// Writes the packet log and, withDump, a dump of the calling thread's process.
void WriteCrashReport(const char* reason, const char* detail, bool withDump) {
    if (InterlockedCompareExchange(&g_crashReportState, 1, 0) != 0) {
        if (g_crashReportThread == GetCurrentThreadId())
            return;
        for (DWORD waited = 0; g_crashReportState == 1 && waited < kDumpTimeoutMs + 5000; waited += 100)
            Sleep(100);
        return;
    }
    g_crashReportThread = GetCurrentThreadId();
    CrashReportDone done;

    wchar_t stem[MAX_PATH], path[MAX_PATH];
    if (!CrashReportStem(stem, _countof(stem))) {
        Log(L"Crash report: none, the logs path %s is too long", Vars.szLogPath);
        return;
    }
    char header[512];
    _snprintf_s(header, sizeof(header), _TRUNCATE, "D2BS crash report: %s (%s), profile %ls, process %u, thread %u", reason, detail ? detail : "",
                Vars.szProfile, GetCurrentProcessId(), GetCurrentThreadId());
    PruneCrashFiles(L"crash-*.packets.txt", kPacketLogsKept - 1);
    _snwprintf_s(path, _countof(path), _TRUNCATE, L"%s.packets.txt", stem);
    if (WritePacketLog(path, header))
        Log(L"Crash report: packet log %s", path);
    else
        Log(L"Crash report: no packet log, %s could not be created (%u)", path, GetLastError());
    if (!withDump)
        return;

    // Old dumps go first, so they can't keep the free space under the guard for good.
    PruneCrashFiles(L"crash-*.dmp", kCrashDumpsKept - 1);
    ULARGE_INTEGER freeBytes;
    if (!GetDiskFreeSpaceExW(Vars.szLogPath, &freeBytes, NULL, NULL)) {
        Log(L"Crash report: no dump, the free space in %s could not be read (%u)", Vars.szLogPath, GetLastError());
        return;
    }
    if (freeBytes.QuadPart < kDumpMinFreeBytes) {
        Log(L"Crash report: no dump, less than 2 GB free in %s", Vars.szLogPath);
        return;
    }
    _snwprintf_s(path, _countof(path), _TRUNCATE, L"%s.dmp", stem);

    // The dump is written from a helper thread so this thread's stack is recorded as it is at the Halt, and so a dump
    // that hangs (a thread suspended inside the heap lock) cannot keep the process alive.
    CONTEXT context;
    RtlCaptureContext(&context);
    EXCEPTION_RECORD record;
    memset(&record, 0, sizeof(record));
    record.ExceptionCode = kHaltDumpCode;
    record.ExceptionAddress = (PVOID)context.Eip;
    EXCEPTION_POINTERS pointers = {&record, &context};
    DumpJob job;
    job.path = path;
    job.info.ThreadId = GetCurrentThreadId();
    job.info.ExceptionPointers = &pointers;
    job.info.ClientPointers = FALSE;
    job.written = FALSE;
    job.fromSnapshot = FALSE;
    job.error = 0;
    // Logged before the dump: a timed-out dump may leave a thread suspended inside the heap lock, so nothing after a
    // timeout may allocate, and this line is then the only trace.
    Log(L"Crash report: writing dump %s", path);
    DWORD started = GetTickCount();
    HANDLE hThread = CreateThread(NULL, 0, WriteDumpThread, &job, 0, NULL);
    if (!hThread) {
        Log(L"Crash report: no dump, CreateThread failed (%u)", GetLastError());
        return;
    }
    if (WaitForSingleObject(hThread, kDumpTimeoutMs) != WAIT_OBJECT_0)
        TerminateProcess(GetCurrentProcess(), 0);
    CloseHandle(hThread);
    const wchar_t* source = job.fromSnapshot ? L"a snapshot" : L"the live process";
    if (job.written)
        Log(L"Crash report: dump %s written from %s in %u ms", path, source, GetTickCount() - started);
    else
        Log(L"Crash report: dump %s FAILED (0x%08X) from %s after %u ms", path, job.error, source, GetTickCount() - started);
}
#pragma optimize("", on)
} // namespace

char __fastcall ErrorReportLaunch(const char* crash_file, int a2) {
    GetStackWalk();
    Log(L"Crash File: %hs\n", crash_file);
    // A Halt raises no exception, so Windows writes no dump of it: this one is the only record of the game's state.
    // It is opt-in: a dump is a few hundred MB that only a debugger reads, and the game's memory in it can include
    // account details.
    WriteCrashReport("halt", crash_file, Vars.bHaltDumps != FALSE);
    if (!Vars.bHaltDumps)
        Log(L"Crash report: no dump; set HaltDumps=true in d2bs.ini to write one");
    exit(0);
}

// Fog's crash handler: Fog's top-level exception filter, which replaces D2BS's at startup, calls it with its crash
// info in ECX (+0x18 = the EXCEPTION_POINTERS). Returning 0 makes that filter answer EXCEPTION_CONTINUE_SEARCH, so
// Windows Error Reporting gets the real exception.
DWORD __fastcall FogCrash_Intercept(BYTE* crashInfo, DWORD) {
    EXCEPTION_POINTERS* ptrs = *(EXCEPTION_POINTERS**)(crashInfo + 0x18);
    // A stack overflow leaves this thread about a page of stack, too little for the stack walk and the report, which
    // would fault again; Windows Error Reporting still gets the exception.
    if (ptrs && ptrs->ExceptionRecord && ptrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        return 0;
    if (ptrs && ptrs->ExceptionRecord && ptrs->ContextRecord) {
        ExceptionHandler(ptrs);
        // Packets only: Windows Error Reporting dumps an exception, and a second dump from inside this filter could
        // fault again on a damaged heap and hide the first exception.
        WriteCrashReport("exception", NULL, false);
    }
    return 0;
}