#include "JSScreenHook.h"
#include "D2BS.h"
#include "ScreenHook.h"
#include "Script.h"
#include "File.h"
using namespace std;

// Every delete of a published hook holds globalSection: the derived destructor runs before ~Genhook unlinks the hook.
void hook_finalize(JSFreeOp* fop, JSObject* obj) {
    Genhook* hook = (Genhook*)JS_GetPrivate(obj);
    Genhook::EnterGlobalSection();
    if (hook) {
        JS_SetPrivate(obj, NULL);
        delete hook;
    }
    Genhook::LeaveGlobalSection();
}

namespace {
// remove() is shared by the five hook classes; a private of any other class must never be deleted as a Genhook.
bool IsHookObject(JSContext* cx, JSObject* obj) {
    static JSClass* const hookClasses[] = {&frame_class, &box_class, &line_class, &text_class, &image_class};
    if (!obj)
        return false;
    for (size_t i = 0; i < sizeof(hookClasses) / sizeof(hookClasses[0]); i++)
        if (JS_InstanceOf(cx, obj, hookClasses[i], NULL))
            return true;
    return false;
}

// Hook number arguments take any finite number, truncated as ECMA ToInt32 does; a non-number or a NaN/Infinity is
// ignored, so the default or current value stays (no valueOf/toString call can run script inside a setter).
bool HookNumberArg(const JS::Value& v, int32_t* out) {
    if (v.isInt32()) {
        *out = v.toInt32();
        return true;
    }
    if (v.isDouble() && MOZ_DOUBLE_IS_FINITE(v.toDouble())) {
        *out = JS_DoubleToInt32(v.toDouble());
        return true;
    }
    return false;
}
} // namespace

JSAPI_FUNC(hook_remove) {
    // JS_THIS_OBJECT may read the callee from the rval slot to box a primitive this, so it runs before the rval is
    // set; a null result means an error is pending.
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    if (!obj)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!IsHookObject(cx, obj))
        return JS_TRUE;
    Genhook::EnterGlobalSection();
    Genhook* hook = (Genhook*)JS_GetPrivate(cx, obj);
    if (hook) {
        delete hook;
    }

    JS_SetPrivate(cx, obj, NULL);
    Genhook::LeaveGlobalSection();

    // The handler slots belong to this (the owner) thread; clearing them lets a removed hook's functions be collected.
    JS_SetReservedSlot(obj, HOOK_SLOT_CLICK, JSVAL_VOID);
    JS_SetReservedSlot(obj, HOOK_SLOT_HOVER, JSVAL_VOID);

    return JS_TRUE;
}

// Function to create a frame which gets called on a "new Frame ()"
// Parameters: x, y, xsize, ysize, alignment, automap, onClick, onHover
JSAPI_FUNC(frame_ctor) {
    Script* script = (Script*)JS_GetContextPrivate(cx);

    uint x = 0, y = 0, x2 = 0, y2 = 0;
    Align align = Left;
    bool automap = false;
    jsval click = JSVAL_VOID, hover = JSVAL_VOID;
    int32_t n;

    if (argc > 0 && HookNumberArg(JS_ARGV(cx, vp)[0], &n))
        x = n;
    if (argc > 1 && HookNumberArg(JS_ARGV(cx, vp)[1], &n))
        y = n;
    if (argc > 2 && HookNumberArg(JS_ARGV(cx, vp)[2], &n))
        x2 = n;
    if (argc > 3 && HookNumberArg(JS_ARGV(cx, vp)[3], &n))
        y2 = n;
    if (argc > 4 && HookNumberArg(JS_ARGV(cx, vp)[4], &n))
        align = (Align)n;
    if (argc > 5 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[5]))
        automap = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[5]);
    if (argc > 6 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[6]))
        click = JS_ARGV(cx, vp)[6];
    if (argc > 7 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[7]))
        hover = JS_ARGV(cx, vp)[7];

    JSObject* hook = BuildObject(cx, &frame_class, frame_methods, frame_props);
    if (!hook)
        THROW_ERROR(cx, "Failed to create frame object");

    // framehooks don't work out of game -- they just crash
    FrameHook* pFrameHook = new FrameHook(script, hook, x, y, x2, y2, automap, align, IG);

    JS_SetPrivate(cx, hook, pFrameHook);
    pFrameHook->SetClickHandler(cx, click);
    pFrameHook->SetHoverHandler(cx, hover);
    pFrameHook->Register();

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(hook));

    return JS_TRUE;
}

