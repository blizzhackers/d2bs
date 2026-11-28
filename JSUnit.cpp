#include "JSUnit.h"
#include "D2Helpers.h"
#include "Constants.h"
#include "Helpers.h"
#include "Unit.h"
#include "Core.h"
#include "CriticalSections.h"
#include "D2Skills.h"
#include "MPQStats.h"

EMPTY_CTOR(unit)

void unit_finalize(JSFreeOp* fop, JSObject* obj) {
    Private* lpUnit = (Private*)JS_GetPrivate(obj);

    if (lpUnit) {
        switch (lpUnit->dwPrivateType) {
        case PRIVATE_UNIT: {
            myUnit* unit = (myUnit*)lpUnit;
            delete unit;
            break;
        }
        case PRIVATE_ITEM: {
            invUnit* unit = (invUnit*)lpUnit;
            delete unit;
            break;
        }
        }
    }
    JS_SetPrivate(obj, NULL);
}

// Only a Unit object's private is a myUnit/invUnit; any other object's private is foreign data or, on a class
// without a private, garbage. NULL obj is a failed JS_THIS_OBJECT.
myUnit* GetUnitPrivate(JSContext* cx, JSObject* obj) {
    if (!obj)
        return NULL;
    return (myUnit*)JS_GetInstancePrivate(cx, obj, &unit_class, NULL);
}

// JSBool unit_equal(JSContext *cx, JSObject *obj, jsval v, JSBool *bp) // not 1.8 compatable
//{
//	if(ClientState() == ClientStateInGame) {
//		*bp = JS_InstanceOf(cx, obj, &unit_class_ex.base, NULL);
//		myUnit* one = (myUnit*)JS_GetInstancePrivate(cx, obj, &unit_class_ex.base, NULL);
//				myUnit* two = (myUnit*)JS_GetInstancePrivate(cx, JSVAL_TO_OBJECT(v), &unit_class_ex.base, NULL);
//		UnitAny* pUnit1 = D2CLIENT_FindUnit(one->dwUnitId, one->dwType);
//		UnitAny* pUnit2 = D2CLIENT_FindUnit(two->dwUnitId, two->dwType);
//		if(!pUnit1 || !pUnit2 || pUnit1->dwUnitId != pUnit2->dwUnitId)
//			*bp = JS_FALSE;
//	}
//	return JS_TRUE;
//}

