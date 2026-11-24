//#include "Control.h"
#include "JSControl.h"
#include "Helpers.h"
#include "D2Helpers.h"

EMPTY_CTOR(control)

void control_finalize(JSFreeOp* fop, JSObject* obj) {
    ControlData* pData = ((ControlData*)JS_GetPrivate(obj));

    if (pData) {
        JS_SetPrivate(obj, NULL);
        delete pData;
    }
}

JSAPI_PROP(control_getProperty) {
    // A control that is gone (screen changed, not at the menu) reads as undefined: a JS_FALSE with nothing pending
    // would end the script with no error.
    vp.setUndefined();
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, obj, &control_class, NULL));
    if (!pData)
        return JS_TRUE;

    Control* ctrl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (!ctrl)
        return JS_TRUE;

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    JS_BeginRequest(cx);

    JSType a = JS_TypeOfValue(cx, ID);

    if (JSID_IS_STRING(id)) {
        // TODO: Figure out what this is for
        // JSString* b = JSVAL_TO_STRING(ID);
        // char* pText = JS_EncodeString(cx, b);

        JS_EndRequest(cx);
        return JS_TRUE;
    }
    if (JSID_IS_VOID(id)) {
        JS_EndRequest(cx);
        return JS_TRUE;
    }
    if (JSID_IS_ZERO(id)) {
        JS_EndRequest(cx);
        return JS_TRUE;
    }
    switch (JSID_TO_INT(id)) {
    case CONTROL_TEXT:
        if (ctrl->dwIsCloaked != 33) {
            if (!StoreString(vp, JS_InternUCString(cx, ctrl->dwType == 6 ? ctrl->wText2 : ctrl->wText))) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
        }
        break;
    case CONTROL_X:
        vp.setNumber((double)ctrl->dwPosX);
        // JS_NewNumberValue(cx, ctrl->dwPosX, vp);
        break;
    case CONTROL_Y:
        vp.setNumber((double)ctrl->dwPosY);
        break;
    case CONTROL_XSIZE:
        vp.setNumber((double)ctrl->dwSizeX);
        break;
    case CONTROL_YSIZE:
        vp.setNumber((double)ctrl->dwSizeY);
        break;
    case CONTROL_STATE:
        vp.setNumber((double)(ctrl->dwDisabled - 2));
        break;
    case CONTROL_MAXLENGTH:
        // JS_NewNumberValue(cx, ctrl->dwMaxLength, vp);
        break;
    case CONTROL_TYPE:
        vp.setNumber((double)ctrl->dwType);
        break;
    case CONTROL_VISIBLE:
        // nothing to do yet because we don't know what to do
        break;
    case CONTROL_CURSORPOS:
        // dwCursorPos is an edit-box (type 1) field; other control types keep other data at that offset.
        if (ctrl->dwType == 1)
            vp.setNumber((double)ctrl->dwCursorPos);
        break;
    case CONTROL_SELECTSTART:
        vp.setNumber((double)ctrl->dwSelectStart);
        break;
    case CONTROL_SELECTEND:
        vp.setNumber((double)ctrl->dwSelectEnd);
        break;
    case CONTROL_PASSWORD:
        vp.setBoolean(!!(ctrl->dwIsCloaked == 33));
        break;
    case CONTROL_DISABLED:
        vp.setNumber((double)ctrl->dwDisabled);
        break;
    }
    JS_EndRequest(cx);
    return JS_TRUE;
}

