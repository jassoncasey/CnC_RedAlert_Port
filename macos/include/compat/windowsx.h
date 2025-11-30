/**
 * Red Alert macOS Port - Windows Extensions Compatibility Stub
 *
 * Stub for windowsx.h macros.
 */

#ifndef COMPAT_WINDOWSX_H
#define COMPAT_WINDOWSX_H

#include "windows.h"

// Message crackers
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

#define GET_WM_COMMAND_ID(wp, lp)       LOWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)     ((HWND)(lp))
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(wp)

// Memory macros
#define GlobalAllocPtr(flags, cb)       GlobalLock(GlobalAlloc((flags), (cb)))
#define GlobalFreePtr(lp)               GlobalFree(GlobalHandle(lp))

// Edit control macros
#define Edit_GetText(hwndCtl, lpch, cchMax)     GetWindowText((hwndCtl), (lpch), (cchMax))
#define Edit_SetText(hwndCtl, lpsz)             SetWindowText((hwndCtl), (lpsz))
#define Edit_LimitText(hwndCtl, cchMax)         ((void)SendMessage((hwndCtl), EM_LIMITTEXT, (WPARAM)(cchMax), 0))

// Button control macros
#define Button_GetCheck(hwndCtl)                ((int)(DWORD)SendMessage((hwndCtl), BM_GETCHECK, 0, 0))
#define Button_SetCheck(hwndCtl, check)         ((void)SendMessage((hwndCtl), BM_SETCHECK, (WPARAM)(int)(check), 0))

// Window function stubs
#ifdef __cplusplus
extern "C" {
#endif

BOOL GetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount);
BOOL SetWindowText(HWND hWnd, LPCSTR lpString);
LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HANDLE GlobalHandle(LPCVOID pMem);

#ifdef __cplusplus
}
#endif

// Message constants
#define EM_LIMITTEXT        0x00C5
#define BM_GETCHECK         0x00F0
#define BM_SETCHECK         0x00F1

#endif // COMPAT_WINDOWSX_H