JSAPI_PROP(frame_getProperty) {
    FrameHook* pFramehook = (FrameHook*)JS_GetInstancePrivate(cx, obj, &frame_class, NULL);
    if (!pFramehook) {
        // Hook props are slotful: a getter that leaves vp unset hands back the holder's last stored value.
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case FRAME_X:
        vp.setInt32(pFramehook->GetX());
        break;
    case FRAME_Y:
        vp.setInt32(pFramehook->GetY());
        break;
    case FRAME_XSIZE:
        vp.setInt32(pFramehook->GetXSize());
        break;
    case FRAME_YSIZE:
        vp.setInt32(pFramehook->GetYSize());
        break;
    case FRAME_ALIGN:
        vp.setInt32(pFramehook->GetAlign());
        break;
    case FRAME_VISIBLE:
        vp.setBoolean(pFramehook->GetIsVisible());
        break;
    case FRAME_ZORDER:
        vp.setInt32(pFramehook->GetZOrder());
        break;
    case FRAME_ONCLICK:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_CLICK));
        break;
    case FRAME_ONHOVER:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_HOVER));
        break;
    }
    return JS_TRUE;
}

JSAPI_STRICT_PROP(frame_setProperty) {
    FrameHook* pFramehook = (FrameHook*)JS_GetInstancePrivate(cx, obj, &frame_class, NULL);
    if (!pFramehook)
        return JS_TRUE;

    int32_t n;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case FRAME_X:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetX(n);
        break;
    case FRAME_Y:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetY(n);
        break;
    case FRAME_XSIZE:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetXSize(n);
        break;
    case FRAME_YSIZE:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetYSize(n);
        break;
    case FRAME_ALIGN:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetAlign((Align)n);
        break;
    case FRAME_VISIBLE:
        if (vp.isBoolean())
            pFramehook->SetIsVisible(!!vp.toBoolean());
        break;
    case FRAME_ZORDER:
        if (HookNumberArg(vp.get(), &n))
            pFramehook->SetZOrder((ushort)n);
        break;
    case FRAME_ONCLICK:
        pFramehook->SetClickHandler(cx, vp.get());
        break;
    case FRAME_ONHOVER:
        pFramehook->SetHoverHandler(cx, vp.get());
        break;
    }
    return JS_TRUE;
}

// Box functions

