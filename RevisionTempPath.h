#pragma once

#include <windows.h>

// Initialize before installing hooks; the path is immutable while game threads use it. It also
// removes empty folders that games killed before shutdown left behind.
void InitializeRevisionTempPath();
// GetTempPathA for the patched call site, except that a buffer too small for the path returns 0,
// not the required size: that caller treats any nonzero result as the path's length.
DWORD WINAPI GetRevisionTempPath(DWORD bufferLength, LPSTR buffer);
void RemoveRevisionTempDirectory();