JSAPI_PROP(unit_getProperty) {
    BnetData* pData = *p_D2LAUNCH_BnData;
    GameStructInfo* pInfo = *p_D2CLIENT_GameInfo;
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    JS_BeginRequest(cx);
    char* nProfile;

    switch (JSVAL_TO_INT(ID)) {
    case ME_PID:
        vp.setDouble((jsdouble)GetCurrentProcessId());
        // JS_NewNumberValue(cx, (jsdouble)GetCurrentProcessId(), vp);
        break;
    case ME_PROFILE:
        if (!StoreString(vp, JS_NewUCStringCopyZ(cx, Vars.szProfile))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_GAMEREADY:
        vp.setBoolean(GameReady());
        //*vp = BOOLEAN_TO_JSVAL(GameReady());
        break;
    case ME_ACCOUNT:
        if (!pData) {
            JS_EndRequest(cx);
            return JS_TRUE;
        }
        if (!StoreString(vp, JS_NewStringCopyZ(cx, pData->szAccountName))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        // vp.setString(JS_NewStringCopyZ(cx, pData->szAccountName));
        break;
    case ME_CHARNAME:
        if (!pData) {
            JS_EndRequest(cx);
            return JS_TRUE;
        }
        if (!StoreString(vp, JS_NewStringCopyZ(cx, pData->szPlayerName))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_CHICKENHP:
        vp.setInt32(Vars.nChickenHP);
        break;
    case ME_CHICKENMP:
        vp.setInt32(Vars.nChickenMP);
        break;
    case ME_DIFF:
        vp.setInt32(D2CLIENT_GetDifficulty());
        break;
    case ME_MAXDIFF:
        // Every other BnData read guards a NULL; undefined, so a NULL never returns the slot's last value.
        if (pData)
            vp.setInt32(pData->nMaxDiff);
        else
            vp.setUndefined();
        break;
    case ME_GAMENAME:
        if (!pInfo) {
            JS_EndRequest(cx);
            return JS_TRUE;
        }
        if (!StoreString(vp, JS_NewStringCopyZ(cx, pInfo->szGameName))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_GAMEPASSWORD:
        if (!pInfo) {
            JS_EndRequest(cx);
            return JS_TRUE;
        }
        if (!StoreString(vp, JS_NewStringCopyZ(cx, pInfo->szGamePassword))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_GAMESERVERIP:
        if (!pInfo) {
            JS_EndRequest(cx);
            return JS_TRUE;
        }
        if (!StoreString(vp, JS_NewStringCopyZ(cx, pInfo->szGameServerIp))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_GAMESTARTTIME: {
        // On getTickCount()'s clock: dwGameTime holds GetTickCount's 32 bits, and a game is far shorter than their
        // 49.7-day wrap, so the time since it started restores the high part. Windows XP's 32-bit TickCount64 can be
        // below that time after its wrap, and its start time is dwGameTime itself. 0 (out of game) stays 0.
        DWORD dwStart = Vars.dwGameTime;
        ULONGLONG qwNow = TickCount64();
        DWORD dwAgo = (DWORD)qwNow - dwStart;
        vp.setDouble(!dwStart ? 0 : qwNow >= dwAgo ? (jsdouble)(qwNow - dwAgo) : (jsdouble)dwStart);
        break;
    }
    case ME_GAMETYPE:
        vp.setInt32(*p_D2CLIENT_ExpCharFlag);
        break;
    case ME_PLAYERTYPE:
        if (pData)
            vp.setBoolean(!!(pData->nCharFlags & PLAYER_TYPE_HARDCORE));
        break;
    case ME_ITEMONCURSOR:
        vp.setBoolean(!!(D2CLIENT_GetCursorItem()));
        break;
    case ME_AUTOMAP:
        vp.setBoolean(*p_D2CLIENT_AutomapOn);
        // JS_NewNumberValue(cx, (jsdouble)(*p_D2CLIENT_AutomapOn), vp);
        //*vp = *p_D2CLIENT_AutomapOn;
        break;
    case ME_LADDER:
        if (pData)
            // vp.setBoolean(!!(pData->ladderflag & (LADDERFLAG_SET|LADDERFLAG_EXPANSION_NORMAL)));
            vp.setDouble((jsdouble)pData->ladderflag);
        break;
    case ME_QUITONHOSTILE:
        vp.setBoolean(Vars.bQuitOnHostile);
        break;
    case ME_REALM:
        if (!pData)
            vp.setUndefined();
        else if (!StoreString(vp, JS_NewStringCopyZ(cx, pData->szRealmName))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case ME_REALMSHORT:
        if (!pData)
            vp.setUndefined();
        else if (!StoreString(vp, JS_NewStringCopyZ(cx, pData->szRealmName2))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        break;
    case OOG_SCREENSIZE:
        vp.setInt32(D2GFX_GetScreenSize());
        break;
    case OOG_WINDOWTITLE: {
        wchar_t szTitle[256];
        GetWindowTextW(D2GFX_GetHwnd(), szTitle, 256);
        if (!StoreString(vp, JS_NewUCStringCopyZ(cx, szTitle))) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
    } break;
    case ME_PING:
        vp.setInt32(*p_D2CLIENT_Ping);
        break;
    case ME_FPS:
        vp.setInt32(*p_D2CLIENT_FPS);
        break;
    case ME_LOCALE:
        vp.setInt32(Vars.dwLocale);
        break;
    case OOG_INGAME:
        vp.setBoolean((ClientState() == ClientStateMenu ? false : true));
        break;
    case OOG_QUITONERROR:
        vp.setBoolean(Vars.bQuitOnError);
        break;
    case OOG_MAXGAMETIME:
        vp.setInt32(Vars.dwMaxGameTime);
        break;
    case ME_MERCREVIVECOST:
        vp.setInt32((*p_D2CLIENT_MercReviveCost));
        break;
    case ME_BLOCKKEYS:
        vp.setBoolean(Vars.bBlockKeys);
        break;
    case ME_BLOCKMOUSE:
        vp.setBoolean(Vars.bBlockMouse);
        break;
    case ME_UNSUPPORTED:
        vp.setBoolean(Vars.bEnableUnsupported);
        break;
    case ME_CHARFLAGS:
        if (pData)
            vp.setInt32(pData->nCharFlags);
        break;
    default:
        break;
    }
    JS_EndRequest(cx);
    if (ClientState() != ClientStateInGame)
        return JS_TRUE;
    // JSObject* obj ;
    // JS_ValueToObject(cx,  JS_CALLEE(cx,vp),obj);
    myUnit* lpUnit = GetUnitPrivate(cx, obj);
    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
    if (!pUnit) {
        return JS_TRUE;
    }
    Room1* pRoom = NULL;

    switch (JSVAL_TO_INT(ID)) {
    case UNIT_TYPE:
        vp.setInt32(pUnit->dwType);
        break;
    case UNIT_CLASSID:
        vp.setInt32(pUnit->dwTxtFileNo);
        break;
    case UNIT_MODE:
        vp.setInt32(pUnit->dwMode);
        break;
    case UNIT_NAME: {
        char tmp[128] = "";
        GetUnitName(pUnit, tmp, 128);
        wchar_t* wtmp = AnsiToUnicode(tmp);
        bool stored = StoreString(vp, JS_InternUCString(cx, wtmp));
        delete[] wtmp;
        if (!stored)
            return JS_FALSE;
    } break;
    case ME_MAPID:
        vp.setInt32(*p_D2CLIENT_MapId);
        break;
    case ME_NOPICKUP:
        vp.setBoolean(!!*p_D2CLIENT_NoPickUp);
        break;
    case UNIT_ACT:
        vp.setInt32(pUnit->dwAct + 1);
        break;
    case UNIT_AREA:
        pRoom = D2COMMON_GetRoomFromUnit(pUnit);
        if (pRoom && pRoom->pRoom2 && pRoom->pRoom2->pLevel)
            vp.setInt32(pRoom->pRoom2->pLevel->dwLevelNo);
        break;
    case UNIT_ID:
        JS_BeginRequest(cx);
        vp.setNumber((jsdouble)pUnit->dwUnitId);
        JS_EndRequest(cx);
        break;
    case UNIT_XPOS:
        vp.setInt32(D2CLIENT_GetUnitX(pUnit));
        break;
    case UNIT_YPOS:
        vp.setInt32(D2CLIENT_GetUnitY(pUnit));
        break;
    case UNIT_TARGETX:
        switch (pUnit->dwType) {
        case 0:
        case 1:
        case 3:
            vp.setInt32(pUnit->pPath->xTarget);
        }
        break;
    case UNIT_TARGETY:
        switch (pUnit->dwType) {
        case 0:
        case 1:
        case 3:
            vp.setInt32(pUnit->pPath->yTarget);
        }
        break;
    case UNIT_HP:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 6, 0) >> 8);
        break;
    case UNIT_HPMAX:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 7, 0) >> 8);
        break;
    case UNIT_MP:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 8, 0) >> 8);
        break;
    case UNIT_MPMAX:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 9, 0) >> 8);
        break;
    case UNIT_STAMINA:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 10, 0) >> 8);
        break;
    case UNIT_STAMINAMAX:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 11, 0) >> 8);
        break;
    case UNIT_CHARLVL:
        vp.setInt32(D2COMMON_GetUnitStat(pUnit, 12, 0));
        break;
    case ME_RUNWALK:
        if (pUnit == D2CLIENT_GetPlayerUnit())
            vp.setInt32(*p_D2CLIENT_AlwaysRun);
        break;
    case UNIT_SPECTYPE:
        DWORD SpecType;
        SpecType = NULL;
        if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData) {
            if (pUnit->pMonsterData->fMinion & 1)
                SpecType |= 0x08;
            if (pUnit->pMonsterData->fBoss & 1)
                SpecType |= 0x04;
            if (pUnit->pMonsterData->fChamp & 1)
                SpecType |= 0x02;
            if ((pUnit->pMonsterData->fBoss & 1) && (pUnit->pMonsterData->fNormal & 1))
                SpecType |= 0x01;
            if (pUnit->pMonsterData->fNormal & 1)
                SpecType |= 0x00;
            vp.setInt32(SpecType);
            return JS_TRUE;
        }
        break;
    case UNIT_UNIQUEID:
        if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData->fBoss && pUnit->pMonsterData->fNormal)
            vp.setInt32(pUnit->pMonsterData->wUniqueNo);
        else
            vp.setInt32(-1);
        break;
    case ITEM_CODE: // replace with better method if found
        if (!(pUnit->dwType == UNIT_ITEM) && pUnit->pItemData)
            break;
        ItemTxt* pTxt;
        pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
        if (!pTxt) {
            if (!StoreString(vp, JS_InternString(cx, "Unknown")))
                return JS_FALSE;
            return JS_TRUE;
        }
        char szCode[4];
        memcpy(szCode, pTxt->szCode, 3);
        szCode[3] = 0x00;
        if (!StoreString(vp, JS_InternString(cx, szCode)))
            return JS_FALSE;
        break;
    case ITEM_PREFIX:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[0]))
                if (!StoreString(vp, JS_InternString(cx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[0]))))
                    return JS_FALSE;
        break;
    case ITEM_SUFFIX:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[0]))
                if (!StoreString(vp, JS_InternString(cx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[0]))))
                    return JS_FALSE;
        break;
    case ITEM_PREFIXNUM:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->wMagicPrefix[0]);
        break;
    case ITEM_SUFFIXNUM:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->wMagicSuffix[0]);
        break;

    case ITEM_PREFIXES:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;

            for (int i = 0; i < 3; i++) {
                if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[i])) {
                    JSString* name = JS_InternString(cx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[i]));
                    if (!name)
                        return JS_FALSE;
                    jsval nPrefix = STRING_TO_JSVAL(name);

                    if (!JS_SetElement(cx, pReturnArray, i, &nPrefix))
                        return JS_FALSE;
                }
            }
            vp.setObject(*pReturnArray);
            //*vp = OBJECT_TO_JSVAL(pReturnArray);
        }

        break;
    case ITEM_PREFIXNUMS:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;

            for (int i = 0; i < 3; i++) {
                if (pUnit->pItemData->wMagicPrefix[i]) {
                    jsval nPrefixnum = INT_TO_JSVAL(pUnit->pItemData->wMagicPrefix[i]);

                    if (!JS_SetElement(cx, pReturnArray, i, &nPrefixnum))
                        return JS_FALSE;
                }
            }
            vp.setObject(*pReturnArray);
            //*vp = OBJECT_TO_JSVAL(pReturnArray);
        }

        break;
    case ITEM_SUFFIXES:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;

            for (int i = 0; i < 3; i++) {
                if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[i])) {
                    JSString* name = JS_InternString(cx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[i]));
                    if (!name)
                        return JS_FALSE;
                    jsval nSuffix = STRING_TO_JSVAL(name);

                    if (!JS_SetElement(cx, pReturnArray, i, &nSuffix))
                        return JS_FALSE;
                }
            }
            vp.setObject(*pReturnArray);
            //*vp = OBJECT_TO_JSVAL(pReturnArray);
        }

        break;
    case ITEM_SUFFIXNUMS:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;

            for (int i = 0; i < 3; i++) {
                if (pUnit->pItemData->wMagicSuffix[i]) {
                    jsval nSuffixnum = INT_TO_JSVAL(pUnit->pItemData->wMagicSuffix[i]);

                    if (!JS_SetElement(cx, pReturnArray, i, &nSuffixnum))
                        return JS_FALSE;
                }
            }

            vp.setObject(*pReturnArray);
        }

        break;

    case ITEM_FNAME:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            wchar_t wszfname[256] = L"";
            D2CLIENT_GetItemName(pUnit, wszfname, _countof(wszfname));
            if (wszfname) {
                if (!StoreString(vp, JS_InternUCString(cx, wszfname)))
                    return JS_FALSE;
            }
        }
        break;
    case ITEM_QUALITY:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->dwQuality);
        break;
    case ITEM_NODE:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->NodePage);
        break;
    case ITEM_LOC:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->GameLocation);
        break;
    case ITEM_SIZEX:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
                break;
            vp.setInt32(D2COMMON_GetItemText(pUnit->dwTxtFileNo)->xSize);
        }
        break;
    case ITEM_SIZEY:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
                break;
            vp.setInt32(D2COMMON_GetItemText(pUnit->dwTxtFileNo)->ySize);
        }
        break;
    case ITEM_TYPE:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
            if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
                break;
            vp.setInt32(D2COMMON_GetItemText(pUnit->dwTxtFileNo)->nType);
        }
        break;
    case ITEM_DESC: {
        if (pUnit->dwType != UNIT_ITEM)
            break;

        AutoCriticalRoom* cRoom = new AutoCriticalRoom;

        wchar_t wBuffer[2048] = L"";
        wchar_t bBuffer[1] = {1};
        if (pUnit->pItemData && pUnit->pItemData->pOwnerInventory && pUnit->pItemData->pOwnerInventory->pOwner) {
            ::WriteProcessMemory(GetCurrentProcess(), (void*)GetDllOffset("D2Client.dll", 0x7BCBE8 - 0x400000), bBuffer, 1, NULL);
            ::WriteProcessMemory(GetCurrentProcess(), (void*)GetDllOffset("D2Client.dll", 0x7BCBF4 - 0x400000), &pUnit, 4, NULL);

            // D2CLIENT_LoadItemDesc(D2CLIENT_GetPlayerUnit(), 0);
            D2CLIENT_LoadItemDesc(pUnit->pItemData->pOwnerInventory->pOwner, 0);
            ReadProcessBYTES(GetCurrentProcess(), GetDllOffset("D2Win.dll", 0x841EC8 - 0x400000), wBuffer, 2047);
        }
        delete cRoom;
        if (wcslen(wBuffer) > 0) {
            if (!StoreString(vp, JS_InternUCString(cx, wBuffer)))
                return JS_FALSE;
        }
    }

    break;
    case ITEM_GFX:
        if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->bInvGfxIdx);
        break;
    case UNIT_ITEMCOUNT:
        if (pUnit->pInventory)
            vp.setInt32(pUnit->pInventory->dwItemCount);
        break;
    case ITEM_BODYLOCATION:
        if (pUnit->dwType != UNIT_ITEM)
            break;
        if (pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->BodyLocation);
        break;
    case UNIT_OWNER:
        vp.setNumber((jsdouble)pUnit->dwOwnerId);
        break;
    case UNIT_OWNERTYPE:
        vp.setInt32(pUnit->dwOwnerType);
        break;
    case ITEM_LEVEL:
        if (pUnit->dwType != UNIT_ITEM)
            break;
        if (pUnit->pItemData)
            vp.setInt32(pUnit->pItemData->dwItemLevel);
        break;
    case ITEM_LEVELREQ:
        if (pUnit->dwType != UNIT_ITEM)
            break;
        vp.setInt32(D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit()));
        break;
    case UNIT_DIRECTION:

        if (pUnit->pPath && pUnit->pPath->pRoom1)
            vp.setInt32(pUnit->pPath->bDirection);
        break;
    case OBJECT_TYPE:
        if (pUnit->dwType == UNIT_OBJECT && pUnit->pObjectData) {
            pRoom = D2COMMON_GetRoomFromUnit(pUnit);
            if (pRoom && D2COMMON_GetLevelNoFromRoom(pRoom))
                vp.setInt32(pUnit->pObjectData->Type & 255);
            else
                vp.setInt32(pUnit->pObjectData->Type);
        }
        break;
    case OBJECT_LOCKED:
        if (pUnit->dwType == UNIT_OBJECT && pUnit->pObjectData)
            vp.setInt32(pUnit->pObjectData->ChestLocked);
        break;
    case ME_WSWITCH:
        if (pUnit == D2CLIENT_GetPlayerUnit())
            vp.setInt32(*p_D2CLIENT_bWeapSwitch);
        break;
    default:
        break;
    }

    return JS_TRUE;
}

