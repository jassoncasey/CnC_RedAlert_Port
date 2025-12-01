/**
 * Red Alert macOS Port - Asset Loader Implementation
 */

#include "assetloader.h"
#include "mixfile.h"
#include "shpfile.h"
#include "audfile.h"
#include "palfile.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

// Main archive handles
static MixFileHandle g_mainMix = nullptr;      // MAIN_ALLIED.MIX (vehicles, buildings)
static MixFileHandle g_redalertMix = nullptr;  // REDALERT.MIX (infantry, palettes)
static MixFileHandle g_conquerMix = nullptr;   // CONQUER.MIX (vehicles, buildings)
static MixFileHandle g_hiresMix = nullptr;     // HIRES.MIX (infantry sprites)
static MixFileHandle g_soundsMix = nullptr;    // SOUNDS.MIX (sound effects)
static MixFileHandle g_localMix = nullptr;     // LOCAL.MIX (INI files, palettes)
static MixFileHandle g_snowMix = nullptr;      // SNOW.MIX (snow tileset)
static MixFileHandle g_temperatMix = nullptr;  // TEMPERAT.MIX (temperate tileset)

// Memory-backed nested MIX data (kept alive for duration)
static void* g_conquerData = nullptr;
static void* g_hiresData = nullptr;
static void* g_soundsData = nullptr;
static void* g_localData = nullptr;
static void* g_snowData = nullptr;
static void* g_temperatData = nullptr;

// Current palette (expanded to 8-bit)
static uint8_t g_palette[768] = {0};
static bool g_paletteLoaded = false;

// Movies archive (opened on demand)
static MixFileHandle g_moviesMix = nullptr;
static void* g_moviesData = nullptr;

// Archive search paths for MAIN_ALLIED.MIX
static const char* g_mainPaths[] = {
    "../assets/MAIN_ALLIED.MIX",                              // Dev: from build/
    "../../assets/MAIN_ALLIED.MIX",                           // Dev: from macos/
    "/Users/jasson/workspace/CnC_Red_Alert/assets/MAIN_ALLIED.MIX",  // Absolute path
    "./assets/MAIN_ALLIED.MIX",                               // Current dir
    "/Volumes/CD1/MAIN.MIX",                                  // Mounted game CD
    "/Volumes/CD2/MAIN.MIX",
    nullptr
};

// Archive search paths for REDALERT.MIX
static const char* g_redalertPaths[] = {
    "../assets/REDALERT.MIX",                                 // Dev: from build/
    "../../assets/REDALERT.MIX",                              // Dev: from macos/
    "/Users/jasson/workspace/CnC_Red_Alert/assets/REDALERT.MIX",     // Absolute path
    "./assets/REDALERT.MIX",                                  // Current dir
    nullptr
};

// Archive search paths for standalone MIX files (from quick install package)
static const char* g_conquerPaths[] = {
    "../assets/conquer.mix",
    "../../assets/conquer.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix",
    nullptr
};

static const char* g_hiresPaths[] = {
    "../assets/hires.mix",
    "../../assets/hires.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/hires.mix",
    nullptr
};

static const char* g_soundsPaths[] = {
    "../assets/sounds.mix",
    "../../assets/sounds.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/sounds.mix",
    nullptr
};

static const char* g_localPaths[] = {
    "../assets/local.mix",
    "../../assets/local.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/local.mix",
    nullptr
};

static const char* g_snowPaths[] = {
    "../assets/snow.mix",
    "../../assets/snow.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix",
    nullptr
};

static const char* g_temperatPaths[] = {
    "../assets/temperat.mix",
    "../../assets/temperat.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/temperat.mix",
    nullptr
};

static MixFileHandle OpenNestedMix(MixFileHandle parent, const char* name, void** outData) {
    if (!parent) return nullptr;

    uint32_t size = 0;
    void* data = Mix_AllocReadFile(parent, name, &size);
    if (!data) return nullptr;

    MixFileHandle mix = Mix_OpenMemory(data, size, TRUE);
    if (!mix) {
        free(data);
        return nullptr;
    }

    *outData = data;
    return mix;
}

