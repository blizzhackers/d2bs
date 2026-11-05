#ifndef _OFFSET_H
#define _OFFSET_H

#define INST_INT3 0xCC
#define INST_CALL 0xE8
#define INST_NOP 0x90
#define INST_JMP 0xE9
#define INST_RET 0xC3

typedef struct PatchHook_t {
    void (*pFunc)(DWORD, DWORD, DWORD);
    DWORD dwAddr;
    DWORD dwFunc;
    DWORD dwLen;
    BOOL* enabled;
    BYTE* bOldCode;
} PatchHook;

// The bytes PatchSwap expects at the address, and the bytes it writes in their place.
typedef struct ByteSwap_t {
    const BYTE* bOld;
    const BYTE* bNew;
} ByteSwap;

// The bytes PatchSwapCall expects at the address, and the function the call it writes in their place goes to.
typedef struct CallSwap_t {
    const BYTE* bOld;
    DWORD dwFunc;
} CallSwap;

void DefineOffsets();
DWORD GetDllOffset(int num);
DWORD GetDllOffset(char* DllName, int Offset);

PatchHook* RetrievePatchHooks(PINT pBuffer);
void PatchBytes(DWORD dwAddr, DWORD dwValue, DWORD dwLen);
void PatchJmp(DWORD dwAddr, DWORD dwFunc, DWORD dwLen);
void PatchCall(DWORD dwAddr, DWORD dwFunc, DWORD dwLen);
void PatchSwap(DWORD dwAddr, DWORD dwSwap, DWORD dwLen);
void PatchSwapCall(DWORD dwAddr, DWORD dwSwap, DWORD dwLen);
void InterceptLocalCode(BYTE bInst, DWORD pAddr, DWORD pFunc, DWORD dwLen);
void FillBytes(void* pAddr, BYTE bFill, DWORD dwLen);
BOOL WriteBytes(void* pAddr, void* pData, DWORD dwLen);
void RemovePatches();
void InstallPatches();
void InstallConditional();
void RemoveConditional();

#endif