// Parameters: x, y, xsize, ysize, color, opacity, alignment, automap, onClick, onHover
JSAPI_FUNC(box_ctor) {
    Script* script = (Script*)JS_GetContextPrivate(cx);

    ScreenhookState state = (script->GetState() == OutOfGame) ? OOG : IG;
    uint x = 0, y = 0, x2 = 0, y2 = 0;
    ushort color = 0, opacity = 0;
    Align align = Left;
    bool automap = false;
    jsval click = JSVAL_VOID, hover = JSVAL_VOID;
    int32_t n;

    if (argc > 0 && HookNumberArg(JS_ARGV(cx, vp)[0], &n))
        x = n;
    if (argc > 1 && HookNumberArg(JS_ARGV(cx, vp)[1], &n))
        y = n;
    if (argc > 2 && HookNumberArg(JS_ARGV(cx, vp)[2], &n))
        x2 = n;
    if (argc > 3 && HookNumberArg(JS_ARGV(cx, vp)[3], &n))
        y2 = n;
    if (argc > 4 && HookNumberArg(JS_ARGV(cx, vp)[4], &n))
        color = (ushort)n;
    if (argc > 5 && HookNumberArg(JS_ARGV(cx, vp)[5], &n))
        opacity = (ushort)n;
    if (argc > 6 && HookNumberArg(JS_ARGV(cx, vp)[6], &n))
        align = (Align)n;
    if (argc > 7 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[7]))
        automap = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[7]);
    if (argc > 8 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[8]))
        click = JS_ARGV(cx, vp)[8];
    if (argc > 9 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[9]))
        hover = JS_ARGV(cx, vp)[9];

    JSObject* hook = BuildObject(cx, &box_class, box_methods, box_props);
    if (!hook)
        THROW_ERROR(cx, "Failed to create box object");

    BoxHook* pBoxHook = new BoxHook(script, hook, x, y, x2, y2, color, opacity, automap, align, state);

    JS_SetPrivate(cx, hook, pBoxHook);
    pBoxHook->SetClickHandler(cx, click);
    pBoxHook->SetHoverHandler(cx, hover);
    pBoxHook->Register();

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(hook));

    return JS_TRUE;
}
JSAPI_PROP(box_getProperty) {
    BoxHook* pBoxHook = (BoxHook*)JS_GetInstancePrivate(cx, obj, &box_class, NULL);
    if (!pBoxHook) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case BOX_X:
        vp.setInt32(pBoxHook->GetX());
        break;
    case BOX_Y:
        vp.setInt32(pBoxHook->GetY());
        break;
    case BOX_XSIZE:
        vp.setInt32(pBoxHook->GetXSize());
        break;
    case BOX_YSIZE:
        vp.setInt32(pBoxHook->GetYSize());
        break;
    case BOX_ALIGN:
        vp.setInt32(pBoxHook->GetAlign());
        break;
    case BOX_COLOR:
        vp.setInt32(pBoxHook->GetColor());
        break;
    case BOX_OPACITY:
        vp.setInt32(pBoxHook->GetOpacity());
        break;
    case BOX_VISIBLE:
        vp.setBoolean(pBoxHook->GetIsVisible());
        break;
    case BOX_ZORDER:
        vp.setInt32(pBoxHook->GetZOrder());
        break;
    case BOX_ONCLICK:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_CLICK));
        break;
    case BOX_ONHOVER:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_HOVER));
        break;
    }
    return JS_TRUE;
}

JSAPI_STRICT_PROP(box_setProperty) {
    BoxHook* pBoxHook = (BoxHook*)JS_GetInstancePrivate(cx, obj, &box_class, NULL);
    if (!pBoxHook)
        return JS_TRUE;

    int32_t n;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case BOX_X:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetX(n);
        break;
    case BOX_Y:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetY(n);
        break;
    case BOX_XSIZE:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetXSize(n);
        break;
    case BOX_YSIZE:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetYSize(n);
        break;
    case BOX_OPACITY:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetOpacity((ushort)n);
        break;
    case BOX_COLOR:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetColor((ushort)n);
        break;
    case BOX_ALIGN:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetAlign((Align)n);
        break;
    case BOX_VISIBLE:
        if (vp.isBoolean())
            pBoxHook->SetIsVisible(!!vp.toBoolean());
        break;
    case BOX_ZORDER:
        if (HookNumberArg(vp.get(), &n))
            pBoxHook->SetZOrder((ushort)n);
        break;
    case BOX_ONCLICK:
        pBoxHook->SetClickHandler(cx, vp.get());
        break;
    case BOX_ONHOVER:
        pBoxHook->SetHoverHandler(cx, vp.get());
        break;
    }
    return JS_TRUE;
}

// Line functions

