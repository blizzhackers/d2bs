/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <errno.h>
#include <windows.h>
#include <Shlwapi.h>
#include <new>

#include "JSFileTools.h"
#include "D2BS.h"
#include "File.h"
#include "Helpers.h"

using namespace std;

EMPTY_CTOR(filetools)

JSAPI_FUNC(filetools_remove) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply a file name");

    const wchar_t* file = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!file)
        return JS_FALSE; // JS_GetStringCharsZ reported the OOM
    wchar_t fullpath[_MAX_PATH + _MAX_FNAME];

    if (getPathRelScript(file, _MAX_PATH + _MAX_FNAME, fullpath) == NULL)
        THROW_ERROR(cx, "Invalid file name");

    // A missing file is a normal outcome here, so failure is a false result rather than an exception.
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(_wremove(fullpath) == 0));
    return JS_TRUE;
}

JSAPI_FUNC(filetools_rename) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply an original file name");
    if (argc < 2 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[1]))
        THROW_ERROR(cx, "You must supply a new file name");

    const wchar_t* orig = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    const wchar_t* newName = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[1]));
    if (!orig || !newName)
        return JS_FALSE; // JS_GetStringCharsZ reported the OOM

    wchar_t porig[_MAX_PATH + _MAX_FNAME];
    wchar_t pnewName[_MAX_PATH + _MAX_FNAME];

    if (getPathRelScript(orig, _MAX_PATH + _MAX_FNAME, porig) == NULL)
        THROW_ERROR(cx, "Invalid original file name");

    if (getPathRelScript(newName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL)
        THROW_ERROR(cx, "Invalid new file name");

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(_wrename(porig, pnewName) == 0));
    return JS_TRUE;
}

JSAPI_FUNC(filetools_copy) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply an original file name");
    if (argc < 2 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[1]))
        THROW_ERROR(cx, "You must supply a new file name");

    const wchar_t* orig = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    const wchar_t* newName = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[1]));
    if (!orig || !newName)
        return JS_FALSE; // JS_GetStringCharsZ reported the OOM
    wchar_t porig[_MAX_PATH + _MAX_FNAME];
    wchar_t pnewName[_MAX_PATH + _MAX_FNAME];
    bool overwrite = false;

    if (argc > 2 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[2]))
        overwrite = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[2]);

    if (getPathRelScript(orig, _MAX_PATH + _MAX_FNAME, porig) == NULL)
        THROW_ERROR(cx, "Invalid original file name");
    if (getPathRelScript(newName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL)
        THROW_ERROR(cx, "Invalid new file name");

    // Filesystem failures return false instead of throwing: scripts copy files that may be missing.
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    // Defense in depth: the source's read-share mode already refuses a write open of the same file, but a
    // self-copy must never reach _O_TRUNC. Both paths are GetFullPathNameW-normalized.
    if (_wcsicmp(porig, pnewName) == 0)
        return JS_TRUE;

    FILE* src = NULL;
    if (_wfopen_s(&src, porig, L"rb") != 0 || src == NULL)
        return JS_TRUE;

    // _O_EXCL makes "keep an existing target" atomic; _O_TRUNC replaces it only when the script asked.
    // Deny-all sharing, as master's _wfopen_s had: a second copy to the same target fails instead of interleaving.
    int fd = -1;
    if (_wsopen_s(&fd, pnewName, _O_WRONLY | _O_CREAT | _O_BINARY | (overwrite ? _O_TRUNC : _O_EXCL), _SH_DENYRW,
                  _S_IREAD | _S_IWRITE) != 0) {
        fclose(src);
        return JS_TRUE;
    }
    FILE* dst = _wfdopen(fd, L"wb");
    if (dst == NULL) {
        _close(fd);
        fclose(src);
        _wremove(pnewName);
        return JS_TRUE;
    }

    char buf[4096];
    bool ok = true;
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            ok = false;
            break;
        }
    }
    if (ferror(src))
        ok = false;
    if (fclose(dst) != 0) // fclose flushes; a failed flush means the target is incomplete
        ok = false;
    fclose(src);

    if (!ok)
        _wremove(pnewName); // a partial target must not look like a finished copy
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(ok));
    return JS_TRUE;
}

