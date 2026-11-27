#include "JSRoom.h"
#include "CriticalSections.h"
#include "JSPresetUnit.h"
#include "D2Structs.h"
#include "D2Ptrs.h"
#include "D2BS.h"
#include "D2Helpers.h"
#include "JSParty.h"
#include "JSUnit.h"

EMPTY_CTOR(room)

namespace {
// A Room object keeps this copy instead of the game's Room2 pointer: the game frees a level's rooms on its own schedule (a
// level once it is idle, which can be soon after it is built, and every level at an act change), while a script can hold
// the object indefinitely. A room's level and position never change within a game, so they answer the properties and,
// with the game session, find the live room again.
struct RoomSnapshot {
    DWORD dwPrivateType; // PRIVATE_ROOM, first so natives that read any private's type turn a Room away
    DWORD dwSession;     // Vars.dwGameSession when built
    DWORD dwLevelNo;
    DWORD dwPosX;
    DWORD dwPosY;
    DWORD dwSizeX;
    DWORD dwSizeY;
    bool bHasNumber;
    DWORD dwNumber;
    DWORD dwStaffTombLevel;
};

// The caller holds the room lock and pRoom2 is live. Leaves the type and session alone.
void FillSnapshot(RoomSnapshot* pSnap, Room2* pRoom2) {
    Level* pLevel = pRoom2->pLevel;
    pSnap->dwLevelNo = pLevel ? pLevel->dwLevelNo : 0;
    pSnap->dwPosX = pRoom2->dwPosX;
    pSnap->dwPosY = pRoom2->dwPosY;
    pSnap->dwSizeX = pRoom2->dwSizeX;
    pSnap->dwSizeY = pRoom2->dwSizeY;
    pSnap->bHasNumber = pRoom2->dwPresetType != 2 || pRoom2->pType2Info;
    pSnap->dwNumber = pRoom2->dwPresetType != 2 ? (DWORD)-1 : pRoom2->pType2Info ? pRoom2->pType2Info->dwRoomNumber : 0;
    pSnap->dwStaffTombLevel = pLevel && pLevel->pMisc ? pLevel->pMisc->dwStaffTombLevel : 0;
}

RoomSnapshot* GetSnapshot(JSContext* cx, JSObject* obj) {
    return obj ? (RoomSnapshot*)JS_GetInstancePrivate(cx, obj, &room_class, NULL) : NULL;
}

// The caller holds the room lock. Looks only in the game the room came from and the player's current act. An act change
// frees the old act before the player's own pointer moves on, so that pointer is trusted only while it is the client's
// loaded act. A level whose rooms the game freed is rebuilt, as getRoom(area) does, but only while the game is ready: a
// player busy between steps can still walk a loaded level, as before.
Level* FindRoomLevel(const RoomSnapshot* pSnap) {
    if (!pSnap || !pSnap->dwLevelNo || !(pSnap->dwSession & 1) || pSnap->dwSession != Vars.dwGameSession)
        return NULL;

    if (!PlayerActLoaded())
        return NULL;
    Act* pAct = *p_D2CLIENT_CurrentAct;

    for (Level* pLevel = pAct->pMisc->pLevelFirst; pLevel; pLevel = pLevel->pNextLevel) {
        if (pLevel->dwLevelNo != pSnap->dwLevelNo)
            continue;
        if (!pLevel->pRoom2First && GameReady())
            D2COMMON_InitLevel(pLevel);
        return pLevel->pRoom2First ? pLevel : NULL;
    }
    return NULL;
}

// The caller holds the room lock.
Room2* FindLiveRoom(const RoomSnapshot* pSnap) {
    Level* pLevel = FindRoomLevel(pSnap);
    if (!pLevel)
        return NULL;

    for (Room2* pRoom2 = pLevel->pRoom2First; pRoom2; pRoom2 = pRoom2->pRoom2Next) {
        if (pRoom2->dwPosX == pSnap->dwPosX && pRoom2->dwPosY == pSnap->dwPosY)
            return pRoom2;
    }
    return NULL;
}

// The caller holds the room lock and pRoom2, if not NULL, is live. Builds nothing once the game has been left, as getParty
// does: a Room carries an open session, so it can never match a room in the next game.
JSObject* BuildRoomObject(JSContext* cx, Room2* pRoom2) {
    if (!pRoom2 || !OpenGameSession())
        return NULL;

    RoomSnapshot* pSnap = new RoomSnapshot;
    pSnap->dwPrivateType = PRIVATE_ROOM;
    pSnap->dwSession = Vars.dwGameSession;
    FillSnapshot(pSnap, pRoom2);

    JSObject* jsroom = BuildObject(cx, &room_class, room_methods, room_props, pSnap);
    if (!jsroom)
        delete pSnap;
    return jsroom;
}
} // namespace

