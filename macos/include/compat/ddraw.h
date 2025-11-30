/**
 * Red Alert macOS Port - DirectDraw Compatibility Stub
 *
 * Stub types for DirectDraw. Actual rendering uses Metal.
 * These types allow original code to compile.
 */

#ifndef COMPAT_DDRAW_H
#define COMPAT_DDRAW_H

#include "platform.h"

// DirectDraw return codes
#define DD_OK                                   0
#define DDERR_GENERIC                           0x80004005
#define DDERR_INVALIDPARAMS                     0x80070057
#define DDERR_OUTOFMEMORY                       0x8007000E
#define DDERR_UNSUPPORTED                       0x80004001
#define DDERR_SURFACELOST                       0x887601C2
#define DDERR_WASSTILLDRAWING                   0x8876021C
#define DDERR_SURFACEBUSY                       0x887601E0
#define DDERR_NOTFLIPPABLE                      0x88760228

// Cooperative level flags
#define DDSCL_FULLSCREEN                        0x00000001
#define DDSCL_ALLOWREBOOT                       0x00000002
#define DDSCL_NOWINDOWCHANGES                   0x00000004
#define DDSCL_NORMAL                            0x00000008
#define DDSCL_EXCLUSIVE                         0x00000010
#define DDSCL_ALLOWMODEX                        0x00000040

// Surface capabilities
#define DDSCAPS_3DDEVICE                        0x00000001
#define DDSCAPS_BACKBUFFER                      0x00000004
#define DDSCAPS_COMPLEX                         0x00000008
#define DDSCAPS_FLIP                            0x00000010
#define DDSCAPS_FRONTBUFFER                     0x00000020
#define DDSCAPS_OFFSCREENPLAIN                  0x00000040
#define DDSCAPS_PALETTE                         0x00000100
#define DDSCAPS_PRIMARYSURFACE                  0x00000200
#define DDSCAPS_SYSTEMMEMORY                    0x00000800
#define DDSCAPS_VIDEOMEMORY                     0x00004000
#define DDSCAPS_LOCALVIDMEM                     0x10000000
#define DDSCAPS_NONLOCALVIDMEM                  0x20000000

// Surface description flags
#define DDSD_CAPS                               0x00000001
#define DDSD_HEIGHT                             0x00000002
#define DDSD_WIDTH                              0x00000004
#define DDSD_PITCH                              0x00000008
#define DDSD_PIXELFORMAT                        0x00001000
#define DDSD_BACKBUFFERCOUNT                    0x00000020
#define DDSD_LPSURFACE                          0x00000800

// Color key flags
#define DDCKEY_COLORSPACE                       0x00000001
#define DDCKEY_DESTBLT                          0x00000002
#define DDCKEY_DESTOVERLAY                      0x00000004
#define DDCKEY_SRCBLT                           0x00000008
#define DDCKEY_SRCOVERLAY                       0x00000010

// Blt flags
#define DDBLT_COLORFILL                         0x00000400
#define DDBLT_WAIT                              0x01000000
#define DDBLT_KEYSRC                            0x00008000
#define DDBLT_KEYDEST                           0x00002000

// Lock flags
#define DDLOCK_WAIT                             0x00000001
#define DDLOCK_READONLY                         0x00000010
#define DDLOCK_WRITEONLY                        0x00000020
#define DDLOCK_SURFACEMEMORYPTR                 0x00000000

// Flip flags
#define DDFLIP_WAIT                             0x00000001

// Palette capabilities
#define DDPCAPS_8BIT                            0x00000004
#define DDPCAPS_ALLOW256                        0x00000040

// Pixel format flags
#define DDPF_PALETTEINDEXED8                    0x00000020
#define DDPF_RGB                                0x00000040

// Forward declarations
struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDirectDraw *LPDIRECTDRAW;
typedef struct IDirectDrawSurface *LPDIRECTDRAWSURFACE;
typedef struct IDirectDrawPalette *LPDIRECTDRAWPALETTE;
typedef struct IDirectDrawClipper *LPDIRECTDRAWCLIPPER;

