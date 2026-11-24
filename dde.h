#pragma once
#include <windows.h>
#include <ddeml.h>

BOOL SendDDE(int mode, char* pszDDEServer, char* pszTopic, char* pszItem, char* pszData, char* result, unsigned int size);