void room_finalize(JSFreeOp* fop, JSObject* obj) {
    RoomSnapshot* pSnap = (RoomSnapshot*)JS_GetPrivate(obj);

    if (pSnap) {
        JS_SetPrivate(obj, NULL);
        delete pSnap;
    }
}

JSAPI_PROP(room_getProperty) {
    RoomSnapshot* pSnap = GetSnapshot(cx, obj);

    if (!pSnap)
        return JS_TRUE;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case ROOM_NUM:
        if (pSnap->bHasNumber)
            vp.setInt32(pSnap->dwNumber);
        break;
    case ROOM_XPOS:
        vp.setInt32(pSnap->dwPosX);
        break;
    case ROOM_YPOS:
        vp.setInt32(pSnap->dwPosY);
        break;
    case ROOM_XSIZE:
        vp.setInt32(pSnap->dwSizeX * 5);
        break;
    case ROOM_YSIZE:
        vp.setInt32(pSnap->dwSizeY * 5);
        break;
    case ROOM_AREA:
    case ROOM_LEVEL:
        if (pSnap->dwLevelNo)
            vp.setInt32(pSnap->dwLevelNo);
        break;

    case ROOM_CORRECTTOMB:
        if (pSnap->dwStaffTombLevel)
            vp.setInt32(pSnap->dwStaffTombLevel);
        break;

    default:
        break;
    }

    return JS_TRUE;
}

