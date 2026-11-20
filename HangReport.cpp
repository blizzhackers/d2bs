#include <windows.h>
#include <tlhelp32.h>
#include <DbgHelp.h>
#include <intrin.h>

#include "HangReport.h"
// SpiderMonkey's headers (through D2BS.h) warn C4512 once per file that includes them.
#pragma warning(push)
#pragma warning(disable : 4512)
#include "D2BS.h"
#include "D2Helpers.h"
#include "Helpers.h"
#include "D2Handlers.h"
#include "D2Intercepts.h"
#include "D2Ptrs.h"
#include "ScriptEngine.h"
#include "ScreenHook.h"
#pragma warning(pop)

// The watchdog compares the game thread's timer ticks against Windows' own verdict on the window, so it reports what
// D2Bot's "not responding" check also sees. The header, the diagnosis and the game thread's block are written with
// syscalls and static buffers, not the heap or the CRT: the stuck game thread may own a heap lock or a CRT lock for good,
// and that much of the report must still reach the disk.
namespace {
const DWORD kPollMs = 250;
const DWORD kGapResetMs = 1500;
const DWORD kTextBudgetMs = 3000;
const DWORD kDumpWaitMs = 8000;
const DWORD kStopWaitMs = 3000;
const int kFullReportsMax = 3; // per process; bounds the other-thread passes and the log volume, not the diagnosis
const int kFollowUpsMax = 3;
const int kMaxThreads = 128;
const int kMaxModules = 256;
const int kMaxLocks = 24;
const int kNameSlots = 128;
const int kGameFrames = 48;
const int kOtherFrames = 16;
const DWORD kHangPacketLogsKept = 20;
const DWORD kHangDumpsKept = 2;
const DWORD kHangDumpCode = 0xE0D20002;
const ULONGLONG kDumpMinFreeBytes = 2ULL << 30;

const MINIDUMP_TYPE kHangDumpType =
    (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo |
                    MiniDumpWithUnloadedModules | MiniDumpIgnoreInaccessibleMemory);
// The crash report's snapshot flags without handles: the game keeps running during a hang dump, and the snapshot's
// handle walk faults when a handle closes mid-capture (rig: ntdll!PsspHandleDumper reading near NULL, every time the
// game thread resumed during the dump).
const DWORD kHangSnapshotFlags = 0x00000001 /* PSS_CAPTURE_VA_CLONE */ | 0x00000080 /* PSS_CAPTURE_THREADS */ |
                                 0x00000100 /* PSS_CAPTURE_THREAD_CONTEXT */ | 0x00000200 /* PSS_CAPTURE_THREAD_CONTEXT_EXTENDED */ |
                                 0x04000000 /* PSS_CREATE_BREAKAWAY_OPTIONAL */ | 0x08000000 /* PSS_CREATE_BREAKAWAY */ |
                                 0x20000000 /* PSS_CREATE_USE_VM_ALLOCATIONS */ | 0x80000000 /* PSS_CREATE_RELEASE_SECTION */;
const int kIsProcessSnapshotCallback = 16;
typedef DWORD(WINAPI* PssCaptureSnapshotFn)(HANDLE process, DWORD flags, DWORD threadContextFlags, HANDLE* snapshot);
typedef DWORD(WINAPI* PssFreeSnapshotFn)(HANDLE process, HANDLE snapshot);
typedef DWORD(WINAPI* PssWalkMarkerCreateFn)(void* allocator, HANDLE* marker);
typedef DWORD(WINAPI* PssWalkMarkerFreeFn)(HANDLE marker);
typedef DWORD(WINAPI* PssWalkSnapshotFn)(HANDLE snapshot, int infoClass, HANDLE marker, void* buffer, DWORD bufferLength);
const int kPssWalkThreads = 3; // PSS_WALK_THREADS

// PSS_THREAD_ENTRY (processsnapshot.h, Windows 8.1), x86
struct PssThreadEntry {
    DWORD exitStatus;
    void* teb;
    DWORD processId;
    DWORD threadId;
    ULONG_PTR affinityMask;
    int priority;
    int basePriority;
    void* lastSyscallFirstArgument;
    WORD lastSyscallNumber;
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    void* win32StartAddress;
    FILETIME captureTime;
    DWORD flags;
    WORD suspendCount;
    WORD sizeOfContextRecord;
    CONTEXT* contextRecord;
};
static_assert(sizeof(PssThreadEntry) == 92, "PSS_THREAD_ENTRY is 92 bytes on x86");
typedef LONG(NTAPI* NtQueryVirtualMemoryFn)(HANDLE process, PVOID address, int infoClass, PVOID info, SIZE_T length, SIZE_T* returned);
typedef LONG(NTAPI* NtQueryInformationThreadFn)(HANDLE thread, int infoClass, PVOID info, ULONG length, PULONG returned);

struct NtString {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
};

struct NtObjectAttributes {
    ULONG Length;
    HANDLE RootDirectory;
    NtString* ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
};

struct NtIoStatus {
    LONG Status;
    ULONG_PTR Information;
};

typedef LONG(NTAPI* NtCreateFileFn)(HANDLE* file, ACCESS_MASK access, NtObjectAttributes* attributes, NtIoStatus* io, PLARGE_INTEGER allocationSize,
                                    ULONG fileAttributes, ULONG share, ULONG disposition, ULONG options, PVOID eaBuffer, ULONG eaLength);

struct ModuleInfo {
    DWORD base, end, stamp;
    char name[48];
};

struct LockRef {
    const char* name;
    CRITICAL_SECTION* cs;
};

struct LockState {
    BOOL read;
    DWORD owner;
    LONG lockCount;
    LONG recursion;
};

struct NameSlot {
    volatile LONG tid; // 0 free, -1 being written
    wchar_t name[48];
};

struct Capture {
    BOOL held;
    BOOL ctxOk;
    LONG ticksDone; // g_ticksDone while the thread was held
    DWORD prevSuspend;
    DWORD err;
    DWORD got;
    CONTEXT ctx;
};

// A text buffer appended to the d2bs log in whole lines; each flush opens, appends and closes, so nothing stays open.
struct Writer {
    char buf[16384];
    int len;
    int lost; // flushes that did not reach the log, since the report reset it
    char line[1100];
    wchar_t path[MAX_PATH * 2];
    wchar_t ntPath[MAX_PATH * 2 + 8];
};

struct DumpJob {
    wchar_t path[MAX_PATH];
    wchar_t partial[MAX_PATH + 16];
    int episode;
    DWORD gameTid;
    // The game thread's registers as the snapshot holds them, so they match the dumped stack; false when the snapshot
    // walk could not find them, and the dump then has no exception context.
    BOOL hasCtx;
    LONG ticks; // the game thread's tick count during this episode's stall
    HANDLE snapshot;
    HANDLE marker;
    HANDLE file;
    CONTEXT ctx;
    EXCEPTION_RECORD rec;
};

volatile LONG g_ticks = 0;
volatile LONG g_ticksDone = 0; // g_ticks once that tick's resume time is stored
LONG g_episodeTicks = 0;       // the watchdog's: g_ticks during the open episode's stall
volatile DWORD g_tickTime = 0;
// The first tick after a gap inside the open episode: when a stall ended, even while a report was being written.
volatile DWORD g_resumeTime = 0;
volatile DWORD g_openTick = 0; // the open episode's last tick before the stall
volatile LONG g_tickState = 0;
HWND volatile g_hwnd = NULL;
volatile DWORD g_gameTid = 0;
volatile LONG g_quiesced = 0;
volatile LONG g_engineStarted = 0;
volatile LONG g_off = 0;
volatile LONG g_exited = 0;
volatile LONG g_dumpBusy = 0;
volatile LONG g_dumpWritten = 0;
volatile LONG g_dumpOff = 0;

HANDLE g_hWatch = NULL;
HANDLE g_hStop = NULL;
HANDLE g_hDumpGo = NULL;
HANDLE g_hDumpDone = NULL;
HANDLE g_hDumpThread = NULL;
HANDLE g_hGame = NULL;
DWORD g_watchTid = 0;
volatile DWORD g_dumpTid = 0;

PssCaptureSnapshotFn g_pssCapture = NULL;
PssFreeSnapshotFn g_pssFree = NULL;
PssWalkMarkerCreateFn g_pssMarkerCreate = NULL;
PssWalkMarkerFreeFn g_pssMarkerFree = NULL;
PssWalkSnapshotFn g_pssWalk = NULL;
NtQueryVirtualMemoryFn g_ntQueryVirtualMemory = NULL;
NtQueryInformationThreadFn g_ntQueryInformationThread = NULL;
NtCreateFileFn g_ntCreateFile = NULL;

char g_version[64];
LockRef g_locks[kMaxLocks];
int g_lockCount = 0;
int g_gameLock = -1;
int g_loaderLock = -1;
bool g_oldLockCount = false; // Windows 2000, XP and Server 2003 RTM count a critical section's LockCount up from -1
// Vars.szLogPath normalized once at startup: NtCreateFile takes no "\\", "." or ".." parts, which a DLL in a drive root
// ("C:\\logs\") or an odd loader path would leave in it.
wchar_t g_logDir[MAX_PATH * 2];
LockState g_lockStates[kMaxLocks];
ModuleInfo g_mods[kMaxModules];
int g_modCount = 0;
NameSlot g_names[kNameSlots];
DWORD g_tids[kMaxThreads * 4];
BYTE g_gameStack[65536];
BYTE g_otherStack[16384];
__declspec(align(8)) BYTE g_mappedName[1024];
Capture g_gameCap;
BOOL g_capWindowHung = TRUE; // IsHungAppWindow just after g_gameCap was taken
Capture g_otherCap;
Writer g_w;      // the watchdog's
Writer g_dumpW;  // the dump helper's
DumpJob g_dumpJob;
int g_episodeDumpLost = 0; // the watchdog's: this episode's dump job lost a log write

// ---------------------------------------------------------------------------------------------------------------------
// Output, CRT-free: wvsprintfA formats into the caller's buffer, and the log is opened and closed with each flush.

void BuildLogPath(wchar_t* out) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    // Copied once: a profile switch on another thread can change the name while this runs.
    wchar_t profile[256];
    int n = 0;
    for (; n < 255 && Vars.szProfile[n]; n++)
        profile[n] = Vars.szProfile[n];
    profile[n] = L'\0';
    // The same file LogNoFormat appends to; a path too long for the buffer leaves it empty.
    if (!g_logDir[0] || lstrlenW(g_logDir) + n + 24 >= MAX_PATH * 2) {
        out[0] = L'\0';
        return;
    }
    wsprintfW(out, L"%sd2bs-%s-%04u%02u%02u.log", g_logDir, profile, t.wYear, t.wMonth, t.wDay);
}