JSAPI_STRICT_PROP(unit_setProperty) {
    jsval ID;
    JS_IdToValue(cx, id, &ID);
    switch (JSVAL_TO_INT(ID)) {
    case ME_CHICKENHP:
        if (vp.isInt32())
            Vars.nChickenHP = vp.toInt32();
        break;
    case ME_CHICKENMP:
        if (vp.isInt32())
            Vars.nChickenMP = vp.toInt32();
        break;
    case ME_QUITONHOSTILE:
        if (vp.isBoolean())
            Vars.bQuitOnHostile = vp.toBoolean();
        break;
    case OOG_QUITONERROR:
        if (vp.isBoolean())
            Vars.bQuitOnError = vp.toBoolean();
        break;
    case OOG_MAXGAMETIME:
        if (vp.isInt32())
            Vars.dwMaxGameTime = vp.toInt32();
        break;
    case ME_BLOCKKEYS:
        if (vp.isBoolean())
            Vars.bBlockKeys = vp.toBoolean();
        break;
    case ME_BLOCKMOUSE:
        if (vp.isBoolean())
            Vars.bBlockMouse = vp.toBoolean();
        break;
    case ME_RUNWALK:
        *p_D2CLIENT_AlwaysRun = !!vp.toInt32();
        break;
    case ME_AUTOMAP:
        *p_D2CLIENT_AutomapOn = vp.toBoolean() ? 1 : 0;
        break;
    case ME_NOPICKUP:
        *p_D2CLIENT_NoPickUp = !!vp.toInt32();
        break;
    }
    return JS_TRUE;
}

JSAPI_FUNC(unit_getUnit) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    if (argc < 1)
        return JS_TRUE;

    int nType = -1;
    uint32 nClassId = (uint32)-1;
    uint32 nMode = (uint32)-1;
    uint32 nUnitId = (uint32)-1;
    // NULL unless the name was encoded here, so only an engine-owned string reaches JS_free.
    char* szName = NULL;

    JS_BeginRequest(cx);
    if (argc > 0 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        nType = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    if (argc > 1 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[1])) {
        szName = JS_EncodeStringToUTF8(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[1]));
        // NULL is an engine failure (out of memory); the engine has reported it.
        if (!szName) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
    }

    if (argc > 1 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[1]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nClassId);

    if (argc > 2 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[2]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[2]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[2], &nMode);

    if (argc > 3 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[3]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[3]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[3], &nUnitId);
    JS_EndRequest(cx);

    UnitAny* pUnit = NULL;

    if (nType == 100)
        pUnit = D2CLIENT_GetCursorItem();
    else if (nType == 101) {
        pUnit = D2CLIENT_GetSelectedUnit();
        if (!pUnit)
            pUnit = (*p_D2CLIENT_SelectedInvItem);
    } else
        pUnit = GetUnit(szName, nClassId, nType, nMode, nUnitId);

    if (!pUnit) {
        JS_free(cx, szName);
        return JS_TRUE;
    }

    myUnit* pmyUnit = new myUnit;

    if (!pmyUnit) {
        JS_free(cx, szName);
        return JS_TRUE;
    }

    pmyUnit->_dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwClassId = nClassId;
    pmyUnit->dwMode = nMode;
    pmyUnit->dwType = pUnit->dwType;
    pmyUnit->dwUnitId = pUnit->dwUnitId;
    // _TRUNCATE: a name too long for the buffer is cut, not fatal; no unit name is that long, so the cut name matches nothing.
    strncpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szName ? szName : "", _TRUNCATE);
    JS_free(cx, szName);

    JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);

    // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
    if (!jsunit) {
        delete pmyUnit;
        return JS_FALSE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));

    return JS_TRUE;
}

JSAPI_FUNC(unit_getNext) {
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    Private* unit = (Private*)GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!unit)
        return JS_TRUE;

    if (unit->dwPrivateType == PRIVATE_UNIT) {
        myUnit* lpUnit = (myUnit*)unit;
        UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

        if (!pUnit)
            return JS_TRUE;
        JS_BeginRequest(cx);
        if (argc > 0 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
            char* szText = JS_EncodeStringToUTF8(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
            // NULL is an engine failure (out of memory); the engine has reported it.
            if (!szText) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
            // _TRUNCATE: a name too long for the buffer is cut, not fatal; no unit name is that long, so the cut name matches nothing.
            strncpy_s(lpUnit->szName, sizeof(lpUnit->szName), szText, _TRUNCATE);
            JS_free(cx, szText);
        }

        if (argc > 0 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]) && !(argc > 1 && JSVAL_IS_NULL(JS_ARGV(cx, vp)[1])))
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], (uint32*)&(lpUnit->dwClassId));

        if (argc > 1 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]) && !(argc > 2 && JSVAL_IS_NULL(JS_ARGV(cx, vp)[2])))
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], (uint32*)&(lpUnit->dwMode));
        JS_EndRequest(cx);
        pUnit = GetNextUnit(pUnit, lpUnit->szName, lpUnit->dwClassId, lpUnit->dwType, lpUnit->dwMode);

        if (!pUnit) {
            // Same thing as bobode's fix for finalize
            /*JSObject* obj = JS_THIS_OBJECT(cx, vp);
            //JS_ClearScope(cx, obj);
            if(JS_ValueToObject(cx, JSVAL_NULL, &obj) == JS_FALSE)
                    return JS_TRUE;*/
            JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        } else {
            lpUnit->dwUnitId = pUnit->dwUnitId;
            JS_SetPrivate(cx, JS_THIS_OBJECT(cx, vp), lpUnit);
            JS_SET_RVAL(cx, vp, JSVAL_TRUE);
        }
    } else if (unit->dwPrivateType == PRIVATE_ITEM) {
        invUnit* pmyUnit = (invUnit*)unit;
        if (!pmyUnit)
            return JS_TRUE;

        UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
        UnitAny* pOwner = D2CLIENT_FindUnit(pmyUnit->dwOwnerId, pmyUnit->dwOwnerType);
        if (!pUnit || !pOwner)
            return JS_TRUE;
        JS_BeginRequest(cx);
        if (argc > 0 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
            char* szText = JS_EncodeStringToUTF8(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
            if (!szText) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
            strncpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szText, _TRUNCATE);
            JS_free(cx, szText);
        }

        if (argc > 0 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]) && !(argc > 1 && JSVAL_IS_NULL(JS_ARGV(cx, vp)[1])))
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], (uint32*)&(pmyUnit->dwClassId));

        if (argc > 1 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]) && !(argc > 2 && JSVAL_IS_NULL(JS_ARGV(cx, vp)[2])))
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], (uint32*)&(pmyUnit->dwMode));
        JS_EndRequest(cx);
        UnitAny* nextItem = GetInvNextUnit(pUnit, pOwner, pmyUnit->szName, pmyUnit->dwClassId, pmyUnit->dwMode);
        if (!nextItem) {
            // set current object to null breaks the unit_finilize cleanup cycle
            /*JSObject* obj = JS_THIS_OBJECT(cx, vp);
            //JS_ClearScope(cx, obj);







            if(JS_ValueToObject(cx, JSVAL_NULL, &obj) == JS_FALSE)
                    return JS_TRUE;
            */
            JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        } else {
            pmyUnit->dwUnitId = nextItem->dwUnitId;
            JS_SetPrivate(cx, JS_THIS_OBJECT(cx, vp), pmyUnit);
            JS_SET_RVAL(cx, vp, JSVAL_TRUE);
        }
    }

    return JS_TRUE;
}

