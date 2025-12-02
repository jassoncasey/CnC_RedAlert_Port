/**
 * Red Alert macOS Port - File I/O Implementation
 *
 * POSIX implementations of Windows file I/O functions.
 */

#include "compat/windows.h"

#include <fcntl.h>
#include <limits.h>  // PATH_MAX
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

// Thread-local error code
static thread_local DWORD g_lastError = 0;

/**
 * Convert Windows error codes to/from errno
 */
static DWORD ErrnoToWin32(int err) {
    switch (err) {
        case 0:         return ERROR_SUCCESS;
        case ENOENT:    return ERROR_FILE_NOT_FOUND;
        case EACCES:    return ERROR_ACCESS_DENIED;
        case EEXIST:    return ERROR_FILE_EXISTS;
        case EBADF:     return ERROR_INVALID_HANDLE;
        case ENOMEM:    return ERROR_OUTOFMEMORY;
        case EINVAL:    return ERROR_INVALID_PARAMETER;
        case ENOSPC:    return ERROR_DISK_FULL;
        default:        return ERROR_GEN_FAILURE;
    }
}

DWORD GetLastError(void) {
    return g_lastError;
}

void SetLastError(DWORD dwErrCode) {
    g_lastError = dwErrCode;
}

/**
 * CreateFile - Open or create a file
 *
 * Maps Windows CreateFile to POSIX open()
 */
HANDLE CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    void* lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    (void)dwShareMode;           // Sharing not fully supported on POSIX
    (void)lpSecurityAttributes;  // Security not supported
    (void)dwFlagsAndAttributes;  // Attributes not supported
    (void)hTemplateFile;         // Template not supported

    if (!lpFileName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    // Convert access flags
    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE)) {
        flags = O_RDWR;
    } else if (dwDesiredAccess & GENERIC_WRITE) {
        flags = O_WRONLY;
    } else {
        flags = O_RDONLY;
    }

    // Convert creation disposition
    switch (dwCreationDisposition) {
        case CREATE_NEW:
            flags |= O_CREAT | O_EXCL;
            break;
        case CREATE_ALWAYS:
            flags |= O_CREAT | O_TRUNC;
            break;
        case OPEN_EXISTING:
            // No additional flags needed
            break;
        case OPEN_ALWAYS:
            flags |= O_CREAT;
            break;
        case TRUNCATE_EXISTING:
            flags |= O_TRUNC;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
    }

    // Open the file
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  // 0644
    int fd = open(lpFileName, flags, mode);

    if (fd < 0) {
        SetLastError(ErrnoToWin32(errno));
        return INVALID_HANDLE_VALUE;
    }

    SetLastError(ERROR_SUCCESS);
    // Store fd as handle (offset by 1 to avoid NULL == 0 confusion)
    return (HANDLE)(intptr_t)(fd + 1);
}

/**
 * Convert HANDLE back to file descriptor
 */
static int HandleToFd(HANDLE hFile) {
    if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
        return -1;
    }
    return (int)((intptr_t)hFile - 1);
}

/**
 * ReadFile - Read data from a file
 */