// Helper to open standalone or nested MIX
static MixFileHandle OpenMixFile(const char** paths, const char* name,
                                  MixFileHandle parent, void** outData) {
    // First try standalone files (from quick install package - preferred)
    for (int i = 0; paths && paths[i]; i++) {
        MixFileHandle mix = Mix_Open(paths[i]);
        if (mix) {
            printf("AssetLoader: Opened %s\n", paths[i]);
            return mix;
        }
    }

    // Fall back to nested in parent archive
    if (parent && name) {
        MixFileHandle mix = OpenNestedMix(parent, name, outData);
        if (mix) {
            printf("AssetLoader: Opened %s from parent\n", name);
            return mix;
        }
    }

    return nullptr;
}

BOOL Assets_Init(void) {
    // Open MAIN_ALLIED.MIX (for fallback nested archives)
    for (int i = 0; g_mainPaths[i]; i++) {
        g_mainMix = Mix_Open(g_mainPaths[i]);
        if (g_mainMix) {
            printf("AssetLoader: Opened %s\n", g_mainPaths[i]);
            break;
        }
    }

    // Open REDALERT.MIX (for fallback nested archives)
    for (int i = 0; g_redalertPaths[i]; i++) {
        g_redalertMix = Mix_Open(g_redalertPaths[i]);
        if (g_redalertMix) {
            printf("AssetLoader: Opened %s\n", g_redalertPaths[i]);
            break;
        }
    }

    // Open content MIX files - prefer standalone from quick install package
    g_conquerMix = OpenMixFile(g_conquerPaths, "CONQUER.MIX", g_mainMix, &g_conquerData);
    g_hiresMix = OpenMixFile(g_hiresPaths, "HIRES.MIX", g_redalertMix, &g_hiresData);
    g_soundsMix = OpenMixFile(g_soundsPaths, "SOUNDS.MIX", g_mainMix, &g_soundsData);
    g_localMix = OpenMixFile(g_localPaths, "LOCAL.MIX", g_redalertMix, &g_localData);
    g_snowMix = OpenMixFile(g_snowPaths, nullptr, nullptr, nullptr);
    g_temperatMix = OpenMixFile(g_temperatPaths, nullptr, nullptr, nullptr);

    // Check if we have required archives
    if (!g_conquerMix && !g_hiresMix) {
        printf("AssetLoader: No content archives found! Please extract ra-quickinstall.zip to assets/\n");
        return FALSE;
    }

    // Try to load default palette (SNOW.PAL or generate fallback)
    uint8_t pal[768];
    if (Assets_LoadPalette("SNOW.PAL", pal)) {
        Assets_SetPalette(pal);
        printf("AssetLoader: Loaded SNOW.PAL\n");
    } else if (Assets_LoadPalette("TEMPERAT.PAL", pal)) {
        Assets_SetPalette(pal);
        printf("AssetLoader: Loaded TEMPERAT.PAL\n");
    } else {
        // Generate a grayscale fallback palette
        for (int i = 0; i < 256; i++) {
            g_palette[i*3+0] = (uint8_t)i;
            g_palette[i*3+1] = (uint8_t)i;
            g_palette[i*3+2] = (uint8_t)i;
        }
        g_paletteLoaded = true;
        printf("AssetLoader: Using fallback grayscale palette\n");
    }

    return TRUE;
}

void Assets_Shutdown(void) {
    if (g_snowMix) { Mix_Close(g_snowMix); g_snowMix = nullptr; }
    if (g_temperatMix) { Mix_Close(g_temperatMix); g_temperatMix = nullptr; }
    if (g_conquerMix) { Mix_Close(g_conquerMix); g_conquerMix = nullptr; }
    if (g_hiresMix) { Mix_Close(g_hiresMix); g_hiresMix = nullptr; }
    if (g_soundsMix) { Mix_Close(g_soundsMix); g_soundsMix = nullptr; }
    if (g_localMix) { Mix_Close(g_localMix); g_localMix = nullptr; }
    if (g_mainMix) { Mix_Close(g_mainMix); g_mainMix = nullptr; }
    if (g_redalertMix) { Mix_Close(g_redalertMix); g_redalertMix = nullptr; }

    // Memory freed by Mix_Close (ownsData = TRUE)
    g_conquerData = nullptr;
    g_hiresData = nullptr;
    g_soundsData = nullptr;
    g_localData = nullptr;
    g_snowData = nullptr;
    g_temperatData = nullptr;

    g_paletteLoaded = false;
}