JSAPI_FUNC(unit_cancel) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    jsint mode = -1;
    if (argc > 0) {
        JS_BeginRequest(cx);
        if (JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]))
            JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], (uint32*)&mode);
        JS_EndRequest(cx);
    }

    {
        // Every case runs the game's own UI code, which only the game thread may run unlocked: a panel closed in the
        // middle of a frame lets the frame skip its draw-list rebuild yet still draw, and a stale entry halts the game.
        // Readiness is checked again under the lock: an area change may start while this waits for it, and
        // IsScrollingText waits for a ready game that only the game thread, blocked on this lock, can bring about.
        AutoCriticalRoom cRoom;
        DWORD automapOn = *p_D2CLIENT_AutomapOn;
        if (!GameReady())
            mode = -1;
        else if (argc == 0) {
            if (IsScrollingText())
                mode = 3;
            else if (D2CLIENT_GetCurrentInteractingNPC())
                mode = 2;
            else if (D2CLIENT_GetCursorItem())
                mode = 1;
            else
                mode = 0;
        }

        switch (mode) {
        case 0:
            D2CLIENT_CloseInteract();
            break;
        case 1:
            D2CLIENT_ClickMap(0, 10, 10, 0x08);
            break;
        case 2:
            D2CLIENT_CloseNPCInteract();
            break;
        case 3:
            D2CLIENT_ClearScreen();
            break;
        }

        *p_D2CLIENT_AutomapOn = automapOn;
    }
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(unit_repair) {
    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    BYTE aPacket[17] = {NULL};
    aPacket[0] = 0x35;
    *(DWORD*)&aPacket[1] = *p_D2CLIENT_RecentInteractId;
    aPacket[16] = 0x80;
    D2NET_SendPacket(17, 1, aPacket);

    // note: this crashes while minimized
    //	D2CLIENT_PerformNpcAction(pUnit,1, NULL);
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);

    return JS_TRUE;
}

JSAPI_FUNC(unit_useMenu) {
    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    // A menu entry runs the game's own UI code (opening a store sets the panel mode), which only the game thread may
    // run unlocked, and only while the game is ready (an area change may start while this waits for the lock).
    AutoCriticalRoom cRoom;
    if (!GameReady())
        return JS_TRUE;
    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(ClickNPCMenu(pUnit->dwTxtFileNo, JSVAL_TO_INT(JS_ARGV(cx, vp)[0]))));

    return JS_TRUE;
}

JSAPI_FUNC(unit_interact) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit || pUnit == D2CLIENT_GetPlayerUnit())
        return JS_TRUE;

    if (pUnit->dwType == UNIT_ITEM && pUnit->dwMode != ITEM_MODE_ON_GROUND && pUnit->dwMode != ITEM_MODE_BEING_DROPPED) {
        int nLocation = GetItemLocation(pUnit);

        BYTE aPacket[13] = {NULL};

        if (nLocation == LOCATION_INVENTORY || nLocation == LOCATION_STASH) {
            aPacket[0] = 0x20;
            *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
            *(DWORD*)&aPacket[5] = D2CLIENT_GetPlayerUnit()->pPath->xPos;
            *(DWORD*)&aPacket[9] = D2CLIENT_GetPlayerUnit()->pPath->yPos;
            D2NET_SendPacket(13, 1, aPacket);
            return JS_TRUE;
        } else if (nLocation == LOCATION_BELT) {
            aPacket[0] = 0x26;
            *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
            *(DWORD*)&aPacket[5] = 0;
            *(DWORD*)&aPacket[9] = 0;
            D2NET_SendPacket(13, 1, aPacket);
            return JS_TRUE;
        }
    }

    if (pUnit->dwType == UNIT_OBJECT && argc == 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0])) {
        // TODO: check the range on argv[0] to make sure it won't crash the game - Done! TechnoHunter
        jsint nWaypointID;
        JS_BeginRequest(cx);
        if (!JS_ValueToECMAInt32(cx, JS_ARGV(cx, vp)[0], &nWaypointID)) {
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        JS_EndRequest(cx);
        int retVal = 0;
        if (FillBaseStat("levels", nWaypointID, "Waypoint", &retVal, sizeof(int)))
            if (retVal == 255)
                return JS_TRUE;

        {
            // Taking the waypoint closes its panel, game UI code that only the game thread may run unlocked, and only
            // while the game is ready. The id comes from the script's unit: pUnit was found without the lock and may be
            // gone by now.
            AutoCriticalRoom cRoom;
            if (!GameReady())
                return JS_TRUE;
            D2CLIENT_TakeWaypoint(lpUnit->dwUnitId, nWaypointID);
            if (!D2CLIENT_GetUIState(UI_GAME))
                D2CLIENT_CloseInteract();
        }

        JS_SET_RVAL(cx, vp, JSVAL_TRUE);
        return JS_TRUE;
    }
    //	else if(pUnit->dwType == UNIT_PLAYER && argc == 1 && JSVAL_IS_INT(argv[0]) && JSVAL_TO_INT(argv[0]) == 1)
    //	{
    // Accept Trade
    //	}
    else {
        // ClickMap waits for the game lock, as long as a frame in which pUnit can be freed: find it again under the lock.
        AutoCriticalRoom cRoom;
        if (!GameReady())
            return JS_TRUE;
        pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
        if (!pUnit)
            return JS_TRUE;
        JS_SET_RVAL(cx, vp, JSVAL_TRUE);
        ClickMap(0, D2CLIENT_GetUnitX(pUnit), D2CLIENT_GetUnitY(pUnit), FALSE, pUnit);
        // D2CLIENT_Interact(pUnit, 0x45);
    }

    return JS_TRUE;
}

bool InsertStatsToGenericObject(UnitAny* pUnit, StatList* pStatList, JSContext* pJSContext, JSObject* pGenericObject);
bool InsertStatsNow(Stat* pStat, int nStat, JSContext* cx, JSObject* pArray);