// Parameters: x, y, x2, y2, color, automap, click, hover
JSAPI_FUNC(line_ctor) {
    Script* script = (Script*)JS_GetContextPrivate(cx);

    ScreenhookState state = (script->GetState() == OutOfGame) ? OOG : IG;
    int x = 0, y = 0, x2 = 0, y2 = 0;
    ushort color = 0;
    bool automap = false;
    jsval click = JSVAL_VOID, hover = JSVAL_VOID;
    int32_t n;

    if (argc > 0 && HookNumberArg(JS_ARGV(cx, vp)[0], &n))
        x = n;
    if (argc > 1 && HookNumberArg(JS_ARGV(cx, vp)[1], &n))
        y = n;
    if (argc > 2 && HookNumberArg(JS_ARGV(cx, vp)[2], &n))
        x2 = n;
    if (argc > 3 && HookNumberArg(JS_ARGV(cx, vp)[3], &n))
        y2 = n;
    if (argc > 4 && HookNumberArg(JS_ARGV(cx, vp)[4], &n))
        color = (ushort)n;
    if (argc > 5 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[5]))
        automap = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[5]);
    if (argc > 6 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[6]))
        click = JS_ARGV(cx, vp)[6];
    if (argc > 7 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[7]))
        hover = JS_ARGV(cx, vp)[7];

    JSObject* hook = BuildObject(cx, &line_class, line_methods, line_props);
    if (!hook)
        THROW_ERROR(cx, "Failed to create line object");

    LineHook* pLineHook = new LineHook(script, hook, x, y, x2, y2, color, automap, Left, state);

    JS_SetPrivate(cx, hook, pLineHook);
    pLineHook->SetClickHandler(cx, click);
    pLineHook->SetHoverHandler(cx, hover);
    pLineHook->Register();

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(hook));

    return JS_TRUE;
}

JSAPI_PROP(line_getProperty) {
    LineHook* pLineHook = (LineHook*)JS_GetInstancePrivate(cx, obj, &line_class, NULL);
    if (!pLineHook) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case LINE_X:
        vp.setInt32(pLineHook->GetX());
        break;
    case LINE_Y:
        vp.setInt32(pLineHook->GetY());
        break;
    case LINE_XSIZE:
        vp.setInt32(pLineHook->GetX2());
        break;
    case LINE_YSIZE:
        vp.setInt32(pLineHook->GetY2());
        break;
    case LINE_COLOR:
        vp.setInt32(pLineHook->GetColor());
        break;
    case LINE_VISIBLE:
        vp.setBoolean(pLineHook->GetIsVisible());
        break;
    case LINE_ZORDER:
        vp.setInt32(pLineHook->GetZOrder());
        break;
    case LINE_ONCLICK:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_CLICK));
        break;
    case LINE_ONHOVER:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_HOVER));
        break;
    }
    return JS_TRUE;
}

JSAPI_STRICT_PROP(line_setProperty) {
    LineHook* pLineHook = (LineHook*)JS_GetInstancePrivate(cx, obj, &line_class, NULL);
    if (!pLineHook)
        return JS_TRUE;

    int32_t n;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case LINE_X:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetX(n);
        break;
    case LINE_Y:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetY(n);
        break;
    case LINE_XSIZE:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetX2(n);
        break;
    case LINE_YSIZE:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetY2(n);
        break;
    case LINE_COLOR:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetColor((ushort)n);
        break;
    case LINE_VISIBLE:
        if (vp.isBoolean())
            pLineHook->SetIsVisible(!!vp.toBoolean());
        break;
    case LINE_ZORDER:
        if (HookNumberArg(vp.get(), &n))
            pLineHook->SetZOrder((ushort)n);
        break;
    case LINE_ONCLICK:
        pLineHook->SetClickHandler(cx, vp.get());
        break;
    case LINE_ONHOVER:
        pLineHook->SetHoverHandler(cx, vp.get());
        break;
    }
    return JS_TRUE;
}

// Function to create a text which gets called on a "new text ()"

