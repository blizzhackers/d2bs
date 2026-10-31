#include "RevisionTempPath.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s (Windows error %lu)\n", __LINE__, #expr, GetLastError()); return 1; } } while (0)

int main(int argc, char** argv) {
    CHECK(argc == 2);
    InitializeRevisionTempPath();
    if (strcmp(argv[1], "init") == 0)
        return 0;
    char path[MAX_PATH];
    if (strcmp(argv[1], "invalid-base") == 0) {
        DWORD length = GetRevisionTempPath(sizeof(path), path);
        // An unexpected success created a folder outside the test's TEMP; remove it before failing.
        if (length)
            RemoveRevisionTempDirectory();
        CHECK(length == 0);
        CHECK(path[0] == '\0');
        CHECK(GetLastError() != ERROR_SUCCESS);
        return 0;
    }
    if (strcmp(argv[1], "self") == 0) {
        DWORD length = GetRevisionTempPath(sizeof(path), path);
        CHECK(length > 0 && length == strlen(path));
        CHECK(path[length - 1] == '\\');
        char again[MAX_PATH];
        CHECK(GetRevisionTempPath(length + 1, again) == length);
        CHECK(strcmp(again, path) == 0);
        memset(again, 'x', sizeof(again));
        CHECK(GetRevisionTempPath(length, again) == 0);
        CHECK(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
        CHECK(again[0] == '\0' && again[length] == 'x');
        again[0] = 'x';
        CHECK(GetRevisionTempPath(0, again) == 0 && again[0] == 'x');
        CHECK(GetRevisionTempPath(sizeof(again), NULL) == 0);

        // Cleanup must leave a nonempty directory and its contents alone.
        char file[MAX_PATH];
        sprintf_s(file, "%srevision-probe.dll", path);
        HANDLE handle = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        CHECK(handle != INVALID_HANDLE_VALUE);
        CloseHandle(handle);
        RemoveRevisionTempDirectory();
        CHECK(GetFileAttributesA(file) != INVALID_FILE_ATTRIBUTES);
        CHECK(DeleteFileA(file));
        RemoveRevisionTempDirectory();
        CHECK(GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES);

        // An ordinary file where the directory should be must not become a usable path.
        path[length - 1] = '\0';
        handle = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        CHECK(handle != INVALID_HANDLE_VALUE);
        CloseHandle(handle);
        CHECK(GetRevisionTempPath(sizeof(again), again) == 0 && again[0] == '\0');
        CHECK(DeleteFileA(path));
        CHECK(GetRevisionTempPath(sizeof(again), again) == length);
        RemoveRevisionTempDirectory();
        puts("PASS: stable paths, buffer bounds, file collision, safe cleanup");
        return 0;
    }

    bool isolated = strcmp(argv[1], "isolated") == 0;
    CHECK(isolated || strcmp(argv[1], "shared") == 0);
    DWORD length = isolated ? GetRevisionTempPath(sizeof(path), path) : GetTempPathA(sizeof(path), path);
    CHECK(length > 0 && length < sizeof(path));
    char file[MAX_PATH];
    sprintf_s(file, "%srevision-probe.dll", path);
    // Match the game's extraction: CREATE_ALWAYS, GENERIC_WRITE, no sharing. Keep it open
    // until the parent has attempted the same filename from every concurrent client.
    HANDLE handle = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("BLOCKED %lu\n", GetLastError());
        fflush(stdout);
        return 2;
    }
    DWORD pid = GetCurrentProcessId(), written;
    CHECK(WriteFile(handle, &pid, sizeof(pid), &written, NULL) && written == sizeof(pid));
    printf("READY %s\n", file);
    fflush(stdout);
    CHECK(getchar() == '\n');
    CloseHandle(handle);
    handle = CreateFileA(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(handle != INVALID_HANDLE_VALUE);
    DWORD read = 0, owner = 0;
    CHECK(ReadFile(handle, &owner, sizeof(owner), &read, NULL) && read == sizeof(owner) && owner == pid);
    CloseHandle(handle);
    CHECK(DeleteFileA(file));
    if (isolated)
        RemoveRevisionTempDirectory();
    return 0;
}