JSAPI_FUNC(filetools_exists) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid file name");
    const wchar_t* file = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    wchar_t fullpath[_MAX_PATH + _MAX_FNAME];

    if (getPathRelScript(file, _MAX_PATH + _MAX_FNAME, fullpath) == NULL)
        THROW_ERROR(cx, "Invalid file name");

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(!(_waccess(fullpath, 0) != 0 && errno == ENOENT)));

    return JS_TRUE;
}

JSAPI_FUNC(filetools_readText) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply a file name");
    const wchar_t* fname = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!fname)
        return JS_FALSE; // JS_GetStringCharsZ reported the OOM
    FILE* fptr = fileOpenRelScript(fname, L"r", cx);

    // If fileOpenRelScript failed, it already reported the error
    if (fptr == NULL)
        return JS_FALSE;

    long size;
    size_t readCount;
    char* contents;

    // ftell is -1 past 2 GB, so the size stays signed and is checked before it sizes a buffer
    if (fseek(fptr, 0, SEEK_END) != 0 || (size = ftell(fptr)) < 0 || size > kMaxTextFileBytes) {
        fclose(fptr);
        THROW_ERROR(cx, "File is too large to read as text");
    }
    rewind(fptr);

    contents = new (std::nothrow) char[size + 1];
    if (!contents) {
        fclose(fptr);
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }
    readCount = fread(contents, sizeof(char), size, fptr);
    contents[readCount] = 0;

    // Check to see if we had an error; ferror needs the stream open, so it comes before fclose, and the
    // message reads errno, so it is taken before any cleanup call can change it
    if (ferror(fptr)) {
        const char* error = _strerror("Read failed");
        fclose(fptr);
        delete[] contents;
        THROW_ERROR(cx, error);
    }
    fclose(fptr);

    int offset = 0;
    if (readCount > 2 && contents[0] == (char)0xEF && contents[1] == (char)0xBB && contents[2] == (char)0xBF) { // skip BOM
        offset = 3;
    }

    // Convert to JSVAL cleanup and return
    JS_BeginRequest(cx);
    wchar_t* wcontents = AnsiToUnicode(contents + offset);
    bool stored = StoreString(vp, JS_NewUCStringCopyN(cx, wcontents, wcslen(wcontents)));
    JS_EndRequest(cx);
    delete[] wcontents;
    delete[] contents;
    if (!stored)
        return JS_FALSE;
    return JS_TRUE;
}

JSAPI_FUNC(filetools_writeText) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply a file name");

    EnterCriticalSection(&Vars.cFileSection);

    const wchar_t* fname = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    FILE* fptr = fileOpenRelScript(fname, L"w", cx);
    bool result = true;

    // If fileOpenRelScript failed, it already reported the error
    if (fptr == NULL) {
        LeaveCriticalSection(&Vars.cFileSection);
        return JS_FALSE;
    }

    for (uint i = 1; i < argc; i++)
        if (!writeValue(fptr, cx, JS_ARGV(cx, vp)[i], false, true))
            result = false;

    fflush(fptr);
    fclose(fptr);

    LeaveCriticalSection(&Vars.cFileSection);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(result));
    return JS_TRUE;
}

JSAPI_FUNC(filetools_appendText) {
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "You must supply a file name");

    EnterCriticalSection(&Vars.cFileSection);

    const wchar_t* fname = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    FILE* fptr = fileOpenRelScript(fname, L"a+", cx);
    bool result = true;

    // If fileOpenRelScript failed, it already reported the error
    if (fptr == NULL) {
        LeaveCriticalSection(&Vars.cFileSection);
        return JS_FALSE;
    }

    for (uint i = 1; i < argc; i++)
        if (!writeValue(fptr, cx, JS_ARGV(cx, vp)[i], false, true))
            result = false;

    fflush(fptr);
    fclose(fptr);

    LeaveCriticalSection(&Vars.cFileSection);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(result));

    return JS_TRUE;
}