// DDSCAPS structure
typedef struct _DDSCAPS {
    DWORD dwCaps;
} DDSCAPS, *LPDDSCAPS;

// DDCOLORKEY structure
typedef struct _DDCOLORKEY {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY, *LPDDCOLORKEY;

// DDPIXELFORMAT structure
typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
    };
    union {
        DWORD dwRBitMask;
        DWORD dwYBitMask;
    };
    union {
        DWORD dwGBitMask;
        DWORD dwUBitMask;
    };
    union {
        DWORD dwBBitMask;
        DWORD dwVBitMask;
    };
    union {
        DWORD dwRGBAlphaBitMask;
        DWORD dwYUVAlphaBitMask;
    };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

// DDSURFACEDESC structure
typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    union {
        DWORD dwMipMapCount;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    LPVOID lpSurface;
    DDCOLORKEY ddckCKDestOverlay;
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS ddsCaps;
} DDSURFACEDESC, *LPDDSURFACEDESC;

// PALETTEENTRY structure (from Windows)
typedef struct tagPALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY, *LPPALETTEENTRY;

// DDBLTFX structure
typedef struct _DDBLTFX {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwDDROP;
    DWORD dwRotationAngle;
    DWORD dwZBufferOpCode;
    DWORD dwZBufferLow;
    DWORD dwZBufferHigh;
    DWORD dwZBufferBaseDest;
    DWORD dwZDestConstBitDepth;
    union {
        DWORD dwZDestConst;
        LPDIRECTDRAWSURFACE lpDDSZBufferDest;
    };
    DWORD dwZSrcConstBitDepth;
    union {
        DWORD dwZSrcConst;
        LPDIRECTDRAWSURFACE lpDDSZBufferSrc;
    };
    DWORD dwAlphaEdgeBlendBitDepth;
    DWORD dwAlphaEdgeBlend;
    DWORD dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    union {
        DWORD dwAlphaDestConst;
        LPDIRECTDRAWSURFACE lpDDSAlphaDest;
    };
    DWORD dwAlphaSrcConstBitDepth;
    union {
        DWORD dwAlphaSrcConst;
        LPDIRECTDRAWSURFACE lpDDSAlphaSrc;
    };
    union {
        DWORD dwFillColor;
        DWORD dwFillDepth;
        DWORD dwFillPixel;
        LPDIRECTDRAWSURFACE lpDDSPattern;
    };
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX, *LPDDBLTFX;

// IDirectDraw interface (stub - methods return failure)
struct IDirectDraw {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT Compact() = 0;
    virtual HRESULT CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, void* pUnkOuter) = 0;
    virtual HRESULT CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpColorTable, LPDIRECTDRAWPALETTE* lplpDDPalette, void* pUnkOuter) = 0;
    virtual HRESULT CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, void* pUnkOuter) = 0;
    virtual HRESULT DuplicateSurface(LPDIRECTDRAWSURFACE lpDDSurface, LPDIRECTDRAWSURFACE* lplpDupDDSurface) = 0;
    virtual HRESULT EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, void* lpEnumCallback) = 0;
    virtual HRESULT EnumSurfaces(DWORD dwFlags, LPDDSURFACEDESC lpDDSD, LPVOID lpContext, void* lpEnumCallback) = 0;
    virtual HRESULT FlipToGDISurface() = 0;
    virtual HRESULT GetCaps(void* lpDDDriverCaps, void* lpDDHELCaps) = 0;
    virtual HRESULT GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc) = 0;
    virtual HRESULT GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes) = 0;
    virtual HRESULT GetGDISurface(LPDIRECTDRAWSURFACE* lplpGDIDDSurface) = 0;
    virtual HRESULT GetMonitorFrequency(LPDWORD lpdwFrequency) = 0;
    virtual HRESULT GetScanLine(LPDWORD lpdwScanLine) = 0;
    virtual HRESULT GetVerticalBlankStatus(BOOL* lpbIsInVB) = 0;
    virtual HRESULT Initialize(void* lpGUID) = 0;
    virtual HRESULT RestoreDisplayMode() = 0;
    virtual HRESULT SetCooperativeLevel(HWND hWnd, DWORD dwFlags) = 0;
    virtual HRESULT SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP) = 0;
    virtual HRESULT WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent) = 0;
};