// Search paths for loose SHP files (OpenRA bits folder)
static const char* g_shpSearchPaths[] = {
    "../assets/bits/",
    "../../assets/bits/",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/bits/",
    nullptr
};

// Helper to load SHP from loose file
static ShpFileHandle LoadSHPFromFile(const char* name) {
    char path[512];
    for (int i = 0; g_shpSearchPaths[i]; i++) {
        snprintf(path, sizeof(path), "%s%s", g_shpSearchPaths[i], name);
        FILE* f = fopen(path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            void* data = malloc(size);
            if (data && fread(data, 1, size, f) == (size_t)size) {
                fclose(f);
                ShpFileHandle shp = Shp_Load(data, size);
                free(data);
                if (shp) return shp;
            } else {
                fclose(f);
                if (data) free(data);
            }
        }
    }
    return nullptr;
}

ShpFileHandle Assets_LoadSHP(const char* name) {
    if (!name) return nullptr;

    // First try MIX archives: CONQUER.MIX (vehicles/buildings), HIRES.MIX (infantry)
    MixFileHandle searchOrder[] = { g_conquerMix, g_hiresMix, g_mainMix, g_redalertMix, nullptr };

    for (int i = 0; searchOrder[i]; i++) {
        if (Mix_FileExists(searchOrder[i], name)) {
            uint32_t size = 0;
            void* data = Mix_AllocReadFile(searchOrder[i], name, &size);
            if (data) {
                ShpFileHandle shp = Shp_Load(data, size);
                free(data);
                return shp;
            }
        }
    }

    // Fallback: try loose SHP files (OpenRA bits folder)
    ShpFileHandle shp = LoadSHPFromFile(name);
    if (shp) return shp;

    // Try lowercase version
    char lower[256];
    int len = 0;
    while (name[len] && len < 255) {
        lower[len] = (name[len] >= 'A' && name[len] <= 'Z') ? name[len] + 32 : name[len];
        len++;
    }
    lower[len] = '\0';
    return LoadSHPFromFile(lower);
}

AudData* Assets_LoadAUD(const char* name) {
    if (!name) return nullptr;

    // Search in SOUNDS.MIX first, then top-level archives
    MixFileHandle searchOrder[] = { g_soundsMix, g_mainMix, g_redalertMix, nullptr };

    for (int i = 0; searchOrder[i]; i++) {
        if (Mix_FileExists(searchOrder[i], name)) {
            uint32_t size = 0;
            void* data = Mix_AllocReadFile(searchOrder[i], name, &size);
            if (data) {
                AudData* aud = Aud_Load(data, size);
                free(data);
                return aud;
            }
        }
    }

    return nullptr;
}

BOOL Assets_LoadPalette(const char* name, uint8_t* palette) {
    if (!name || !palette) return FALSE;

    // Search in LOCAL.MIX first (contains palettes), then top-level
    MixFileHandle searchOrder[] = { g_localMix, g_mainMix, g_redalertMix, nullptr };

    for (int i = 0; searchOrder[i]; i++) {
        if (Mix_FileExists(searchOrder[i], name)) {
            uint32_t size = 0;
            void* data = Mix_AllocReadFile(searchOrder[i], name, &size);
            if (data && size == 768) {
                memcpy(palette, data, 768);
                free(data);
                return TRUE;
            }
            if (data) free(data);
        }
    }

    return FALSE;
}

const uint8_t* Assets_GetPalette(void) {
    return g_paletteLoaded ? g_palette : nullptr;
}

