#pragma once

#include <windows.h>

// Initialize before installing hooks; the path is immutable while game threads use it.
void InitializeRevisionTempPath();
DWORD WINAPI GetRevisionTempPath(DWORD bufferLength, LPSTR buffer);
void RemoveRevisionTempDirectory();
