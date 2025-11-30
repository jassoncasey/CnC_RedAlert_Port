/**
 * Red Alert macOS Port - Multimedia System Compatibility Stub
 *
 * Stub types for Windows multimedia timer and audio functions.
 * Actual timing uses mach_absolute_time().
 */

#ifndef COMPAT_MMSYSTEM_H
#define COMPAT_MMSYSTEM_H

#include "platform.h"

// Timer resolution
#define TIMERR_NOERROR      0
#define TIMERR_NOCANDO      97
#define TIMERR_BASE         96

// Multimedia result
typedef UINT MMRESULT;

// Time capabilities
typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *LPTIMECAPS;

// Timer callback type
typedef void (CALLBACK *LPTIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

// Timer flags
#define TIME_ONESHOT    0x0000
#define TIME_PERIODIC   0x0001
#define TIME_CALLBACK_FUNCTION  0x0000

// MIDI-related types (stub - we're deferring MIDI)
typedef HANDLE HMIDIOUT;
typedef HANDLE HMIDIIN;
typedef HANDLE HMIDI;

// Wave-related types
typedef HANDLE HWAVEOUT;
typedef HANDLE HWAVEIN;
typedef HANDLE HWAVE;

// Mixer-related types
typedef HANDLE HMIXER;
typedef HANDLE HMIXEROBJ;

// MCI error codes
#define MMSYSERR_NOERROR        0
#define MMSYSERR_ERROR          1
#define MMSYSERR_BADDEVICEID    2
#define MMSYSERR_NOTENABLED     3
#define MMSYSERR_ALLOCATED      4
#define MMSYSERR_INVALHANDLE    5
#define MMSYSERR_NODRIVER       6
#define MMSYSERR_NOMEM          7
#define MMSYSERR_NOTSUPPORTED   8
#define MMSYSERR_BADERRNUM      9
#define MMSYSERR_INVALFLAG      10
#define MMSYSERR_INVALPARAM     11

// Wave format (also in dsound.h, but may be included separately)
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

// Function stubs
#ifdef __cplusplus
extern "C" {
#endif

// Timer functions
DWORD timeGetTime(void);
MMRESULT timeBeginPeriod(UINT uPeriod);
MMRESULT timeEndPeriod(UINT uPeriod);
MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
UINT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent);
MMRESULT timeKillEvent(UINT uTimerID);

// Joystick functions (stub - not implementing)
#define JOYERR_NOERROR      0
#define JOYERR_PARMS        165
#define JOYERR_NOCANDO      166
#define JOYERR_UNPLUGGED    167

typedef struct joyinfo_tag {
    UINT wXpos;
    UINT wYpos;
    UINT wZpos;
    UINT wButtons;
} JOYINFO, *LPJOYINFO;

typedef struct joyinfoex_tag {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwXpos;
    DWORD dwYpos;
    DWORD dwZpos;
    DWORD dwRpos;
    DWORD dwUpos;
    DWORD dwVpos;
    DWORD dwButtons;
    DWORD dwButtonNumber;
    DWORD dwPOV;
    DWORD dwReserved1;
    DWORD dwReserved2;
} JOYINFOEX, *LPJOYINFOEX;

MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji);
MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
UINT joyGetNumDevs(void);

#ifdef __cplusplus
}
#endif

#endif // COMPAT_MMSYSTEM_H
