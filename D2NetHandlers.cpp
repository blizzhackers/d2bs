#include "D2NetHandlers.h"
#include "Script.h"
#include "ScreenHook.h"
#include "Unit.h"
#include "Helpers.h"
#include "Core.h"
#include "Constants.h"
#include "Events.h"
#include "ScriptEngine.h"
#include "Console.h"
#include "D2BS.h"
#include "MPQStats.h"

using namespace std;

Variables Vars = {};

// Mirrors the game's 0x15 handler (0x45D160): it looks the unit up with FindClientSideUnit, moves it
// (0x4654C0), then halts (1336) if GetRoomFromUnit is NULL. The mover returns without moving a unit
// flagged 0x10000, or a player or monster in a death/dead mode, so such a unit with no room yet can
// only halt. A merc is often roomless after a waypoint until its 0x15 arrives (rig-measured, PR #20);
// one dying there hits this. Left to the game: the local player (the mover calls 0x466FE0 for it
// before the skip check) and a unit without a path (the mover halts on it before the skip check).
static bool ReassignWouldHalt(BYTE type, DWORD id, UnitAny* pPlayer) {
    UnitAny* pUnit = D2CLIENT_FindClientSideUnit(id, type);
    if (!pUnit || pUnit == pPlayer || !pUnit->pPath)
        return false;

    bool skipsMove = (pUnit->dwFlags & 0x10000) != 0;
    if (pUnit->dwType == UNIT_PLAYER)
        skipsMove |= pUnit->dwMode == PLAYER_MODE_DEATH || pUnit->dwMode == PLAYER_MODE_DEAD;
    else if (pUnit->dwType == UNIT_MONSTER)
        skipsMove |= pUnit->dwMode == NPC_MODE_DEATH || pUnit->dwMode == NPC_MODE_DEAD;

    return skipsMove && !D2COMMON_GetRoomFromUnit(pUnit);
}

DWORD ReassignPlayerHandler(BYTE* pPacket, DWORD dwSize) {
    BYTE type = pPacket[1];
    DWORD id = *(LPDWORD)&pPacket[2];

    UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();

    if (ReassignWouldHalt(type, id, pPlayer)) {
        // A dropped packet reads as BLOCK in the packet log, the same as a script listener's block.
        Log(L"Dropped S2C 0x15 for unit %u (type %u): the game would skip its move with no room and halt (1336)", id, (DWORD)type);
        return FALSE;
    }

    // Unit ids are unique per type only: the merc can share the player's id.
    if (type == UNIT_PLAYER && pPlayer && id == pPlayer->dwUnitId)
        pPacket[10] = NULL;

    return TRUE;
}

DWORD HPMPUpdateHandler(BYTE* pPacket, DWORD dwSize) {
    WORD Life = *(WORD*)&pPacket[1];
    WORD Mana = *(WORD*)&pPacket[3];

    if ((Life & 0x8000) == 0x8000) {
        Life ^= 0x8000;
    }
    if ((Mana & 0x8000) == 0x8000) {
        Mana ^= 0x8000;
    }
    if ((Mana & 0x4000) == 0x4000) {
        Mana ^= 0x4000;
    }
    Mana *= 2;

    static WORD SaveLife = 0;
    if (SaveLife != Life) {
        SaveLife = Life;
        LifeEvent(Life);
    }

    static WORD SaveMana = 0;
    if (SaveMana != Mana) {
        SaveMana = Mana;
        ManaEvent(Mana);
    }

    return TRUE;
}

DWORD ChatEventHandler(BYTE* pPacket, DWORD dwSize) {
    char* pName = (char*)pPacket + 10;
    char* pMessage = (char*)pPacket + strlen(pName) + 11;
    wchar_t* uc = AnsiToUnicode(pMessage, CP_ACP);
    //   char* enc = UnicodeToAnsi(uc); // convert d2 string to unicode to utf-8 for js compatibility

    if (Vars.bDontCatchNextMsg)
        Vars.bDontCatchNextMsg = FALSE;

    DWORD result = !(ChatEvent(pName, uc));
    //    delete[] enc;
    delete[] uc;

    return result;
}

DWORD NPCTransactionHandler(BYTE* pPacket, DWORD dwSize) {
    char code[5] = "";
    BYTE mode = pPacket[0x02]; // [BYTE Result - 0x00 =  Purchased || 0x01 = Sold || 0x0c = Insuffecient Gold]
    DWORD gid = *(DWORD*)(pPacket + 0x07);

    ItemActionEvent(gid, code, (100 + mode), false);

    return TRUE;
}

