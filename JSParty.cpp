#include "JSParty.h"
#include "D2Structs.h"
#include "D2Helpers.h"
#include "D2Ptrs.h"
#include "JSUnit.h"
#include "JSGlobalClasses.h"
#include "Constants.h"
#include "CriticalSections.h"

EMPTY_CTOR(party)

void party_finalize(JSFreeOp* fop, JSObject* obj) {
    delete (myParty*)JS_GetPrivate(obj);
    JS_SetPrivate(obj, NULL);
}

RosterUnit* FindPartyUnit(const myParty* pParty) {
    // The session check comes first: the next game may reuse this player's gid for someone else.
    if (!pParty || pParty->dwSession != Vars.dwGameSession)
        return NULL;

    for (RosterUnit* pUnit = *p_D2CLIENT_PlayerUnitList; pUnit; pUnit = pUnit->pNext)
        if (pUnit->dwUnitId == pParty->dwUnitId)
            return pUnit;
    return NULL;
}

JSAPI_PROP(party_getProperty) {
    // A slotted getter's value stays in the slot: without this a departed player would read as their last values.
    vp.setUndefined();

    // Also NULL for Party.prototype and for an object that only inherits from a Party.
    myParty* pParty = (myParty*)JS_GetInstancePrivate(cx, obj, &party_class, NULL);
    if (!pParty)
        return JS_TRUE;

    jsval ID;
    JS_IdToValue(cx, id, &ID);
    int tinyid = JSVAL_TO_INT(ID);
    char szName[sizeof(((RosterUnit*)0)->szName) + 1] = "";
    DWORD dwValue = 0;

    {
        AutoCriticalRoom cRoom;
        RosterUnit* pUnit = FindPartyUnit(pParty);
        if (!pUnit)
            return JS_TRUE;

        switch (tinyid) {
        case PARTY_NAME:
            memcpy(szName, pUnit->szName, sizeof(pUnit->szName));
            break;
        case PARTY_X:
            dwValue = pUnit->Xpos;
            break;
        case PARTY_Y:
            dwValue = pUnit->Ypos;
            break;
        case PARTY_AREA:
            dwValue = pUnit->dwLevelId;
            break;
        case PARTY_GID:
            dwValue = pUnit->dwUnitId;
            break;
        case PARTY_LIFE:
            dwValue = pUnit->dwPartyLife;
            break;
        case PARTY_CLASSID:
            dwValue = pUnit->dwClassId;
            break;
        case PARTY_LEVEL:
            dwValue = pUnit->wLevel;
            break;
        case PARTY_FLAG:
            dwValue = pUnit->dwPartyFlags;
            break;
        case PARTY_ID:
            dwValue = pUnit->wPartyId;
            break;
        default:
            return JS_TRUE;
        }
    }

    // Values are built after the lock is released: an allocation can run the GC, which must not stall the game thread.
    JS_BeginRequest(cx);
    if (tinyid == PARTY_NAME) {
        JSString* str = JS_NewStringCopyZ(cx, szName);
        if (!str) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        vp.setString(str);
    } else if (tinyid == PARTY_GID)
        vp.setNumber((jsdouble)dwValue);
    else
        vp.setInt32(dwValue);
    JS_EndRequest(cx);
    return JS_TRUE;
}

JSAPI_FUNC(party_getNext) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    JSObject* self = JS_THIS_OBJECT(cx, vp);
    myParty* pParty = (myParty*)JS_GetInstancePrivate(cx, self, &party_class, NULL);
    bool bNext = false;

    if (pParty) {
        AutoCriticalRoom cRoom;
        // When this player has left there is no place in the list to go on from, so that pass ends.
        RosterUnit* pUnit = FindPartyUnit(pParty);
        if (pUnit && pUnit->pNext) {
            pParty->dwUnitId = pUnit->pNext->dwUnitId;
            bNext = true;
        }
    }

    JS_SET_RVAL(cx, vp, bNext ? OBJECT_TO_JSVAL(self) : JSVAL_FALSE);
    return JS_TRUE;
}

JSAPI_FUNC(my_getParty) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    char* szName = NULL;
    uint32 nPlayerId = 0;
    myUnit* lpUnit = NULL;

    if (argc == 1) {
        jsval arg = JS_ARGV(cx, vp)[0];

        if (JSVAL_IS_STRING(arg)) {
            szName = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(arg));
            if (!szName)
                return JS_FALSE;
        } else if (JSVAL_IS_INT(arg)) {
            JS_BeginRequest(cx);
            if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &nPlayerId)) {
                JS_EndRequest(cx);
                THROW_ERROR(cx, "Unable to get ID");
            }
            JS_EndRequest(cx);
            if (!nPlayerId)
                return JS_TRUE;
        } else if (!JSVAL_IS_PRIMITIVE(arg)) {
            // Only a player Unit can name a roster entry, and any other object's private data is not a unit.
            lpUnit = (myUnit*)JS_GetInstancePrivate(cx, JSVAL_TO_OBJECT(arg), &unit_class, NULL);
            if (!lpUnit || lpUnit->dwType != UNIT_PLAYER)
                return JS_TRUE;
        } else
            return JS_TRUE;
    }

    bool bFound = false, bUnitGone = false;
    myParty party = {PRIVATE_PARTY, 0, 0};

    {
        AutoCriticalRoom cRoom;
        // The game may have ended since WaitForGameReady(); between games the session is even.
        if (Vars.dwGameSession & 1) {
            if (lpUnit) {
                UnitAny* inUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
                if (inUnit)
                    nPlayerId = inUnit->dwUnitId;
                else
                    bUnitGone = true;
            }

            for (RosterUnit* pScan = bUnitGone ? NULL : *p_D2CLIENT_PlayerUnitList; pScan; pScan = pScan->pNext) {
                if (argc != 1 || (szName ? _stricmp(pScan->szName, szName) == 0 : pScan->dwUnitId == nPlayerId)) {
                    party.dwUnitId = pScan->dwUnitId;
                    party.dwSession = Vars.dwGameSession;
                    bFound = true;
                    break;
                }
            }
        }
    }

    JS_free(cx, szName);

    if (bUnitGone)
        THROW_ERROR(cx, "Unable to get Unit");

    if (!bFound)
        return JS_TRUE;

    myParty* pParty = new myParty(party);
    JSObject* jsUnit = BuildObject(cx, &party_class, party_methods, party_props, pParty);
    // BuildObject attaches the private only when it succeeds.
    if (!jsUnit) {
        delete pParty;
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsUnit));

    return JS_TRUE;
}