JSAPI_FUNC(unit_getStat) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    // No stat id reads as false: JS_ConvertArguments' missing-argument report would name the function from the
    // callee slot, which the rval above has overwritten.
    if (argc < 1)
        return JS_TRUE;

    jsint nStat = 0;
    jsint nSubIndex = 0;
    JS_BeginRequest(cx);
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i/i", &nStat, &nSubIndex)) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }
    JS_EndRequest(cx);

    if (nStat >= STAT_HP && nStat <= STAT_MAXSTAMINA)
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex) >> 8));
    else if (nStat == STAT_EXP || nStat == STAT_LASTEXP || nStat == STAT_NEXTEXP) {
        jsval rval;
        rval = JS_NumberValue((unsigned int)D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex));
        JS_SET_RVAL(cx, vp, rval);
    } else if (nStat == STAT_ITEMLEVELREQ)
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit())));
    else if (nStat == -1) {
        Stat aStatList[256] = {NULL};
        StatList* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);

        if (pStatList) {
            DWORD dwStats = D2COMMON_CopyStatList(pStatList, (Stat*)aStatList, 256);

            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));

            for (int j = 0; j < pUnit->pStats->StatVec.wCount; j++) {
                bool inListAlready = false;
                for (DWORD k = 0; k < dwStats; k++) {
                    if (aStatList[k].dwStatValue == pUnit->pStats->StatVec.pStats[j].dwStatValue &&
                        aStatList[k].wStatIndex == pUnit->pStats->StatVec.pStats[j].wStatIndex && aStatList[k].wSubIndex == pUnit->pStats->StatVec.pStats[j].wSubIndex)
                        inListAlready = true;
                }
                if (!inListAlready) {

                    aStatList[dwStats].dwStatValue = pUnit->pStats->StatVec.pStats[j].dwStatValue;
                    aStatList[dwStats].wStatIndex = pUnit->pStats->StatVec.pStats[j].wStatIndex;
                    aStatList[dwStats].wSubIndex = pUnit->pStats->StatVec.pStats[j].wSubIndex;
                    dwStats++;
                }
            }
            for (UINT i = 0; i < dwStats; i++) {
                JS_BeginRequest(cx);
                JSObject* pArrayInsert = JS_NewArrayObject(cx, 0, NULL);
                if (!pArrayInsert || !JS_AddRoot(cx, &pArrayInsert)) {
                    JS_EndRequest(cx);
                    return JS_FALSE;
                }

                jsval nIndex = INT_TO_JSVAL(aStatList[i].wStatIndex);
                jsval nSubIndex = INT_TO_JSVAL(aStatList[i].wSubIndex);
                jsval nValue = INT_TO_JSVAL(aStatList[i].dwStatValue);

                jsval aObj = OBJECT_TO_JSVAL(pArrayInsert);

                if (!JS_SetElement(cx, pArrayInsert, 0, &nIndex) || !JS_SetElement(cx, pArrayInsert, 1, &nSubIndex) ||
                    !JS_SetElement(cx, pArrayInsert, 2, &nValue) || !JS_SetElement(cx, pReturnArray, i, &aObj)) {
                    JS_RemoveRoot(cx, &pArrayInsert);
                    JS_EndRequest(cx);
                    return JS_FALSE;
                }
                JS_RemoveRoot(cx, &pArrayInsert);
                JS_EndRequest(cx);
            }
        }
    } else if (nStat == -2) {
        JSObject* pArray = JS_NewArrayObject(cx, 0, NULL);
        if (!pArray)
            return JS_FALSE;
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pArray));

        if (!InsertStatsToGenericObject(pUnit, pUnit->pStats, cx, pArray) ||
            !InsertStatsToGenericObject(pUnit, D2COMMON_GetStatList(pUnit, NULL, 0x40), cx, pArray))
            return JS_FALSE;
        // InsertStatsToGenericObject(pUnit, pUnit->pStats->pNext, cx, pArray);  // only check the current unit stats!
        //	InsertStatsToGenericObject(pUnit, pUnit->pStats->pSetList, cx, pArray);
    } else {
        long result = D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex);
        if (result == 0) // if stat isnt found look up preset list
        {
            StatList* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);
            Stat aStatList[256] = {NULL};
            if (pStatList) {
                DWORD dwStats = D2COMMON_CopyStatList(pStatList, (Stat*)aStatList, 256);
                for (UINT i = 0; i < dwStats; i++) {
                    if (nStat == aStatList[i].wStatIndex && nSubIndex == aStatList[i].wSubIndex)
                        result = (aStatList[i].dwStatValue);
                }
            }
        }
        jsval rval = JS_RVAL(cx, vp);
        rval = JS_NumberValue(result);
        JS_SET_RVAL(cx, vp, rval);
    }
    return JS_TRUE;
}

bool InsertStatsToGenericObject(UnitAny* pUnit, StatList* pStatList, JSContext* cx, JSObject* pArray) {
    Stat* pStat = NULL;
    if (!pStatList)
        return true;
    if ((pStatList->dwUnitId == pUnit->dwUnitId && pStatList->dwUnitType == pUnit->dwType) || pStatList->pUnit == pUnit) {
        pStat = pStatList->StatVec.pStats;

        for (int nStat = 0; nStat < pStatList->StatVec.wCount; nStat++) {
            if (!InsertStatsNow(pStat, nStat, cx, pArray))
                return false;
        }
    }
    if ((pStatList->dwFlags >> 24 & 0x80)) {
        pStat = pStatList->SetStatVec.pStats;

        for (int nStat = 0; nStat < pStatList->SetStatVec.wCount; nStat++) {
            if (!InsertStatsNow(pStat, nStat, cx, pArray))
                return false;
        }
    }
    return true;
}

// False on any engine failure (out of memory, already reported), which unit_getStat turns into JS_FALSE.
bool InsertStatsNow(Stat* pStat, int nStat, JSContext* cx, JSObject* pArray) {
    if (pStat[nStat].wSubIndex > 0x200) {
        // subindex is the skill id and level
        int skill = pStat[nStat].wSubIndex >> 6, level = pStat[nStat].wSubIndex & 0x3F, charges = 0, maxcharges = 0;
        if (pStat[nStat].dwStatValue > 0x200) {
            charges = pStat[nStat].dwStatValue & 0xFF;
            maxcharges = pStat[nStat].dwStatValue >> 8;
        }
        JSObject* val = BuildObject(cx, NULL);
        if (!val)
            return false;
        jsval jsskill = INT_TO_JSVAL(skill), jslevel = INT_TO_JSVAL(level), jscharges = INT_TO_JSVAL(charges), jsmaxcharges = INT_TO_JSVAL(maxcharges);
        // val is an anonymous object that holds properties
        if (!JS_SetProperty(cx, val, "skill", &jsskill) || !JS_SetProperty(cx, val, "level", &jslevel))
            return false;
        if (maxcharges > 0) {
            if (!JS_SetProperty(cx, val, "charges", &jscharges) || !JS_SetProperty(cx, val, "maxcharges", &jsmaxcharges))
                return false;
        }
        // find where we should put it
        jsval index = JSVAL_VOID, obj = OBJECT_TO_JSVAL(val);
        if (!JS_GetElement(cx, pArray, pStat[nStat].wStatIndex, &index))
            return false;
        if (index != JSVAL_VOID) {
            // modify the existing object by stuffing it into an array
            if (!JS_IsArrayObject(cx, JSVAL_TO_OBJECT(index))) {
                // it's not an array, build one
                JS_BeginRequest(cx);
                JSObject* arr = JS_NewArrayObject(cx, 0, NULL);
                if (!arr || !JS_AddRoot(cx, &arr)) {
                    JS_EndRequest(cx);
                    return false;
                }
                jsval arr2 = OBJECT_TO_JSVAL(arr);
                if (!JS_SetElement(cx, arr, 0, &index) || !JS_SetElement(cx, arr, 1, &obj) || !JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, &arr2)) {
                    JS_RemoveRoot(cx, &arr);
                    JS_EndRequest(cx);
                    return false;
                }
                JS_RemoveRoot(cx, &arr);
                JS_EndRequest(cx);
            } else {
                // it is an array, append the new value
                JSObject* arr = JSVAL_TO_OBJECT(index);
                jsuint len = 0;
                if (!JS_GetArrayLength(cx, arr, &len))
                    return false;
                JS_BeginRequest(cx);
                if (!JS_SetElement(cx, arr, len, &obj)) {
                    JS_EndRequest(cx);
                    return false;
                }
                JS_EndRequest(cx);
            }
        } else {
            JS_BeginRequest(cx);
            if (!JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, &obj)) {
                JS_EndRequest(cx);
                return false;
            }
            JS_EndRequest(cx);
        }
    } else {
        // Make sure to bit shift life, mana and stamina properly!
        int value = pStat[nStat].dwStatValue;
        if (pStat[nStat].wStatIndex >= 6 && pStat[nStat].wStatIndex <= 11)
            value = value >> 8;

        jsval index = JSVAL_VOID, val = INT_TO_JSVAL(value);
        if (!JS_GetElement(cx, pArray, pStat[nStat].wStatIndex, &index))
            return false;
        if (index == JSVAL_VOID) {
            // the array index doesn't exist, make it
            JSObject* arr = JS_NewArrayObject(cx, 0, NULL);
            if (!arr)
                return false;
            index = OBJECT_TO_JSVAL(arr);
            JS_BeginRequest(cx);
            if (!JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, &index)) {
                JS_EndRequest(cx);
                return false;
            }
            JS_EndRequest(cx);
        }
        // index now points to the correct array index
        JS_BeginRequest(cx);
        if (!JS_SetElement(cx, JSVAL_TO_OBJECT(index), pStat[nStat].wSubIndex, &val)) {
            JS_EndRequest(cx);
            return false;
        }
        JS_EndRequest(cx);
    }
    return true;
}

JSAPI_FUNC(unit_getState) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    // argv is not padded to nargs: with no state id, argv[0] is a stale stack slot.
    if (argc < 1)
        THROW_ERROR(cx, "Invalid arguments");

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    jsint nState;
    JS_BeginRequest(cx);
    if (JS_ValueToInt32(cx, JS_ARGV(cx, vp)[0], &nState) == JS_FALSE) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }
    JS_EndRequest(cx);

    // TODO: make these constants so we know what we're checking here
    if (nState > 183 || nState < 0)
        return JS_TRUE;

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(!!D2COMMON_GetUnitState(pUnit, nState)));

    return JS_TRUE;
}

JSAPI_FUNC(item_getFlags) {
    // Every early JS_TRUE return must leave a value: an unset rval hands the script this function (truthy).
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit || pUnit->dwType != UNIT_ITEM)
        return JS_TRUE;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(pUnit->pItemData->dwFlags));

    return JS_TRUE;
}

JSAPI_FUNC(item_getFlag) {
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit || pUnit->dwType != UNIT_ITEM)
        return JS_TRUE;

    jsint nFlag = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(!!(nFlag & pUnit->pItemData->dwFlags)));

    return JS_TRUE;
}

