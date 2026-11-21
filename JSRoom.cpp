#include "JSRoom.h"
#include "CriticalSections.h"
#include "JSPresetUnit.h"
#include "D2Structs.h"
#include "D2Ptrs.h"
#include "D2BS.h"
#include "D2Helpers.h"
#include "JSParty.h"

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

    UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();
    Act* pAct = *p_D2CLIENT_CurrentAct;
    if (!pPlayer || !pAct || pPlayer->pAct != pAct || !pAct->pMisc)
        return NULL;

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
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));
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
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));

    DWORD nType = NULL;
    DWORD nClass = NULL;

    if (argc > 0 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        nType = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    if (argc > 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[1]))
        nClass = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);

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

    if (!pRoom2->pRoom1) {
        bAdded = TRUE;
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
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

            // BuildObject attaches the private only when it succeeds.
            JSObject* jsUnit = BuildObject(cx, &presetunit_class, NULL, presetunit_props, mypUnit);
            if (!jsUnit) {
                delete mypUnit;
                bOk = JS_FALSE;
                break;
            }

            jsval a = OBJECT_TO_JSVAL(jsUnit);
            JS_BeginRequest(cx);
            bOk = JS_SetElement(cx, pReturnArray, dwArrayCount, &a);
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
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));

    bool bAdded = FALSE;
    CollMap* pCol = NULL;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    if (!pRoom2->pRoom1) {
        bAdded = TRUE;
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    }

    JSObject* jsobjy = JS_NewUint16Array(cx, (pRoom2->dwSizeX * 5) * (pRoom2->dwSizeY * 5));
    JS_AddRoot(cx, &jsobjy);

    if (!jsobjy) {
        JS_RemoveRoot(cx, &jsobjy);
        if (bAdded)
            D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        delete cRoom;
        return JS_TRUE;
    }

    if (pRoom2->pRoom1)
        pCol = pRoom2->pRoom1->Coll;

    if (!pCol) {
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

    WORD* p = pCol->pMapStart;
    JS_BeginRequest(cx);
    for (int j = y; j < nLimitY; j++) {

        int nCurrentArrayX = 0;
        for (int i = x; i < nLimitX; i++) {
            jsval nNode = INT_TO_JSVAL(*p);

            if (!JS_SetElement(cx, jsobjy, nCurrentArrayY * nCx + nCurrentArrayX, &nNode)) {
                if (bAdded)
                    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY,
                                            D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                JS_RemoveRoot(cx, &jsobjy);
                JS_EndRequest(cx);
                delete cRoom;
                return JS_TRUE;
            }

            nCurrentArrayX++;
            p++;
        }
        nCurrentArrayY++;
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsobjy));
    JS_RemoveRoot(cx, &jsobjy);
    JS_EndRequest(cx);
    delete cRoom;
    return JS_TRUE;
}
JSAPI_FUNC(room_getCollision) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));

    bool bAdded = FALSE;
    CollMap* pCol = NULL;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }

    JSObject* jsobjy = JS_NewArrayObject(cx, NULL, NULL);
    JS_AddRoot(cx, &jsobjy);
    if (!jsobjy) {
        JS_RemoveRoot(cx, &jsobjy);
        delete cRoom;
        return JS_TRUE;
    }

    if (!pRoom2->pRoom1) {
        bAdded = TRUE;
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    }

    if (pRoom2->pRoom1)
        pCol = pRoom2->pRoom1->Coll;

    if (!pCol) {
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

    WORD* p = pCol->pMapStart;
    JS_BeginRequest(cx);
    for (int j = y; j < nLimitY; j++) {
        JSObject* jsobjx = JS_NewArrayObject(cx, NULL, NULL);

        int nCurrentArrayX = 0;
        for (int i = x; i < nLimitX; i++) {
            jsval nNode = INT_TO_JSVAL(*p);

            if (!JS_SetElement(cx, jsobjx, nCurrentArrayX, &nNode)) {
                if (bAdded)
                    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY,
                                            D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                JS_RemoveRoot(cx, &jsobjy);
                JS_EndRequest(cx);
                delete cRoom;
                return JS_TRUE;
            }

            nCurrentArrayX++;
            p++;
        }

        jsval jsu = OBJECT_TO_JSVAL(jsobjx);

        if (!JS_SetElement(cx, jsobjy, nCurrentArrayY, &jsu)) {
            if (bAdded)
                D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY,
                                        D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
            JS_RemoveRoot(cx, &jsobjy);
            JS_EndRequest(cx);
            delete cRoom;
            return JS_TRUE;
        }
        nCurrentArrayY++;
    }

    if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsobjy));
    JS_RemoveRoot(cx, &jsobjy);
    JS_EndRequest(cx);
    delete cRoom;
    return JS_TRUE;
}

