#ifndef PARTY_H
#define PARTY_H

#include "js32.h"
#include <windows.h>

struct RosterUnit;

// A Party object holds the player's gid and the game session it was made in, never a roster pointer: the game frees a
// roster entry the moment its player leaves, and frees them all when the game ends.
struct myParty {
    DWORD dwPrivateType; // PRIVATE_PARTY
    DWORD dwUnitId;
    DWORD dwSession;
};

CLASS_CTOR(party);
void party_finalize(JSFreeOp* fop, JSObject* obj);
JSAPI_PROP(party_getProperty);

JSAPI_FUNC(party_getNext);

JSAPI_FUNC(my_getParty);

// The current roster entry of a Party object's player, or NULL once that player or game is gone. Call with the game lock
// held, and drop the pointer before releasing it.
RosterUnit* FindPartyUnit(const myParty* pParty);

enum party_tinyid { PARTY_AREA, PARTY_X, PARTY_Y, PARTY_GID, PARTY_LIFE, PARTY_NAME, PARTY_FLAG, PARTY_ID, PARTY_CLASSID, PARTY_LEVEL };

static JSPropertySpec party_props[] = {{"x", PARTY_X, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"y", PARTY_Y, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"area", PARTY_AREA, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"gid", PARTY_GID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"life", PARTY_LIFE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"partyflag", PARTY_FLAG, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"partyid", PARTY_ID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"name", PARTY_NAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"classid", PARTY_CLASSID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {"level", PARTY_LEVEL, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(party_getProperty), JSOP_NULLWRAPPER},
                                       {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}};

static JSFunctionSpec party_methods[] = {JS_FS("getNext", party_getNext, 0, FUNCTION_FLAGS), JS_FS_END};

#endif