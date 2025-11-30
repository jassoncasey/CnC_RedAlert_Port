/**
 * Red Alert macOS Port - Timing Implementation
 *
 * POSIX/Mach implementations of Windows timing functions.
 */

#include "compat/windows.h"

#include <mach/mach_time.h>
#include <unistd.h>
#include <sched.h>

// Mach timebase info for converting to nanoseconds
static mach_timebase_info_data_t g_timebaseInfo;
static uint64_t g_startTime;
static bool g_initialized = false;

/**
 * Initialize timing system
 */
static void InitTiming() {
    if (!g_initialized) {
        mach_timebase_info(&g_timebaseInfo);
        g_startTime = mach_absolute_time();
        g_initialized = true;
    }
}

/**
 * Convert mach absolute time to milliseconds since start
 */
static DWORD MachTimeToMillis(uint64_t machTime) {
    // Convert to nanoseconds first
    uint64_t nanos = (machTime - g_startTime) * g_timebaseInfo.numer / g_timebaseInfo.denom;
    // Then to milliseconds
    return (DWORD)(nanos / 1000000ULL);
}

/**
 * GetTickCount - Get milliseconds since program start
 *
 * Windows returns ms since system boot, but games typically
 * only care about relative time, so start time is fine.
 */
DWORD GetTickCount(void) {
    InitTiming();
    return MachTimeToMillis(mach_absolute_time());
}

/**
 * timeGetTime - Multimedia timer (same as GetTickCount for our purposes)
 */
DWORD timeGetTime(void) {
    return GetTickCount();
}

/**
 * Sleep - Suspend execution for specified milliseconds
 */
void Sleep(DWORD dwMilliseconds) {
    if (dwMilliseconds == 0) {
        // Yield to other threads
        sched_yield();
    } else {
        usleep(dwMilliseconds * 1000);
    }
}

/**
 * QueryPerformanceCounter - High-resolution timer
 *
 * Returns current value of high-resolution performance counter.
 */
BOOL QueryPerformanceCounter(LONGLONG* lpPerformanceCount) {
    if (!lpPerformanceCount) {
        return FALSE;
    }

    InitTiming();
    *lpPerformanceCount = (LONGLONG)mach_absolute_time();
    return TRUE;
}

/**
 * QueryPerformanceFrequency - Get performance counter frequency
 *
 * Returns ticks per second. On macOS with mach_absolute_time,
 * we need to account for the timebase conversion.
 */
BOOL QueryPerformanceFrequency(LONGLONG* lpFrequency) {
    if (!lpFrequency) {
        return FALSE;
    }

    InitTiming();

    // Frequency in ticks per second
    // mach_absolute_time uses a timebase where:
    //   nanoseconds = ticks * numer / denom
    // So: ticks_per_second = 1e9 * denom / numer
    *lpFrequency = (LONGLONG)(1000000000ULL * g_timebaseInfo.denom / g_timebaseInfo.numer);
    return TRUE;
}
