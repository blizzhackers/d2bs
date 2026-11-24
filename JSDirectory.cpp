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

// TODO: Rewrite this crap :(

#define _USE_32BIT_TIME_T

#include <direct.h>
#include <io.h>
#include <cerrno>

#include "JSDirectory.h"
#include "D2BS.h"
#include "File.h"
#include "Helpers.h"
//#include "js32.h"

////////////////////////////////////////////////////////////////////////////////
// Directory stuff
////////////////////////////////////////////////////////////////////////////////

EMPTY_CTOR(dir)

// A search pattern names entries inside the folder only: a separator or a drive colon would reach outside it, and
// Win32 strips trailing dots/spaces then resolves "." and "..", so a dots-and-spaces pattern names the folder or its
// parent.
// Wildcards stay allowed, which is why isValidPath (it rejects * and ?) is not used on the pattern.
static bool IsValidSearchPattern(const wchar_t* search) {
    if (wcspbrk(search, L"\\/:") != NULL)
        return false;
    size_t len = wcslen(search);
    return len == 0 || wcsspn(search, L". ") != len;
}

// Joins <script path>\<folder>\<search> without a doubled separator (scripts pass trailing-slash names such as
// "libs/core/"). False when the result does not fit: _wfindfirst cannot open a pattern that long anyway.
static bool BuildSearchPattern(wchar_t* out, size_t outLen, const wchar_t* folder, const wchar_t* search) {
    size_t len = wcslen(folder);
    const wchar_t* sep = (len == 0 || folder[len - 1] == L'\\' || folder[len - 1] == L'/') ? L"" : L"\\";
    return _snwprintf_s(out, outLen, _TRUNCATE, L"%s\\%s%s%s", Vars.szScriptPath, folder, sep, search) >= 0;
}

JSAPI_FUNC(my_openDir) {
    // No name, or more than one argument, opens nothing: undefined (the rval slot would otherwise hold the callee).
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (argc != 1)
        return JS_TRUE;

    if (!JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid arguments");
    const wchar_t* name = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!name)
        return JS_FALSE;

    // DirData holds _MAX_FNAME characters and Folder listings rely on <script path>\<name> fitting _MAX_PATH;
    // swprintf_s and wcscpy_s end the process on overflow (no invalid-parameter handler), so refuse a long name first.
    wchar_t path[_MAX_PATH];
    if (wcslen(name) >= _MAX_FNAME || _snwprintf_s(path, _MAX_PATH, _TRUNCATE, L"%s\\%s", Vars.szScriptPath, name) < 0)
        THROW_ERROR(cx, "Invalid folder name");

    if (!isValidPath(name)) {
        Log(L"The following path was deemed invalid: %s. (%s, %s)", name, L"JSDirectory.cpp", L"my_openDir");
        THROW_ERROR(cx, "Invalid folder name");
    }

    if ((_wmkdir(path) == -1) && (errno == ENOENT)) {
        char* n = UnicodeToAnsi(name);
        char* p = UnicodeToAnsi(path);
        JS_ReportError(cx, "Couldn't get directory %s, path '%s' not found", n, p);
        delete[] n;
        delete[] p;
        return JS_FALSE;
    } else {
        DirData* d = new DirData(name);
        JSObject* jsdir = BuildObject(cx, &folder_class, dir_methods, dir_props, d);
        // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
        if (!jsdir) {
            delete d;
            return JS_FALSE;
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsdir));
    }

    return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// dir_getFiles
// searches a directory for files with a specified extension(*.* assumed)
//
// Added by lord2800
////////////////////////////////////////////////////////////////////////////////

JSAPI_FUNC(dir_getFiles) {
    if (argc > 1)
        THROW_ERROR(cx, "Invalid arguments");

    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    // Instance check, not JS_GetPrivate: Folder.prototype or a borrowed `this` carries no DirData.
    DirData* d = (DirData*)JS_GetInstancePrivate(cx, self, &folder_class, NULL);

    JSObject* jsarray = JS_NewArrayObject(cx, 0, NULL);
    if (!jsarray)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsarray));
    // A non-Folder `this` lists nothing, like a vanished folder (the Folder class's wrong-`this` value).
    if (!d)
        return JS_TRUE;
    // The default stays native: the engine gives a native no argument slot past argc to store it in.
    const wchar_t* search = L"*.*";
    if (argc == 1) {
        // A conversion that throws (a toString that throws, an object with no toString) returns NULL.
        JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
        if (!str)
            return JS_FALSE;
        search = JS_GetStringCharsZ(cx, str);
        if (!search)
            THROW_ERROR(cx, "Failed to get search string");
    }
    if (!IsValidSearchPattern(search))
        THROW_ERROR(cx, "Invalid search pattern");

    // An absolute pattern, never a chdir: the cwd is process-wide and other scripts list folders concurrently.
    wchar_t pattern[_MAX_PATH + 2 * _MAX_FNAME];
    if (!BuildSearchPattern(pattern, _countof(pattern), d->name, search))
        return JS_TRUE;

    long hFile;
    _wfinddata_t found;
    if ((hFile = _wfindfirst(pattern, &found)) != -1L) {
        JS_BeginRequest(cx);
        jsint element = 0;
        do {
            if ((found.attrib & _A_SUBDIR))
                continue;
            JSString* name = JS_NewUCStringCopyZ(cx, found.name);
            if (!name) {
                JS_EndRequest(cx);
                _findclose(hFile);
                return JS_FALSE;
            }
            jsval file = STRING_TO_JSVAL(name);
            if (!JS_SetElement(cx, jsarray, element++, &file)) {
                JS_EndRequest(cx);
                _findclose(hFile);
                return JS_FALSE;
            }
        } while (_wfindnext(hFile, &found) == 0);
        JS_EndRequest(cx);
        _findclose(hFile);
    }
    return JS_TRUE;
}