// Parameters: text, x, y, color, font, align, automap, onClick, onHover
JSAPI_FUNC(text_ctor) {
    Script* script = (Script*)JS_GetContextPrivate(cx);

    ScreenhookState state = (script->GetState() == OutOfGame) ? OOG : IG;
    uint x = 0, y = 0;
    ushort color = 0, font = 0;
    Align align = Left;
    bool automap = false;
    jsval click = JSVAL_VOID, hover = JSVAL_VOID;
    int32_t n;
    const wchar_t* szText = L"";

    if (argc > 0 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        szText = JS_GetStringCharsZ(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
    if (!szText)
        return JS_FALSE;
    if (argc > 1 && HookNumberArg(JS_ARGV(cx, vp)[1], &n))
        x = n;
    if (argc > 2 && HookNumberArg(JS_ARGV(cx, vp)[2], &n))
        y = n;
    if (argc > 3 && HookNumberArg(JS_ARGV(cx, vp)[3], &n))
        color = (ushort)n;
    if (argc > 4 && HookNumberArg(JS_ARGV(cx, vp)[4], &n))
        font = (ushort)n;
    if (argc > 5 && HookNumberArg(JS_ARGV(cx, vp)[5], &n))
        align = (Align)n;
    if (argc > 6 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[6]))
        automap = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[6]);
    if (argc > 7 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[7]))
        click = JS_ARGV(cx, vp)[7];
    if (argc > 8 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[8]))
        hover = JS_ARGV(cx, vp)[8];

    JSObject* hook = BuildObject(cx, &text_class, text_methods, text_props);
    if (!hook)
        THROW_ERROR(cx, "Failed to create text object");

    TextHook* pTextHook = new TextHook(script, hook, szText, x, y, font, color, automap, align, state);
    // The text is copied in the constructor; a failed copy fails the constructor rather than leave a hook with no text.
    if (!pTextHook->GetText()) {
        delete pTextHook;
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }

    JS_SetPrivate(cx, hook, pTextHook);
    pTextHook->SetClickHandler(cx, click);
    pTextHook->SetHoverHandler(cx, hover);
    pTextHook->Register();

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(hook));

    return JS_TRUE;
}

JSAPI_PROP(text_getProperty) {
    TextHook* pTextHook = (TextHook*)JS_GetInstancePrivate(cx, obj, &text_class, NULL);
    if (!pTextHook) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case TEXT_X:
        vp.setInt32(pTextHook->GetX());
        break;
    case TEXT_Y:
        vp.setInt32(pTextHook->GetY());
        break;
    case TEXT_COLOR:
        vp.setInt32(pTextHook->GetColor());
        break;
    case TEXT_FONT:
        vp.setInt32(pTextHook->GetFont());
        break;
    case TEXT_TEXT:
        // Never NULL on a constructed hook; the empty-string fallback keeps a NULL away from JS_InternUCString.
        if (!StoreString(vp, JS_InternUCString(cx, pTextHook->GetText() ? pTextHook->GetText() : L"")))
            return JS_FALSE;
        break;
    case TEXT_ALIGN:
        vp.setInt32(pTextHook->GetAlign());
        break;
    case TEXT_VISIBLE:
        vp.setBoolean(pTextHook->GetIsVisible());
        break;
    case TEXT_ZORDER:
        vp.setInt32(pTextHook->GetZOrder());
        break;
    case TEXT_ONCLICK:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_CLICK));
        break;
    case TEXT_ONHOVER:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_HOVER));
        break;
    }
    return JS_TRUE;
}

JSAPI_STRICT_PROP(text_setProperty) {
    TextHook* pTextHook = (TextHook*)JS_GetInstancePrivate(cx, obj, &text_class, NULL);
    if (!pTextHook)
        return JS_TRUE;

    int32_t n;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case TEXT_X:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetX(n);
        break;
    case TEXT_Y:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetY(n);
        break;
    case TEXT_COLOR:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetColor((ushort)n);
        break;
    case TEXT_FONT:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetFont((ushort)n);
        break;
    case TEXT_TEXT:
        if (vp.isString()) {
            const wchar_t* pText = JS_GetStringCharsZ(cx, vp.toString());
            if (!pText)
                return JS_FALSE;
            if (!pTextHook->SetText(pText)) {
                JS_ReportOutOfMemory(cx);
                return JS_FALSE;
            }
        }
        break;
    case TEXT_ALIGN:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetAlign((Align)n);
        break;
    case TEXT_VISIBLE:
        if (vp.isBoolean())
            pTextHook->SetIsVisible(!!vp.toBoolean());
        break;
    case TEXT_ZORDER:
        if (HookNumberArg(vp.get(), &n))
            pTextHook->SetZOrder((ushort)n);
        break;
    case TEXT_ONCLICK:
        pTextHook->SetClickHandler(cx, vp.get());
        break;
    case TEXT_ONHOVER:
        pTextHook->SetHoverHandler(cx, vp.get());
        break;
    }
    return JS_TRUE;
}