JSAPI_FUNC(room_getNext) {
    // Every Room method reads `this` before it sets rval: CallArgs.h documents that JS_THIS_OBJECT may need the callee in
    // vp[0].
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);
    if (!pSnap)
        return JS_TRUE;

    AutoCriticalRoom cRoom;
    Room2* pRoom2 = FindLiveRoom(pSnap);
    if (!pRoom2 || !pRoom2->pRoom2Next)
        return JS_TRUE;

    FillSnapshot(pSnap, pRoom2->pRoom2Next);
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(room_getPresetUnits) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);

    // A foreign this gets the neutral result before any argument runs script code.
    if (!pSnap)
        return JS_TRUE;

    // The type and class id convert before the game lock is taken, since a valueOf run under it stalls the game thread.
    // Each converts as `value >>> 0`; a result of 0, which a value that converts to NaN also gives, means any.
    uint32 nType = 0;
    uint32 nClass = 0;
    JS_BeginRequest(cx);
    JSBool bConverted = (argc < 1 || JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &nType)) &&
                        (argc < 2 || JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nClass));
    JS_EndRequest(cx);
    if (!bConverted)
        return JS_FALSE;

    bool bAdded = FALSE;
    DWORD dwArrayCount = NULL;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    if (!pReturnArray || !JS_AddRoot(cx, &pReturnArray)) {
        delete cRoom;
        return JS_FALSE;
    }

    // A room's preset list outlives its room data, and loading the data again appends another copy of some presets, so
    // the room is loaded only when it has neither.
    if (!pRoom2->pPreset && !pRoom2->pRoom1) {
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        // Unloaded afterwards only if the load built the room: a load that failed left nothing to remove.
        bAdded = pRoom2->pRoom1 != NULL;
    }

    // An engine failure (out of memory) still unloads the room data and the root, then goes back to the engine as one.
    JSBool bOk = JS_TRUE;
    for (PresetUnit* pUnit = pRoom2->pPreset; pUnit && bOk; pUnit = pUnit->pPresetNext) {
        if ((pUnit->dwType == nType || nType == NULL) && (pUnit->dwTxtFileNo == nClass || nClass == NULL)) {
            myPresetUnit* mypUnit = new myPresetUnit;

            mypUnit->dwPosX = pUnit->dwPosX;
            mypUnit->dwPosY = pUnit->dwPosY;
            mypUnit->dwRoomX = pRoom2->dwPosX;
            mypUnit->dwRoomY = pRoom2->dwPosY;
            mypUnit->dwType = pUnit->dwType;
            mypUnit->dwId = pUnit->dwTxtFileNo;
            mypUnit->dwLevel = pRoom2->pLevel->dwLevelNo;

            // BuildObject attaches the private only when it succeeds.
            JSObject* jsUnit = BuildObject(cx, &presetunit_class, NULL, presetunit_props, mypUnit);
            if (!jsUnit) {
                delete mypUnit;
                bOk = JS_FALSE;
                break;
            }

            // Defined, not set: an Array.prototype index setter would run script with the game lock held.
            JS_BeginRequest(cx);
            bOk = JS_DefineElement(cx, pReturnArray, dwArrayCount, OBJECT_TO_JSVAL(jsUnit), NULL, NULL, JSPROP_ENUMERATE);
            JS_EndRequest(cx);
            dwArrayCount++;
        }
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

    if (bOk)
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
    JS_RemoveRoot(cx, &pReturnArray);
    delete cRoom;
    return bOk;
}
JSAPI_FUNC(room_getCollisionTypeArray) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);

    bool bAdded = FALSE;
    CollMap* pCol = NULL;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    if (!pRoom2->pRoom1) {
        LoadRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        bAdded = pRoom2->pRoom1 != NULL;
    }

    JSObject* jsobjy = JS_NewUint16Array(cx, (pRoom2->dwSizeX * 5) * (pRoom2->dwSizeY * 5));
    if (!jsobjy || !JS_AddRoot(cx, &jsobjy)) {
        if (bAdded)
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        delete cRoom;
        return JS_FALSE;
    }

    if (pRoom2->pRoom1)
        pCol = pRoom2->pRoom1->Coll;

    if (!pCol || !pCol->pMapStart) {
        JS_RemoveRoot(cx, &jsobjy);
        if (bAdded)
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        delete cRoom;
        return JS_TRUE;
    }

    int x = pCol->dwPosGameX - pRoom2->pLevel->dwPosX * 5;
    int y = pCol->dwPosGameY - pRoom2->pLevel->dwPosY * 5;
    int nCx = pCol->dwSizeGameX;
    int nCy = pCol->dwSizeGameY;

    int nLimitX = x + nCx;
    int nLimitY = y + nCy;

    int nCurrentArrayY = NULL;

    // An engine failure (out of memory) still unloads the room data and the root, then goes back to the engine as one.
    JSBool bOk = JS_TRUE;
    WORD* p = pCol->pMapStart;
    JS_BeginRequest(cx);
    for (int j = y; j < nLimitY && bOk; j++) {
        int nCurrentArrayX = 0;
        for (int i = x; i < nLimitX && bOk; i++) {
            jsval nNode = INT_TO_JSVAL(*p);
            bOk = JS_SetElement(cx, jsobjy, nCurrentArrayY * nCx + nCurrentArrayX, &nNode);
            nCurrentArrayX++;
            p++;
        }
        nCurrentArrayY++;
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

    if (bOk)
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsobjy));
    JS_RemoveRoot(cx, &jsobjy);
    JS_EndRequest(cx);
    delete cRoom;
    return bOk;
}
JSAPI_FUNC(room_getCollision) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);

    bool bAdded = FALSE;
    CollMap* pCol = NULL;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    JSObject* jsobjy = JS_NewArrayObject(cx, NULL, NULL);
    if (!jsobjy || !JS_AddRoot(cx, &jsobjy)) {
        delete cRoom;
        return JS_FALSE;
    }

    if (!pRoom2->pRoom1) {
        LoadRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        bAdded = pRoom2->pRoom1 != NULL;
    }

    if (pRoom2->pRoom1)
        pCol = pRoom2->pRoom1->Coll;

    if (!pCol || !pCol->pMapStart) {
        JS_RemoveRoot(cx, &jsobjy);
        if (bAdded)
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        delete cRoom;
        return JS_TRUE;
    }

    int x = pCol->dwPosGameX - pRoom2->pLevel->dwPosX * 5;
    int y = pCol->dwPosGameY - pRoom2->pLevel->dwPosY * 5;
    int nCx = pCol->dwSizeGameX;
    int nCy = pCol->dwSizeGameY;

    int nLimitX = x + nCx;
    int nLimitY = y + nCy;

    int nCurrentArrayY = NULL;

    // An engine failure (out of memory) still unloads the room data and the root, then goes back to the engine as one.
    JSBool bOk = JS_TRUE;
    WORD* p = pCol->pMapStart;
    JS_BeginRequest(cx);
    for (int j = y; j < nLimitY && bOk; j++) {
        JSObject* jsobjx = JS_NewArrayObject(cx, NULL, NULL);
        if (!jsobjx) {
            bOk = JS_FALSE;
            break;
        }

        // Defined, not set: an Array.prototype index setter would run script with the game lock held.
        int nCurrentArrayX = 0;
        for (int i = x; i < nLimitX && bOk; i++) {
            bOk = JS_DefineElement(cx, jsobjx, nCurrentArrayX, INT_TO_JSVAL(*p), NULL, NULL, JSPROP_ENUMERATE);
            nCurrentArrayX++;
            p++;
        }

        if (bOk)
            bOk = JS_DefineElement(cx, jsobjy, nCurrentArrayY, OBJECT_TO_JSVAL(jsobjx), NULL, NULL, JSPROP_ENUMERATE);
        nCurrentArrayY++;
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

    if (bOk)
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsobjy));
    JS_RemoveRoot(cx, &jsobjy);
    JS_EndRequest(cx);
    delete cRoom;
    return bOk;
}

