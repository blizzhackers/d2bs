#include "JSSocket.h"
#include "D2BS.h"
#include "Helpers.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

struct SocketData {
    int mode;
    SOCKET socket;
};
EMPTY_CTOR(socket)

JSAPI_PROP(socket_getProperty) {
    SocketData* sdata = (SocketData*)JS_GetInstancePrivate(cx, obj, &socket_class, NULL);

    if (sdata) {
        jsval ID;
        JS_IdToValue(cx, id, &ID);
        switch (JSVAL_TO_INT(ID)) {
            struct timeval timeout;
            int result;
        case SOCKET_READABLE:
            fd_set read_set;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100; // 100 ms
            FD_ZERO(&read_set);
            FD_SET(sdata->socket, &read_set);

            result = select(1, &read_set, NULL, NULL, &timeout);
            vp.setInt32(result);
            break;

        case SOCKET_WRITEABLE:
            fd_set write_set;

            timeout.tv_sec = 0;
            timeout.tv_usec = 100; // 100 ms
            FD_ZERO(&write_set);
            FD_SET(sdata->socket, &write_set);

            result = select(1, NULL, &write_set, NULL, &timeout);

            vp.setInt32(result);
            break;
        }
    }

    return JS_TRUE;
}

JSAPI_STRICT_PROP(socket_setProperty) {
    return JS_TRUE;
}

// Every exit after WSAStartup releases what was taken; the object owns one WSAStartup reference until close/finalize.
static void ReleaseSocketData(SocketData* sData) {
    if (sData->socket != INVALID_SOCKET) {
        closesocket(sData->socket);
        WSACleanup();
        sData->socket = INVALID_SOCKET;
    }
}

static SocketData* GetSocketThis(JSContext* cx, jsval* vp) {
    JSObject* self = JS_THIS_OBJECT(cx, vp);
    return self ? (SocketData*)JS_GetInstancePrivate(cx, self, &socket_class, NULL) : NULL;
}

// A host is allowed only when it equals one whole Hosts entry (case-insensitive); entries are separated by
// , ; space or tab. An empty host is refused outright.
static bool IsAllowedHost(const char* host) {
    if (host[0] == '\0')
        return false;
    char hosts[sizeof(Vars.szHosts)];
    strcpy_s(hosts, sizeof(hosts), Vars.szHosts);
    char* context = NULL;
    for (char* entry = strtok_s(hosts, ",; \t", &context); entry; entry = strtok_s(NULL, ",; \t", &context)) {
        if (_stricmp(entry, host) == 0)
            return true;
    }
    return false;
}

JSAPI_FUNC(socket_open) {
    JS_SET_RVAL(cx, vp, JSVAL_FALSE);
    if (argc < 2)
        return JS_TRUE;
    if (!JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid hostname specified");
    char* hostName = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!hostName)
        return JS_FALSE;
    // Host check before the port conversion, so a refused host reports the host error first.
    if (!IsAllowedHost(hostName)) {
        JS_free(cx, hostName);
        THROW_ERROR(cx, "Invalid hostname specified");
    }
    int32 port = 0;
    if (!JS_ValueToInt32(cx, JS_ARGV(cx, vp)[1], &port)) {
        JS_free(cx, hostName);
        THROW_ERROR(cx, "Could not convert parameter 2");
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        JS_free(cx, hostName);
        return JS_TRUE; // rval false
    }
    SocketData* Sdata = new SocketData;
    Sdata->mode = 0;
    Sdata->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Sdata->socket == INVALID_SOCKET) {
        WSACleanup();
        delete Sdata;
        JS_free(cx, hostName);
        THROW_ERROR(cx, "Failed to create the socket");
    }

    struct hostent* host = gethostbyname(hostName);
    JS_free(cx, hostName);
    const char* error = NULL;
    if (host == NULL)
        error = "Cannot find host";
    else {
        SOCKADDR_IN SockAddr;
        SockAddr.sin_port = htons((u_short)port);
        SockAddr.sin_family = AF_INET;
        SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
        if (connect(Sdata->socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
            error = "Failed to connect";
    }
    if (error) {
        ReleaseSocketData(Sdata);
        delete Sdata;
        THROW_ERROR(cx, error);
    }

    JSObject* res = BuildObject(cx, &socket_class, socket_methods, socket_props, Sdata);
    if (!res) {
        ReleaseSocketData(Sdata);
        delete Sdata;
        THROW_ERROR(cx, "Failed to define the socket object");
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
    return JS_TRUE;
}

JSAPI_FUNC(socket_close) {
    SocketData* sData = GetSocketThis(cx, vp);
    if (!sData)
        THROW_ERROR(cx, "Invalid socket object");
    // Idempotent: the finalizer runs ReleaseSocketData again, and each WSAStartup gets exactly one WSACleanup.
    ReleaseSocketData(sData);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JSAPI_FUNC(socket_send) {
    SocketData* sData = GetSocketThis(cx, vp);
    if (!sData)
        THROW_ERROR(cx, "Invalid socket object");
    if (sData->socket == INVALID_SOCKET)
        THROW_ERROR(cx, "Socket is closed");
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "send: the argument must be a string");
    char* msg = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!msg)
        return JS_FALSE;
    // send may accept fewer bytes than asked, so loop until the whole message is out.
    int total = (int)strlen(msg), sent = 0;
    while (sent < total) {
        int n = send(sData->socket, msg + sent, total - sent, 0);
        if (n == SOCKET_ERROR)
            break;
        sent += n;
    }
    JS_free(cx, msg);
    if (sent < total)
        THROW_ERROR(cx, "Socket Error");
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JSAPI_FUNC(socket_read) {
    SocketData* sData = GetSocketThis(cx, vp);
    if (!sData)
        THROW_ERROR(cx, "Invalid socket object");
    if (sData->socket == INVALID_SOCKET)
        THROW_ERROR(cx, "Socket is closed");
    char buffer[10000];
    // One recv per call: a second recv would block the script until more data arrives.
    int iResult = recv(sData->socket, buffer, sizeof(buffer), 0);
    if (iResult == SOCKET_ERROR)
        THROW_ERROR(cx, "Socket Error");
    // Converted by length so a full buffer has no terminator to find and an embedded NUL is kept.
    int wlen = iResult > 0 ? MultiByteToWideChar(CP_UTF8, 0, buffer, iResult, NULL, 0) : 0;
    wchar_t* wret = new wchar_t[wlen + 1];
    if (wlen > 0)
        MultiByteToWideChar(CP_UTF8, 0, buffer, iResult, wret, wlen);
    bool stored = StoreString(vp, JS_NewUCStringCopyN(cx, wret, wlen));
    delete[] wret;
    if (!stored)
        return JS_FALSE;
    return JS_TRUE;
}

void socket_finalize(JSFreeOp* fop, JSObject* obj) {
    SocketData* sData = (SocketData*)JS_GetPrivate(obj);
    if (sData) {
        ReleaseSocketData(sData);
        delete sData;
        JS_SetPrivate(obj, NULL);
    }
}