// JSAPI_FUNC(item_getPrice)
//{
//	DEPRECATED;
//
//	if(!WaitForGameReady())
//		THROW_WARNING(cx, vp,  "Game not ready");
//
//	int diff = D2CLIENT_GetDifficulty();
//	//D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, diff, *p_D2CLIENT_ItemPriceList, NPCID, buysell)
//	int buysell = 0;
//	int NPCID = 148;
//
//	myUnit* lpUnit = (myUnit*)JS_GetPrivate(cx, JS_THIS_OBJECT(cx, vp));
//
//	if(!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
//		return JS_TRUE;
//
//	UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
//
//	if(!pUnit)
//		return JS_TRUE;
//
//	if(argc>0)
//	{
//		if(JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[0]))
//		{
//			myUnit* pmyNpc = (myUnit*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]));
//
//			if(!pmyNpc || (pmyNpc->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
//				return JS_TRUE;
//
//			UnitAny* pNpc = D2CLIENT_FindUnit(pmyNpc->dwUnitId, pmyNpc->dwType);
//
//			if(!pNpc)
//				return JS_TRUE;
//
//			NPCID = pNpc->dwTxtFileNo;
//		}
//		else if(JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
//			NPCID = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
//	}
//	if(argc>1)
//		buysell = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
//	if(argc>2)
//		diff = JSVAL_TO_INT(JS_ARGV(cx, vp)[2]);
//
//	*rval = INT_TO_JSVAL(D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, diff, *p_D2CLIENT_ItemPriceList, NPCID, buysell));
//
//	return JS_TRUE;
//}

JSAPI_FUNC(item_getItemCost) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    jsint nMode;
    UnitAny* npc = D2CLIENT_GetCurrentInteractingNPC();
    jsint nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A); // defaults to Charsi's NPC id
    jsint nDifficulty = D2CLIENT_GetDifficulty();

    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit || pUnit->dwType != UNIT_ITEM)
        return JS_TRUE;

    nMode = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    if (argc > 1) {
        if (JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[1])) {
            myUnit* pmyNpc = GetUnitPrivate(cx, JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[1]));

            if (!pmyNpc || (pmyNpc->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
                return JS_TRUE;

            UnitAny* pNpc = D2CLIENT_FindUnit(pmyNpc->dwUnitId, pmyNpc->dwType);

            if (!pNpc)
                return JS_TRUE;
            nNpcClassId = pNpc->dwTxtFileNo;
        } else if (JSVAL_IS_INT(JS_ARGV(cx, vp)[1]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[1])) {
            JS_BeginRequest(cx);
            if (!JS_ValueToECMAInt32(cx, JS_ARGV(cx, vp)[1], &nNpcClassId)) {
                JS_EndRequest(cx);
                return JS_FALSE;
            }
            JS_EndRequest(cx);
        }
        // TODO:: validate the base stat table sizes to make sure the game doesn't crash with checking values past the end of the table
        int retVal = 0;
        if (FillBaseStat("monstats", nNpcClassId, "inventory", &retVal, sizeof(int)) && retVal == 0)
            nNpcClassId = 0x9A; // invalid npcid incoming! default to charsi to allow the game to continue
    }

    if (argc > 2 && JSVAL_IS_INT(JS_ARGV(cx, vp)[2]))
        nDifficulty = JSVAL_TO_INT(JS_ARGV(cx, vp)[2]);

    switch (nMode) {
    case 0: // Buy
    case 1: // Sell
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *p_D2CLIENT_ItemPriceList, nNpcClassId, nMode)));
        break;
    case 2: // Repair
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *p_D2CLIENT_ItemPriceList, nNpcClassId, 3)));
        break;
    default:
        break;
    }

    return JS_TRUE;
}

JSAPI_FUNC(unit_getItems) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    UnitAny* pUnit = NULL;
    if (lpUnit && (lpUnit->_dwPrivateType & PRIVATE_UNIT) == PRIVATE_UNIT)
        pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    // Nothing to list (not a unit, no inventory, an empty one, or gone) gets an empty array: the documented Unit[]
    // return, which `getItems().some(...)` callers rely on.
    if (!pUnit || !pUnit->pInventory || !pUnit->pInventory->pFirstItem) {
        JSObject* pEmpty = JS_NewArrayObject(cx, 0, NULL);
        if (!pEmpty)
            return JS_FALSE;
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pEmpty));
        return JS_TRUE;
    }
    JS_BeginRequest(cx);
    JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
    if (!pReturnArray || !JS_AddRoot(cx, &pReturnArray)) {
        JS_EndRequest(cx);
        return JS_FALSE;
    }

    DWORD dwArrayCount = 0;

    for (UnitAny* pItem = pUnit->pInventory->pFirstItem; pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem), dwArrayCount++) {
        invUnit* pmyUnit = new invUnit;

        if (!pmyUnit)
            continue;

        pmyUnit->_dwPrivateType = PRIVATE_UNIT;
        pmyUnit->szName[0] = NULL;
        pmyUnit->dwMode = pItem->dwMode;
        pmyUnit->dwClassId = pItem->dwTxtFileNo;
        pmyUnit->dwUnitId = pItem->dwUnitId;
        pmyUnit->dwType = UNIT_ITEM;
        pmyUnit->dwOwnerId = pUnit->dwUnitId;
        pmyUnit->dwOwnerType = pUnit->dwType;

        JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);
        if (!jsunit) {
            delete pmyUnit;
            JS_EndRequest(cx);
            JS_RemoveRoot(cx, &pReturnArray);
            THROW_ERROR(cx, "Failed to build item array");
        }
        jsval a = OBJECT_TO_JSVAL(jsunit);
        if (!JS_SetElement(cx, pReturnArray, dwArrayCount, &a)) {
            JS_RemoveRoot(cx, &pReturnArray);
            JS_EndRequest(cx);
            return JS_FALSE;
        }
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
    JS_RemoveRoot(cx, &pReturnArray);
    JS_EndRequest(cx);
    return JS_TRUE;
}

JSAPI_FUNC(unit_getSkill) {
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    if (argc == NULL || argc > 3)
        return JS_TRUE;

    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    bool nCharge = false;
    jsint nSkillId = NULL;
    jsint nExt = NULL;

    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));
    if (!pmyUnit)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
    if (!pUnit)
        return JS_TRUE;

    if (argc >= 1) {
        if (!JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
            return JS_TRUE;

        nSkillId = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    }

    if (argc >= 2) {
        if (!JSVAL_IS_INT(JS_ARGV(cx, vp)[1]))
            return JS_TRUE;

        nExt = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
    }

    if (argc == 3) {
        if (!JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[2]))
            return JS_TRUE;

        nCharge = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[2]);
    }

    if (argc == 1) {
        WORD wLeftSkillId = pUnit->pInfo->pLeftSkill->pSkillInfo->wSkillId;
        WORD wRightSkillId = pUnit->pInfo->pRightSkill->pSkillInfo->wSkillId;
        switch (nSkillId) {
        case 0: {
            int row = 0;
            if (FillBaseStat("skills", wRightSkillId, "skilldesc", &row, sizeof(int)))
                if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
                    wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
                    JSString* name = JS_NewUCStringCopyZ(cx, szName);
                    if (!name)
                        return JS_FALSE;
                    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(name));
                }
        } break;
        case 1: {
            int row = 0;
            if (FillBaseStat("skills", wLeftSkillId, "skilldesc", &row, sizeof(int)))
                if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
                    wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
                    JSString* name = JS_NewUCStringCopyZ(cx, szName);
                    if (!name)
                        return JS_FALSE;
                    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(name));
                }
        } break;

        case 2:
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(wRightSkillId));
            break;
        case 3:
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(wLeftSkillId));
            break;
        case 4: {
            JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
            if (!pReturnArray)
                return JS_FALSE;
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
            int i = 0;
            for (Skill* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
                JSObject* pArrayInsert = JS_NewArrayObject(cx, 0, NULL);
                if (!pArrayInsert || !JS_AddRoot(cx, &pArrayInsert))
                    return JS_FALSE;

                jsval nId = INT_TO_JSVAL(pSkill->pSkillInfo->wSkillId);
                jsval nBase = INT_TO_JSVAL(pSkill->dwSkillLevel);
                jsval nTotal = INT_TO_JSVAL(D2COMMON_GetSkillLevel(pUnit, pSkill, 1));
                JS_BeginRequest(cx);
                jsval aObj = OBJECT_TO_JSVAL(pArrayInsert);

                if (!JS_SetElement(cx, pArrayInsert, 0, &nId) || !JS_SetElement(cx, pArrayInsert, 1, &nBase) ||
                    !JS_SetElement(cx, pArrayInsert, 2, &nTotal) || !JS_SetElement(cx, pReturnArray, i, &aObj)) {
                    JS_RemoveRoot(cx, &pArrayInsert);
                    JS_EndRequest(cx);
                    return JS_FALSE;
                }
                JS_RemoveRoot(cx, &pArrayInsert);
                JS_EndRequest(cx);
                i++;
            }
            break;
        }
        default:
            JS_SET_RVAL(cx, vp, JSVAL_FALSE);
            break;
        }
        return JS_TRUE;
    }

    if (pUnit && pUnit->pInfo && pUnit->pInfo->pFirstSkill) {
        for (Skill* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
            if (pSkill->pSkillInfo && pSkill->pSkillInfo->wSkillId == nSkillId) {
                if ((argc == 2 && pSkill->IsCharge == 0) || (argc == 3 && (nCharge == false || pSkill->IsCharge == 1))) {
                    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetSkillLevel(pUnit, pSkill, nExt)));
                    return JS_TRUE;
                }
            }
        }
    }

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    return JS_TRUE;
}

