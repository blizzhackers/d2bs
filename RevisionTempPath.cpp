#include "RevisionTempPath.h"
#include <stdio.h>
#include <string.h>

namespace {
char revisionTempPath[MAX_PATH] = "";
DWORD revisionTempError = ERROR_PATH_NOT_FOUND;
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
    // directory; an interrupted check can leave a nonempty directory for the OS/user to clean.
    if (revisionTempPath[0])
        RemoveDirectoryA(revisionTempPath);
}