JSAPI_FUNC(dir_getFolders) {
    if (argc > 1)
        THROW_ERROR(cx, "Invalid arguments");

    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    // Instance check, not JS_GetPrivate: Folder.prototype or a borrowed `this` carries no DirData.
    DirData* d = (DirData*)JS_GetInstancePrivate(cx, self, &folder_class, NULL);

    JSObject* jsarray = JS_NewArrayObject(cx, 0, NULL);
    if (!jsarray)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsarray));
    // A non-Folder `this` lists nothing, like a vanished folder (the Folder class's wrong-`this` value).
    if (!d)
        return JS_TRUE;
    // The default stays native: the engine gives a native no argument slot past argc to store it in.
    const wchar_t* search = L"*.*";
    if (argc == 1) {
        // A conversion that throws (a toString that throws, an object with no toString) returns NULL.
        JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
        if (!str)
            return JS_FALSE;
        search = JS_GetStringCharsZ(cx, str);
        if (!search)
            THROW_ERROR(cx, "Failed to get search string");
    }
    if (!IsValidSearchPattern(search))
        THROW_ERROR(cx, "Invalid search pattern");

    // An absolute pattern, never a chdir: the cwd is process-wide and other scripts list folders concurrently.
    wchar_t pattern[_MAX_PATH + 2 * _MAX_FNAME];
    if (!BuildSearchPattern(pattern, _countof(pattern), d->name, search))
        return JS_TRUE;

    long hFile;
    _wfinddata_t found;
    if ((hFile = _wfindfirst(pattern, &found)) != -1L) {
        jsint element = 0;
        JS_BeginRequest(cx);
        do {
            if (!wcscmp(found.name, L"..") || !wcscmp(found.name, L".") || !(found.attrib & _A_SUBDIR))
                continue;
            JSString* name = JS_NewUCStringCopyZ(cx, found.name);
            if (!name) {
                JS_EndRequest(cx);
                _findclose(hFile);
                return JS_FALSE;
            }
            jsval file = STRING_TO_JSVAL(name);
            if (!JS_SetElement(cx, jsarray, element++, &file)) {
                JS_EndRequest(cx);
                _findclose(hFile);
                return JS_FALSE;
            }
        } while (_wfindnext(hFile, &found) == 0);
        JS_EndRequest(cx);
        _findclose(hFile);
    }
    return JS_TRUE;
}

JSAPI_FUNC(dir_create) {
    // A foreign or prototype this has no folder; reading past a plain object's slots would be a garbage pointer.
    DirData* d = (DirData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &folder_class, NULL);
    if (!d) {
        JS_SET_RVAL(cx, vp, JSVAL_VOID);
        return JS_TRUE;
    }
    wchar_t path[_MAX_PATH];
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "No path passed to dir.create()");
    const wchar_t* name = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!name)
        return JS_FALSE;

    // swprintf_s below and DirData's wcscpy_s end the process on overflow (no invalid-parameter handler), and
    // isValidPath accepts names longer than _MAX_PATH, so the joined <script path>\<folder>\<name> is bounded first.
    if (wcslen(Vars.szScriptPath) + wcslen(d->name) + wcslen(name) + 3 > _MAX_PATH || !isValidPath(name))
        THROW_ERROR(cx, "Invalid folder name");

    swprintf_s(path, _MAX_PATH, L"%s\\%s\\%s", Vars.szScriptPath, d->name, name);
    if (_wmkdir(path) == -1 && (errno == ENOENT)) {
        JS_ReportError(cx, "Couldn't create directory %hs, path %hs not found", name, path);
        return JS_FALSE;
    } else {
        DirData* d = new DirData(name);
        JSObject* jsdir = BuildObject(cx, &folder_class, dir_methods, dir_props, d);
        if (!jsdir) {
            delete d;
            return JS_FALSE;
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsdir));
    }
    return JS_TRUE;
}

JSAPI_FUNC(dir_delete) {
    DirData* d = (DirData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &folder_class, NULL);
    if (!d) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }

    wchar_t path[_MAX_PATH];
    swprintf_s(path, _MAX_PATH, L"%s\\%s", Vars.szScriptPath, d->name);

    if (_wrmdir(path) == -1) {
        // TODO: Make an optional param that specifies recursive delete
        if (errno == ENOTEMPTY)
            THROW_ERROR(cx, "Tried to delete directory, but it is not empty or is the current working directory");
        if (errno == ENOENT)
            THROW_ERROR(cx, "Path not found");
    }
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);

    return JS_TRUE;
}

JSAPI_PROP(dir_getProperty) {
    DirData* d = (DirData*)JS_GetInstancePrivate(cx, obj, &folder_class, NULL);

    if (!d) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case DIR_NAME:
        if (!StoreString(vp, JS_InternUCString(cx, d->name)))
            return JS_FALSE;
        break;
    }
    return JS_TRUE;
}

void dir_finalize(JSFreeOp* fop, JSObject* obj) {
    DirData* d = (DirData*)JS_GetPrivate(obj);
    delete d;
}
