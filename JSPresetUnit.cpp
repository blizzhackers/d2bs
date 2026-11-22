#include "JSPresetUnit.h"

#include "D2Ptrs.h"
#include "CriticalSections.h"
#include "D2Helpers.h"

EMPTY_CTOR(presetunit)

namespace {
// Reads the area, type or class id argument. undefined and null give 0 (any type or class); other values convert as
// JavaScript numbers ("" and false give 0 too), and one that converts to NaN ("obj", {}) gives a value no preset has.
// A failed conversion (a valueOf that throws) leaves its exception pending and returns false.
JSBool PresetArg(JSContext* cx, jsval v, uint32* pOut) {
    *pOut = 0;
    if (JSVAL_IS_VOID(v) || JSVAL_IS_NULL(v))
        return JS_TRUE;
    double d = 0;
    JS_BeginRequest(cx);
    JSBool bOk = JS_ValueToNumber(cx, v, &d);
    JS_EndRequest(cx);
    if (bOk)
        *pOut = d != d ? (uint32)-1 : JS_DoubleToUint32(d);
    return bOk;
}
} // namespace

void presetunit_finalize(JSFreeOp* fop, JSObject* obj) {
    myPresetUnit* pUnit = (myPresetUnit*)JS_GetPrivate(obj);

    if (pUnit) {
        JS_SetPrivate(obj, NULL);
        delete pUnit;
    }
}

JSAPI_PROP(presetunit_getProperty) {
    myPresetUnit* pUnit = (myPresetUnit*)JS_GetPrivate(cx, obj);

    if (!pUnit)
        return JS_TRUE;

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case PUNIT_TYPE:
        vp.setInt32(pUnit->dwType);
        break;
    case PUNIT_ROOMX:
        vp.setInt32(pUnit->dwRoomX);
        break;
    case PUNIT_ROOMY:
        vp.setInt32(pUnit->dwRoomY);
        break;
    case PUNIT_X:
        vp.setInt32(pUnit->dwPosX);
        break;
    case PUNIT_Y:
        vp.setInt32(pUnit->dwPosY);
        break;
    case PUNIT_ID:
        vp.setInt32(pUnit->dwId);
        break;
    case PUINT_LEVEL:
        vp.setInt32(pUnit->dwLevel);
    default:
        break;
    }
    return JS_TRUE;
}

JSAPI_FUNC(my_getPresetUnits) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 1) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }

    // The area, type and class id convert before the level is looked up: a conversion can run script code (valueOf),
    // which must not run while this native holds a level pointer.
    uint32 levelId;
    uint32 nClassId = NULL;
    uint32 nType = NULL;
    if (!PresetArg(cx, JS_ARGV(cx, vp)[0], &levelId))
        return JS_FALSE;
    if (argc >= 2 && !PresetArg(cx, JS_ARGV(cx, vp)[1], &nType))
        return JS_FALSE;
    if (argc >= 3 && !PresetArg(cx, JS_ARGV(cx, vp)[2], &nClassId))
        return JS_FALSE;

    Level* pLevel = GetLevel(levelId);

    if (!pLevel)
        THROW_ERROR(cx, "getPresetUnits failed, couldn't access the level!");

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;

    bool bAddedRoom = FALSE;
    DWORD dwArrayCount = NULL;

    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    if (!pReturnArray || !JS_AddRoot(cx, &pReturnArray)) {
        delete cRoom;
        return JS_FALSE;
    }

    // An engine failure (out of memory) still unloads the room data and the root, then goes back to the engine as one.
    JSBool bOk = JS_TRUE;
    JS_BeginRequest(cx);
    for (Room2* pRoom = pLevel->pRoom2First; pRoom && bOk; pRoom = pRoom->pRoom2Next) {
        bAddedRoom = FALSE;

        if (!pRoom->pPreset) {
            D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
            bAddedRoom = TRUE;
        }

        for (PresetUnit* pUnit = pRoom->pPreset; pUnit && bOk; pUnit = pUnit->pPresetNext) {
            // Does it fit?
            if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
                myPresetUnit* mypUnit = new myPresetUnit;

                mypUnit->dwPosX = pUnit->dwPosX;
                mypUnit->dwPosY = pUnit->dwPosY;
                mypUnit->dwRoomX = pRoom->dwPosX;
                mypUnit->dwRoomY = pRoom->dwPosY;
                mypUnit->dwType = pUnit->dwType;
                mypUnit->dwId = pUnit->dwTxtFileNo;
                mypUnit->dwLevel = levelId;

                // BuildObject attaches the private only when it succeeds.
                JSObject* unit = BuildObject(cx, &presetunit_class, NULL, presetunit_props, mypUnit);
                if (!unit) {
                    delete mypUnit;
                    bOk = JS_FALSE;
                    break;
                }

                jsval a = OBJECT_TO_JSVAL(unit);
                bOk = JS_SetElement(cx, pReturnArray, dwArrayCount, &a);

                dwArrayCount++;
            }
        }

        if (bAddedRoom) {
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
            bAddedRoom = FALSE;
        }
    }
    if (bOk)
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));

    JS_RemoveRoot(cx, &pReturnArray);
    JS_EndRequest(cx);
    delete cRoom;
    return bOk;
}

JSAPI_FUNC(my_getPresetUnit) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 1) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }

    // The area, type and class id convert before the level is looked up: a conversion can run script code (valueOf),
    // which must not run while this native holds a level pointer.
    uint32 levelId;
    uint32 nClassId = NULL;
    uint32 nType = NULL;
    if (!PresetArg(cx, JS_ARGV(cx, vp)[0], &levelId))
        return JS_FALSE;
    if (argc >= 2 && !PresetArg(cx, JS_ARGV(cx, vp)[1], &nType))
        return JS_FALSE;
    if (argc >= 3 && !PresetArg(cx, JS_ARGV(cx, vp)[2], &nClassId))
        return JS_FALSE;

    Level* pLevel = GetLevel(levelId);

    if (!pLevel)
        THROW_ERROR(cx, "getPresetUnits failed, couldn't access the level!");

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;

    bool bAddedRoom = FALSE;

    for (Room2* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {

        bAddedRoom = FALSE;

        if (!pRoom->pRoom1) {
            D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
            bAddedRoom = TRUE;
        }

        for (PresetUnit* pUnit = pRoom->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
            // Does it fit?
            if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
                // Yes it fits! Return it
                myPresetUnit* mypUnit = new myPresetUnit;

                mypUnit->dwPosX = pUnit->dwPosX;
                mypUnit->dwPosY = pUnit->dwPosY;
                mypUnit->dwRoomX = pRoom->dwPosX;
                mypUnit->dwRoomY = pRoom->dwPosY;
                mypUnit->dwType = pUnit->dwType;
                mypUnit->dwId = pUnit->dwTxtFileNo;
                mypUnit->dwLevel = levelId;

                // BuildObject attaches the private only when it succeeds. An engine failure (out of memory) still
                // unloads the room data, then goes back to the engine as one.
                JSObject* obj = BuildObject(cx, &presetunit_class, NULL, presetunit_props, mypUnit);
                if (!obj) {
                    delete mypUnit;
                    if (bAddedRoom)
                        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                    delete cRoom;
                    return JS_FALSE;
                }
                JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
                // The unit keeps copies of the preset's values, so the room loaded for the lookup unloads here too.
                if (bAddedRoom)
                    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                delete cRoom;
                return JS_TRUE;
            }
        }

        if (bAddedRoom) {
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
            bAddedRoom = FALSE;
        }
    }
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    delete cRoom;
    return JS_TRUE;
}
