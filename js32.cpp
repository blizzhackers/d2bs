#include "ScriptEngine.h"
#include "js32.h"
#include "D2Helpers.h"
#include "Helpers.h"
#include "File.h"
//#include <cstdarg>

JSObject* BuildObject(JSContext* cx, JSClass* classp, JSFunctionSpec* funcs, JSPropertySpec* props, void* priv, JSObject* proto, JSObject* parent) {
    JS_BeginRequest(cx);

    JSObject* obj = JS_NewObject(cx, classp, proto, parent);

    if (obj) {
        // Rooted while its functions and properties are defined. A failed registration is an engine failure (out of
        // memory) like the others here: the object is dropped, with no root to remove.
        if (!JS_AddObjectRoot(cx, &obj)) {
            JS_EndRequest(cx);
            return NULL;
        }
        if (obj && funcs && !JS_DefineFunctions(cx, obj, funcs))
            obj = NULL;
        if (obj && props && !JS_DefineProperties(cx, obj, props))
            obj = NULL;
        if (obj && priv)
            JS_SetPrivate(cx, obj, priv);
        JS_RemoveObjectRoot(cx, &obj);
    }
    JS_EndRequest(cx);
    return obj;
}
JSScript* JS_CompileFile(JSContext* cx, JSObject* globalObject, std::wstring fileName) {
    char* nFileName = UnicodeToAnsi(fileName.c_str());
    std::ifstream t(fileName.c_str(), std::ios::binary);
    std::string str;

    // Sized from the stream and read once, with every step checked: a failed stream reports -1 here, which must become
    // a script error rather than a C++ throw (include() has no handler, so a throw ends the game).
    std::streamoff size = -1;
    if (t.is_open()) {
        t.seekg(0, std::ios::end);
        size = t.tellg();
        t.seekg(0, std::ios::beg);
    }
    if (size < 0 || !t) {
        JS_ReportError(cx, "Couldn't open the script file %s", nFileName);
        delete[] nFileName;
        return NULL;
    }
    str.resize((size_t)size);
    if (size > 0 && (!t.read(&str[0], size) || t.gcount() != size)) {
        JS_ReportError(cx, "Couldn't read the script file %s", nFileName);
        delete[] nFileName;
        return NULL;
    }
    t.close();

    if (str.size() >= 3 && (unsigned char)str[0] == 0xEF && (unsigned char)str[1] == 0xBB && (unsigned char)str[2] == 0xBF) { // replace utf8-bom with space
        str[0] = ' ';
        str[1] = ' ';
        str[2] = ' ';
    }

    JS::RootedObject obj(cx, globalObject);
    JS::CompileOptions opts(cx);
    opts.setUTF8(true).setFileAndLine(nFileName, 1);
    JSScript* rval = JS::Compile(cx, obj, opts, str.c_str(), str.size());
    delete[] nFileName;

    return rval;
}
JSBool JSVAL_IS_OBJECT(jsval v) {
    return !JSVAL_IS_PRIMITIVE(v);
}
void* JS_GetPrivate(JSContext* cx, JSObject* obj) {
    return JS_GetPrivate(obj);
}
void JS_SetPrivate(JSContext* cx, JSObject* obj, void* data) {
    return JS_SetPrivate(obj, data);
}