JSAPI_FUNC(room_getNearby) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);
    JSObject* jsobj = JS_NewArrayObject(cx, NULL, NULL);

    if (!jsobj)
        return JS_FALSE;

    AutoCriticalRoom cRoom;
    Room2* pRoom2 = FindLiveRoom(pSnap);
    DWORD dwCount = 0;
    JS_BeginRequest(cx);
    for (DWORD i = 0; pRoom2 && i < pRoom2->dwRoomsNear; ++i) {
        if (!pRoom2->pRoom2Near[i])
            continue;
        // FindLiveRoom found the room in an open session, which the room lock keeps open, so a NULL here is an engine
        // failure (out of memory) and goes back to the engine as one.
        JSObject* jsroom = BuildRoomObject(cx, pRoom2->pRoom2Near[i]);
        if (!jsroom) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        // Defined, not set: an Array.prototype index setter would run script with the game lock held.
        if (!JS_DefineElement(cx, jsobj, dwCount++, OBJECT_TO_JSVAL(jsroom), NULL, NULL, JSPROP_ENUMERATE)) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
    }
    JS_EndRequest(cx);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsobj));
    return JS_TRUE;
}

// Don't know whether it works or not
JSAPI_FUNC(room_getStat) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    RoomSnapshot* pSnap = GetSnapshot(cx, self);

    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    if (!pSnap || argc < 1 || JSVAL_IS_VOID(JS_ARGV(cx, vp)[0]) || JSVAL_IS_NULL(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    // The stat converts before the game lock is taken, since a valueOf run under it stalls the game thread. Any number or
    // numeric string names a stat; a value that converts to NaN ("abc", {}) names none and stays null.
    double dStat = 0;
    JS_BeginRequest(cx);
    JSBool bConverted = JS_ValueToNumber(cx, JS_ARGV(cx, vp)[0], &dStat);
    JS_EndRequest(cx);
    if (!bConverted)
        return JS_FALSE;
    if (MOZ_DOUBLE_IS_NaN(dStat))
        return JS_TRUE;
    jsint nStat = JS_DoubleToInt32(dStat);

    bool bAdded = false;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }
    if (!pRoom2->pRoom1) {
        LoadRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        bAdded = pRoom2->pRoom1 != NULL;
    }

    // A stat the room has no data for stays null, and room data added above is unloaded on every path.
    if (Room1* pRoom1 = pRoom2->pRoom1) {
        CollMap* pCol = pRoom1->Coll;
        if (nStat == 0) // xStart
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom1->dwXStart));
        else if (nStat == 1)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom1->dwYStart));
        else if (nStat == 2)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom1->dwXSize));
        else if (nStat == 3)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom1->dwYSize));
        else if (nStat == 4)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom2->dwPosX));
        else if (nStat == 5)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom2->dwPosY));
        else if (nStat == 6)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom2->dwSizeX));
        else if (nStat == 7)
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pRoom2->dwSizeY));
        //	else if(nStat == 8)
        //		*rval = INT_TO_JSVAL(pRoom2->pRoom1->dwYStart); // God knows??!!??!?!?!?!
        else if (pCol) {
            if (nStat == 9)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwPosGameX));
            else if (nStat == 10)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwPosGameY));
            else if (nStat == 11)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwSizeGameX));
            else if (nStat == 12)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwSizeGameY));
            else if (nStat == 13)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwPosRoomX));
            else if (nStat == 14)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwPosRoomY));
            else if (nStat == 15)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwSizeRoomX));
            else if (nStat == 16)
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwSizeRoomY));
        }
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    delete cRoom;
    return JS_TRUE;
}