// LoadCellFile and getPathRelScript both build "<script path>\<name>" in
// _MAX_FNAME + _MAX_PATH characters, so the joined name must fit before isValidPath's verdict can be trusted.
static bool IsValidImageName(const wchar_t* name) {
    return wcslen(Vars.szScriptPath) + 1 + wcslen(name) < _MAX_FNAME + _MAX_PATH && isValidPath(name);
}

// Function to create a image which gets called on a "new Image ()"

// Parameters: image, x, y, color, align, automap, onClick, onHover
JSAPI_FUNC(image_ctor) {
    Script* script = (Script*)JS_GetContextPrivate(cx);

    ScreenhookState state = (script->GetState() == OutOfGame) ? OOG : IG;
    uint x = 0, y = 0;
    ushort color = 0;
    Align align = Left;
    bool automap = false;
    jsval click = JSVAL_VOID, hover = JSVAL_VOID;
    int32_t n;
    const wchar_t* szText = L"";
    wchar_t path[_MAX_FNAME + _MAX_PATH];

    if (argc > 0 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        szText = JS_GetStringCharsZ(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
    if (!szText)
        return JS_FALSE;
    if (argc > 1 && HookNumberArg(JS_ARGV(cx, vp)[1], &n))
        x = n;
    if (argc > 2 && HookNumberArg(JS_ARGV(cx, vp)[2], &n))
        y = n;
    if (argc > 3 && HookNumberArg(JS_ARGV(cx, vp)[3], &n))
        color = (ushort)n;
    if (argc > 4 && HookNumberArg(JS_ARGV(cx, vp)[4], &n))
        align = (Align)n;
    if (argc > 5 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[5]))
        automap = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[5]);
    if (argc > 6 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[6]))
        click = JS_ARGV(cx, vp)[6];
    if (argc > 7 && JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[7]))
        hover = JS_ARGV(cx, vp)[7];

    if (!IsValidImageName(szText))
        THROW_ERROR(cx, "Invalid image file path");
    swprintf_s(path, _countof(path), L"%s", szText);

    JSObject* hook = BuildObject(cx, &image_class, image_methods, image_props);
    if (!hook)
        THROW_ERROR(cx, "Failed to create image object");

    ImageHook* pImageHook = new ImageHook(script, hook, path, x, y, color, automap, align, state, 3);
    // The file name is copied in the constructor; a failed copy fails the constructor rather than leave a hook with no name.
    if (!pImageHook->GetImage()) {
        delete pImageHook;
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }

    JS_SetPrivate(cx, hook, pImageHook);
    pImageHook->SetClickHandler(cx, click);
    pImageHook->SetHoverHandler(cx, hover);
    pImageHook->Register();

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(hook));

    return JS_TRUE;
}

JSAPI_PROP(image_getProperty) {
    ImageHook* pImageHook = (ImageHook*)JS_GetInstancePrivate(cx, obj, &image_class, NULL);
    if (!pImageHook) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case IMAGE_X:
        vp.setInt32(pImageHook->GetX());
        break;
    case IMAGE_Y:
        vp.setInt32(pImageHook->GetY());
        break;
    case IMAGE_LOCATION:
        // Never NULL on a constructed hook; the empty-string fallback keeps a NULL away from JS_InternUCString.
        if (!StoreString(vp, JS_InternUCString(cx, pImageHook->GetImage() ? pImageHook->GetImage() : L"")))
            return JS_FALSE;
        break;
    case IMAGE_ALIGN:
        vp.setInt32(pImageHook->GetAlign());
        break;
    case IMAGE_VISIBLE:
        vp.setBoolean(pImageHook->GetIsVisible());
        break;
    case IMAGE_ZORDER:
        vp.setInt32(pImageHook->GetZOrder());
        break;
    case IMAGE_ONCLICK:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_CLICK));
        break;
    case IMAGE_ONHOVER:
        vp.set(JS_GetReservedSlot(obj, HOOK_SLOT_HOVER));
        break;
    }
    return JS_TRUE;
}

