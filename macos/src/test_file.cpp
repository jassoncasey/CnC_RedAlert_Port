/**
 * Test file I/O implementation
 */

#include "compat/windows.h"
#include <cstdio>
#include <cstring>

int main() {
    printf("Testing file I/O...\n");

    // Test 1: Create and write file
    const char* testFile = "/tmp/ra_test_file.txt";
    const char* testData = "Hello from Red Alert macOS port!";

    HANDLE hFile = CreateFile(testFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("FAIL: CreateFile for write failed (error %u)\n", GetLastError());
        return 1;
    }
    printf("OK: CreateFile for write\n");

    DWORD bytesWritten = 0;
    if (!WriteFile(hFile, testData, (DWORD)strlen(testData), &bytesWritten, NULL)) {
        printf("FAIL: WriteFile failed (error %u)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    printf("OK: WriteFile (%u bytes)\n", bytesWritten);

    CloseHandle(hFile);
    printf("OK: CloseHandle (write)\n");

    // Test 2: Open and read file
    hFile = CreateFile(testFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("FAIL: CreateFile for read failed (error %u)\n", GetLastError());
        return 1;
    }
    printf("OK: CreateFile for read\n");

    // Test 3: GetFileSize
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == (DWORD)-1) {
        printf("FAIL: GetFileSize failed (error %u)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    printf("OK: GetFileSize = %u bytes\n", fileSize);

    // Test 4: Read file
    char buffer[256] = {0};
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        printf("FAIL: ReadFile failed (error %u)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    printf("OK: ReadFile (%u bytes): \"%s\"\n", bytesRead, buffer);

    // Test 5: SetFilePointer
    DWORD newPos = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    if (newPos == (DWORD)-1) {
        printf("FAIL: SetFilePointer failed (error %u)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    printf("OK: SetFilePointer to beginning (pos = %u)\n", newPos);

    CloseHandle(hFile);
    printf("OK: CloseHandle (read)\n");

    // Test 6: DeleteFile
    if (!DeleteFile(testFile)) {
        printf("FAIL: DeleteFile failed (error %u)\n", GetLastError());
        return 1;
    }
    printf("OK: DeleteFile\n");

    // Test 7: FindFirstFile
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("/tmp/*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("WARN: FindFirstFile failed (error %u)\n", GetLastError());
    } else {
        printf("OK: FindFirstFile - first file: %s\n", findData.cFileName);

        // Find a few more
        int count = 1;
        while (FindNextFile(hFind, &findData) && count < 3) {
            printf("    FindNextFile: %s\n", findData.cFileName);
            count++;
        }
        FindClose(hFind);
        printf("OK: FindClose\n");
    }

    printf("\nAll file I/O tests passed!\n");
    return 0;
}