JSAPI_FUNC(room_getFirst) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);
    if (!pSnap)
        return JS_TRUE;

    AutoCriticalRoom cRoom;
    Level* pLevel = FindRoomLevel(pSnap);
    if (!pLevel)
        return JS_TRUE;

    // FindRoomLevel found the level in an open session, which the room lock keeps open, so a NULL here is an engine
    // failure (out of memory) and goes back to the engine as one.
    JSObject* jsroom = BuildRoomObject(cx, pLevel->pRoom2First);
    if (!jsroom)
        return JS_FALSE;

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
    return JS_TRUE;
}

JSAPI_FUNC(room_unitInRoom) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);
    if (!pSnap || argc < 1 || !JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    // Class-checked: a non-Unit argument's private (a Room snapshot, a PresetUnit) is not a myUnit.
    myUnit* pmyUnit = GetUnitPrivate(cx, JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    AutoCriticalRoom cRoom;
    // The unit's room is read only while the player's act is the loaded one, as for every Room method: an act change frees
    // the old act before the player's pointer moves on.
    if (!PlayerActLoaded())
        return JS_TRUE;
    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    Room1* pRoom1 = D2COMMON_GetRoomFromUnit(pUnit);

    if (!pRoom1 || !pRoom1->pRoom2 || !pRoom1->pRoom2->pLevel)
        return JS_TRUE;

    // Compared by game, level and position: the game can reuse a freed room's address for another room.
    Room2* pUnitRoom = pRoom1->pRoom2;
    JS_SET_RVAL(cx, vp,
                BOOLEAN_TO_JSVAL(pSnap->dwSession == Vars.dwGameSession && pUnitRoom->pLevel->dwLevelNo == pSnap->dwLevelNo && pUnitRoom->dwPosX == pSnap->dwPosX &&
                                 pUnitRoom->dwPosY == pSnap->dwPosY));

    return JS_TRUE;
}

JSAPI_FUNC(room_reveal) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, self);

    BOOL bDrawPresets = false;
    if (argc == 1 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[0]))
        bDrawPresets = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[0]);

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(RevealRoom(pRoom2, bDrawPresets)));
    delete cRoom;
    return JS_TRUE;
}