JSAPI_STRICT_PROP(image_setProperty) {
    ImageHook* pImageHook = (ImageHook*)JS_GetInstancePrivate(cx, obj, &image_class, NULL);
    if (!pImageHook)
        return JS_TRUE;

    int32_t n;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case IMAGE_X:
        if (HookNumberArg(vp.get(), &n))
            pImageHook->SetX(n);
        break;
    case IMAGE_Y:
        if (HookNumberArg(vp.get(), &n))
            pImageHook->SetY(n);
        break;
    case IMAGE_LOCATION:
        if (vp.isString()) {
            const wchar_t* pimage = JS_GetStringCharsZ(cx, vp.toString());
            if (!pimage)
                return JS_FALSE;
            if (!IsValidImageName(pimage))
                THROW_ERROR(cx, "Invalid image file path");
            if (!pImageHook->SetImage(pimage)) {
                JS_ReportOutOfMemory(cx);
                return JS_FALSE;
            }
        }
        break;
    case IMAGE_ALIGN:
        if (HookNumberArg(vp.get(), &n))
            pImageHook->SetAlign((Align)n);
        break;
    case IMAGE_VISIBLE:
        if (vp.isBoolean())
            pImageHook->SetIsVisible(!!vp.toBoolean());
        break;
    case IMAGE_ZORDER:
        if (HookNumberArg(vp.get(), &n))
            pImageHook->SetZOrder((ushort)n);
        break;
    case IMAGE_ONCLICK:
        pImageHook->SetClickHandler(cx, vp.get());
        break;
    case IMAGE_ONHOVER:
        pImageHook->SetHoverHandler(cx, vp.get());
        break;
    }
    return JS_TRUE;
}

JSAPI_FUNC(screenToAutomap) {
    if (argc == 1) {
        // the arg must be an object with an x and a y that we can convert
        if (JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[0])) {
            // get the params
            JSObject* arg = JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]);
            jsval x, y;
            // A failed read (a throwing getter) already has its exception pending; a THROW_ERROR would replace it.
            if (JS_GetProperty(cx, arg, "x", &x) == JS_FALSE || JS_GetProperty(cx, arg, "y", &y) == JS_FALSE)
                return JS_FALSE;
            if (!JSVAL_IS_INT(x) || !JSVAL_IS_INT(y))
                THROW_ERROR(cx, "Input has an x or y, but they aren't the correct type!");
            int32 ix, iy;
            if (JS_ValueToInt32(cx, x, &ix) == JS_FALSE || JS_ValueToInt32(cx, y, &iy) == JS_FALSE)
                THROW_ERROR(cx, "Failed to convert x and/or y values");
            POINT result = ScreenToAutomap(ix, iy);
            x = INT_TO_JSVAL(result.x);
            y = INT_TO_JSVAL(result.y);
            JSObject* res = JS_NewObject(cx, NULL, NULL, NULL);
            if (!res)
                return JS_FALSE;
            // These writes fail only with an exception already pending (out of memory, or a script setter on Object.prototype).
            if (JS_SetProperty(cx, res, "x", &x) == JS_FALSE || JS_SetProperty(cx, res, "y", &y) == JS_FALSE)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
        } else
            THROW_ERROR(cx, "Invalid object specified to screenToAutomap");
    } else if (argc == 2) {
        // the args must be ints
        if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0]) && JSVAL_IS_INT(JS_ARGV(cx, vp)[1])) {
            int32 ix, iy;
            jsval* argv = JS_ARGV(cx, vp);
            if (JS_ValueToInt32(cx, argv[0], &ix) == JS_FALSE || JS_ValueToInt32(cx, argv[1], &iy) == JS_FALSE)
                THROW_ERROR(cx, "Failed to convert x and/or y values");
            // convert the values
            POINT result = ScreenToAutomap(ix, iy);
            argv[0] = INT_TO_JSVAL(result.x);
            argv[1] = INT_TO_JSVAL(result.y);
            JSObject* res = JS_NewObject(cx, NULL, NULL, NULL);
            if (!res)
                return JS_FALSE;
            if (JS_SetProperty(cx, res, "x", &argv[0]) == JS_FALSE || JS_SetProperty(cx, res, "y", &argv[1]) == JS_FALSE)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
        } else
            THROW_ERROR(cx, "screenToAutomap expects two arguments to be two integers");
    } else
        THROW_ERROR(cx, "Invalid arguments specified for screenToAutomap");
    return JS_TRUE;
}

