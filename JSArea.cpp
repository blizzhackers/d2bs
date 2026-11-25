#include "JSArea.h"
#include "D2Ptrs.h"
#include "JSExits.h"
#include "CriticalSections.h"
#include "MapHeader.h"

EMPTY_CTOR(area)

void area_finalize(JSFreeOp* fop, JSObject* obj) {
    myArea* pArea = (myArea*)JS_GetPrivate(obj);

    if (pArea) {

        JS_SetPrivate(obj, NULL);
        delete pArea;
    }
}

JSAPI_PROP(area_getProperty) {
    // Class-checked: a getter reached through an object that inherits from an area gets that object as obj, which
    // has no area private to read and no exits slot to cache in. It throws like a level that is not loaded.
    myArea* pArea = (myArea*)JS_GetInstancePrivate(cx, obj, &area_class, NULL);
    if (!pArea)
        THROW_ERROR(cx, "Area not loaded");

    // Waits out a load like getPath, so a read mid-walk does not throw; a blocking listener fails fast instead.
    if (!WaitForGameReady())
        THROW_ERROR(cx, "Area not loaded");

    // The level is looked up and read under one hold of the game lock, which the game thread needs to free an act's
    // levels; the hold ends before any value is built, since building one can run the GC.
    bool bLoaded = false;
    DWORD dwPosX = 0, dwPosY = 0, dwSizeX = 0, dwSizeY = 0, dwLevelNo = 0;
    {
        AutoCriticalRoom cRoom;
        Level* pLevel = GetLevel(pArea->AreaId);
        if (pLevel) {
            bLoaded = true;
            dwPosX = pLevel->dwPosX;
            dwPosY = pLevel->dwPosY;
            dwSizeX = pLevel->dwSizeX;
            dwSizeY = pLevel->dwSizeY;
            dwLevelNo = pLevel->dwLevelNo;
        }
    }
    if (!bLoaded)
        THROW_ERROR(cx, "Area not loaded");

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case AUNIT_EXITS: {
        JS_BeginRequest(cx);
        // Cached in the area's reserved slot, so the GC keeps the list exactly as long as the area object.
        jsval exitList = JS_GetReservedSlot(obj, 0);
        if (JSVAL_IS_VOID(exitList)) {
            JS::RootedObject exitArray(cx, JS_NewArrayObject(cx, 0, NULL));
            if (!exitArray) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }

            ExitArray exits;
            bool bMapped = false;
            {
                ScopedActMap map(pArea->AreaId);
                bMapped = map.ok();
                if (bMapped)
                    map->GetExits(exits);
            }
            // The act can change between the read above and this map; that is the same not-loaded case.
            if (!bMapped) {
                JS_EndRequest(cx);
                THROW_ERROR(cx, "Area not loaded");
            }
            int count = exits.size();
            for (int i = 0; i < count; i++) {
                myExit* exit = new myExit;
                exit->id = exits[i].Target;
                exit->x = exits[i].Position.first;
                exit->y = exits[i].Position.second;
                exit->type = exits[i].Type;
                exit->tileid = exits[i].TileId;
                exit->level = pArea->AreaId;

                JSObject* pExit = BuildObject(cx, &exit_class, NULL, exit_props, exit);
                if (!pExit) {
                    delete exit;
                    // A partial list is not kept, so the next read builds the exits again.
                    JS_EndRequest(cx);
                    THROW_ERROR(cx, "Failed to create exit object!");
                }
                // Defined, not set: an inherited index setter would take the exit instead of the array and run
                // script code mid-build, which could read exits again and cache a second list.
                if (!JS_DefineElement(cx, exitArray, i, OBJECT_TO_JSVAL(pExit), NULL, NULL, JSPROP_ENUMERATE)) {
                    JS_EndRequest(cx);
                    return JS_FALSE;
                }
            }
            exitList = OBJECT_TO_JSVAL(exitArray);
            JS_SetReservedSlot(obj, 0, exitList);
        }
        vp.set(exitList);
    }
        JS_EndRequest(cx);
        break;
    case AUNIT_NAME: {
        LevelTxt* pTxt = D2COMMON_GetLevelText(pArea->AreaId);
        if (pTxt) {
            JSString* name = JS_InternString(cx, pTxt->szName);
            if (!name)
                return JS_FALSE;
            vp.setString(name);
        }
    } break;
    case AUNIT_X:
        vp.setInt32(dwPosX);
        break;
    case AUNIT_Y:
        vp.setInt32(dwPosY);
        break;
    case AUNIT_XSIZE:
        vp.setInt32(dwSizeX);
        break;
    case AUNIT_YSIZE:
        vp.setInt32(dwSizeY);
        break;
    case AUNIT_ID:
        vp.setInt32(dwLevelNo);
        break;
    }

    return JS_TRUE;
}

JSAPI_FUNC(my_getArea) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    if (!WaitForGameReady())
        THROW_ERROR(cx, "Get Area: Game not ready");

    int32 nArea = GetPlayerArea();

    if (argc == 1) {

        if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0])) {
            JS_BeginRequest(cx);
            JS_ValueToECMAInt32(cx, JS_ARGV(cx, vp)[0], &nArea);
            JS_EndRequest(cx);
        } else
            THROW_ERROR(cx, "Invalid parameter passed to getArea!");
    }

    if (nArea < 0)
        THROW_ERROR(cx, "Invalid parameter passed to getArea!");

    Level* pLevel = GetLevel(nArea);

    if (!pLevel) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }

    myArea* pArea = new myArea;
    if (!pArea) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }

    pArea->AreaId = nArea;

    JSObject* unit = BuildObject(cx, &area_class, NULL, area_props, pArea);
    if (!unit) {
        delete pArea;
        pArea = NULL;
        THROW_ERROR(cx, "Failed to build area unit!");
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(unit));

    return JS_TRUE;
}
