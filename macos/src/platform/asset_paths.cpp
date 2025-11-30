/**
 * Red Alert macOS Port - Asset Path Management
 *
 * Searches for game assets in multiple locations.
 * See ASSETS.md for documentation.
 */

#include "asset_paths.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

// Asset search paths in priority order
static const char* SEARCH_PATHS[] = {
    "~/Library/Application Support/RedAlert/assets",  // User installation
    "./assets",                                        // Portable/adjacent to app
    "../assets",                                       // Development builds
    "/Volumes/CD1/INSTALL",                           // Mounted Allied ISO
    "/Volumes/CD2/INSTALL",                           // Mounted Soviet ISO
    nullptr
};

// Required asset files to verify
static const char* REQUIRED_ASSETS[] = {
    "REDALERT.MIX",  // Core game data (encrypted)
    nullptr
};

// Optional asset files
static const char* OPTIONAL_ASSETS[] = {
    "MAIN_ALLIED.MIX",  // Allied campaign
    "MAIN_SOVIET.MIX",  // Soviet campaign
    "AUD.MIX",          // Setup audio
    "SETUP.MIX",        // Setup graphics
    nullptr
};

// Cached asset path
static char g_assetPath[512] = {0};
static bool g_assetPathValid = false;

/**
 * Expand ~ to home directory
 */
static void ExpandPath(const char* path, char* outPath, size_t outSize) {
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                home = pw->pw_dir;
            }
        }
        if (home) {
            snprintf(outPath, outSize, "%s%s", home, path + 1);
        } else {
            strncpy(outPath, path, outSize - 1);
        }
    } else {
        strncpy(outPath, path, outSize - 1);
    }
    outPath[outSize - 1] = '\0';
}

/**
 * Check if a file exists
 */
static bool FileExists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Check if a directory contains a required asset
 */
static bool DirectoryHasAsset(const char* dir, const char* asset) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, asset);
    return FileExists(path);
}

bool Assets_FindPath(char* outPath, size_t outSize) {
    // Return cached path if available
    if (g_assetPathValid) {
        strncpy(outPath, g_assetPath, outSize - 1);
        outPath[outSize - 1] = '\0';
        return true;
    }

    // Search through all paths
    for (int i = 0; SEARCH_PATHS[i] != nullptr; i++) {
        char expandedPath[512];
        ExpandPath(SEARCH_PATHS[i], expandedPath, sizeof(expandedPath));

        // Check if this directory has a required asset
        if (DirectoryHasAsset(expandedPath, REQUIRED_ASSETS[0])) {
            strncpy(g_assetPath, expandedPath, sizeof(g_assetPath) - 1);
            g_assetPathValid = true;
            strncpy(outPath, expandedPath, outSize - 1);
            outPath[outSize - 1] = '\0';
            return true;
        }
    }

    // Also check for unencrypted MIX files (AUD.MIX) as fallback
    for (int i = 0; SEARCH_PATHS[i] != nullptr; i++) {
        char expandedPath[512];
        ExpandPath(SEARCH_PATHS[i], expandedPath, sizeof(expandedPath));

        if (DirectoryHasAsset(expandedPath, "AUD.MIX")) {
            strncpy(g_assetPath, expandedPath, sizeof(g_assetPath) - 1);
            g_assetPathValid = true;
            strncpy(outPath, expandedPath, outSize - 1);
            outPath[outSize - 1] = '\0';
            return true;
        }
    }

    outPath[0] = '\0';
    return false;
}

bool Assets_GetFilePath(const char* filename, char* outPath, size_t outSize) {
    char assetDir[512];
    if (!Assets_FindPath(assetDir, sizeof(assetDir))) {
        return false;
    }

    snprintf(outPath, outSize, "%s/%s", assetDir, filename);
    return FileExists(outPath);
}

void Assets_ClearCache(void) {
    g_assetPath[0] = '\0';
    g_assetPathValid = false;
}

void Assets_PrintSearchPaths(void) {
    printf("Asset search paths:\n");
    for (int i = 0; SEARCH_PATHS[i] != nullptr; i++) {
        char expandedPath[512];
        ExpandPath(SEARCH_PATHS[i], expandedPath, sizeof(expandedPath));

        bool exists = FileExists(expandedPath);
        bool hasAssets = exists && DirectoryHasAsset(expandedPath, REQUIRED_ASSETS[0]);

        printf("  %d. %s", i + 1, expandedPath);
        if (hasAssets) {
            printf(" [FOUND - has REDALERT.MIX]");
        } else if (exists) {
            printf(" [exists but no REDALERT.MIX]");
        } else {
            printf(" [not found]");
        }
        printf("\n");
    }
}

bool Assets_VerifyInstallation(void) {
    char assetDir[512];
    if (!Assets_FindPath(assetDir, sizeof(assetDir))) {
        return false;
    }

    printf("Asset directory: %s\n", assetDir);
    printf("\nRequired assets:\n");

    bool allFound = true;
    for (int i = 0; REQUIRED_ASSETS[i] != nullptr; i++) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", assetDir, REQUIRED_ASSETS[i]);
        bool found = FileExists(path);
        printf("  [%s] %s\n", found ? "OK" : "MISSING", REQUIRED_ASSETS[i]);
        if (!found) allFound = false;
    }

    printf("\nOptional assets:\n");
    for (int i = 0; OPTIONAL_ASSETS[i] != nullptr; i++) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", assetDir, OPTIONAL_ASSETS[i]);
        bool found = FileExists(path);
        printf("  [%s] %s\n", found ? "OK" : "missing", OPTIONAL_ASSETS[i]);
    }

    return allFound;
}