JSAPI_FUNC(room_getNearby) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));
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
        jsval jsu = OBJECT_TO_JSVAL(jsroom);

        if (!JS_SetElement(cx, jsobj, dwCount++, &jsu)) {
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
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));

    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    jsint nStat = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    bool bAdded = false;

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;
    Room2* pRoom2 = GameReady() ? FindLiveRoom(pSnap) : NULL;
    if (!pRoom2) {
        delete cRoom;
        return JS_TRUE;
    }
    if (!pRoom2->pRoom1) {
        bAdded = true;
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
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
                JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pCol->dwPosGameY));
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
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));
    if (!pSnap)
        return JS_TRUE;

    AutoCriticalRoom cRoom;
    Level* pLevel = FindRoomLevel(pSnap);
    if (!pLevel)
        return JS_TRUE;

    JSObject* jsroom = BuildRoomObject(cx, pLevel->pRoom2First);
    if (!jsroom)
        return JS_TRUE;

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));

    return JS_TRUE;
}

JSAPI_FUNC(room_unitInRoom) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));
    if (!pSnap || argc < 1 || !JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    myUnit* pmyUnit = (myUnit*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    AutoCriticalRoom cRoom;
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
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    RoomSnapshot* pSnap = GetSnapshot(cx, JS_THIS_OBJECT(cx, vp));

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

    if (!WaitForGameReady())
        THROW_ERROR(cx, "Get Room Game not ready");

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;

    if (argc == 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0])) {
        uint32 levelId;
        JS_BeginRequest(cx);
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &levelId);
        JS_EndRequest(cx);
        if (levelId != 0) // 1 Parameter, AreaId
        {
            Level* pLevel = GetLevel(levelId);

            if (!pLevel || !pLevel->pRoom2First) {
                delete cRoom;
                return JS_TRUE;
            }

            JSObject* jsroom = BuildRoomObject(cx, pLevel->pRoom2First);
            if (!jsroom) {
                delete cRoom;
                return JS_TRUE;
            }
            delete cRoom;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
            return JS_TRUE;
        } else if (levelId == 0) {
            // WaitForGameReady ran before the lock was taken, so the player can be NULL by now.
            UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();
            Room1* pRoom1 = pPlayer ? D2COMMON_GetRoomFromUnit(pPlayer) : NULL;

            if (!pRoom1 || !pRoom1->pRoom2) {
                delete cRoom;
                return JS_TRUE;
            }

            JSObject* jsroom = BuildRoomObject(cx, pRoom1->pRoom2);
            delete cRoom;
            if (!jsroom)
                return JS_TRUE;

            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
            return JS_TRUE;
        }
    } else if (argc == 3 || argc == 2) // area ,x and y
    {
        Level* pLevel = NULL;

        uint32 levelId;
        JS_BeginRequest(cx);
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &levelId);
        JS_EndRequest(cx);
        if (argc == 3)
            pLevel = GetLevel(levelId);
        else if (D2CLIENT_GetPlayerUnit() && D2CLIENT_GetPlayerUnit()->pPath && D2CLIENT_GetPlayerUnit()->pPath->pRoom1 &&
                 D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2)
            pLevel = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel;

        if (!pLevel || !pLevel->pRoom2First) {
            delete cRoom;
            return JS_TRUE;
        }

        uint32 nX = NULL;
        uint32 nY = NULL;
        JS_BeginRequest(cx);
        if (argc == 2) {
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &nX);
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nY);
        } else if (argc == 3) {
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nX);
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[2], &nY);
        }
        JS_EndRequest(cx);
        if (!nX || !nY) {
            delete cRoom;
            return JS_TRUE;
        }

        // Scan for the room with the matching x,y coordinates.
        for (Room2* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
            bool bAdded = FALSE;
            if (!pRoom->pRoom1) {
                D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
                bAdded = TRUE;
            }

            CollMap* map = pRoom->pRoom1 ? pRoom->pRoom1->Coll : NULL;
            if (map && nX >= map->dwPosGameX && nY >= map->dwPosGameY && nX < (map->dwPosGameX + map->dwSizeGameX) && nY < (map->dwPosGameY + map->dwSizeGameY)) {
                if (bAdded)
                    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

                JSObject* jsroom = BuildRoomObject(cx, pRoom);
                delete cRoom;
                if (!jsroom)
                    return JS_TRUE;

                JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
                return JS_TRUE;
            }

            if (bAdded)
                D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        }

        JSObject* jsroom = BuildRoomObject(cx, pLevel->pRoom2First);
        delete cRoom;
        if (!jsroom)
            return JS_TRUE;

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
        return JS_TRUE;
    } else {
        // WaitForGameReady ran before the lock was taken, so the player's room chain can be NULL by now.
        UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();
        Room1* pRoom1 = pPlayer && pPlayer->pPath ? pPlayer->pPath->pRoom1 : NULL;
        Level* pLevel = pRoom1 && pRoom1->pRoom2 ? pRoom1->pRoom2->pLevel : NULL;
        JSObject* jsroom = pLevel ? BuildRoomObject(cx, pLevel->pRoom2First) : NULL;
        delete cRoom;
        if (!jsroom)
            return JS_TRUE;

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsroom));
        return JS_TRUE;
    }
    delete cRoom;
    return JS_TRUE;
}