JSAPI_FUNC(item_shop) {

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    // The mode is the last argument; with none, argv[argc - 1] would be `this`.
    uint32 nModeArg = 0;
    if (argc < 1 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[argc - 1]) ||
        !JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[argc - 1], &nModeArg))
        return JS_TRUE;

    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    AutoCriticalRoom* cRoom = new AutoCriticalRoom;

    if (*p_D2CLIENT_TransactionDialog != 0 || *p_D2CLIENT_TransactionDialogs != 0 || *p_D2CLIENT_TransactionDialogs_2 != 0) {
        delete cRoom;
        return JS_TRUE;
    }

    myUnit* lpItem = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!lpItem || (lpItem->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
        delete cRoom;
        return JS_TRUE;
    }

    UnitAny* pItem = D2CLIENT_FindUnit(lpItem->dwUnitId, lpItem->dwType);

    if (!pItem || pItem->dwType != UNIT_ITEM) {
        delete cRoom;
        return JS_TRUE;
    }

    if (!D2CLIENT_GetUIState(UI_NPCSHOP)) {
        delete cRoom;
        return JS_TRUE;
    }

    UnitAny* pNPC = D2CLIENT_GetCurrentInteractingNPC();
    DWORD dwMode = nModeArg;

    // Check if we are interacted.
    if (!pNPC) {
        delete cRoom;
        return JS_TRUE;
    }

    // Check for proper mode.
    if ((dwMode != 1) && (dwMode != 2) && (dwMode != 6)) {
        delete cRoom;
        return JS_TRUE;
    }

    // Selling an Item
    if (dwMode == 1) {
        // Check if we own the item!
        if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != D2CLIENT_GetPlayerUnit()->dwUnitId) {
            delete cRoom;
            return JS_TRUE;
        }

        D2CLIENT_ShopAction(pNPC, pItem, 1, 0, 0, 1, 1, NULL);
    } else {
        // Make sure the item is owned by the NPC interacted with.
        if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != pNPC->dwUnitId) {
            delete cRoom;
            return JS_TRUE;
        }

        D2CLIENT_ShopAction(pNPC, pItem, 0, 0, 0, dwMode, 1, NULL);
    }

    /*BYTE pPacket[17] = {NULL};

    if(dwMode == 2 || dwMode == 6)
            pPacket[0] = 0x32;
    else pPacket[0] = 0x33;

    *(DWORD*)&pPacket[1] = pNPC->dwUnitId;
    *(DWORD*)&pPacket[5] = pItem->dwUnitId;

    if(dwMode == 1) // Sell
    {
            if(D2CLIENT_GetCursorItem() && D2CLIENT_GetCursorItem() == pItem)
                    *(DWORD*)&pPacket[9] = 0x04;
            else
                    *(DWORD*)&pPacket[9] = 0;
    }
    else if(dwMode == 2) // Buy
    {
            if(pItem->pItemData->dwFlags & 0x10)
                    *(DWORD*)&pPacket[9] = 0x00;
            else
                    *(DWORD*)&pPacket[9] = 0x02;
    }
    else
            *(BYTE*)&pPacket[9+3] = 0x80;

    int nBuySell = NULL;

    if(dwMode == 2 || dwMode == 6)
            nBuySell = NULL;
    else nBuySell = 1;

    *(DWORD*)&pPacket[13] = D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pItem, D2CLIENT_GetDifficulty(), *p_D2CLIENT_ItemPriceList, pNPC->dwTxtFileNo,
    nBuySell);

    D2NET_SendPacket(sizeof(pPacket), 1, pPacket);*/

    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    delete cRoom;
    return JS_TRUE;
}

JSAPI_FUNC(unit_getParent) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    if (pUnit->dwType == UNIT_MONSTER) {
        DWORD dwOwnerId = D2CLIENT_GetMonsterOwner(pUnit->dwUnitId);
        if (dwOwnerId == -1)
            return JS_TRUE;

        // The game's pet list (0x7BB5BC) gives the owner's id without its type. Pets belong to players, and ids overlap
        // across unit types, so only players are searched.
        UnitAny* pOwner = GetUnit(NULL, (DWORD)-1, UNIT_PLAYER, (DWORD)-1, dwOwnerId);
        if (!pOwner)
            return JS_TRUE;

        myUnit* pmyUnit = new myUnit;
        if (!pmyUnit)
            return JS_TRUE;

        pmyUnit->_dwPrivateType = PRIVATE_UNIT;
        pmyUnit->dwUnitId = pOwner->dwUnitId;
        pmyUnit->dwClassId = pOwner->dwTxtFileNo;
        pmyUnit->dwMode = pOwner->dwMode;
        pmyUnit->dwType = pOwner->dwType;
        pmyUnit->szName[0] = NULL;

        JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);
        // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
        if (!jsunit) {
            delete pmyUnit;
            return JS_FALSE;
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));
        return JS_TRUE;
    } else if (pUnit->dwType == UNIT_OBJECT) {
        if (pUnit->pObjectData) {
            char szBuffer[128] = "";
            strcpy_s(szBuffer, sizeof(szBuffer), pUnit->pObjectData->szOwner);

            if (!StoreString(vp, JS_InternString(cx, szBuffer)))
                return JS_FALSE;
        }
    } else if (pUnit->dwType == UNIT_ITEM) {
        if (pUnit->pItemData && pUnit->pItemData->pOwnerInventory && pUnit->pItemData->pOwnerInventory->pOwner) {
            myUnit* pmyUnit = new myUnit;

            if (!pmyUnit)
                return JS_TRUE;

            pmyUnit->_dwPrivateType = PRIVATE_UNIT;
            pmyUnit->dwUnitId = pUnit->pItemData->pOwnerInventory->pOwner->dwUnitId;
            pmyUnit->dwClassId = pUnit->pItemData->pOwnerInventory->pOwner->dwTxtFileNo;
            pmyUnit->dwMode = pUnit->pItemData->pOwnerInventory->pOwner->dwMode;
            pmyUnit->dwType = pUnit->pItemData->pOwnerInventory->pOwner->dwType;
            pmyUnit->szName[0] = NULL;
            JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);
            if (!jsunit) {
                delete pmyUnit;
                return JS_FALSE;
            }

            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));
        }
    } else if (pUnit->dwType == UNIT_MISSILE) {
        // Undefined when the owner is not among the client's units (dead, or out of sight); the missile's owner and
        // ownertype properties still give its id and type.
        UnitAny* pOwner = FindMissileOwner(pUnit);
        if (!pOwner)
            return JS_TRUE;

        myUnit* pmyUnit = new myUnit;
        pmyUnit->_dwPrivateType = PRIVATE_UNIT;
        pmyUnit->dwUnitId = pOwner->dwUnitId;
        pmyUnit->dwClassId = pOwner->dwTxtFileNo;
        pmyUnit->dwMode = pOwner->dwMode;
        pmyUnit->dwType = pOwner->dwType;
        pmyUnit->szName[0] = NULL;

        JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);
        if (!jsunit) {
            delete pmyUnit;
            return JS_FALSE;
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));
    }

    return JS_TRUE;
}

// Works only on players sinces monsters _CANT_ have mercs!
JSAPI_FUNC(unit_getMerc) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* lpUnit = GetUnitPrivate(cx, self);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    DWORD dwMercId = 0, dwMercClass = 0;
    bool bFound = false;
    {
        // GetMercUnit walks room unit lists the game relinks: copy what the wrapper needs and let go before BuildObject.
        AutoCriticalRoom cRoom;
        UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
        UnitAny* pMerc = pUnit && pUnit->dwType == UNIT_PLAYER ? GetMercUnit(pUnit) : NULL;
        if (pMerc) {
            bFound = true;
            dwMercId = pMerc->dwUnitId;
            dwMercClass = pMerc->dwTxtFileNo;
        }
    }
    if (!bFound)
        return JS_TRUE;

    myUnit* pmyUnit = new myUnit;
    pmyUnit->_dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwUnitId = dwMercId;
    pmyUnit->dwClassId = dwMercClass;
    pmyUnit->dwMode = NULL;
    pmyUnit->dwType = UNIT_MONSTER;
    pmyUnit->szName[0] = NULL;
    JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyUnit);
    // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
    if (!jsunit) {
        delete pmyUnit;
        return JS_FALSE;
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));
    return JS_TRUE;
}

