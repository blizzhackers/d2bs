#include "D2Helpers.h"
#include "dde.h"

// DDEML requires a callback even for a client-only instance. SendDDE makes only synchronous transactions and starts no
// advise loop, so every callback it can get is a notification that needs no answer.
static HDDEDATA CALLBACK DdeClientCallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, ULONG_PTR, ULONG_PTR) {
    return NULL;
}

BOOL SendDDE(int mode, char* pszDDEServer, char* pszTopic, char* pszItem, char* pszData, char* result, unsigned int size) {
    DWORD pidInst = 0;
    HCONV hConv;
    DWORD dwTimeout = 5000;

    Log(L"SendDDE(%d, %hs, %hs, %hs, %hs, %u)", mode, pszDDEServer, pszTopic, pszItem, pszData, size);

    int ret = DdeInitialize(&pidInst, DdeClientCallback, APPCMD_CLIENTONLY, 0);
    if (ret != DMLERR_NO_ERROR) {
        Log(L"DdeInitialize Error: %X", ret);
        return FALSE;
    }

    HSZ hszDDEServer = DdeCreateStringHandle(pidInst, (strlen(pszDDEServer) == 0 ? "\"\"" : pszDDEServer), CP_WINANSI);
    HSZ hszTopic = DdeCreateStringHandle(pidInst, (strlen(pszTopic) == 0 ? "\"\"" : pszTopic), CP_WINANSI);
    HSZ hszCommand = DdeCreateStringHandle(pidInst, (strlen(pszItem) == 0 ? "\"\"" : pszItem), CP_WINANSI);

    if (!(hszDDEServer && hszTopic && hszCommand)) {
        UINT err = DdeGetLastError(pidInst);
        Log(L"Error creating DDE Handles: %d", err);
        if (hszDDEServer)
            DdeFreeStringHandle(pidInst, hszDDEServer);
        if (hszTopic)
            DdeFreeStringHandle(pidInst, hszTopic);
        if (hszCommand)
            DdeFreeStringHandle(pidInst, hszCommand);
        DdeUninitialize(pidInst);
        return FALSE;
    }

    hConv = DdeConnect(pidInst, hszDDEServer, hszTopic, 0);

    // A server that is not running is an ordinary result ("" or false), not a setup failure, so TRUE stays.
    if (!hConv) {
        Log(L"DdeConnect Error: %X", DdeGetLastError(pidInst));
    } else {
        switch (mode) {
        case 0: {
            HDDEDATA DdeSrvData = DdeClientTransaction(0, 0, hConv, hszCommand, CF_TEXT, XTYP_REQUEST, dwTimeout, 0);
            if (DdeSrvData) {
                DdeGetData(DdeSrvData, (LPBYTE)result, size, 0);
                DdeFreeDataHandle(DdeSrvData);
            }
            break;
        }
        case 1:
            DdeClientTransaction((LPBYTE)pszData, strlen(pszData) + 1, hConv, hszCommand, CF_TEXT, XTYP_POKE, dwTimeout, 0);
            break;
        case 2:
            DdeClientTransaction((LPBYTE)pszData, strlen(pszData) + 1, hConv, 0L, 0, XTYP_EXECUTE, dwTimeout, 0);
            break;
        }
    }

    DdeFreeStringHandle(pidInst, hszDDEServer);
    DdeFreeStringHandle(pidInst, hszTopic);
    DdeFreeStringHandle(pidInst, hszCommand);
    DdeUninitialize(pidInst);

    return TRUE;
}
