#include "RevisionTempPath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {
char revisionTempPath[MAX_PATH] = "";
DWORD revisionTempError = ERROR_PATH_NOT_FOUND;
const char revisionFolderPrefix[] = "d2bs-revision-";

bool ProcessIsGone(DWORD processId) {
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processId);
    // A running game denies other processes every right, so only this error means no such process.
    if (!process)
        return GetLastError() == ERROR_INVALID_PARAMETER;
    bool exited = WaitForSingleObject(process, 0) == WAIT_OBJECT_0;
    CloseHandle(process);
    return exited;
}

// A game killed before Shutdown leaves its folder behind. RemoveDirectoryA removes only empty
// folders, so one still holding an interrupted check's DLL stays.
void RemoveStaleRevisionFolders(const char* base) {
    char pattern[MAX_PATH];
    if (_snprintf_s(pattern, sizeof(pattern), _TRUNCATE, "%s%s*", base, revisionFolderPrefix) < 0)
        return;
    WIN32_FIND_DATAA found;
    HANDLE search = FindFirstFileA(pattern, &found);
    if (search == INVALID_HANDLE_VALUE)
        return;
    const size_t prefixLength = sizeof(revisionFolderPrefix) - 1;
    do {
        // The pattern can also match a short (8.3) name, so check the long name itself.
        if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
            _strnicmp(found.cFileName, revisionFolderPrefix, prefixLength) != 0)
            continue;
        const char* digits = found.cFileName + prefixLength;
        char* end;
        unsigned long processId = strtoul(digits, &end, 10);
        if (*digits < '0' || *digits > '9' || *end || processId == GetCurrentProcessId() || !ProcessIsGone(processId))
            continue;
        char path[MAX_PATH];
        if (_snprintf_s(path, sizeof(path), _TRUNCATE, "%s%s", base, found.cFileName) >= 0)
            RemoveDirectoryA(path);
    } while (FindNextFileA(search, &found));
    FindClose(search);
}
}

void InitializeRevisionTempPath() {
    revisionTempPath[0] = '\0';
    char base[MAX_PATH];
    DWORD length = GetTempPathA(sizeof(base), base);
    if (!length) {
        revisionTempError = GetLastError();
        return;
    }
    if (length >= sizeof(base)) {
        revisionTempError = ERROR_INSUFFICIENT_BUFFER;
        return;
    }
    RemoveStaleRevisionFolders(base);
    // Concurrent processes always have different PIDs, including duplicate profile launches.
    // Reusing a PID after exit is safe: the game overwrites its extracted revision DLL.
    if (_snprintf_s(revisionTempPath, sizeof(revisionTempPath), _TRUNCATE, "%sd2bs-revision-%lu\\", base, GetCurrentProcessId()) < 0) {
        revisionTempPath[0] = '\0';
        revisionTempError = ERROR_INSUFFICIENT_BUFFER;
    }
}

DWORD WINAPI GetRevisionTempPath(DWORD bufferLength, LPSTR buffer) {
    if (buffer && bufferLength)
        buffer[0] = '\0';
    if (!revisionTempPath[0]) {
        SetLastError(revisionTempError);
        return 0;
    }
    DWORD length = (DWORD)strlen(revisionTempPath);
    // This call site reserves room for the revision filename, but checks only for zero on
    // failure. Unlike GetTempPathA, never return a required size that it would use as a length.
    if (!buffer || length >= bufferLength) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    if (!CreateDirectoryA(revisionTempPath, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS) {
            SetLastError(error);
            return 0;
        }
        DWORD attributes = GetFileAttributesA(revisionTempPath);
        if (attributes == INVALID_FILE_ATTRIBUTES)
            return 0;
        if (!(attributes & FILE_ATTRIBUTE_DIRECTORY) || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            SetLastError(ERROR_DIRECTORY);
            return 0;
        }
    }
    memcpy(buffer, revisionTempPath, length + 1);
    return length;
}

void RemoveRevisionTempDirectory() {
    // The game removes the extracted DLL itself. Never delete contents, or another client's
    // directory; a folder left by a kill is removed by a later game's startup if it is empty.
    if (revisionTempPath[0])
        RemoveDirectoryA(revisionTempPath);
}
