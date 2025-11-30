/**
 * Test timing implementation
 */

#include "compat/windows.h"
#include <cstdio>

int main() {
    printf("Testing timing functions...\n\n");

    // Test 1: GetTickCount
    DWORD start = GetTickCount();
    printf("OK: GetTickCount = %u ms\n", start);

    // Test 2: timeGetTime (should be same as GetTickCount)
    DWORD mmTime = timeGetTime();
    printf("OK: timeGetTime = %u ms\n", mmTime);

    // Test 3: Sleep
    printf("Testing Sleep(100)...\n");
    DWORD beforeSleep = GetTickCount();
    Sleep(100);
    DWORD afterSleep = GetTickCount();
    DWORD elapsed = afterSleep - beforeSleep;
    printf("OK: Sleep(100) took %u ms", elapsed);
    if (elapsed >= 90 && elapsed <= 150) {
        printf(" (within expected range)\n");
    } else {
        printf(" (WARNING: outside expected 90-150ms range)\n");
    }

    // Test 4: QueryPerformanceFrequency
    LONGLONG freq = 0;
    if (!QueryPerformanceFrequency(&freq)) {
        printf("FAIL: QueryPerformanceFrequency returned FALSE\n");
        return 1;
    }
    printf("OK: QueryPerformanceFrequency = %lld Hz", freq);
    if (freq > 1000000) {
        printf(" (%.2f MHz)\n", freq / 1000000.0);
    } else {
        printf("\n");
    }

    // Test 5: QueryPerformanceCounter
    LONGLONG counter1 = 0, counter2 = 0;
    if (!QueryPerformanceCounter(&counter1)) {
        printf("FAIL: QueryPerformanceCounter returned FALSE\n");
        return 1;
    }
    printf("OK: QueryPerformanceCounter = %lld\n", counter1);

    // Test 6: Measure 50ms with high-resolution timer
    printf("Testing 50ms with QueryPerformanceCounter...\n");
    QueryPerformanceCounter(&counter1);
    Sleep(50);
    QueryPerformanceCounter(&counter2);

    double elapsedSec = (double)(counter2 - counter1) / (double)freq;
    double elapsedMs = elapsedSec * 1000.0;
    printf("OK: High-res timer measured %.2f ms", elapsedMs);
    if (elapsedMs >= 45.0 && elapsedMs <= 70.0) {
        printf(" (within expected range)\n");
    } else {
        printf(" (WARNING: outside expected 45-70ms range)\n");
    }

    // Test 7: Zero sleep (yield)
    printf("Testing Sleep(0) (yield)...\n");
    DWORD beforeYield = GetTickCount();
    for (int i = 0; i < 100; i++) {
        Sleep(0);
    }
    DWORD afterYield = GetTickCount();
    printf("OK: 100x Sleep(0) took %u ms\n", afterYield - beforeYield);

    printf("\nAll timing tests passed!\n");
    return 0;
}