// The NT form of an absolute DOS path, or false for a form this does not convert.
bool NtPath(const wchar_t* dos, wchar_t* out, int cap) {
    const wchar_t* prefix;
    const wchar_t* rest;
    if (dos[0] == L'\\' && dos[1] == L'\\' && (dos[2] == L'?' || dos[2] == L'.') && dos[3] == L'\\') {
        prefix = L"\\??\\";
        rest = dos + 4;
    } else if (dos[0] == L'\\' && dos[1] == L'\\') {
        prefix = L"\\??\\UNC\\";
        rest = dos + 2;
    } else if (dos[0] && dos[1] == L':' && dos[2] == L'\\') {
        prefix = L"\\??\\";
        rest = dos;
    } else
        return false;
    if (lstrlenW(prefix) + lstrlenW(rest) >= cap)
        return false;
    lstrcpyW(out, prefix);
    lstrcatW(out, rest);
    return true;
}

// NtCreateFile when the path converts: CreateFileW builds its NT path on the process heap, which the stuck thread may
// hold for good, so it is used only for a path NtPath can't convert, never as a retry of a failed NtCreateFile.
HANDLE OpenLogAppend(Writer& w) {
    if (g_ntCreateFile && NtPath(w.path, w.ntPath, _countof(w.ntPath))) {
        NtString name;
        name.Length = (USHORT)(lstrlenW(w.ntPath) * sizeof(wchar_t));
        name.MaximumLength = name.Length;
        name.Buffer = w.ntPath;
        NtObjectAttributes attributes = {sizeof(NtObjectAttributes), NULL, &name, 0x40 /* OBJ_CASE_INSENSITIVE */, NULL, NULL};
        NtIoStatus io = {0, 0};
        HANDLE h = NULL;
        if (g_ntCreateFile(&h, FILE_APPEND_DATA | SYNCHRONIZE, &attributes, &io, NULL, FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 3 /* FILE_OPEN_IF */,
                           0x20 /* FILE_SYNCHRONOUS_IO_NONALERT */ | 0x40 /* FILE_NON_DIRECTORY_FILE */, NULL, 0) >= 0)
            return h;
        return INVALID_HANDLE_VALUE;
    }
    return CreateFileW(w.path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                       NULL);
}

void Flush(Writer& w) {
    if (!w.len)
        return;
    BuildLogPath(w.path);
    bool ok = false;
    if (w.path[0]) {
        HANDLE h = OpenLogAppend(w);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            ok = WriteFile(h, w.buf, (DWORD)w.len, &written, NULL) && written == (DWORD)w.len;
            CloseHandle(h);
        }
    }
    if (!ok)
        w.lost++;
    w.len = 0;
}

// One line: prefixed like LogNoFormat's lines, or indented as a continuation of the line before it.
void Line(Writer& w, bool prefixed, const char* fmt, ...) {
    int n = 0;
    if (prefixed) {
        SYSTEMTIME t;
        GetLocalTime(&t);
        n = wsprintfA(w.line, "[%02u/%02u/%02u %02u:%02u:%02u] D2BS %u: ", t.wMonth, t.wDay, t.wYear % 100, t.wHour, t.wMinute, t.wSecond,
                      GetCurrentProcessId());
    } else {
        w.line[0] = ' ';
        w.line[1] = ' ';
        n = 2;
    }
    va_list ap;
    va_start(ap, fmt);
    n += wvsprintfA(w.line + n, fmt, ap); // writes at most 1024 bytes
    va_end(ap);
    w.line[n++] = '\r';
    w.line[n++] = '\n';
    if (w.len + n > (int)sizeof(w.buf))
        Flush(w);
    CopyMemory(w.buf + w.len, w.line, n);
    w.len += n;
}

