/**
 * Red Alert macOS Port - Winsock Compatibility Stub
 *
 * Networking is DEFERRED. This header provides only type definitions
 * needed for original code to compile. All functions are no-ops.
 *
 * Uses macOS native socket types where possible to avoid conflicts.
 */

#ifndef COMPAT_WINSOCK_H
#define COMPAT_WINSOCK_H

#include "platform.h"

// Include macOS native socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// Socket type - use int on macOS (same as native)
typedef int SOCKET;
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)

// Windows uses closesocket, macOS uses close
#define closesocket close

// Address families (use native values, just provide Windows names)
#ifndef AF_IPX
#define AF_IPX          6
#endif

// IPX is not supported on macOS - stub it
#define NSPROTO_IPX     1000

// WSA Error codes (map to errno where possible)
#define WSABASEERR              10000
#define WSAEINTR                (WSABASEERR+4)
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSANOTINITIALISED       (WSABASEERR+93)

// WSADATA structure
typedef struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    WORD iMaxSockets;
    WORD iMaxUdpDg;
    char* lpVendorInfo;
} WSADATA, *LPWSADATA;

// Windows-specific typedefs for native types
typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval *LPTIMEVAL;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr *LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in *LPSOCKADDR_IN;
typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr *LPIN_ADDR;
typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent *LPHOSTENT;

// ioctlsocket - use ioctl on macOS
#include <sys/ioctl.h>
#define ioctlsocket ioctl
// FIONREAD and FIONBIO are already defined in sys/filio.h

// Async socket events (Windows message-based - not supported on macOS)
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

// WSA Functions - stubs that return error (networking is deferred)
#ifdef __cplusplus
extern "C" {
#endif

// These are the only truly Windows-specific functions we need to stub
inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
    (void)wVersionRequested;
    if (lpWSAData) {
        lpWSAData->wVersion = 0x0101;
        lpWSAData->wHighVersion = 0x0101;
        lpWSAData->szDescription[0] = '\0';
        lpWSAData->szSystemStatus[0] = '\0';
        lpWSAData->iMaxSockets = 0;
        lpWSAData->iMaxUdpDg = 0;
        lpWSAData->lpVendorInfo = nullptr;
    }
    return WSANOTINITIALISED;  // Always fail - networking deferred
}

inline int WSACleanup(void) {
    return 0;
}

inline int WSAGetLastError(void) {
    return WSANOTINITIALISED;
}

inline void WSASetLastError(int iError) {
    (void)iError;
}

// Async functions - not supported (always return error)
inline HANDLE WSAAsyncGetHostByName(HWND hWnd, UINT wMsg, const char* name, char* buf, int buflen) {
    (void)hWnd; (void)wMsg; (void)name; (void)buf; (void)buflen;
    return NULL;
}

inline HANDLE WSAAsyncGetHostByAddr(HWND hWnd, UINT wMsg, const char* addr, int len, int type, char* buf, int buflen) {
    (void)hWnd; (void)wMsg; (void)addr; (void)len; (void)type; (void)buf; (void)buflen;
    return NULL;
}

inline int WSACancelAsyncRequest(HANDLE hAsyncTaskHandle) {
    (void)hAsyncTaskHandle;
    return SOCKET_ERROR;
}

inline int WSAAsyncSelect(SOCKET s, HWND hWnd, UINT wMsg, long lEvent) {
    (void)s; (void)hWnd; (void)wMsg; (void)lEvent;
    return SOCKET_ERROR;
}

#ifdef __cplusplus
}
#endif

#endif // COMPAT_WINSOCK_H