JSAPI_STRICT_PROP(control_setProperty) {
    // A write to a control that is gone is dropped: a JS_FALSE with nothing pending would end the script with no error.
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, obj, &control_class, NULL)); // JS_THIS_OBJECT(cx, &vp.get())));
    if (!pData)
        return JS_TRUE;

    Control* ctrl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (!ctrl)
        return JS_TRUE;

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case CONTROL_TEXT:
        if (ctrl->dwType == 1 && vp.isString()) {
            const wchar_t* szwText = JS_GetStringCharsZ(cx, vp.toString());
            if (!szwText)
                return JS_FALSE;
            D2WIN_SetControlText(ctrl, szwText);
        }
        break;
    case CONTROL_STATE:
        if (vp.isInt32()) {
            int32 nState;
            JS_BeginRequest(cx);
            if (!JS_ValueToECMAInt32(cx, vp.get(), &nState) || nState < 0 || nState > 3) {
                JS_EndRequest(cx);
                THROW_ERROR(cx, "Invalid state value");
            }
            ctrl->dwDisabled = (DWORD)(nState + 2);
            JS_EndRequest(cx);
        }
        break;
    case CONTROL_CURSORPOS:
        if (ctrl->dwType == 1 && vp.isInt32()) {
            JS_BeginRequest(cx);
            uint32 dwPos;
            if (!JS_ValueToECMAUint32(cx, vp.get(), &dwPos)) {
                JS_EndRequest(cx);
                THROW_ERROR(cx, "Invalid cursor position value");
            }
            JS_EndRequest(cx);
            ctrl->dwCursorPos = dwPos;
        }
        break;
    case CONTROL_DISABLED:
        if (vp.isInt32()) {
            ctrl->dwDisabled = (DWORD)vp.toInt32();
        }
        break;
    }

    return JS_TRUE;
}

JSAPI_FUNC(control_getNext) {
    // Every early JS_TRUE return must leave a value: an unset rval hands the script this function (truthy).
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &control_class, NULL));
    if (!pData)
        return JS_TRUE;

    Control* pControl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (pControl && pControl->pNext)
        pControl = pControl->pNext;
    else
        pControl = NULL;

    if (pControl) {
        pData->pControl = pControl;
        pData->dwType = pData->pControl->dwType;
        pData->dwX = pData->pControl->dwPosX;
        pData->dwY = pData->pControl->dwPosY;
        pData->dwSizeX = pData->pControl->dwSizeX;
        pData->dwSizeY = pData->pControl->dwSizeY;
        JS_SetPrivate(cx, JS_THIS_OBJECT(cx, vp), pData);
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(JS_THIS_OBJECT(cx, vp)));
    } else {
        //		JSObject* obj = JS_THIS_OBJECT(cx, vp);
        ////		JS_ClearScope(cx, obj);
        //		if(JS_ValueToObject(cx, JSVAL_NULL, &obj) == JS_FALSE)  //leaks, prevents finilize
        //			return JS_TRUE;
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    }

    return JS_TRUE;
}

JSAPI_FUNC(control_click) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &control_class, NULL));
    if (!pData)
        return JS_TRUE;

    Control* pControl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (!pControl)
        return JS_TRUE;

    uint32 x = (uint32)-1, y = (uint32)-1;

    if (argc > 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0]) && JSVAL_IS_INT(JS_ARGV(cx, vp)[1])) {
        JS_BeginRequest(cx);
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &x);
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &y);
        JS_EndRequest(cx);
    }

    clickControl(pControl, x, y);

    return JS_TRUE;
}