void Assets_SetPalette(const uint8_t* palette) {
    if (!palette) return;

    // Expand 6-bit VGA palette to 8-bit
    for (int i = 0; i < 768; i++) {
        // VGA uses 6-bit colors (0-63), expand to 8-bit (0-255)
        // Formula: (value * 255) / 63 ≈ (value << 2) | (value >> 4)
        uint8_t val = palette[i] & 0x3F;
        g_palette[i] = (val << 2) | (val >> 4);
    }
    g_paletteLoaded = true;
}

void Assets_SHPToRGBA(const ShpFrame* frame, uint32_t* output, uint8_t transparent) {
    if (!frame || !frame->pixels || !output || !g_paletteLoaded) return;

    int size = frame->width * frame->height;
    for (int i = 0; i < size; i++) {
        uint8_t idx = frame->pixels[i];
        if (idx == transparent) {
            output[i] = 0x00000000;  // Fully transparent
        } else {
            uint8_t r = g_palette[idx * 3 + 0];
            uint8_t g = g_palette[idx * 3 + 1];
            uint8_t b = g_palette[idx * 3 + 2];
            output[i] = (0xFF << 24) | (b << 16) | (g << 8) | r;  // ABGR for Metal
        }
    }
}

void* Assets_LoadRaw(const char* name, uint32_t* outSize) {
    if (!name || !outSize) return nullptr;
    *outSize = 0;

    // Search all archives (tileset archives first for terrain files)
    MixFileHandle searchOrder[] = { g_snowMix, g_temperatMix, g_localMix, g_conquerMix, g_hiresMix, g_soundsMix, g_mainMix, g_redalertMix, nullptr };

    for (int i = 0; searchOrder[i]; i++) {
        if (Mix_FileExists(searchOrder[i], name)) {
            return Mix_AllocReadFile(searchOrder[i], name, outSize);
        }
    }

    return nullptr;
}

// Try to open movies archive (called on first VQA load)
static void EnsureMoviesOpen(void) {
    if (g_moviesMix) return;  // Already open

    // Try to find MOVIES2.MIX inside MAIN.MIX from CD
    MixFileHandle mainCd = nullptr;
    const char* cdPaths[] = { "/Volumes/CD1/MAIN.MIX", "/Volumes/CD2/MAIN.MIX", nullptr };

    for (int i = 0; cdPaths[i]; i++) {
        mainCd = Mix_Open(cdPaths[i]);
        if (mainCd) {
            printf("Movies: Opened %s\n", cdPaths[i]);
            break;
        }
    }

    if (!mainCd) {
        // Try MAIN_ALLIED.MIX
        mainCd = g_mainMix;
    }

    if (!mainCd) return;

    // Look for MOVIES2.MIX or MOVIES1.MIX inside
    const char* moviesNames[] = { "MOVIES2.MIX", "MOVIES1.MIX", "MOVIES.MIX", nullptr };
    for (int i = 0; moviesNames[i]; i++) {
        if (Mix_FileExists(mainCd, moviesNames[i])) {
            uint32_t size = 0;
            void* data = Mix_AllocReadFile(mainCd, moviesNames[i], &size);
            if (data) {
                g_moviesMix = Mix_OpenMemory(data, size, TRUE);
                if (g_moviesMix) {
                    g_moviesData = data;
                    printf("Movies: Opened %s (%u MB, %d files)\n",
                           moviesNames[i], size / (1024*1024), Mix_GetFileCount(g_moviesMix));
                    break;
                } else {
                    free(data);
                }
            }
        }
    }

    // Close temp CD handle if we opened it separately
    if (mainCd && mainCd != g_mainMix) {
        // Note: Don't close - movies data points into it
        // This is a memory leak but acceptable for the movies archive
    }
}

void* Assets_LoadVQA(const char* name, uint32_t* outSize) {
    if (!name || !outSize) return nullptr;
    *outSize = 0;

    // Make sure movies archive is open
    EnsureMoviesOpen();

    if (!g_moviesMix) return nullptr;

    // Search for VQA in movies archive
    if (Mix_FileExists(g_moviesMix, name)) {
        return Mix_AllocReadFile(g_moviesMix, name, outSize);
    }

    return nullptr;
}

BOOL Assets_HasMovies(void) {
    EnsureMoviesOpen();
    return g_moviesMix != nullptr;
}
