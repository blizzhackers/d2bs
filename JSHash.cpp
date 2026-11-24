#include "JSHash.h"
#include "Hash.h"
#include "File.h"
#include "D2BS.h"
#include "Helpers.h"

JSAPI_FUNC(my_md5) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    char* input = JS_EncodeStringToUTF8(cx, str);
    if (!input)
        return JS_FALSE;
    char* result = md5(input);
    JS_free(cx, input);
    // A NULL digest is a CryptoAPI failure: null, never an unset rval (that would hand back the callee).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_InternString(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha1) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    char* input = JS_EncodeStringToUTF8(cx, str);
    if (!input)
        return JS_FALSE;
    char* result = sha1(input);
    JS_free(cx, input);
    // A NULL digest is a CryptoAPI failure: null, never an unset rval (that would hand back the callee).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_InternString(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha256) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    char* input = JS_EncodeStringToUTF8(cx, str);
    if (!input)
        return JS_FALSE;
    char* result = sha256(input);
    JS_free(cx, input);
    // A NULL digest is a CryptoAPI failure: null, never an unset rval (that would hand back the callee).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_InternString(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha384) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    char* input = JS_EncodeStringToUTF8(cx, str);
    if (!input)
        return JS_FALSE;
    char* result = sha384(input);
    JS_free(cx, input);
    // A NULL digest is a CryptoAPI failure: null, never an unset rval (that would hand back the callee).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_InternString(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha512) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    char* input = JS_EncodeStringToUTF8(cx, str);
    if (!input)
        return JS_FALSE;
    char* result = sha512(input);
    JS_free(cx, input);
    // A NULL digest is a CryptoAPI failure: null, never an unset rval (that would hand back the callee).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_InternString(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_md5_file) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    const wchar_t* file = JS_GetStringCharsZ(cx, str);
    if (!file)
        return JS_FALSE;
    if (!(file[0] && isValidPath(file)))
        THROW_ERROR(cx, "Invalid file path!");

    wchar_t path[_MAX_FNAME + _MAX_PATH];
    // isValidPath bounds this join today; _TRUNCATE keeps a future bound change from ending the process.
    if (_snwprintf_s(path, _countof(path), _TRUNCATE, L"%s\\%s", Vars.szScriptPath, file) < 0)
        THROW_ERROR(cx, "Invalid file path!");

    char* result = md5_file(path);
    // NULL = the file could not be opened or read: null, not an exception (a missing file is an expected case).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_NewStringCopyZ(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha1_file) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    const wchar_t* file = JS_GetStringCharsZ(cx, str);
    if (!file)
        return JS_FALSE;
    if (!(file[0] && isValidPath(file)))
        THROW_ERROR(cx, "Invalid file path!");

    wchar_t path[_MAX_FNAME + _MAX_PATH];
    // isValidPath bounds this join today; _TRUNCATE keeps a future bound change from ending the process.
    if (_snwprintf_s(path, _countof(path), _TRUNCATE, L"%s\\%s", Vars.szScriptPath, file) < 0)
        THROW_ERROR(cx, "Invalid file path!");

    char* result = sha1_file(path);
    // NULL = the file could not be opened or read: null, not an exception (a missing file is an expected case).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_NewStringCopyZ(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha256_file) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    const wchar_t* file = JS_GetStringCharsZ(cx, str);
    if (!file)
        return JS_FALSE;
    if (!(file[0] && isValidPath(file)))
        THROW_ERROR(cx, "Invalid file path!");

    wchar_t path[_MAX_FNAME + _MAX_PATH];
    // isValidPath bounds this join today; _TRUNCATE keeps a future bound change from ending the process.
    if (_snwprintf_s(path, _countof(path), _TRUNCATE, L"%s\\%s", Vars.szScriptPath, file) < 0)
        THROW_ERROR(cx, "Invalid file path!");

    char* result = sha256_file(path);
    // NULL = the file could not be opened or read: null, not an exception (a missing file is an expected case).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_NewStringCopyZ(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha384_file) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    const wchar_t* file = JS_GetStringCharsZ(cx, str);
    if (!file)
        return JS_FALSE;
    if (!(file[0] && isValidPath(file)))
        THROW_ERROR(cx, "Invalid file path!");

    wchar_t path[_MAX_FNAME + _MAX_PATH];
    // isValidPath bounds this join today; _TRUNCATE keeps a future bound change from ending the process.
    if (_snwprintf_s(path, _countof(path), _TRUNCATE, L"%s\\%s", Vars.szScriptPath, file) < 0)
        THROW_ERROR(cx, "Invalid file path!");

    char* result = sha384_file(path);
    // NULL = the file could not be opened or read: null, not an exception (a missing file is an expected case).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_NewStringCopyZ(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}

JSAPI_FUNC(my_sha512_file) {
    if (argc != 1)
        THROW_ERROR(cx, "Invalid arguments");

    // Both steps return NULL with their error already reported (a throwing toString, or out of memory).
    JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str)
        return JS_FALSE;
    const wchar_t* file = JS_GetStringCharsZ(cx, str);
    if (!file)
        return JS_FALSE;
    if (!(file[0] && isValidPath(file)))
        THROW_ERROR(cx, "Invalid file path!");

    wchar_t path[_MAX_FNAME + _MAX_PATH];
    // isValidPath bounds this join today; _TRUNCATE keeps a future bound change from ending the process.
    if (_snwprintf_s(path, _countof(path), _TRUNCATE, L"%s\\%s", Vars.szScriptPath, file) < 0)
        THROW_ERROR(cx, "Invalid file path!");

    char* result = sha512_file(path);
    // NULL = the file could not be opened or read: null, not an exception (a missing file is an expected case).
    if (!result) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    bool stored = StoreString(vp, JS_NewStringCopyZ(cx, result));
    delete[] result;
    return stored ? JS_TRUE : JS_FALSE;
}