DWORD EventMessagesHandler(BYTE* pPacket, DWORD dwSize) {
    // packet breakdown: http://www.edgeofnowhere.cc/viewtopic.php?t=392307
    BYTE mode = pPacket[1];
    DWORD param1 = *(DWORD*)(pPacket + 3);
    BYTE param2 = pPacket[7];
    char name1[16] = "", name2[28] = "";
    strcpy_s(name1, 16, (char*)pPacket + 8);
    strcpy_s(name2, 16, (char*)pPacket + 24);
    wchar_t* wname2 = NULL;

    char* tables[3] = {"", "monstats", "objects"};
    char* columns[3] = {"", "NameStr", "Name"};

    switch (mode) {
    case 0x06: // name1 slain by name2
        /*BYTE Param2 = Unit Type of Slayer (0x00 = Player, 0x01 = NPC)
          if Type = NPC, DWORD Param1 = Monster Id Code from MPQ (points to string for %Name2)*/
        if (param2 == UNIT_MONSTER || param2 == UNIT_OBJECT) {
            WORD localeId;
            FillBaseStat(tables[param2], param1, columns[param2], &localeId, sizeof(WORD));
            wname2 = D2LANG_GetLocaleText(localeId);
        }
        break;
    case 0x07: // player relation
    {
        for (RosterUnit* player = *p_D2CLIENT_PlayerUnitList; player != NULL; player = player->pNext)
            if (player->dwUnitId == param1)
                strcpy_s(name1, 16, player->szName);
        switch (param2) {
        case 0x03: // hostile
            if (Vars.bQuitOnHostile)
                D2CLIENT_ExitGame();
            break;
        }
    } break;
    case 0x0a: // name1 has items in his box
        if (name1[0] == 0)
            strcpy_s(name1, 16, "You");
        break;
    }

    if (!wname2) {
        wname2 = AnsiToUnicode(name2, CP_ACP);
        GameActionEvent(mode, param1, param2, name1, wname2);
        delete[] wname2;
    } else
        GameActionEvent(mode, param1, param2, name1, wname2);

    return TRUE;
}

// The game's item handlers take an item out of its inventory before they move it and halt when that fails (Halt 3166
// for a belt item). The client's put-in-belt handler ignores a failed placement into an occupied slot and leaves the
// item in belt mode linked to no inventory, so the next packet that moves it halts. Placing such an item back into its
// slot, with the game's own placement, before the packet runs gives the handler the item where it expects it.
void RelinkOrphanedBeltItem(BYTE* pPacket, DWORD dwSize) {
    if (dwSize < 8)
        return;
    UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();
    if (!pPlayer || !pPlayer->pInventory)
        return;
    UnitAny* pItem = D2CLIENT_FindClientSideUnit(*(DWORD*)(pPacket + 4), UNIT_ITEM);
    if (!pItem || pItem->dwMode != ITEM_MODE_IN_BELT || !pItem->pItemData || pItem->pItemData->pOwnerInventory)
        return;
    int slot = D2CLIENT_GetUnitX(pItem);
    if (slot >= 0 && slot < 16 && D2COMMON_PlaceItemInBeltSlot(pPlayer->pInventory, pItem, slot))
        Log(L"Belt: put item 0x%X back in belt slot %d before packet 0x%02X 0x%02X", pItem->dwUnitId, slot, pPacket[0], pPacket[1]);
    else
        Log(L"Belt: item 0x%X has no inventory and belt slot %d is taken; packet 0x%02X 0x%02X may halt the game", pItem->dwUnitId, slot,
            pPacket[0], pPacket[1]);
}

DWORD ItemActionHandler(BYTE* pPacket, DWORD dwSize) {
    // TODO: fix this code later by changing the way it's parsed
    INT64 icode = 0;
    char code[5] = "";
    BYTE mode = pPacket[1];
    DWORD gid = *(DWORD*)&pPacket[4];
    BYTE dest = ((pPacket[13] & 0x1C) >> 2);

    switch (dest) {
    case 0:
    case 2:
        icode = *(INT64*)(pPacket + 15) >> 0x04;
        break;
    case 3:
    case 4:
    case 6:
        if (!((mode == 0 || mode == 2) && dest == 3)) {
            if (mode != 0xF && mode != 1 && mode != 12)
                icode = *(INT64*)(pPacket + 17) >> 0x1C;
            else
                icode = *(INT64*)(pPacket + 15) >> 0x04;
        } else
            icode = *(INT64*)(pPacket + 17) >> 0x05;
        break;
    default:
        break;
    }

    // Converting and Cleaning
    memcpy(code, &icode, 4);
    if (code[3] == ' ')
        code[3] = '\0';

    /*if(strcmp(code, "gld") == 0)
            GoldDropEvent(gid, mode);
    else*/
    ItemActionEvent(gid, code, mode, (pPacket[0] == 0x9d));

    return TRUE;
}

DWORD DelayedStateHandler(BYTE* pPacket, DWORD dwSize) {
    if (pPacket[6] == AFFECT_JUST_PORTALED)
        return FALSE;

    return TRUE;
}
