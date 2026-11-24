#ifndef __FILE_H__
#define __FILE_H__

#include "js32.h"

char* readLine(FILE* fptr, bool locking);
bool writeValue(FILE* fptr, JSContext* cx, jsval value, bool isBinary, bool locking);
FILE* fileOpenRelScript(const wchar_t* filename, const wchar_t* mode, JSContext* cx);
wchar_t* getPathRelScript(const wchar_t* filename, int bufLen, wchar_t* fullPath);
bool isValidPath(const wchar_t* name);

// SpiderMonkey 24 strings hold at most 2^28 - 1 chars (JSString::MAX_LENGTH, not exported); a larger file can
// never become a script string, so it is refused before any allocation.
const long kMaxTextFileBytes = (1L << 28) - 1;

#endif