// UTF-8, as LogNoFormat writes the log (UnicodeToAnsi's default), encoded here rather than by an API so the text path
// stays call-free. A character that doesn't fit whole is dropped with the rest; an unpaired surrogate becomes '?'.
void Narrow(char* out, int cap, const wchar_t* in, int inLen) {
    int n = 0;
    for (int i = 0; (inLen < 0 || i < inLen) && in[i]; i++) {
        unsigned int c = in[i];
        if (c >= 0xD800 && c <= 0xDBFF && (inLen < 0 || i + 1 < inLen) && in[i + 1] >= 0xDC00 && in[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (in[i + 1] - 0xDC00);
            i++;
        } else if (c >= 0xD800 && c <= 0xDFFF)
            c = '?';
        int len = c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
        if (n + len > cap - 1)
            break;
        if (len == 1)
            out[n++] = (char)c;
        else if (len == 2) {
            out[n++] = (char)(0xC0 | (c >> 6));
            out[n++] = (char)(0x80 | (c & 0x3F));
        } else if (len == 3) {
            out[n++] = (char)(0xE0 | (c >> 12));
            out[n++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[n++] = (char)(0x80 | (c & 0x3F));
        } else {
            out[n++] = (char)(0xF0 | (c >> 18));
            out[n++] = (char)(0x80 | ((c >> 12) & 0x3F));
            out[n++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[n++] = (char)(0x80 | (c & 0x3F));
        }
    }
    out[n] = '\0';
}

// Seconds with one decimal, as "12.3".
void Secs(char* out, DWORD ms) {
    wsprintfA(out, "%u.%u", ms / 1000, (ms % 1000) / 100);
}

// ---------------------------------------------------------------------------------------------------------------------
// Process facts, read with syscalls into static buffers.

bool ReadMem(DWORD address, void* out, DWORD size) {
    SIZE_T got = 0;
    return ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(ULONG_PTR)address, out, size, &got) && got == size;
}

void MappedName(DWORD base, char* out, int cap) {
    out[0] = '\0';
    SIZE_T returned = 0;
    if (!g_ntQueryVirtualMemory ||
        g_ntQueryVirtualMemory(GetCurrentProcess(), (PVOID)(ULONG_PTR)base, 2 /* MemoryMappedFilenameInformation */, g_mappedName,
                               sizeof(g_mappedName) - sizeof(wchar_t), &returned) < 0)
        return;
    NtString* s = (NtString*)g_mappedName;
    int len = s->Length / sizeof(wchar_t);
    const wchar_t* name = s->Buffer;
    for (int i = 0; i < len; i++)
        if (s->Buffer[i] == L'\\')
            name = s->Buffer + i + 1;
    Narrow(out, cap, name, len - (int)(name - s->Buffer));
}

// The images in the address space (the first kMaxModules), found by walking it: the loader's own module list needs the
// loader lock. An image ends where the kernel's regions of its allocation end, not at its header's SizeOfImage, which
// the module can rewrite.
void BuildModules(void) {
    g_modCount = 0;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD address = 0x10000;
    DWORD top = (DWORD)(ULONG_PTR)si.lpMaximumApplicationAddress;
    while (address < top) {
        MEMORY_BASIC_INFORMATION m;
        if (!VirtualQuery((LPCVOID)(ULONG_PTR)address, &m, sizeof(m)))
            break;
        DWORD base = (DWORD)(ULONG_PTR)m.BaseAddress;
        DWORD next = base + (DWORD)m.RegionSize;
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS32 nt;
        if (g_modCount && m.AllocationBase == (PVOID)(ULONG_PTR)g_mods[g_modCount - 1].base && next > g_mods[g_modCount - 1].end)
            g_mods[g_modCount - 1].end = next;
        else if (g_modCount < kMaxModules && m.Type == MEM_IMAGE && m.BaseAddress == m.AllocationBase && ReadMem(base, &dos, sizeof(dos)) &&
                 dos.e_magic == IMAGE_DOS_SIGNATURE &&
                 ReadMem(base + dos.e_lfanew, &nt, sizeof(nt)) && nt.Signature == IMAGE_NT_SIGNATURE) {
            ModuleInfo& mi = g_mods[g_modCount++];
            mi.base = base;
            mi.end = next;
            mi.stamp = nt.FileHeader.TimeDateStamp;
            MappedName(base, mi.name, sizeof(mi.name));
            if (!mi.name[0])
                lstrcpyA(mi.name, "?");
        }
        if (next <= address)
            break;
        address = next;
    }
}

int FindModule(DWORD address) {
    for (int i = 0; i < g_modCount; i++)
        if (address >= g_mods[i].base && address < g_mods[i].end)
            return i;
    return -1;
}

void ModOff(char* out, DWORD address) {
    int i = FindModule(address);
    if (i < 0)
        wsprintfA(out, "0x%08X", address);
    else
        wsprintfA(out, "%s+0x%X", g_mods[i].name, address - g_mods[i].base);
}

// True when the byte before address is executable code: a pointer into a module's data is no return address, however its
// bytes read.
bool ExecutableBefore(DWORD address) {
    MEMORY_BASIC_INFORMATION m;
    const DWORD exec = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    return VirtualQuery((LPCVOID)(ULONG_PTR)(address - 1), &m, sizeof(m)) && m.State == MEM_COMMIT && (m.Protect & exec) &&
           !(m.Protect & PAGE_GUARD);
}

// 1 for a direct call (target set), 2 for an indirect one, 0 when the bytes before address are no call: a return
// address on the stack follows the call that pushed it.
int LooksLikeCall(DWORD address, DWORD* target) {
    BYTE b[7];
    if (address < 7 || !ExecutableBefore(address) || !ReadMem(address - 7, b, sizeof(b)))
        return 0;
    if (b[2] == 0xE8) {
        *target = address + *(LONG*)&b[3];
        return 1;
    }
    // FF /2 in each length: b[k] is the FF, b[k + 1] its ModRM.
    if (b[5] == 0xFF && (b[6] & 0x38) == 0x10 && ((b[6] & 0xC0) == 0xC0 || ((b[6] & 0xC0) == 0 && (b[6] & 7) != 4 && (b[6] & 7) != 5)))
        return 2;
    if (b[4] == 0xFF && (b[5] & 0x38) == 0x10 &&
        (((b[5] & 0xC0) == 0x40 && (b[5] & 7) != 4) || ((b[5] & 0xC0) == 0 && (b[5] & 7) == 4 && (b[6] & 7) != 5)))
        return 2;
    if (b[3] == 0xFF && (b[4] & 0xC7) == 0x44 && (b[4] & 0x38) == 0x10)
        return 2;
    if (b[1] == 0xFF && (b[2] & 0x38) == 0x10 && ((b[2] & 0xC7) == 0x05 || ((b[2] & 0xC0) == 0x80 && (b[2] & 7) != 4)))
        return 2;
    if (b[0] == 0xFF && (b[1] & 0x38) == 0x10 && ((b[1] & 0xC7) == 0x84 || ((b[1] & 0xC7) == 0x04 && (b[2] & 7) == 5)))
        return 2;
    return 0;
}

const ULONGLONG kCpuUnknown = ~0ULL;

// The thread's kernel + user time in ms, or kCpuUnknown.
ULONGLONG ThreadCpuMs(HANDLE h) {
    FILETIME created, exited, kernel, user;
    if (!h || !GetThreadTimes(h, &created, &exited, &kernel, &user))
        return kCpuUnknown;
    ULARGE_INTEGER k, u;
    k.LowPart = kernel.dwLowDateTime;
    k.HighPart = kernel.dwHighDateTime;
    u.LowPart = user.dwLowDateTime;
    u.HighPart = user.dwHighDateTime;
    return (k.QuadPart + u.QuadPart) / 10000;
}

// CPU used between two samples, or kCpuUnknown when either is.
ULONGLONG CpuDelta(ULONGLONG from, ULONGLONG to) {
    return from == kCpuUnknown || to == kCpuUnknown || to < from ? kCpuUnknown : to - from;
}

// CPU seconds as "12.3", "?" when unknown; whole seconds go through a DWORD (136 years).
void SecsCpu(char* out, ULONGLONG ms) {
    if (ms == kCpuUnknown)
        lstrcpyA(out, "?");
    else
        wsprintfA(out, "%u.%u", (DWORD)(ms / 1000), (DWORD)((ms % 1000) / 100));
}

void ReadLocks(void) {
    for (int i = 0; i < g_lockCount; i++) {
        CRITICAL_SECTION cs;
        LockState& st = g_lockStates[i];
        st.read = ReadMem((DWORD)(ULONG_PTR)g_locks[i].cs, &cs, sizeof(cs));
        st.owner = st.read ? (DWORD)(ULONG_PTR)cs.OwningThread : 0;
        st.lockCount = st.read ? cs.LockCount : 0;
        st.recursion = st.read ? cs.RecursionCount : 0;
    }
}

// Two LockCount encodings: from Windows Server 2003 SP1 on, bit 0 clear = locked and waiters in the bits above it; on
// 2000, XP and Server 2003 RTM, -1 = free and each Enter, the owner's recursive ones included, adds one.
bool LockHeld(const LockState& st) {
    if (!st.read || st.owner == 0)
        return false;
    return g_oldLockCount ? st.lockCount >= 0 : !(st.lockCount & 1);
}

LONG LockWaiters(const LockState& st) {
    if (!LockHeld(st))
        return 0;
    if (g_oldLockCount) {
        LONG waiters = st.lockCount - (st.recursion - 1);
        return waiters > 0 ? waiters : 0;
    }
    return ((-1) - st.lockCount) >> 2;
}

struct ThreadBasicInfo { // THREAD_BASIC_INFORMATION, x86
    LONG exitStatus;
    PVOID teb;
    HANDLE process; // CLIENT_ID.UniqueProcess
    HANDLE thread;
    ULONG_PTR affinity;
    LONG priority;
    LONG basePriority;
};

// False when the owner can't be read too: a thread that isn't provably this process's is never suspended.
bool ThreadInThisProcess(HANDLE h) {
    ThreadBasicInfo tbi;
    if (!g_ntQueryInformationThread || g_ntQueryInformationThread(h, 0 /* ThreadBasicInformation */, &tbi, sizeof(tbi), NULL) < 0)
        return false;
    return (DWORD)(ULONG_PTR)tbi.process == GetCurrentProcessId();
}

// "game", a registered name, or the thread's start address.
void Label(char* out, int cap, DWORD tid) {
    if (tid == g_gameTid) {
        lstrcpyA(out, "game");
        return;
    }
    for (int i = 0; i < kNameSlots; i++) {
        if ((DWORD)g_names[i].tid != tid)
            continue;
        wchar_t copy[48];
        CopyMemory(copy, g_names[i].name, sizeof(copy));
        copy[47] = L'\0';
        MemoryBarrier();
        if ((DWORD)g_names[i].tid == tid) {
            Narrow(out, cap, copy, -1);
            return;
        }
    }
    out[0] = '\0';
    HANDLE h = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);
    if (!h)
        return;
    DWORD start = 0;
    if (g_ntQueryInformationThread && g_ntQueryInformationThread(h, 9 /* ThreadQuerySetWin32StartAddress */, &start, sizeof(start), NULL) >= 0 &&
        start) {
        char where[96];
        ModOff(where, start);
        wsprintfA(out, "start %s", where);
    }
    CloseHandle(h);
}

// Holds one thread at a time and calls only syscalls while it is held: the thread may own the process heap, a CRT lock
// or the loader lock, and waiting on any of them here would leave it suspended for good.
void CaptureThread(HANDLE h, Capture* c, BYTE* buf, DWORD cap) {
    c->held = FALSE;
    c->ctxOk = FALSE;
    c->got = 0;
    c->err = 0;
    c->prevSuspend = 0;
    DWORD prev = SuspendThread(h);
    if (prev == (DWORD)-1) {
        c->err = GetLastError();
        return;
    }
    c->held = TRUE;
    c->prevSuspend = prev;
    __try {
        c->ctx.ContextFlags = CONTEXT_FULL;
        BOOL ctxOk = GetThreadContext(h, &c->ctx); // also waits for the suspend to take effect
        c->ticksDone = g_ticksDone;                 // so this count is the one its registers belong to
        if (ctxOk) {
            c->ctxOk = TRUE;
            MEMORY_BASIC_INFORMATION m;
            // Never a guard page: reading one would consume the thread's stack-growth guard, and its next growth would
            // fault.
            if (VirtualQuery((LPCVOID)(ULONG_PTR)c->ctx.Esp, &m, sizeof(m)) && m.State == MEM_COMMIT &&
                !(m.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
                DWORD top = (DWORD)(ULONG_PTR)m.BaseAddress + (DWORD)m.RegionSize;
                DWORD n = top - c->ctx.Esp;
                if (n > cap)
                    n = cap;
                SIZE_T got = 0;
                ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(ULONG_PTR)c->ctx.Esp, buf, n, &got); // a partial copy is kept
                c->got = (DWORD)got;
            }
        } else
            c->err = GetLastError();
    } __finally {
        ResumeThread(h);
    }
}

// Return addresses found by scanning the copied stack: D2BS builds with /Ox, which drops frame pointers, so the EBP
// chain alone misses most frames. Entries on the EBP chain are marked '*'; stale entries are possible.
void Frames(Writer& w, const Capture& c, const BYTE* buf, int maxFrames) {
    DWORD chain[64];
    int chainLen = 0;
    DWORD lo = c.ctx.Esp, ebp = c.ctx.Ebp;
    // Offsets, not end addresses: EBP is a general register under /Ox, and a value near 0xFFFFFFFF would wrap.
    while (chainLen < 64 && c.got >= 8 && ebp >= lo && ebp - lo <= c.got - 8) {
        chain[chainLen++] = ebp + 4 - lo;
        DWORD next = *(const DWORD*)(buf + (ebp - lo));
        if (next <= ebp)
            break;
        ebp = next;
    }
    Line(w, false, "frames (stack scan, stale entries possible; * = on the EBP chain):");
    int frames = 0;
    for (DWORD off = 0; off + 4 <= c.got && frames < maxFrames; off += 4) {
        DWORD v = *(const DWORD*)(buf + off);
        DWORD target = 0;
        if (FindModule(v) < 0)
            continue;
        int kind = LooksLikeCall(v, &target);
        if (!kind)
            continue;
        bool onChain = false;
        for (int i = 0; i < chainLen && !onChain; i++)
            onChain = chain[i] == off;
        char at[96], to[96];
        ModOff(at, v);
        if (kind == 1 && FindModule(target) >= 0) {
            ModOff(to, target);
            Line(w, false, "%cesp+0x%04X %s (calls %s)", onChain ? '*' : ' ', off, at, to);
        } else
            Line(w, false, "%cesp+0x%04X %s", onChain ? '*' : ' ', off, at);
        frames++;
    }
    if (!frames)
        Line(w, false, "no return addresses recognized in the %u bytes copied", c.got);
}

void ThreadBlock(Writer& w, int n, DWORD tid, HANDLE h, const Capture& c, const BYTE* buf, int maxFrames) {
    char label[112];
    Label(label, sizeof(label), tid);
    char cpuText[24];
    SecsCpu(cpuText, ThreadCpuMs(h));
    if (!c.held) {
        Line(w, true, "Hang report %d: thread %u (%s) could not be held (error %u)", n, tid, label, c.err);
        return;
    }
    Line(w, true, "Hang report %d: thread %u (%s) CPU %s s, suspended before capture %u", n, tid, label, cpuText, c.prevSuspend);
    if (!c.ctxOk) {
        Line(w, false, "no registers (error %u)", c.err);
        return;
    }
    char eip[96];
    ModOff(eip, c.ctx.Eip);
    Line(w, false, "EIP 0x%08X %s ESP 0x%08X EBP 0x%08X EFL 0x%08X", c.ctx.Eip, eip, c.ctx.Esp, c.ctx.Ebp, c.ctx.EFlags);
    Line(w, false, "EAX 0x%08X EBX 0x%08X ECX 0x%08X EDX 0x%08X ESI 0x%08X EDI 0x%08X", c.ctx.Eax, c.ctx.Ebx, c.ctx.Ecx, c.ctx.Edx,
         c.ctx.Esi, c.ctx.Edi);
    Line(w, false, "stack 0x%08X, 0x%X bytes read", c.ctx.Esp, c.got);
    Frames(w, c, buf, maxFrames);
}

const char* StateText(LONG state) {
    switch (state) {
    case ClientStateMenu:
        return "at the menu";
    case ClientStateInGame:
        return "in game";
    case ClientStateBusy:
        return "while loading";
    default:
        return "with no game or menu";
    }
}

// A completed tick past the stall's count: the episode's end, as the watchdog reads it. A tick still inside HangTick
// does not count, as its thread may be stuck there.
bool RanSince(LONG done, LONG stallTicks) {
    return done - stallTicks > 0;
}

// A lock held by another thread whose address sits near the top of the game thread's stack: most likely the one it waits
// on (EnterCriticalSection's argument), or -1.
int LockOnGameStack(void) {
    const Capture& g = g_gameCap;
    if (!g.ctxOk)
        return -1;
    for (DWORD off = 0; off + 4 <= g.got && off < 512; off += 4) {
        DWORD v = *(const DWORD*)(g_gameStack + off);
        for (int i = 0; i < g_lockCount; i++)
            if ((DWORD)(ULONG_PTR)g_locks[i].cs == v && LockHeld(g_lockStates[i]) && g_lockStates[i].owner != g_gameTid)
                return i;
    }
    return -1;
}

// First match wins; each is a reading of the evidence, printed as one.
void Diagnose(Writer& w, int n, ULONGLONG cpuMs, DWORD windowMs) {
    const Capture& g = g_gameCap;
    char label[112], busy[24], window[24];
    SecsCpu(busy, cpuMs);
    Secs(window, windowMs);
    // The hang ended between the episode's start and the capture: what was read is the recovered game, not the hang.
    if (g.held && RanSince(g.ticksDone, g_episodeTicks)) {
        Line(w, true, "Hang report %d: Diagnosis: none, the game thread ran again before it was read; its block below is from after the hang",
             n);
        return;
    }
    if (!g_capWindowHung) {
        Line(w, true, "Hang report %d: Diagnosis: none, the game window responded again before the game thread was read", n);
        return;
    }
    if (g.held && g.prevSuspend > 0) {
        Line(w, true, "Hang report %d: Diagnosis: the game thread was already suspended (suspend count %u)", n, g.prevSuspend);
        return;
    }
    int onStack = LockOnGameStack();
    if (onStack >= 0) {
        Label(label, sizeof(label), g_lockStates[onStack].owner);
        Line(w, true, "Hang report %d: Diagnosis: the game thread is probably waiting for the %s lock, held by thread %u (%s); %d waiting", n,
             g_locks[onStack].name, g_lockStates[onStack].owner, label, LockWaiters(g_lockStates[onStack]));
        return;
    }
    // Another thread can hold the game lock while the game thread has released it to wait on something else (a script
    // event's reply), so without the stack evidence above this is an observation, not the cause.
    int game = g_gameLock;
    if (game >= 0 && LockHeld(g_lockStates[game]) && g_lockStates[game].owner != g_gameTid) {
        Label(label, sizeof(label), g_lockStates[game].owner);
        Line(w, true, "Hang report %d: Diagnosis: the game lock is held by thread %u (%s), %d waiting", n, g_lockStates[game].owner, label,
             LockWaiters(g_lockStates[game]));
        return;
    }
    int loader = g_loaderLock;
    if (loader >= 0 && LockHeld(g_lockStates[loader]) && g_lockStates[loader].owner != g_gameTid) {
        Label(label, sizeof(label), g_lockStates[loader].owner);
        Line(w, true, "Hang report %d: Diagnosis: the loader lock is held by thread %u (%s)", n, g_lockStates[loader].owner, label);
        return;
    }
    if (cpuMs == kCpuUnknown) {
        Line(w, true, "Hang report %d: Diagnosis: unknown, the game thread's CPU time could not be read", n);
        return;
    }
    // Half, not all: a spinning thread on a machine with more games than cores gets only part of a core. Below half, CPU
    // time can't tell a wait from a thread starved of CPU, so the line says both.
    if (windowMs && cpuMs * 2 >= (ULONGLONG)windowMs)
        Line(w, true, "Hang report %d: Diagnosis: busy, the game thread used %s s of CPU in the last %s s", n, busy, window);
    else
        Line(w, true, "Hang report %d: Diagnosis: waiting or starved of CPU, the game thread used %s s of CPU in the last %s s", n, busy, window);
}

void LocksLine(Writer& w, int n) {
    char text[1000];
    int len = wsprintfA(text, "Hang report %d: locks:", n);
    for (int i = 0; i < g_lockCount && len < 900; i++) {
        const LockState& st = g_lockStates[i];
        bool always = i == g_gameLock || i == g_loaderLock;
        if (!LockHeld(st) && !always)
            continue;
        if (!st.read)
            len += wsprintfA(text + len, " %s ?;", g_locks[i].name);
        else if (!LockHeld(st))
            len += wsprintfA(text + len, " %s free;", g_locks[i].name);
        else
            len += wsprintfA(text + len, " %s held by %u%s, recursion %d, %d waiting;", g_locks[i].name, st.owner,
                             st.owner == g_gameTid ? " (game)" : "", st.recursion, LockWaiters(st));
    }
    Line(w, true, "%s", text);
}

void ModulesBlock(Writer& w, int n) {
    Line(w, true, "Hang report %d: modules (base size timestamp name):", n);
    for (int i = 0; i < g_modCount; i++)
        Line(w, false, "0x%08X 0x%08X 0x%08X %s", g_mods[i].base, g_mods[i].end - g_mods[i].base, g_mods[i].stamp, g_mods[i].name);
}

bool MustStop(void) {
    return g_quiesced || WaitForSingleObject(g_hStop, 0) == WAIT_OBJECT_0;
}

// ---------------------------------------------------------------------------------------------------------------------
// The dump helper: created at arm time, so no thread is created while the game is stuck.

BOOL CALLBACK HangDumpCallback(PVOID fromSnapshot, PMINIDUMP_CALLBACK_INPUT input, PMINIDUMP_CALLBACK_OUTPUT output) {
    if (input && output) {
        if ((int)input->CallbackType == kIsProcessSnapshotCallback && fromSnapshot)
            output->Status = S_FALSE;
        else if (input->CallbackType == ReadMemoryFailureCallback)
            output->Status = S_OK;
        else if (input->CallbackType == CancelCallback) {
            // A crash or halt report needs dbghelp, which this dump holds: it gives way.
            output->CheckCancel = TRUE;
            output->Cancel = g_quiesced ? TRUE : FALSE;
        }
    }
    return TRUE;
}

// Each handle leaves the job just before its own release, and a fault in one release (a damaged snapshot) still lets the
// others run: a release is never repeated, and none is skipped.
// A release that faults or fails turns hang dumps off for the process: the snapshot it held may live on, and another
// dump would add one more.
void ReleaseDumpJob(DumpJob& job) {
    bool ok = true;
    HANDLE h = job.marker;
    job.marker = NULL;
    __try {
        if (h && g_pssMarkerFree(h) != ERROR_SUCCESS)
            ok = false;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    h = job.snapshot;
    job.snapshot = NULL;
    __try {
        if (h && g_pssFree(GetCurrentProcess(), h) != ERROR_SUCCESS)
            ok = false;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    h = job.file;
    job.file = NULL;
    __try {
        if (h && !CloseHandle(h))
            ok = false;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok)
        InterlockedExchange(&g_dumpOff, 1);
}

// The game thread's registers from the snapshot itself: the report's own sample can be seconds older, and a game thread that
// moved since (a busy loop reuses its stack without a tick) would pair that sample with newer stack memory.
void SnapshotGameContext(DumpJob& job) {
    job.hasCtx = FALSE;
    if (!g_pssWalk || !g_pssMarkerCreate || g_pssMarkerCreate(NULL, &job.marker) != ERROR_SUCCESS) {
        job.marker = NULL;
        return;
    }
    PssThreadEntry entry;
    while (g_pssWalk(job.snapshot, kPssWalkThreads, job.marker, &entry, sizeof(entry)) == ERROR_SUCCESS) {
        if (entry.threadId != job.gameTid || !entry.contextRecord)
            continue;
        DWORD size = entry.sizeOfContextRecord < sizeof(CONTEXT) ? entry.sizeOfContextRecord : sizeof(CONTEXT);
        ZeroMemory(&job.ctx, sizeof(job.ctx));
        CopyMemory(&job.ctx, entry.contextRecord, size);
        job.rec.ExceptionAddress = (PVOID)(ULONG_PTR)job.ctx.Eip;
        job.hasCtx = TRUE;
        break;
    }
}

// Snapshot only: a live in-process dump suspends threads and allocates, and a wedge there would freeze the game for
// good. No timeout kill either: a stuck dump costs this helper, and dbghelp only while MiniDumpWriteDump runs (later
// stack walks and crash dumps then wait out their bound and are skipped).
void WriteHangDump(void) {
    DumpJob& job = g_dumpJob;
    Writer& w = g_dumpW;
    __try {
        DWORD started = GetTickCount();
        // Whether the dump is still wanted, before anything is pruned for it.
        if (MustStop()) {
            Line(w, true, "Hang report %d: no dump, D2BS is shutting down or the game is crashing", job.episode);
            return;
        }
        // A game that ran again, or whose window responds again, is no longer hung: a dump of it would use up the
        // process's one dump.
        if (RanSince(g_ticksDone, job.ticks)) {
            Line(w, true, "Hang report %d: no dump, the game thread ran again before the snapshot", job.episode);
            return;
        }
        if (!IsHungAppWindow(g_hwnd)) {
            Line(w, true, "Hang report %d: no dump, the game window responds again", job.episode);
            return;
        }
        // In the job, not locals, so a fault inside dbghelp still releases them (HangDumpThread).
        job.snapshot = NULL;
        job.marker = NULL;
        DWORD error = g_pssCapture(GetCurrentProcess(), kHangSnapshotFlags, CONTEXT_ALL, &job.snapshot);
        if (error != ERROR_SUCCESS) {
            job.snapshot = NULL;
            Line(w, true, "Hang report %d: no dump, the process snapshot failed (%u)", job.episode, error);
            return;
        }
        // The same three, once the snapshot is held: it is the hang only if nothing changed up to here.
        if (MustStop() || RanSince(g_ticksDone, job.ticks) || !IsHungAppWindow(g_hwnd)) {
            ReleaseDumpJob(job);
            Line(w, true, "Hang report %d: no dump, %s", job.episode,
                 MustStop() ? "D2BS is shutting down or the game is crashing" : "the game recovered during or just after the snapshot");
            return;
        }
        // Old dumps are pruned only once the snapshot passed every recovery check, so a recovered game never costs a kept one.
        PruneCrashFiles(L"hang-*.dmp", kHangDumpsKept - 1);
        PruneCrashFiles(L"hang-*.dmp.partial", 0);
        ULARGE_INTEGER freeBytes;
        if (!GetDiskFreeSpaceExW(Vars.szLogPath, &freeBytes, NULL, NULL)) {
            error = GetLastError();
            ReleaseDumpJob(job);
            Line(w, true, "Hang report %d: no dump, the free space in the logs folder could not be read (%u)", job.episode, error);
            return;
        }
        if (freeBytes.QuadPart < kDumpMinFreeBytes) {
            ReleaseDumpJob(job);
            Line(w, true, "Hang report %d: no dump, less than 2 GB free in the logs folder", job.episode);
            return;
        }
        SnapshotGameContext(job);
        job.file = CreateFileW(job.partial, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (job.file == INVALID_HANDLE_VALUE) {
            error = GetLastError();
            job.file = NULL;
            ReleaseDumpJob(job);
            Line(w, true, "Hang report %d: no dump, the file could not be created (%u)", job.episode, error);
            return;
        }
        // dbghelp only for the dump itself; bounded, as another walker may be the stuck one.
        if (!DbgHelpLock(5000)) {
            ReleaseDumpJob(job);
            DeleteFileW(job.partial);
            Line(w, true, "Hang report %d: no dump, a stack walk or another dump kept dbghelp busy for 5 s", job.episode);
            return;
        }
        if (MustStop()) {
            // A crash or halt began meanwhile: its own report gets dbghelp.
            DbgHelpUnlock();
            ReleaseDumpJob(job);
            DeleteFileW(job.partial);
            Line(w, true, "Hang report %d: no dump, D2BS is shutting down or the game is crashing", job.episode);
            return;
        }
        EXCEPTION_POINTERS pointers = {&job.rec, &job.ctx};
        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = job.gameTid;
        info.ExceptionPointers = &pointers;
        info.ClientPointers = FALSE;
        MINIDUMP_CALLBACK_INFORMATION callback = {HangDumpCallback, job.snapshot};
        BOOL written = MiniDumpWriteDump(job.snapshot, GetCurrentProcessId(), job.file, kHangDumpType, job.hasCtx ? &info : NULL, NULL, &callback);
        error = written ? 0 : GetLastError();
        DbgHelpUnlock();
        ReleaseDumpJob(job);
        char path[MAX_PATH * 3]; // UTF-8 can take 3 bytes per character
        Narrow(path, sizeof(path), job.path, -1);
        bool cancelled = !written && error == (DWORD)HRESULT_FROM_WIN32(ERROR_CANCELLED);
        if (written && !MoveFileExW(job.partial, job.path, MOVEFILE_REPLACE_EXISTING)) {
            written = FALSE;
            error = GetLastError();
        }
        if (written) {
            InterlockedExchange(&g_dumpWritten, 1);
            Line(w, true, "Hang report %d: dump %s written from a snapshot in %u ms%s", job.episode, path, GetTickCount() - started,
                 job.hasCtx ? "" : "; it has no exception stream: select the game thread in the debugger");
        } else if (cancelled) {
            DeleteFileW(job.partial);
            Line(w, true, "Hang report %d: dump %s cancelled after %u ms: D2BS is shutting down or the game is crashing", job.episode, path,
                 GetTickCount() - started);
        } else {
            DeleteFileW(job.partial);
            Line(w, true, "Hang report %d: dump %s FAILED (0x%08X) after %u ms", job.episode, path, error, GetTickCount() - started);
        }
    } __finally {
        Flush(w);
    }
}

DWORD g_dumpFaultCode = 0;
DWORD g_dumpFaultAddress = 0;

int DumpFault(EXCEPTION_POINTERS* p) {
    g_dumpFaultCode = p->ExceptionRecord->ExceptionCode;
    g_dumpFaultAddress = (DWORD)(ULONG_PTR)p->ExceptionRecord->ExceptionAddress;
    return EXCEPTION_EXECUTE_HANDLER;
}

DWORD WINAPI HangDumpThread(LPVOID) {
    g_dumpTid = GetCurrentThreadId();
    HangNameThread(L"D2BS hang dump");
    // The first user32 call, while the game is healthy (see Arm): later ones only read the window's state.
    IsHungAppWindow(g_hwnd);
    // Go first: when Go and Stop are both set, the posted job is still taken (and skipped, as MustStop holds) and the
    // busy flag the watchdog set is cleared.
    HANDLE events[2] = {g_hDumpGo, g_hStop};
    while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0) {
        g_dumpW.len = 0;
        g_dumpW.lost = 0;
        if (MustStop()) {
            // A crash or halt began after the job was posted.
            Line(g_dumpW, true, "Hang report %d: no dump, D2BS is shutting down or the game is crashing", g_dumpJob.episode);
            Flush(g_dumpW);
        } else {
            __try {
                WriteHangDump();
            } __except (DumpFault(GetExceptionInformation())) {
                DbgHelpUnlock(); // a no-op unless the fault came inside MiniDumpWriteDump, which holds it
                ReleaseDumpJob(g_dumpJob); // contains its own faults: nothing above this thread catches
                DeleteFileW(g_dumpJob.partial);
                InterlockedExchange(&g_dumpOff, 1); // a fault inside dbghelp would likely come again
                g_dumpW.len = 0;
                Line(g_dumpW, true, "Hang report %d: no dump, stopped by exception 0x%08X at 0x%08X; no more hang dumps in this process",
                     g_dumpJob.episode, g_dumpFaultCode, g_dumpFaultAddress);
                Flush(g_dumpW);
            }
        }
        // Done before the flag is cleared, so the next claim never sees this job's Done.
        SetEvent(g_hDumpDone);
        InterlockedExchange(&g_dumpBusy, 0);
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// The report.

void PacketLine(Writer& w, int n) {
    BYTE opcode;
    char dir;
    DWORD tick;
    if (PacketInD2BS(g_gameTid, &opcode, &dir, &tick))
        Line(w, true, "Hang report %d: the game thread is still in D2BS with %s packet 0x%02X (since tick %u)", n,
             dir == 'R' ? "received" : "sent", opcode, tick);
}

// The packet log and the dump come after the text: they use the heap and the CRT, which the stuck thread may hold. The
// packet log is written on the watchdog (the dump runs on its helper, and the wait for it is bounded): if the packet log
// blocks for good, the report's text is on disk but its follow-ups and recovery line are lost.
void PacketsAndDump(Writer& w, int n, DWORD stallMs) {
    if (MustStop())
        return;
    wchar_t stem[MAX_PATH], path[MAX_PATH];
    if (!CrashReportStem(stem, _countof(stem), L"hang-")) {
        Line(w, true, "Hang report %d: no packet log, the logs path is too long", n);
        return;
    }
    char header[512], stall[24], narrowPath[MAX_PATH * 3];
    Secs(stall, stallMs);
    char profile[96];
    Narrow(profile, sizeof(profile), Vars.szProfile, -1);
    wsprintfA(header, "D2BS hang report %d: game thread %u, D2BS timer silent for %s s and the window not responding, profile %s, process %u", n, g_gameTid, stall, profile,
              GetCurrentProcessId());
    PruneCrashFiles(L"hang-*.packets.txt", kHangPacketLogsKept - 1);
    if (MustStop())
        return;
    wsprintfW(path, L"%s.packets.txt", stem);
    Narrow(narrowPath, sizeof(narrowPath), path, -1);
    if (WritePacketLog(path, header, false))
        Line(w, true, "Hang report %d: packet log %s", n, narrowPath);
    else
        Line(w, true, "Hang report %d: no packet log, %s could not be created (%u)", n, narrowPath, GetLastError());
    Flush(w);

    if (!Vars.bHaltDumps) {
        Line(w, true, "Hang report %d: no dump; set HaltDumps=true in d2bs.ini to write one", n);
        return;
    }
    if (!g_pssCapture) {
        Line(w, true, "Hang report %d: no dump, process snapshots need Windows 8.1 or later", n);
        return;
    }
    if (!g_hDumpThread) {
        Line(w, true, "Hang report %d: no dump, the dump thread could not be started", n);
        return;
    }
    // The single job slot is claimed first: the helper sets its result flags before it frees the slot, so a claim that
    // succeeds sees them. A job posted after a crash or shutdown began is skipped by the helper, and a running dump
    // cancels itself (HangDumpCallback).
    if (InterlockedCompareExchange(&g_dumpBusy, 1, 0) != 0) {
        Line(w, true, "Hang report %d: no dump, the earlier one is still being written", n);
        return;
    }
    if (g_dumpWritten || g_dumpOff) {
        InterlockedExchange(&g_dumpBusy, 0);
        if (g_dumpWritten)
            Line(w, true, "Hang report %d: no dump, one was already written for this process", n);
        else
            Line(w, true, "Hang report %d: no dump, an earlier attempt failed by exception or in its cleanup", n);
        return;
    }
    if (MustStop()) {
        InterlockedExchange(&g_dumpBusy, 0);
        Line(w, true, "Hang report %d: no dump, D2BS is shutting down or the game is crashing", n);
        return;
    }
    DumpJob& job = g_dumpJob;
    wsprintfW(job.path, L"%s.dmp", stem);
    wsprintfW(job.partial, L"%s.dmp.partial", stem);
    job.episode = n;
    job.gameTid = g_gameTid;
    job.hasCtx = FALSE; // set by the helper from the snapshot
    job.ticks = g_episodeTicks;
    ZeroMemory(&job.rec, sizeof(job.rec));
    job.rec.ExceptionCode = kHangDumpCode;
    Narrow(narrowPath, sizeof(narrowPath), job.path, -1);
    Line(w, true, "Hang report %d: writing dump %s", n, narrowPath);
    Flush(w);
    ResetEvent(g_hDumpDone);
    SetEvent(g_hDumpGo);
    HANDLE events[2] = {g_hStop, g_hDumpDone};
    DWORD waited = WaitForMultipleObjects(2, events, FALSE, kDumpWaitMs);
    if (waited == WAIT_TIMEOUT)
        Line(w, true, "Hang report %d: dump still being written after %u s; left to finish, the game is not stopped", n, kDumpWaitMs / 1000);
    else if (waited == WAIT_OBJECT_0 + 1)
        g_episodeDumpLost = g_dumpW.lost; // this job's own lines, set before Done
}

// Every episode gets the diagnosis, the locks, the game thread and the packet log; full adds the other threads and the
// module table (both bounded: kMaxThreads, kTextBudgetMs, kMaxModules), the part worth capping in a process that stalls
// again and again.
void Report(int n, DWORD stallMs, ULONGLONG cpu0, DWORD cpuStart, bool full) {
    Writer& w = g_w;
    w.len = 0;
    w.lost = 0;
    g_episodeDumpLost = 0;
    DWORD started = GetTickCount();
    int threads = 1, gone = 0, notHeld = 0, skipped = 0;
    if (MustStop())
        return;
    __try {
        char stall[24];
        Secs(stall, stallMs);
        Line(w, true,
             "Hang report %d: D2BS's timer has not run for %s s and Windows marks the game window hung; game thread %u, last tick %s, D2BS %s, "
             "report thread %u",
             n, stall, g_gameTid, StateText(g_tickState), g_version, GetCurrentThreadId());
        Flush(w); // a kill from here on still leaves the header

        if (MustStop())
            return;
        CaptureThread(g_hGame, &g_gameCap, g_gameStack, sizeof(g_gameStack));
        g_capWindowHung = IsHungAppWindow(g_hwnd);
        if (!g_gameCap.held)
            notHeld++;
        ULONGLONG cpu1 = ThreadCpuMs(g_hGame);
        DWORD cpuWindow = GetTickCount() - cpuStart; // from cpu0's sample to cpu1's
        ReadLocks();
        BuildModules();
        Diagnose(w, n, CpuDelta(cpu0, cpu1), cpuWindow);
        LocksLine(w, n);
        PacketLine(w, n);
        ThreadBlock(w, n, g_gameTid, g_hGame, g_gameCap, g_gameStack, kGameFrames);
        Flush(w);

        // The thread list is taken after the game thread's block is on disk: if toolhelp needs a lock the stuck thread
        // holds, the rest of this report and its follow-ups are lost, the part already written is not.
        if (MustStop())
            return;
        int count = 0;
        HANDLE snap = full ? CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0) : INVALID_HANDLE_VALUE;
        if (snap != INVALID_HANDLE_VALUE) {
            THREADENTRY32 te;
            te.dwSize = sizeof(te);
            DWORD pid = GetCurrentProcessId();
            if (Thread32First(snap, &te)) {
                do {
                    DWORD id = te.th32ThreadID;
                    if (te.th32OwnerProcessID != pid || id == g_gameTid || id == GetCurrentThreadId() || id == g_dumpTid)
                        continue;
                    if (count < (int)_countof(g_tids))
                        g_tids[count++] = te.th32ThreadID;
                    else
                        skipped++; // past the list's room: counted with the threads the limits leave out
                } while (Thread32Next(snap, &te));
            }
            CloseHandle(snap);
        } else if (full)
            Line(w, true, "Hang report %d: no thread list (error %u)", n, GetLastError());
        else
            Line(w, true, "Hang report %d: other threads and modules are written for the first %d hangs of a process only", n,
                 kFullReportsMax);

        for (int i = 0; i < count; i++) {
            DWORD tid = g_tids[i];
            if (tid == g_gameTid || tid == GetCurrentThreadId() || tid == g_dumpTid)
                continue;
            if (MustStop()) {
                Line(w, true, "Hang report %d: aborted, D2BS is shutting down or the game is crashing", n);
                Flush(w);
                return;
            }
            if (threads >= kMaxThreads || GetTickCount() - started > kTextBudgetMs) {
                skipped++;
                continue;
            }
            HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, tid);
            if (!h) {
                gone++;
                continue;
            }
            // The id was this process's when the list was taken; it may have been reused since by another process's
            // thread. The open handle pins it, so the owner read now holds.
            if (!ThreadInThisProcess(h)) {
                CloseHandle(h);
                gone++;
                continue;
            }
            CaptureThread(h, &g_otherCap, g_otherStack, sizeof(g_otherStack));
            if (!g_otherCap.held)
                notHeld++;
            ThreadBlock(w, n, tid, h, g_otherCap, g_otherStack, kOtherFrames);
            CloseHandle(h);
            threads++;
            Flush(w);
        }
        if (skipped)
            Line(w, true, "Hang report %d: %d threads not captured (time or thread limit reached)", n, skipped);
        if (full)
            ModulesBlock(w, n);
        Flush(w);

        PacketsAndDump(w, n, stallMs);
        if (MustStop())
            return; // stopped at a checkpoint: no summary for a report that did not finish
        Line(w, true, "Hang report %d: written in %u ms; threads held: %d, not held: %d, not opened or no longer this process's: %d%s", n,
             GetTickCount() - started, threads - notHeld, notHeld, gone,
             w.lost || g_episodeDumpLost ? "; some of it could not be written to the log" : "");
        Flush(w);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // Caught here, not in Fog's filter, which would turn the hang into a crash.
        w.len = 0;
        Line(w, true, "Hang report %d: stopped by exception 0x%08X; hang reports are off for this process", n, GetExceptionCode());
        Flush(w);
        InterlockedExchange(&g_off, 1);
    }
}

// The game thread alone, for follow-ups.
void ShortReport(int n, const char* what, DWORD stallMs, ULONGLONG cpuMs, DWORD windowMs) {
    if (MustStop())
        return;
    Writer& w = g_w;
    w.len = 0;
    __try {
        CaptureThread(g_hGame, &g_gameCap, g_gameStack, sizeof(g_gameStack));
        g_capWindowHung = IsHungAppWindow(g_hwnd);
        // Ran again, or its window responds, since the watchdog's check: no longer hung, and the watchdog's next poll
        // writes the recovery or "responds again" line.
        if (!g_capWindowHung || (g_gameCap.held && RanSince(g_gameCap.ticksDone, g_episodeTicks)))
            return;
        BuildModules();
        char stall[24], cpu[24], window[24], eip[96];
        Secs(stall, stallMs);
        SecsCpu(cpu, cpuMs);
        Secs(window, windowMs);
        if (g_gameCap.ctxOk)
            ModOff(eip, g_gameCap.ctx.Eip);
        else
            lstrcpyA(eip, "?");
        Line(w, true, "Hang report %d: %s after %s s: EIP %s, %s s of CPU in the last %s s", n, what, stall, eip, cpu, window);
        if (g_gameCap.ctxOk)
            Frames(w, g_gameCap, g_gameStack, 6);
        Flush(w);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        w.len = 0;
        Line(w, true, "Hang report %d: stopped by exception 0x%08X; hang reports are off for this process", n, GetExceptionCode());
        Flush(w);
        InterlockedExchange(&g_off, 1);
    }
}

int AddLock(const char* name, CRITICAL_SECTION* cs) {
    if (!cs || g_lockCount >= kMaxLocks)
        return -1;
    g_locks[g_lockCount].name = name;
    g_locks[g_lockCount].cs = cs;
    return g_lockCount++;
}

typedef LONG(NTAPI* RtlGetVersionFn)(OSVERSIONINFOW* info);

// At startup, from Startup inside DllMain: the loader lock is this thread's own there, so the module lookups can't
// block, while on the watchdog they could wait on a stuck thread that holds it.
void ResolveAtStart(void) {
    DWORD dirLen = GetFullPathNameW(Vars.szLogPath, _countof(g_logDir), g_logDir, NULL);
    if (!dirLen || dirLen >= _countof(g_logDir))
        lstrcpynW(g_logDir, Vars.szLogPath, _countof(g_logDir));
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    g_ntQueryVirtualMemory = (NtQueryVirtualMemoryFn)GetProcAddress(ntdll, "NtQueryVirtualMemory");
    g_ntQueryInformationThread = (NtQueryInformationThreadFn)GetProcAddress(ntdll, "NtQueryInformationThread");
    g_ntCreateFile = (NtCreateFileFn)GetProcAddress(ntdll, "NtCreateFile");
    g_pssCapture = (PssCaptureSnapshotFn)GetProcAddress(kernel32, "PssCaptureSnapshot");
    g_pssFree = (PssFreeSnapshotFn)GetProcAddress(kernel32, "PssFreeSnapshot");
    if (!g_pssFree)
        g_pssCapture = NULL;
    g_pssMarkerCreate = (PssWalkMarkerCreateFn)GetProcAddress(kernel32, "PssWalkMarkerCreate");
    g_pssMarkerFree = (PssWalkMarkerFreeFn)GetProcAddress(kernel32, "PssWalkMarkerFree");
    g_pssWalk = (PssWalkSnapshotFn)GetProcAddress(kernel32, "PssWalkSnapshot");
    if (!g_pssMarkerFree)
        g_pssMarkerCreate = NULL;
    Narrow(g_version, sizeof(g_version), D2BS_VERSION, -1);
    // RtlGetVersion, not GetVersion: a compatibility mode makes GetVersion report the emulated Windows.
    // The new encoding came with Server 2003 SP1, which XP x64 (5.2 workstation) is built on.
    RtlGetVersionFn rtlGetVersion = (RtlGetVersionFn)GetProcAddress(ntdll, "RtlGetVersion");
    OSVERSIONINFOEXW os;
    ZeroMemory(&os, sizeof(os));
    os.dwOSVersionInfoSize = sizeof(os);
    if (rtlGetVersion && rtlGetVersion((OSVERSIONINFOW*)&os) >= 0)
        g_oldLockCount = os.dwMajorVersion < 5 || (os.dwMajorVersion == 5 && os.dwMinorVersion <= 1) ||
                         (os.dwMajorVersion == 5 && os.dwMinorVersion == 2 && os.wServicePackMajor == 0 && os.wProductType != VER_NT_WORKSTATION);

    g_gameLock = AddLock("game", &Vars.cGameLoopSection);
    AddLock("script engine", &ScriptEngine::lock);
    AddLock("script list", ScriptEngine::ListLockForReport());
    AddLock("event", &Vars.cEventSection);
    AddLock("print", &Vars.cPrintSection);
    AddLock("console", &Vars.cConsoleSection);
    AddLock("room", &Vars.cRoomSection);
    AddLock("misc", &Vars.cMiscSection);
    AddLock("unit list", &Vars.cUnitListSection);
    AddLock("file", &Vars.cFileSection);
    AddLock("flush cache", &Vars.cFlushCacheSection);
    AddLock("screenhook", &Vars.cScreenhookSection);
    AddLock("screenhook list", Genhook::SectionForReport());
    AddLock("text hook", &Vars.cTextHookSection);
    AddLock("image hook", &Vars.cImageHookSection);
    AddLock("box hook", &Vars.cBoxHookSection);
    AddLock("frame hook", &Vars.cFrameHookSection);
    AddLock("line hook", &Vars.cLineHookSection);
    AddLock("D2Net packet", p_D2NET_CriticalPacketSection);
    AddLock("Storm read", p_STORM_ReadLock);
    // PEB.LoaderLock (x86 offset 0xA0) is undocumented, so it is used only when it points into ntdll.
    CRITICAL_SECTION* loader = *(CRITICAL_SECTION**)(ULONG_PTR)(__readfsdword(0x30) + 0xA0);
    MEMORY_BASIC_INFORMATION m;
    if (loader && VirtualQuery(loader, &m, sizeof(m)) && m.AllocationBase == (PVOID)ntdll)
        g_loaderLock = AddLock("loader", loader);
}

// While the game is healthy: the first user32 call converts this thread to a GUI thread, which can load modules under
// the loader lock. Its lines go through the watchdog's writer, not Log: the game may already be stuck holding the heap.
bool Arm(void) {
    g_hGame = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_gameTid);
    if (!g_hGame) {
        g_w.len = 0;
        Line(g_w, true, "Hang watch: off, the game thread could not be opened (%u)", GetLastError());
        Flush(g_w);
        return false;
    }
    IsWindow(g_hwnd);
    IsHungAppWindow(g_hwnd);

    if (Vars.bHaltDumps && g_pssCapture) {
        g_hDumpGo = CreateEventW(NULL, FALSE, FALSE, NULL);
        g_hDumpDone = CreateEventW(NULL, TRUE, FALSE, NULL);
        if (g_hDumpGo && g_hDumpDone)
            g_hDumpThread = CreateThread(NULL, 0, HangDumpThread, NULL, 0, NULL);
        if (!g_hDumpThread) {
            if (g_hDumpGo)
                CloseHandle(g_hDumpGo);
            if (g_hDumpDone)
                CloseHandle(g_hDumpDone);
            g_hDumpGo = g_hDumpDone = NULL;
        }
    }
    g_w.len = 0;
    Line(g_w, true, "Hang watch: on, game thread %u, reports when D2BS's timer has not run for %u s and Windows marks the window not responding", g_gameTid, Vars.dwHangReportMs / 1000);
    Flush(g_w);
    return true;
}

DWORD WINAPI HangWatchThread(LPVOID) {
    HangNameThread(L"D2BS hang watch");
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    const DWORD threshold = Vars.dwHangReportMs;
    bool armed = false, open = false, timerLineDone = false;
    LONG lastTicks = 0;
    DWORD stall = 0, lastWake = GetTickCount(), cpuStart = 0, openTickTime = 0, nextCheck = 0;
    ULONGLONG cpu0 = 0;
    int episodes = 0, followUps = 0;

    while (WaitForSingleObject(g_hStop, kPollMs) == WAIT_TIMEOUT) {
        DWORD now = GetTickCount();
        DWORD gap = now - lastWake;
        lastWake = now;
        if (g_off || g_quiesced || !Vars.bActive) {
            stall = 0;
            continue;
        }
        if (!armed) {
            if (!g_engineStarted || !g_ticks || !g_hwnd)
                continue;
            if (!Arm())
                break;
            armed = true;
            lastTicks = g_ticks;
            stall = 0;
            continue;
        }
        // In an open episode only a completed tick ends it, so the recovery line gets the resume time that tick stored;
        // a first tick still between its count and that store leaves the stall, and its follow-ups, running.
        LONG ticks = g_ticks;
        if (open && g_ticksDone - lastTicks <= 0)
            ticks = lastTicks;
        if (ticks != lastTicks) {
            lastTicks = ticks;
            if (open) {
                // The game thread's own resume time: a stall that ended while a report was being written is not
                // stretched to the report's end.
                // Only a resume strictly inside this episode counts; an older one is at or before openTickTime.
                DWORD resumed = g_resumeTime, took = now - openTickTime;
                if (resumed - openTickTime - 1 < took)
                    took = resumed - openTickTime;
                char tookText[24];
                Secs(tookText, took);
                g_w.len = 0;
                Line(g_w, true, "Hang report %d: the game thread ran again after %s s", episodes, tookText);
                Flush(g_w);
                open = false;
            }
            stall = 0;
            continue;
        }
        // The stall counts only time the watchdog itself ran: a debugger break, ProcDump or system sleep freezes it too.
        if (gap > kGapResetMs) {
            stall = 0;
            nextCheck = threshold; // an open episode's next follow-up counts from the restart too
            continue;
        }
        if (!stall) {
            cpu0 = ThreadCpuMs(g_hGame);
            cpuStart = now;
        }
        stall += gap;
        if (!IsWindow(g_hwnd)) {
            InterlockedExchange(&g_off, 1); // the game window is gone: the game is exiting
            g_w.len = 0;
            Line(g_w, true, "Hang watch: off, the game window is gone");
            Flush(g_w);
            continue;
        }
        if (open) {
            // Checked every threshold for as long as the episode lasts; follow-up reports stop after kFollowUpsMax.
            if (stall >= nextCheck) {
                nextCheck += threshold;
                char took[24];
                Secs(took, now - openTickTime);
                if (IsHungAppWindow(g_hwnd)) {
                    if (followUps < kFollowUpsMax) {
                        followUps++;
                        ShortReport(episodes, "still not responding", now - openTickTime, CpuDelta(cpu0, ThreadCpuMs(g_hGame)), now - cpuStart);
                    }
                } else {
                    // The window pumps again but D2BS's timer still does not run: the hang as Windows sees it is over.
                    g_w.len = 0;
                    Line(g_w, true, "Hang report %d: the game window responds again after %s s, but D2BS's timer has not run", episodes, took);
                    Flush(g_w);
                    open = false;
                }
                lastWake = GetTickCount(); // the report's own time is not a watchdog freeze
            }
            continue;
        }
        if (stall < threshold)
            continue;
        // Only what Windows marks hung, the closest local proxy for a bot manager's not-responding check (D2Bot#'s
        // Process.Responding): a starved or replaced timer alone is not.
        if (IsHungAppWindow(g_hwnd)) {
            // The last tick's time, then the count: HangTick stores the count before the time, so a time read here that
            // belongs to a newer tick comes with a changed count.
            // The episode's start is published before the count check, so HangTick measures a resume that comes right
            // after the check against this episode; a value left by a failed check is replaced at the next open.
            DWORD lastTickTime = g_tickTime;
            g_openTick = lastTickTime;
            MemoryBarrier();
            if (g_ticks != lastTicks)
                continue;
            episodes++;
            open = true;
            followUps = 0;
            nextCheck = stall + threshold;
            openTickTime = lastTickTime;
            g_episodeTicks = lastTicks;
            Report(episodes, now - openTickTime, cpu0, cpuStart, episodes <= kFullReportsMax);
            lastWake = GetTickCount();
        } else if (stall >= 3 * threshold && !timerLineDone) {
            timerLineDone = true;
            char quiet[24];
            Secs(quiet, stall);
            g_w.len = 0;
            Line(g_w, true, "Hang watch: D2BS's timer has not run for %s s while the game window still responds", quiet);
            Flush(g_w);
        }
    }
    InterlockedExchange(&g_exited, 1);
    return 0;
}
} // namespace

void HangStart(void) {
    if (!Vars.dwHangReportMs)
        return;
    ResolveAtStart();
    g_hStop = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!g_hStop)
        return;
    // Runs once DllMain returns; it arms once HangEngineStarted has run and the game thread has ticked.
    g_hWatch = CreateThread(NULL, 0, HangWatchThread, NULL, 0, &g_watchTid);
    if (!g_hWatch) {
        CloseHandle(g_hStop);
        g_hStop = NULL;
    }
}