// IDirectDrawSurface interface (stub)
struct IDirectDrawSurface {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT AddAttachedSurface(LPDIRECTDRAWSURFACE lpDDSAttachedSurface) = 0;
    virtual HRESULT AddOverlayDirtyRect(LPRECT lpRect) = 0;
    virtual HRESULT Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) = 0;
    virtual HRESULT BltBatch(void* lpDDBltBatch, DWORD dwCount, DWORD dwFlags) = 0;
    virtual HRESULT BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) = 0;
    virtual HRESULT DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSAttachedSurface) = 0;
    virtual HRESULT EnumAttachedSurfaces(LPVOID lpContext, void* lpEnumSurfacesCallback) = 0;
    virtual HRESULT EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, void* lpfnCallback) = 0;
    virtual HRESULT Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) = 0;
    virtual HRESULT GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface) = 0;
    virtual HRESULT GetBltStatus(DWORD dwFlags) = 0;
    virtual HRESULT GetCaps(LPDDSCAPS lpDDSCaps) = 0;
    virtual HRESULT GetClipper(LPDIRECTDRAWCLIPPER* lplpDDClipper) = 0;
    virtual HRESULT GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) = 0;
    virtual HRESULT GetDC(HDC* lphDC) = 0;
    virtual HRESULT GetFlipStatus(DWORD dwFlags) = 0;
    virtual HRESULT GetOverlayPosition(LPLONG lplX, LPLONG lplY) = 0;
    virtual HRESULT GetPalette(LPDIRECTDRAWPALETTE* lplpDDPalette) = 0;
    virtual HRESULT GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat) = 0;
    virtual HRESULT GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc) = 0;
    virtual HRESULT Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc) = 0;
    virtual HRESULT IsLost() = 0;
    virtual HRESULT Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) = 0;
    virtual HRESULT ReleaseDC(HDC hDC) = 0;
    virtual HRESULT Restore() = 0;
    virtual HRESULT SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper) = 0;
    virtual HRESULT SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) = 0;
    virtual HRESULT SetOverlayPosition(LONG lX, LONG lY) = 0;
    virtual HRESULT SetPalette(LPDIRECTDRAWPALETTE lpDDPalette) = 0;
    virtual HRESULT Unlock(LPVOID lpSurfaceData) = 0;
    virtual HRESULT UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, void* lpDDOverlayFx) = 0;
    virtual HRESULT UpdateOverlayDisplay(DWORD dwFlags) = 0;
    virtual HRESULT UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSReference) = 0;
};

// IDirectDrawPalette interface (stub)
struct IDirectDrawPalette {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetCaps(LPDWORD lpdwCaps) = 0;
    virtual HRESULT GetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) = 0;
    virtual HRESULT Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags, LPPALETTEENTRY lpColorTable) = 0;
    virtual HRESULT SetEntries(DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries) = 0;
};

// IDirectDrawClipper interface (stub)
struct IDirectDrawClipper {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetClipList(LPRECT lpRect, void* lpClipList, LPDWORD lpdwSize) = 0;
    virtual HRESULT GetHWnd(HWND* lphWnd) = 0;
    virtual HRESULT Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags) = 0;
    virtual HRESULT IsClipListChanged(BOOL* lpbChanged) = 0;
    virtual HRESULT SetClipList(void* lpClipList, DWORD dwFlags) = 0;
    virtual HRESULT SetHWnd(DWORD dwFlags, HWND hWnd) = 0;
};

// DirectDrawCreate function stub
#ifdef __cplusplus
extern "C" {
#endif

HRESULT DirectDrawCreate(void* lpGUID, LPDIRECTDRAW* lplpDD, void* pUnkOuter);

#ifdef __cplusplus
}
#endif

// Helper macro
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#endif // COMPAT_DDRAW_H
