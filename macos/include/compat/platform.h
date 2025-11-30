/**
 * Red Alert macOS Port - Platform Definitions
 *
 * Core type definitions and macros for cross-platform compatibility.
 * This header provides Windows-compatible types for macOS.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>
#include <cstddef>

// Platform detection
#define PLATFORM_MACOS 1

// Calling conventions (no-op on macOS)
#define __cdecl
#define __stdcall
#define __fastcall
#define CALLBACK
#define WINAPI
#define APIENTRY

// Windows basic types
// Note: When compiling Objective-C++, BOOL is already defined by objc/objc.h as 'bool'
// We only define it for pure C++ compilation
#if !defined(__OBJC__) && !defined(BOOL)
typedef int32_t  BOOL;
#endif
typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint32_t ULONG;
typedef int16_t  SHORT;
typedef uint16_t USHORT;
typedef int32_t  INT;
typedef uint32_t UINT;
typedef int64_t  LONGLONG;
typedef uint64_t ULONGLONG;
typedef char     CHAR;
typedef wchar_t  WCHAR;
typedef float    FLOAT;

// Pointer types
typedef void       *PVOID;
typedef void       *LPVOID;
typedef const void *LPCVOID;
typedef BYTE    *LPBYTE;
typedef WORD    *LPWORD;
typedef DWORD   *LPDWORD;
typedef LONG    *LPLONG;
typedef CHAR    *LPSTR;
typedef const CHAR *LPCSTR;
typedef WCHAR   *LPWSTR;
typedef const WCHAR *LPCWSTR;

// Handle types (opaque pointers on macOS)
typedef void    *HANDLE;
typedef HANDLE   HWND;
typedef HANDLE   HDC;
typedef HANDLE   HINSTANCE;
typedef HANDLE   HMODULE;
typedef HANDLE   HBITMAP;
typedef HANDLE   HBRUSH;
typedef HANDLE   HFONT;
typedef HANDLE   HICON;
typedef HANDLE   HCURSOR;
typedef HANDLE   HMENU;
typedef HANDLE   HPALETTE;
typedef HANDLE   HPEN;
typedef HANDLE   HRGN;
typedef HANDLE   HGLOBAL;
typedef HANDLE   HLOCAL;

// Special values (NULL already defined by system headers)
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

// Boolean constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Size types
typedef size_t    SIZE_T;
typedef intptr_t  INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t  LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;

// Result types
typedef LONG HRESULT;
typedef LONG LRESULT;
typedef UINT WPARAM;
typedef LONG LPARAM;

// Success/failure macros
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)
#define S_OK          ((HRESULT)0)
#define S_FALSE       ((HRESULT)1)
#define E_FAIL        ((HRESULT)0x80004005L)
#define E_INVALIDARG  ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)

// Min/Max macros
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// Byte order macros (assuming little-endian ARM64)
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w) ((BYTE)(((DWORD_PTR)(w) >> 8) & 0xff))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))

// RGB macro (Windows format: 0x00BBGGRR)
#define RGB(r, g, b) ((DWORD)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | ((DWORD)(BYTE)(b)) << 16))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))

// Structure packing (Watcom used #pragma pack)
#define PACKED __attribute__((packed))

// Export/Import (not needed for static linking)
#define DLLEXPORT
#define DLLIMPORT

#endif // PLATFORM_H
