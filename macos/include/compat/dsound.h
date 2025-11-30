/**
 * Red Alert macOS Port - DirectSound Compatibility Stub
 *
 * Stub types for DirectSound. Actual audio uses CoreAudio.
 * These types allow original code to compile.
 */

#ifndef COMPAT_DSOUND_H
#define COMPAT_DSOUND_H

#include "platform.h"

// DirectSound return codes
#define DS_OK                           0
#define DSERR_ALLOCATED                 0x8878000A
#define DSERR_CONTROLUNAVAIL            0x8878001E
#define DSERR_INVALIDPARAM              0x80070057
#define DSERR_INVALIDCALL               0x88780032
#define DSERR_GENERIC                   0x80004005
#define DSERR_PRIOLEVELNEEDED           0x88780046
#define DSERR_OUTOFMEMORY               0x8007000E
#define DSERR_BADFORMAT                 0x88780064
#define DSERR_UNSUPPORTED               0x80004001
#define DSERR_NODRIVER                  0x88780078
#define DSERR_ALREADYINITIALIZED        0x88780082
#define DSERR_BUFFERLOST                0x88780096

// Cooperative level flags
#define DSSCL_NORMAL                    0x00000001
#define DSSCL_PRIORITY                  0x00000002
#define DSSCL_EXCLUSIVE                 0x00000003
#define DSSCL_WRITEPRIMARY              0x00000004

// Buffer capabilities
#define DSBCAPS_PRIMARYBUFFER           0x00000001
#define DSBCAPS_STATIC                  0x00000002
#define DSBCAPS_LOCHARDWARE             0x00000004
#define DSBCAPS_LOCSOFTWARE             0x00000008
#define DSBCAPS_CTRL3D                  0x00000010
#define DSBCAPS_CTRLFREQUENCY           0x00000020
#define DSBCAPS_CTRLPAN                 0x00000040
#define DSBCAPS_CTRLVOLUME              0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY      0x00000100
#define DSBCAPS_STICKYFOCUS             0x00004000
#define DSBCAPS_GLOBALFOCUS             0x00008000
#define DSBCAPS_GETCURRENTPOSITION2     0x00010000
#define DSBCAPS_CTRLDEFAULT             0x000000E0

// Play flags
#define DSBPLAY_LOOPING                 0x00000001

// Lock flags
#define DSBLOCK_FROMWRITECURSOR         0x00000001
#define DSBLOCK_ENTIREBUFFER            0x00000002

// Buffer status
#define DSBSTATUS_PLAYING               0x00000001
#define DSBSTATUS_BUFFERLOST            0x00000002
#define DSBSTATUS_LOOPING               0x00000004

// Volume range
#define DSBVOLUME_MIN                   -10000
#define DSBVOLUME_MAX                   0

// Pan range
#define DSBPAN_LEFT                     -10000
#define DSBPAN_CENTER                   0
#define DSBPAN_RIGHT                    10000

// Forward declarations
struct IDirectSound;
struct IDirectSoundBuffer;

typedef struct IDirectSound *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

// WAVEFORMATEX structure
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;

typedef const WAVEFORMATEX *LPCWAVEFORMATEX;

// Wave format tags
#define WAVE_FORMAT_PCM                 1

// DSBUFFERDESC structure
typedef struct _DSBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

// DSCAPS structure
typedef struct _DSCAPS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD dwPrimaryBuffers;
    DWORD dwMaxHwMixingAllBuffers;
    DWORD dwMaxHwMixingStaticBuffers;
    DWORD dwMaxHwMixingStreamingBuffers;
    DWORD dwFreeHwMixingAllBuffers;
    DWORD dwFreeHwMixingStaticBuffers;
    DWORD dwFreeHwMixingStreamingBuffers;
    DWORD dwMaxHw3DAllBuffers;
    DWORD dwMaxHw3DStaticBuffers;
    DWORD dwMaxHw3DStreamingBuffers;
    DWORD dwFreeHw3DAllBuffers;
    DWORD dwFreeHw3DStaticBuffers;
    DWORD dwFreeHw3DStreamingBuffers;
    DWORD dwTotalHwMemBytes;
    DWORD dwFreeHwMemBytes;
    DWORD dwMaxContigFreeHwMemBytes;
    DWORD dwUnlockTransferRateHwBuffers;
    DWORD dwPlayCpuOverheadSwBuffers;
    DWORD dwReserved1;
    DWORD dwReserved2;
} DSCAPS, *LPDSCAPS;

// DSBCAPS structure
typedef struct _DSBCAPS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwUnlockTransferRate;
    DWORD dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

// IDirectSound interface (stub)
struct IDirectSound {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, void* pUnkOuter) = 0;
    virtual HRESULT GetCaps(LPDSCAPS pDSCaps) = 0;
    virtual HRESULT DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER* ppDSBufferDuplicate) = 0;
    virtual HRESULT SetCooperativeLevel(HWND hwnd, DWORD dwLevel) = 0;
    virtual HRESULT Compact() = 0;
    virtual HRESULT GetSpeakerConfig(LPDWORD pdwSpeakerConfig) = 0;
    virtual HRESULT SetSpeakerConfig(DWORD dwSpeakerConfig) = 0;
    virtual HRESULT Initialize(void* pcGuidDevice) = 0;
};

// IDirectSoundBuffer interface (stub)
struct IDirectSoundBuffer {
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT GetCaps(LPDSBCAPS pDSBufferCaps) = 0;
    virtual HRESULT GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor) = 0;
    virtual HRESULT GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) = 0;
    virtual HRESULT GetVolume(LPLONG plVolume) = 0;
    virtual HRESULT GetPan(LPLONG plPan) = 0;
    virtual HRESULT GetFrequency(LPDWORD pdwFrequency) = 0;
    virtual HRESULT GetStatus(LPDWORD pdwStatus) = 0;
    virtual HRESULT Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc) = 0;
    virtual HRESULT Lock(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) = 0;
    virtual HRESULT Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) = 0;
    virtual HRESULT SetCurrentPosition(DWORD dwNewPosition) = 0;
    virtual HRESULT SetFormat(LPCWAVEFORMATEX pcfxFormat) = 0;
    virtual HRESULT SetVolume(LONG lVolume) = 0;
    virtual HRESULT SetPan(LONG lPan) = 0;
    virtual HRESULT SetFrequency(DWORD dwFrequency) = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) = 0;
    virtual HRESULT Restore() = 0;
};

// DirectSoundCreate function stub
#ifdef __cplusplus
extern "C" {
#endif

HRESULT DirectSoundCreate(void* pcGuidDevice, LPDIRECTSOUND* ppDS, void* pUnkOuter);

#ifdef __cplusplus
}
#endif

#endif // COMPAT_DSOUND_H