void HangStop(HangStopMode mode) {
    InterlockedExchange(&g_quiesced, 1);
    if (!g_hWatch || mode == HangStopProcessExit)
        return;
    SetEvent(g_hStop);
    if (mode == HangStopJoin) {
        if (WaitForSingleObject(g_hWatch, kStopWaitMs) != WAIT_OBJECT_0)
            Log(L"Hang watch: did not stop within %u s", kStopWaitMs / 1000);
        return;
    }
    // Under the loader lock the watchdog can't be joined (its thread exit needs that lock): it is signalled and given up
    // to 1 s. Best effort only: a watchdog blocked on the heap or the loader lock outlives it, and g_exited is set a few
    // instructions before the thread leaves D2BS code; a full stop before unload is the unload work D2BS still lacks.
    DWORD started = GetTickCount();
    while (!g_exited && GetTickCount() - started < 1000)
        Sleep(10);
}

void HangTick(HWND hwnd) {
    if (!Vars.dwHangReportMs)
        return;
    DWORD now = GetTickCount();
    DWORD previous = g_tickTime;
    g_tickState = ClientState();
    if (!g_hwnd) {
        g_gameTid = GetCurrentThreadId();
        MemoryBarrier();
        g_hwnd = hwnd;
    }
    LONG tick = InterlockedIncrement(&g_ticks);
    // After the count, a full barrier, as the watchdog publishes g_openTick before reading the count: either it sees this
    // tick and opens no episode, or this sees the episode's start. Only the first resume inside the open episode is kept,
    // as a later stall and resume while the report is still being written would stretch it.
    if (now - previous > 1000 && g_resumeTime - g_openTick - 1 >= now - g_openTick)
        g_resumeTime = now;
    g_tickTime = now; // after the count: see the episode open in HangWatchThread
    InterlockedExchange(&g_ticksDone, tick);
}

void HangEngineStarted(void) {
    InterlockedExchange(&g_engineStarted, 1);
}

void HangQuiesce(void) {
    InterlockedExchange(&g_quiesced, 1);
}

void HangNameThread(const wchar_t* name) {
    if (!name)
        return;
    LONG self = (LONG)GetCurrentThreadId();
    for (int i = 0; i < kNameSlots; i++) {
        if (InterlockedCompareExchange(&g_names[i].tid, -1, 0) != 0)
            continue;
        lstrcpynW(g_names[i].name, name, _countof(g_names[i].name));
        MemoryBarrier();
        g_names[i].tid = self;
        return;
    }
}

void HangForgetThread(void) {
    LONG self = (LONG)GetCurrentThreadId();
    for (int i = 0; i < kNameSlots; i++)
        if (g_names[i].tid == self)
            InterlockedCompareExchange(&g_names[i].tid, 0, self);
}