JSAPI_FUNC(my_getRoom) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    GameWait wait;
    if (!wait.Ready())
        THROW_ERROR(cx, "Get Room Game not ready");

    // Every argument converts before the game lock is taken: a conversion can run script code (valueOf), and script code
    // run under the lock stalls the game thread. A failed conversion (a valueOf that throws) leaves its exception pending,
    // so it goes back to the engine. One argument is always an area id, so a numeric string or a Number names the same area
    // as an int.
    bool bByArea = argc == 1;
    bool bByPoint = argc == 2 || argc == 3;
    uint32 levelId = 0;
    uint32 nX = 0;
    uint32 nY = 0;
    JSBool bConverted = JS_TRUE;
    JS_BeginRequest(cx);
    if (bByArea)
        bConverted = JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &levelId);
    else if (argc == 2)
        bConverted = JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &nX) && JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nY);
    else if (argc == 3)
        bConverted = JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &levelId) && JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nX) &&
                     JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[2], &nY);
    JS_EndRequest(cx);
    if (!bConverted)
        return JS_FALSE;

    // The room is looked up and built under one hold of the game lock. A game being left has closed its session while
    // its player and rooms still exist, and finds no room. In an open session, which the room lock keeps open, a NULL
    // BuildRoomObject below is an engine failure (out of memory) and goes back to the engine as one. A room or level
    // missing while the game is busy under the lock sends the call back to wait.
    const bool bPlayerRoom = bByArea && levelId == 0;
    // A point with a zero coordinate names no room. It still opens the session and looks its level up, as other calls
    // do, but never waits again for a load that begins after the call's first wait.
    const bool bNoPoint = bByPoint && (!nX || !nY);
    AutoCriticalRoom* cRoom = NULL;
    Room2* pPlayerRoom = NULL;
    Level* pLevel = NULL;
    // A retry happens with the lock released, so a later pass can find the next game: it finds no room, as a pass in a
    // closed session does, and is checked first so it never opens that game's session.
    DWORD dwSession = 0;
    while (wait.Ready()) {
        cRoom = new AutoCriticalRoom;
        if ((dwSession && Vars.dwGameSession != dwSession) || !OpenGameSession()) {
            delete cRoom;
            return JS_TRUE;
        }
        dwSession = Vars.dwGameSession;

        if ((bByArea && !bPlayerRoom) || argc == 3) {
            pLevel = GetLevel(levelId);
        } else {
            // The player's own chain is read only while its act is the loaded one: an act change frees the old act before
            // the player's pointer moves on, and this lock can be handed to other threads in between.
            UnitAny* pPlayer = PlayerActLoaded() ? D2CLIENT_GetPlayerUnit() : NULL;
            Room1* pRoom1 = pPlayer && pPlayer->pPath ? pPlayer->pPath->pRoom1 : NULL;
            if (bPlayerRoom)
                pPlayerRoom = pRoom1 ? pRoom1->pRoom2 : NULL;
            else
                pLevel = pRoom1 && pRoom1->pRoom2 ? pRoom1->pRoom2->pLevel : NULL;
        }
        if (bNoPoint || pPlayerRoom || (pLevel && pLevel->pRoom2First))
            break;

        const bool bRetry = wait.RetryIfBusy();
        delete cRoom;
        cRoom = NULL;
        if (!bRetry)
            break;
    }
    if (!cRoom)
        return JS_TRUE;
    if (bNoPoint) {
        delete cRoom;
        return JS_TRUE;
    }

    Room2* pFound = bPlayerRoom ? pPlayerRoom : pLevel->pRoom2First;
    if (bByPoint) {
        // Scan for the room with the matching x,y coordinates; none holding it leaves the level's first room.
        for (Room2* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
            bool bAdded = FALSE;
            if (!pRoom->pRoom1) {
                LoadRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                bAdded = pRoom->pRoom1 != NULL;
            }

            CollMap* map = pRoom->pRoom1 ? pRoom->pRoom1->Coll : NULL;
            bool bHolds = map && nX >= map->dwPosGameX && nY >= map->dwPosGameY && nX < (map->dwPosGameX + map->dwSizeGameX) && nY < (map->dwPosGameY + map->dwSizeGameY);

            if (bAdded)
                D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

            if (bHolds) {
                pFound = pRoom;
                break;
            }
        }
    }

    JSObject* jsroom = BuildRoomObject(cx, pFound);
    delete cRoom;
    if (!jsroom)
        return JS_FALSE;

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
    return JS_TRUE;
}
