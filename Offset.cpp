#define _DEFINE_VARS
#define __D2PTRS_ONCE_
#include "D2Ptrs.h"
#undef __D2PTRS_H__
#define __D2PTRS_LIST_
#include "D2Ptrs.h"
#include "Patch.h"
#include "D2BS.h"
#include "Helpers.h"

#ifndef ArraySize
#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))
#endif

void DefineOffsets() {
    DWORD** p = (DWORD**)d2ptrs_list;
    do {
        **p = GetDllOffset(**p);
    } while (ptrdiff_t(++p) < ((ptrdiff_t)d2ptrs_list) + sizeof(d2ptrs_list));
}

DWORD GetDllOffset(char* DllName, int Offset) {
    HMODULE hMod = GetModuleHandle(NULL);

    if (Offset < 0)
        return (DWORD)GetProcAddress(hMod, (LPCSTR)(-Offset));

    return ((DWORD)hMod) + Offset;
}

DWORD GetDllOffset(int num) {
    static char* dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL",    "D2Lang.DLL", "D2Win.DLL", "D2Net.DLL",  "D2Game.DLL",
                           "D2Launch.DLL", "Fog.DLL",      "BNClient.DLL", "Storm.DLL",  "D2Cmp.DLL", "D2Multi.DLL"};
    if ((num & 0xff) > 12)
        return 0;
    return GetDllOffset(dlls[num & 0xff], num >> 8);
}

