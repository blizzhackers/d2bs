#pragma once

#include <windows.h>

bool ChatEvent(char* lpszNick, wchar_t* lpszMsg);
bool ChatInputEvent(wchar_t* lpszMsg);
void LifeEvent(DWORD dwLife);
void ManaEvent(DWORD dwMana);
void CopyDataEvent(DWORD dwMode, wchar_t* lpszMsg);
void GameMsgEvent(char* lpszMsg);
void GameActionEvent(BYTE mode, DWORD param1, DWORD param2, char* name1, wchar_t* name2);
bool WhisperEvent(char* lpszNick, wchar_t* lpszMsg);
bool KeyDownUpEvent(WPARAM bByte, BYTE bUp);
void PlayerAssignEvent(DWORD dwUnitId);
void MouseClickEvent(int button, POINT pt, bool bUp);
void MouseMoveEvent(POINT pt);
bool ScriptBroadcastEvent(JSContext* cx, uint argc, jsval* argv);
void GoldDropEvent(DWORD GID, BYTE Mode);
void ItemActionEvent(DWORD GID, char* Code, BYTE Mode, bool Global);
bool GamePacketEvent(BYTE* pPacket, DWORD dwSize);
bool GamePacketSentEvent(BYTE* pPacket, DWORD dwSize);
bool RealmPacketEvent(BYTE* pPacket, DWORD dwSize);
void ReleaseGameLock(void);

void TakeGameLock(void);

struct ChatEventHelper {
    const char* name;        // literal event name
    const char* blockerName; // literal name + "blocker"
    char* nick;
    wchar_t* msg;
};

struct CopyDataHelper {
    DWORD mode;
    wchar_t* msg;
};

struct ItemEventHelper {
    DWORD id;
    char* code;
    WORD mode;
    bool global;
};

struct KeyEventHelper {
    BOOL up;
    WPARAM key;
};

struct GameActionEventHelper {
    BYTE mode;
    DWORD param1, param2;
    char* name1;
    wchar_t* name2;
};

struct SingleArgHelper {
    DWORD arg1;
};

struct DoubleArgHelper {
    DWORD arg1, arg2;
};

struct TripleArgHelper {
    DWORD arg1, arg2, arg3;
};
struct QuadArgHelper {
    DWORD arg1, arg2, arg3, arg4;
};

struct BCastEventHelper {
    JSContext* cx;
    jsval* argv;
    uint argc;
    // Written at the first script that listens and copied for each: getters run once, and none run with no listener.
    JSAutoStructuredCloneBuffer** buffers;
    bool written;
    bool failed; // the sender's error is set on cx; no further script gets the message
};
struct PacketEventHelper {
    const char* name;
    BYTE* pPacket;
    DWORD dwSize;
};