BOOL ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
) {
    (void)lpOverlapped;  // Async I/O not supported

    int fd = HandleToFd(hFile);
    if (fd < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
        return FALSE;
    }

    ssize_t result = read(fd, lpBuffer, nNumberOfBytesToRead);

    if (result < 0) {
        SetLastError(ErrnoToWin32(errno));
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
        return FALSE;
    }

    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = (DWORD)result;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

/**
 * WriteFile - Write data to a file
 */
BOOL WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
) {
    (void)lpOverlapped;  // Async I/O not supported

    int fd = HandleToFd(hFile);
    if (fd < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
        return FALSE;
    }

    ssize_t result = write(fd, lpBuffer, nNumberOfBytesToWrite);

    if (result < 0) {
        SetLastError(ErrnoToWin32(errno));
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
        return FALSE;
    }

    if (lpNumberOfBytesWritten) {
        *lpNumberOfBytesWritten = (DWORD)result;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

/**
 * CloseHandle - Close a file handle
 */
BOOL CloseHandle(HANDLE hObject) {
    int fd = HandleToFd(hObject);
    if (fd < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (close(fd) < 0) {
        SetLastError(ErrnoToWin32(errno));
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

/**
 * SetFilePointer - Move file pointer
 */
DWORD SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    LPLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
) {
    (void)lpDistanceToMoveHigh;  // 64-bit offsets not supported

    int fd = HandleToFd(hFile);
    if (fd < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        return (DWORD)-1;
    }

    int whence;
    switch (dwMoveMethod) {
        case FILE_BEGIN:   whence = SEEK_SET; break;
        case FILE_CURRENT: whence = SEEK_CUR; break;
        case FILE_END:     whence = SEEK_END; break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return (DWORD)-1;
    }

    off_t result = lseek(fd, lDistanceToMove, whence);

    if (result < 0) {
        SetLastError(ErrnoToWin32(errno));
        return (DWORD)-1;
    }

    SetLastError(ERROR_SUCCESS);
    return (DWORD)result;
}

/**
 * GetFileSize - Get size of open file
 */
DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    if (lpFileSizeHigh) *lpFileSizeHigh = 0;

    int fd = HandleToFd(hFile);
    if (fd < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        return (DWORD)-1;
    }

    // Save current position
    off_t current = lseek(fd, 0, SEEK_CUR);
    if (current < 0) {
        SetLastError(ErrnoToWin32(errno));
        return (DWORD)-1;
    }

    // Seek to end
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        SetLastError(ErrnoToWin32(errno));
        return (DWORD)-1;
    }

    // Restore position
    lseek(fd, current, SEEK_SET);

    SetLastError(ERROR_SUCCESS);
    return (DWORD)size;
}

/**
 * DeleteFile - Delete a file
 */
BOOL DeleteFileA(LPCSTR lpFileName) {
    if (!lpFileName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (unlink(lpFileName) < 0) {
        SetLastError(ErrnoToWin32(errno));
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

/**
 * GetCurrentDirectory - Get current working directory
 */
DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (!lpBuffer || nBufferLength == 0) {
        // Return required size
        char temp[PATH_MAX];
        if (getcwd(temp, sizeof(temp))) {
            return (DWORD)strlen(temp) + 1;
        }
        return 0;
    }

    if (!getcwd(lpBuffer, nBufferLength)) {
        SetLastError(ErrnoToWin32(errno));
        return 0;
    }

    return (DWORD)strlen(lpBuffer);
}

/**
 * SetCurrentDirectory - Set current working directory
 */
BOOL SetCurrentDirectoryA(LPCSTR lpPathName) {
    if (!lpPathName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (chdir(lpPathName) < 0) {
        SetLastError(ErrnoToWin32(errno));
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

// Find file context
struct FindFileContext {
    DIR* dir;
    char pattern[MAX_PATH];
    char directory[MAX_PATH];
};

/**
 * Simple pattern matching (supports * and ?)
 */
static bool MatchPattern(const char* pattern, const char* str) {
    while (*pattern && *str) {
        if (*pattern == '*') {
            pattern++;
            if (!*pattern) return true;
            while (*str) {
                if (MatchPattern(pattern, str)) return true;
                str++;
            }
            return false;
        } else if (*pattern == '?' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return false;
        }
    }
    while (*pattern == '*') pattern++;
    return !*pattern && !*str;
}

/**
 * FindFirstFile - Begin file enumeration
 */
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
    if (!lpFileName || !lpFindFileData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    // Split path into directory and pattern
    char dirPath[MAX_PATH];
    strncpy(dirPath, lpFileName, MAX_PATH - 1);
    dirPath[MAX_PATH - 1] = '\0';

    char* lastSlash = strrchr(dirPath, '/');
    const char* pattern;

    if (lastSlash) {
        *lastSlash = '\0';
        pattern = lastSlash + 1;
    } else {
        dirPath[0] = '.';
        dirPath[1] = '\0';
        pattern = lpFileName;
    }

    DIR* dir = opendir(dirPath);
    if (!dir) {
        SetLastError(ErrnoToWin32(errno));
        return INVALID_HANDLE_VALUE;
    }

    FindFileContext* ctx = new FindFileContext;
    ctx->dir = dir;
    strncpy(ctx->pattern, pattern, MAX_PATH - 1);
    ctx->pattern[MAX_PATH - 1] = '\0';
    strncpy(ctx->directory, dirPath, MAX_PATH - 1);
    ctx->directory[MAX_PATH - 1] = '\0';

    // Find first matching entry
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (MatchPattern(ctx->pattern, entry->d_name)) {
            strncpy(lpFindFileData->cFileName, entry->d_name, MAX_PATH - 1);
            lpFindFileData->cFileName[MAX_PATH - 1] = '\0';

            // Get file attributes
            char fullPath[MAX_PATH * 2];
            snprintf(fullPath, sizeof(fullPath),
                     "%s/%s", ctx->directory, entry->d_name);

            struct stat st;
            if (stat(fullPath, &st) == 0) {
                bool isDir = S_ISDIR(st.st_mode);
                DWORD attr = isDir ? FILE_ATTRIBUTE_DIRECTORY
                                   : FILE_ATTRIBUTE_NORMAL;
                lpFindFileData->dwFileAttributes = attr;
                lpFindFileData->nFileSizeLow = (DWORD)st.st_size;
                lpFindFileData->nFileSizeHigh = 0;
            } else {
                lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
                lpFindFileData->nFileSizeLow = 0;
                lpFindFileData->nFileSizeHigh = 0;
            }

            SetLastError(ERROR_SUCCESS);
            return (HANDLE)ctx;
        }
    }

    // No matching files found
    delete ctx;
    closedir(dir);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return INVALID_HANDLE_VALUE;
}

/**
 * FindNextFile - Continue file enumeration
 */
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) {
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE || !lpFindFileData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    FindFileContext* ctx = (FindFileContext*)hFindFile;

    struct dirent* entry;
    while ((entry = readdir(ctx->dir)) != nullptr) {
        if (MatchPattern(ctx->pattern, entry->d_name)) {
            strncpy(lpFindFileData->cFileName, entry->d_name, MAX_PATH - 1);
            lpFindFileData->cFileName[MAX_PATH - 1] = '\0';

            // Get file attributes
            char fullPath[MAX_PATH * 2];
            snprintf(fullPath, sizeof(fullPath),
                     "%s/%s", ctx->directory, entry->d_name);

            struct stat st;
            if (stat(fullPath, &st) == 0) {
                bool isDir = S_ISDIR(st.st_mode);
                DWORD attr = isDir ? FILE_ATTRIBUTE_DIRECTORY
                                   : FILE_ATTRIBUTE_NORMAL;
                lpFindFileData->dwFileAttributes = attr;
                lpFindFileData->nFileSizeLow = (DWORD)st.st_size;
                lpFindFileData->nFileSizeHigh = 0;
            } else {
                lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
                lpFindFileData->nFileSizeLow = 0;
                lpFindFileData->nFileSizeHigh = 0;
            }

            SetLastError(ERROR_SUCCESS);
            return TRUE;
        }
    }

    SetLastError(ERROR_NO_MORE_FILES);
    return FALSE;
}

/**
 * FindClose - End file enumeration
 */
BOOL FindClose(HANDLE hFindFile) {
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    FindFileContext* ctx = (FindFileContext*)hFindFile;
    closedir(ctx->dir);
    delete ctx;

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

/**
 * GetModuleHandle - Get handle to current module (stub)
 */
HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    (void)lpModuleName;
    // Return a non-null handle for the main module
    return (HMODULE)(intptr_t)1;
}

/**
 * OutputDebugString - Write debug output
 */
void OutputDebugStringA(LPCSTR lpOutputString) {
    if (lpOutputString) {
        fprintf(stderr, "[DEBUG] %s", lpOutputString);
    }
}