JSAPI_FUNC(control_setText) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &control_class, NULL));
    if (!pData)
        return JS_TRUE;

    Control* pControl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (!pControl)
        return JS_TRUE;

    if (argc < 1)
        THROW_ERROR(cx, "Invalid arguments");
    if (!JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    const wchar_t* szwText = JS_GetStringCharsZ(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
    if (!szwText)
        return JS_FALSE;

    D2WIN_SetControlText(pControl, szwText);
    return JS_TRUE;
}

JSAPI_FUNC(control_getText) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    ControlData* pData = ((ControlData*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &control_class, NULL));
    if (!pData)
        return JS_TRUE;

    Control* pControl = findControl(pData->dwType, (const wchar_t*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
    if (!pControl)
        return JS_TRUE;

    if (pControl->dwType != 4 || !pControl->pFirstText)
        return JS_TRUE;
    JS_BeginRequest(cx);
    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    if (!pReturnArray) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));

    int nArrayCount = 0;

    for (ControlText* pText = pControl->pFirstText; pText; pText = pText->pNext) {
        if (!pText->wText[0])
            continue;

        if (pText->wText[1]) {
            JSObject* pSubArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pSubArray) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }

            for (int i = 0; i < 5; i++) {
                if (pText->wText[i]) {
                    JSString* str = JS_NewUCStringCopyZ(cx, pText->wText[i]);
                    if (!str) {
                        JS_EndRequest(cx);
                        return JS_FALSE;
                    }
                    jsval aString = STRING_TO_JSVAL(str);
                    if (!JS_SetElement(cx, pSubArray, i, &aString)) {
                        JS_EndRequest(cx);
                        return JS_FALSE;
                    }
                }
            }

            jsval sub = OBJECT_TO_JSVAL(pSubArray);
            if (!JS_SetElement(cx, pReturnArray, nArrayCount, &sub)) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
        } else {
            JSString* str = JS_NewUCStringCopyZ(cx, pText->wText[0]);
            if (!str) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
            jsval aString = STRING_TO_JSVAL(str);
            if (!JS_SetElement(cx, pReturnArray, nArrayCount, &aString)) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
        }

        nArrayCount++;
    }
    JS_EndRequest(cx);

    return JS_TRUE;
}

JSAPI_FUNC(my_getControl) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    int32 nType = -1, nX = -1, nY = -1, nXSize = -1, nYSize = -1;
    int32* args[] = {&nType, &nX, &nY, &nXSize, &nYSize};
    JS_BeginRequest(cx);
    for (uint i = 0; i < argc; i++)
        if (JSVAL_IS_INT(JS_ARGV(cx, vp)[i]))
            JS_ValueToECMAInt32(cx, JS_ARGV(cx, vp)[i], args[i]);
    JS_EndRequest(cx);

    Control* pControl = findControl(nType, (const wchar_t*)NULL, -1, nX, nY, nXSize, nYSize);
    if (!pControl)
        return JS_TRUE;

    ControlData* data = new ControlData;
    data->dwType = pControl->dwType;
    data->dwX = pControl->dwPosX;
    data->dwY = pControl->dwPosY;
    data->dwSizeX = pControl->dwSizeX;
    data->dwSizeY = pControl->dwSizeY;
    data->pControl = pControl;

    JSObject* control = BuildObject(cx, &control_class, control_funcs, control_props, data);
    // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
    if (!control) {
        delete data;
        THROW_ERROR(cx, "Failed to build control!");
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(control));
    return JS_TRUE;
}
JSAPI_FUNC(my_getControls) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    if (ClientState() != ClientStateMenu)
        return JS_TRUE;

    DWORD dwArrayCount = NULL;

    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    JS_BeginRequest(cx);
    if (!pReturnArray || !JS_AddRoot(cx, &pReturnArray)) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }
    for (Control* pControl = *p_D2WIN_FirstControl; pControl; pControl = pControl->pNext) {
        ControlData* data = new ControlData;
        data->dwType = pControl->dwType;
        data->dwX = pControl->dwPosX;
        data->dwY = pControl->dwPosY;
        data->dwSizeX = pControl->dwSizeX;
        data->dwSizeY = pControl->dwSizeY;
        data->pControl = pControl;

        JSObject* res = BuildObject(cx, &control_class, control_funcs, control_props, data);
        if (!res) {
            delete data;
            JS_RemoveRoot(cx, &pReturnArray);
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        jsval a = OBJECT_TO_JSVAL(res);
        if (!JS_SetElement(cx, pReturnArray, dwArrayCount, &a)) {
            JS_RemoveRoot(cx, &pReturnArray);
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        dwArrayCount++;
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
    JS_RemoveRoot(cx, &pReturnArray);
    JS_EndRequest(cx);
    return JS_TRUE;
}
