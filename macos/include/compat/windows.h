/**
 * Red Alert macOS Port - Windows API Compatibility Stub
 *
 * Provides Windows types and function stubs for compilation.
 * Actual functionality must be implemented in platform layer.
 */

#ifndef COMPAT_WINDOWS_H
#define COMPAT_WINDOWS_H

#include "platform.h"

// Standard includes
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

// Path limits
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

// Windows message types
#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005
#define WM_ACTIVATE                     0x0006
#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SHOWWINDOW                   0x0018
#define WM_ACTIVATEAPP                  0x001C
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_GETMINMAXINFO                0x0024
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_WINDOWPOSCHANGED             0x0047
#define WM_NOTIFY                       0x004E
#define WM_HELP                         0x0053
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_USER                         0x0400

// Virtual key codes
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04
#define VK_BACK           0x08
#define VK_TAB            0x09
#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14
#define VK_ESCAPE         0x1B
#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

// Mouse button state masks
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010

// File access flags
#define GENERIC_READ                     0x80000000L
#define GENERIC_WRITE                    0x40000000L
#define GENERIC_EXECUTE                  0x20000000L
#define GENERIC_ALL                      0x10000000L

#define FILE_SHARE_READ                  0x00000001
#define FILE_SHARE_WRITE                 0x00000002
#define FILE_SHARE_DELETE                0x00000004

#define CREATE_NEW                       1
#define CREATE_ALWAYS                    2
#define OPEN_EXISTING                    3
#define OPEN_ALWAYS                      4
#define TRUNCATE_EXISTING                5

#define FILE_ATTRIBUTE_READONLY          0x00000001
#define FILE_ATTRIBUTE_HIDDEN            0x00000002
#define FILE_ATTRIBUTE_SYSTEM            0x00000004
#define FILE_ATTRIBUTE_DIRECTORY         0x00000010
#define FILE_ATTRIBUTE_ARCHIVE           0x00000020
#define FILE_ATTRIBUTE_NORMAL            0x00000080

#define FILE_BEGIN                       0
#define FILE_CURRENT                     1
#define FILE_END                         2

// Memory allocation flags
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_ZEROINIT       0x0040
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)
#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)

// Registry (stub)
#define HKEY_CLASSES_ROOT           ((HANDLE)(ULONG_PTR)0x80000000)
#define HKEY_CURRENT_USER           ((HANDLE)(ULONG_PTR)0x80000001)
#define HKEY_LOCAL_MACHINE          ((HANDLE)(ULONG_PTR)0x80000002)

typedef HANDLE HKEY;

// Error codes
#define ERROR_SUCCESS                    0L
#define ERROR_FILE_NOT_FOUND             2L
#define ERROR_PATH_NOT_FOUND             3L
#define ERROR_ACCESS_DENIED              5L
#define ERROR_INVALID_HANDLE             6L
#define ERROR_NOT_ENOUGH_MEMORY          8L
#define ERROR_OUTOFMEMORY                14L
#define ERROR_INVALID_PARAMETER          87L

// RECT structure
typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *LPRECT;

// POINT structure
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *LPPOINT;

// SIZE structure
typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE, *LPSIZE;

// MSG structure
typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG, *LPMSG;

// CRITICAL_SECTION stub
typedef struct _CRITICAL_SECTION {
    void *DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
} CRITICAL_SECTION, *LPCRITICAL_SECTION;

// OVERLAPPED structure (for async I/O)
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

// File time
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;

// Find file data
typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

// Function stubs - these must be implemented in the platform layer
#ifdef __cplusplus
extern "C" {
#endif

// File I/O (stubs - implement in platform layer)
HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   void* lpSecurityAttributes, DWORD dwCreationDisposition,
                   DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#define CreateFile CreateFileA

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
              LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
               LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL CloseHandle(HANDLE hObject);
DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove,
                     LPLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
BOOL DeleteFileA(LPCSTR lpFileName);
#define DeleteFile DeleteFileA

// Memory
HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
HGLOBAL GlobalFree(HGLOBAL hMem);
LPVOID GlobalLock(HGLOBAL hMem);
BOOL GlobalUnlock(HGLOBAL hMem);

// Timing
DWORD GetTickCount(void);
DWORD timeGetTime(void);
void Sleep(DWORD dwMilliseconds);
BOOL QueryPerformanceCounter(LONGLONG* lpPerformanceCount);
BOOL QueryPerformanceFrequency(LONGLONG* lpFrequency);

// Error handling
DWORD GetLastError(void);
void SetLastError(DWORD dwErrCode);

// String functions
int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...);
#define wsprintf wsprintfA

// Critical section (stub implementations)
void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

// Message box (stub - can be implemented with NSAlert)
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_YESNO                    0x00000004L
#define MB_ICONERROR                0x00000010L
#define MB_ICONWARNING              0x00000030L
#define MB_ICONINFORMATION          0x00000040L

#define IDOK                1
#define IDCANCEL            2
#define IDYES               6
#define IDNO                7

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
#define MessageBox MessageBoxA

// Get current directory
DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer);
#define GetCurrentDirectory GetCurrentDirectoryA

BOOL SetCurrentDirectoryA(LPCSTR lpPathName);
#define SetCurrentDirectory SetCurrentDirectoryA

// Find file
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
#define FindFirstFile FindFirstFileA
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
#define FindNextFile FindNextFileA
BOOL FindClose(HANDLE hFindFile);

// Module/Instance
HMODULE GetModuleHandleA(LPCSTR lpModuleName);
#define GetModuleHandle GetModuleHandleA

// Output debug string (goes to stderr on macOS)
void OutputDebugStringA(LPCSTR lpOutputString);
#define OutputDebugString OutputDebugStringA

#ifdef __cplusplus
}
#endif

// Additional macros for Windows compatibility
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#define CopyMemory(Destination, Source, Length) memcpy((Destination), (Source), (Length))
#define MoveMemory(Destination, Source, Length) memmove((Destination), (Source), (Length))
#define FillMemory(Destination, Length, Fill) memset((Destination), (Fill), (Length))

// Case-insensitive string compare
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define _strnicmp strncasecmp

// Deprecated function mappings
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek

#endif // COMPAT_WINDOWS_H