void InstallPatches() {
    for (int x = 0; x < ArraySize(Patches); x++) {
        Patches[x].bOldCode = new BYTE[Patches[x].dwLen];
        ::ReadProcessMemory(GetCurrentProcess(), (void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen, NULL);
        Patches[x].pFunc(Patches[x].dwAddr, Patches[x].dwFunc, Patches[x].dwLen);
    }
    for (int x = 0; x < ArraySize(Permanent); x++)
        Permanent[x].pFunc(Permanent[x].dwAddr, Permanent[x].dwFunc, Permanent[x].dwLen);
}

void RemovePatches() {
    for (int x = 0; x < ArraySize(Patches); x++) {
        WriteBytes((void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen);
        delete[] Patches[x].bOldCode;
    }
}

void InstallConditional() {
    for (int x = 0; x < ArraySize(Conditional); x++) {
        if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE) {
            continue;
        }
        Conditional[x].bOldCode = new BYTE[Conditional[x].dwLen];
        ::ReadProcessMemory(GetCurrentProcess(), (void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen, NULL);
        Conditional[x].pFunc(Conditional[x].dwAddr, Conditional[x].dwFunc, Conditional[x].dwLen);
    }
}

void RemoveConditional() {
    for (int x = 0; x < ArraySize(Conditional); x++) {
        if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE)
            continue;
        WriteBytes((void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen);
        delete[] Conditional[x].bOldCode;
    }
}

BOOL WriteBytes(void* pAddr, void* pData, DWORD dwLen) {
    DWORD dwOld;

    if (!VirtualProtect(pAddr, dwLen, PAGE_READWRITE, &dwOld))
        return FALSE;

    ::memcpy(pAddr, pData, dwLen);
    return VirtualProtect(pAddr, dwLen, dwOld, &dwOld);
}

void FillBytes(void* pAddr, BYTE bFill, DWORD dwLen) {
    BYTE* bCode = new BYTE[dwLen];
    ::memset(bCode, bFill, dwLen);

    WriteBytes(pAddr, bCode, dwLen);

    delete[] bCode;
}

void InterceptLocalCode(BYTE bInst, DWORD pAddr, DWORD pFunc, DWORD dwLen) {
    BYTE* bCode = new BYTE[dwLen];
    ::memset(bCode, 0x90, dwLen);
    DWORD dwFunc = pFunc - (pAddr + 5);

    bCode[0] = bInst;
    *(DWORD*)&bCode[1] = dwFunc;
    WriteBytes((void*)pAddr, bCode, dwLen);

    delete[] bCode;
}

void PatchCall(DWORD dwAddr, DWORD dwFunc, DWORD dwLen) {
    InterceptLocalCode(INST_CALL, dwAddr, dwFunc, dwLen);
}

void PatchJmp(DWORD dwAddr, DWORD dwFunc, DWORD dwLen) {
    InterceptLocalCode(INST_JMP, dwAddr, dwFunc, dwLen);
}

void PatchBytes(DWORD dwAddr, DWORD dwValue, DWORD dwLen) {
    BYTE* bCode = new BYTE[dwLen];
    ::memset(bCode, (BYTE)dwValue, dwLen);

    WriteBytes((LPVOID)dwAddr, bCode, dwLen);

    delete[] bCode;
}

// D2BS can load into a running game, and a thread partway through the instructions a rewrite replaces would resume on
// the wrong bytes: the other threads are suspended for the write, which waits, up to 50 tries, while any of them is
// inside the range (one at its first byte resumes on the new code) or can't be held and inspected. Between suspending
// and resuming nothing may allocate or log, since a suspended thread can hold the heap or CRT lock.
static bool WriteWithThreadsHeld(DWORD dwAddr, const BYTE* bNew, DWORD dwLen) {
    // A snapshot that misses the calling thread failed, so it can't vouch for the others.
    std::vector<DWORD> ids = GetThreadIds();
    DWORD self = GetCurrentThreadId();
    bool listed = false;
    for (size_t k = 0; k < ids.size(); k++)
        listed = listed || ids[k] == self;
    if (!listed)
        return false;
    std::vector<HANDLE> held;
    held.reserve(ids.size());
    for (int attempt = 0; attempt < 50; attempt++) {
        bool safe = true;
        for (size_t k = 0; k < ids.size() && safe; k++) {
            if (ids[k] == self)
                continue;
            // Only a thread that has ended since the snapshot may go unheld.
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | SYNCHRONIZE, FALSE, ids[k]);
            if (!hThread) {
                safe = GetLastError() == ERROR_INVALID_PARAMETER;
                continue;
            }
            if (SuspendThread(hThread) == (DWORD)-1) {
                safe = WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0;
                CloseHandle(hThread);
                continue;
            }
            held.push_back(hThread);
            // SuspendThread returns before the thread stops; GetThreadContext waits for it.
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_CONTROL;
            safe = GetThreadContext(hThread, &ctx) && !(ctx.Eip > dwAddr && ctx.Eip < dwAddr + dwLen);
        }
        BOOL written = safe && WriteBytes((LPVOID)dwAddr, (void*)bNew, dwLen);
        for (size_t k = 0; k < held.size(); k++) {
            ResumeThread(held[k]);
            CloseHandle(held[k]);
        }
        held.clear();
        if (safe)
            return written != FALSE;
        Sleep(1);
    }
    return false;
}

// dwSwap points to a ByteSwap. A rewrite of several instructions lands only on the original bytes: code that already
// differs (another patch, another game version) is left alone, and code that already holds the new bytes (D2BS loaded
// again into the same game process) needs nothing.
void PatchSwap(DWORD dwAddr, DWORD dwSwap, DWORD dwLen) {
    const ByteSwap* swap = (const ByteSwap*)dwSwap;
    if (::memcmp((const void*)dwAddr, swap->bNew, dwLen) == 0)
        return;
    if (::memcmp((const void*)dwAddr, swap->bOld, dwLen) != 0) {
        Log(L"Patch at 0x%08X skipped: the code there is not the expected original", dwAddr);
        return;
    }
    if (!WriteWithThreadsHeld(dwAddr, swap->bNew, dwLen))
        Log(L"Patch at 0x%08X skipped: another thread stayed inside it or could not be inspected, or the page could not be written", dwAddr);
}

PatchHook* RetrievePatchHooks(PINT pBuffer) {
    *pBuffer = ArraySize(Patches);
    return &Patches[0];
}