JSAPI_FUNC(automapToScreen) {
    jsval* argv = JS_ARGV(cx, vp);
    if (argc == 1) {
        // the arg must be an object with an x and a y that we can convert
        if (JSVAL_IS_OBJECT(argv[0])) {
            // get the params
            JSObject* arg = JSVAL_TO_OBJECT(argv[0]);
            jsval x, y;
            // A failed read (a throwing getter) already has its exception pending; a THROW_ERROR would replace it.
            if (JS_GetProperty(cx, arg, "x", &x) == JS_FALSE || JS_GetProperty(cx, arg, "y", &y) == JS_FALSE)
                return JS_FALSE;
            if (!JSVAL_IS_INT(x) || !JSVAL_IS_INT(y))
                THROW_ERROR(cx, "Input has an x or y, but they aren't the correct type!");
            int32 ix, iy;
            if (JS_ValueToInt32(cx, x, &ix) == JS_FALSE || JS_ValueToInt32(cx, y, &iy) == JS_FALSE)
                THROW_ERROR(cx, "Failed to convert x and/or y values");
            POINT result = {ix, iy};
            AutomapToScreen(&result);
            x = INT_TO_JSVAL(result.x);
            y = INT_TO_JSVAL(result.y);
            JSObject* res = JS_NewObject(cx, NULL, NULL, NULL);
            if (!res)
                return JS_FALSE;
            // These writes fail only with an exception already pending (out of memory, or a script setter on Object.prototype).
            if (JS_SetProperty(cx, res, "x", &x) == JS_FALSE || JS_SetProperty(cx, res, "y", &y) == JS_FALSE)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
        } else
            THROW_ERROR(cx, "Invalid object specified to automapToScreen");
    } else if (argc == 2) {
        // the args must be ints
        if (JSVAL_IS_INT(argv[0]) && JSVAL_IS_INT(argv[1])) {
            int32 ix, iy;
            JS_BeginRequest(cx);
            if (JS_ValueToInt32(cx, argv[0], &ix) == JS_FALSE || JS_ValueToInt32(cx, argv[1], &iy) == JS_FALSE) {
                JS_EndRequest(cx);
                THROW_ERROR(cx, "Failed to convert x and/or y values");
            }
            JS_EndRequest(cx);
            // convert the values
            POINT result = {ix, iy};
            AutomapToScreen(&result);
            argv[0] = INT_TO_JSVAL(result.x);
            argv[1] = INT_TO_JSVAL(result.y);
            JSObject* res = JS_NewObject(cx, NULL, NULL, NULL);
            if (!res)
                return JS_FALSE;
            if (JS_SetProperty(cx, res, "x", &argv[0]) == JS_FALSE || JS_SetProperty(cx, res, "y", &argv[1]) == JS_FALSE)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
        } else
            THROW_ERROR(cx, "automapToScreen expects two arguments to be two integers");
    } else
        THROW_ERROR(cx, "Invalid arguments specified for automapToScreen");
    return JS_TRUE;
}