JSAPI_FUNC(unit_getMercHP) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    if (!self)
        return JS_FALSE;
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");
    // Called as a global, `this` is no Unit and the player's merc is read.
    myUnit* lpUnit = GetUnitPrivate(cx, self);

    bool bFound = false;
    DWORD dwHPPercent = 0;
    {
        // GetMercUnit walks room unit lists and the HP lookup walks the unit tables, both relinked by the game thread.
        AutoCriticalRoom cRoom;
        UnitAny* pUnit = lpUnit ? D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType) : D2CLIENT_GetPlayerUnit();
        UnitAny* pMerc = pUnit && pUnit->dwType == UNIT_PLAYER ? GetMercUnit(pUnit) : NULL;
        if (pMerc) {
            bFound = true;
            // The merc's own mode: its dead mode (12) is the player's kick mode, so the owner's mode says nothing here.
            dwHPPercent = pMerc->dwMode == NPC_MODE_DEAD ? 0 : D2CLIENT_GetUnitHPPercent(pMerc->dwUnitId);
        }
    }
    if (bFound)
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL((int)dwHPPercent));
    return JS_TRUE;
}

// unit.setSkill( int skillId OR String skillName, int hand [, int itemGlobalId] );
JSAPI_FUNC(unit_setskill) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    WORD nSkillId = (WORD)-1;
    BOOL nHand = FALSE;
    DWORD itemId = (DWORD)-1;
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);

    if (argc < 1)
        return JS_TRUE;

    if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
        char* name = JS_EncodeStringToUTF8(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
        // NULL is an engine failure (out of memory); the engine has reported it.
        if (!name)
            return JS_FALSE;
        nSkillId = GetSkillByName(name);
        JS_free(cx, name);
    } else if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        nSkillId = (WORD)JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    else
        return JS_TRUE;

    if (argc > 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[1]))
        nHand = !!JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
    else
        return JS_TRUE;

    if (argc == 3 && JSVAL_IS_OBJECT(JS_ARGV(cx, vp)[2])) {
        JSObject* object = JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[2]);
        myUnit* unit = GetUnitPrivate(cx, object);
        if (unit && unit->dwType == UNIT_ITEM)
            itemId = unit->dwUnitId;
    }

    if (SetSkill(cx, nSkillId, nHand, itemId))
        JS_SET_RVAL(cx, vp, JSVAL_TRUE);

    return JS_TRUE;
}

JSAPI_FUNC(my_overhead) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    // A gone unit returns false before the message is read, so its toString() never runs.
    if (!D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType))
        return JS_TRUE;

    // argv is not padded to nargs: with no message, argv[0] is a stale stack slot.
    if (argc < 1)
        THROW_ERROR(cx, "Invalid arguments");

    const wchar_t* lpszText = NULL;
    if (!JSVAL_IS_NULL(JS_ARGV(cx, vp)[0]) && !JSVAL_IS_VOID(JS_ARGV(cx, vp)[0])) {
        JSString* str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
        if (!str)
            return JS_FALSE;
        lpszText = JS_GetStringCharsZ(cx, str);
        if (!lpszText)
            return JS_FALSE;
    }

    // toString() can run script, during which the unit may leave the game: find it again.
    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit)
        return JS_TRUE;

    if (lpszText && lpszText[0]) {
        // Fix overhead msg for locale text
        // wchar_t* tmpw = AnsiToUnicode(lpszText);
        // Convert back to multibyte in locale code page
        char* tmpc = UnicodeToAnsi(lpszText, CP_ACP);
        OverheadMsg* pMsg = D2COMMON_GenerateOverheadMsg(NULL, tmpc, *p_D2CLIENT_OverheadTrigger);
        if (pMsg) {
            // D2COMMON_FixOverheadMsg(pMsg, NULL);
            pUnit->pOMsg = pMsg;
        }
        delete[] tmpc;
        // delete[] tmpw;
    }

    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(my_revive) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    BYTE pPacket[] = {0x41};
    D2NET_SendPacket(1, 1, pPacket);
    return JS_TRUE;
}

JSAPI_FUNC(unit_getItem) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");
    JS_SET_RVAL(cx, vp, JSVAL_VOID);

    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit || !pUnit->pInventory)
        return JS_TRUE;

    uint32 nClassId = (uint32)-1;
    uint32 nMode = (uint32)-1;
    uint32 nUnitId = (uint32)-1;
    char szName[128] = "";

    if (argc > 0 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
        const wchar_t* szText = JS_GetStringCharsZ(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
        // NULL is an engine failure (out of memory); the engine has reported it.
        if (!szText)
            return JS_FALSE;
        char* euc = UnicodeToAnsi(szText, CP_ACP);
        // _TRUNCATE: a name too long for the buffer is cut, not fatal; no item name is that long, so the cut name matches nothing.
        strncpy_s(szName, sizeof(szName), euc, _TRUNCATE);
        delete[] euc;
    }

    if (argc > 0 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[0]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[0], &nClassId);

    if (argc > 1 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[1]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[1], &nMode);

    if (argc > 2 && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[2]) && !JSVAL_IS_NULL(JS_ARGV(cx, vp)[2]))
        JS_ValueToECMAUint32(cx, JS_ARGV(cx, vp)[2], &nUnitId);

    UnitAny* pItem = GetInvUnit(pUnit, szName, nClassId, nMode, nUnitId);

    if (!pItem)
        return JS_TRUE;

    invUnit* pmyItem = new invUnit;

    if (!pmyItem)
        return JS_TRUE;

    pmyItem->_dwPrivateType = PRIVATE_ITEM;
    pmyItem->dwClassId = nClassId;
    pmyItem->dwMode = nMode;
    pmyItem->dwType = pItem->dwType;
    pmyItem->dwUnitId = pItem->dwUnitId;
    pmyItem->dwOwnerId = pmyUnit->dwUnitId;
    pmyItem->dwOwnerType = pmyUnit->dwType;
    strcpy_s(pmyItem->szName, sizeof(pmyItem->szName), szName);

    JSObject* jsunit = BuildObject(cx, &unit_class, unit_methods, unit_props, pmyItem);

    // BuildObject attaches the private only when it succeeds; a NULL is an engine failure (out of memory).
    if (!jsunit) {
        delete pmyItem;
        return JS_FALSE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsunit));

    return JS_TRUE;
}

JSAPI_FUNC(unit_move) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();

    if (!pPlayer || !pUnit)
        return JS_TRUE;

    int32 x, y;

    if (pUnit == pPlayer) {

        if (argc < 2)
            return JS_TRUE;

        if (JS_ValueToInt32(cx, JS_ARGV(cx, vp)[0], &x) == JS_FALSE)
            return JS_FALSE;
        if (JS_ValueToInt32(cx, JS_ARGV(cx, vp)[1], &y) == JS_FALSE)
            return JS_FALSE;
    } else {
        x = D2CLIENT_GetUnitX(pUnit);
        y = D2CLIENT_GetUnitY(pUnit);
    }

    ClickMap(0, (WORD)x, (WORD)y, FALSE, NULL);
    Sleep(50);
    ClickMap(2, (WORD)x, (WORD)y, FALSE, NULL);
    //	D2CLIENT_Move((WORD)x, (WORD)y);
    return JS_TRUE;
}

JSAPI_FUNC(unit_getEnchant) {
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit || pUnit->dwType != UNIT_MONSTER)
        return JS_TRUE;

    int nEnchant = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(0));

    for (int i = 0; i < 9; i++)
        if (pUnit->pMonsterData->anEnchants[i] == nEnchant) {
            JS_SET_RVAL(cx, vp, JSVAL_TRUE);
            break;
        }

    return JS_TRUE;
}

JSAPI_FUNC(unit_getQuest) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 2 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]) || !JSVAL_IS_INT(JS_ARGV(cx, vp)[1]))
        return JS_TRUE;

    jsint nAct = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    jsint nQuest = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetQuestFlag(D2CLIENT_GetQuestInfo(), nAct, nQuest)));

    return JS_TRUE;
}

JSAPI_FUNC(unit_getMinionCount) {
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        return JS_TRUE;

    jsint nType = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    myUnit* pmyUnit = GetUnitPrivate(cx, JS_THIS_OBJECT(cx, vp));

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_TRUE;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit || (pUnit->dwType != UNIT_MONSTER && pUnit->dwType != UNIT_PLAYER))
        return JS_TRUE;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2CLIENT_GetMinionCount(pUnit, (DWORD)nType)));

    return JS_TRUE;
}

JSAPI_FUNC(me_getRepairCost) {
    if (!WaitForGameReady())
        THROW_WARNING(cx, vp, "Game not ready");

    UnitAny* npc = D2CLIENT_GetCurrentInteractingNPC();
    jsint nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A);

    if (argc > 0 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        nNpcClassId = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(D2COMMON_GetRepairCost(NULL, D2CLIENT_GetPlayerUnit(), nNpcClassId, D2CLIENT_GetDifficulty(), *p_D2CLIENT_ItemPriceList, 0)));

    return JS_TRUE;
}
