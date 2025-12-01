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

// Memory-backed nested MIX data (kept alive for duration)
static void* g_conquerData = nullptr;
static void* g_hiresData = nullptr;
static void* g_soundsData = nullptr;
static void* g_localData = nullptr;

// Current palette (expanded to 8-bit)
static uint8_t g_palette[768] = {0};
static bool g_paletteLoaded = false;

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

BOOL Assets_Init(void) {
    // Open MAIN_ALLIED.MIX (vehicles, buildings, sounds)
    for (int i = 0; g_mainPaths[i]; i++) {
        g_mainMix = Mix_Open(g_mainPaths[i]);
        if (g_mainMix) {
            printf("AssetLoader: Opened %s\n", g_mainPaths[i]);
            break;
        }
    }

    // Open REDALERT.MIX (infantry, palettes)
    for (int i = 0; g_redalertPaths[i]; i++) {
        g_redalertMix = Mix_Open(g_redalertPaths[i]);
        if (g_redalertMix) {
            printf("AssetLoader: Opened %s\n", g_redalertPaths[i]);
            break;
        }
    }

    if (!g_mainMix && !g_redalertMix) {
        printf("AssetLoader: No game archives found!\n");
        return FALSE;
    }

    // Open nested archives from MAIN_ALLIED.MIX
    if (g_mainMix) {
        g_conquerMix = OpenNestedMix(g_mainMix, "CONQUER.MIX", &g_conquerData);
        if (g_conquerMix) printf("AssetLoader: Opened CONQUER.MIX from MAIN\n");

        if (!g_soundsMix) {
            g_soundsMix = OpenNestedMix(g_mainMix, "SOUNDS.MIX", &g_soundsData);
            if (g_soundsMix) printf("AssetLoader: Opened SOUNDS.MIX from MAIN\n");
        }
    }

    // Open nested archives from REDALERT.MIX
    if (g_redalertMix) {
        g_hiresMix = OpenNestedMix(g_redalertMix, "HIRES.MIX", &g_hiresData);
        if (g_hiresMix) printf("AssetLoader: Opened HIRES.MIX from REDALERT\n");

        g_localMix = OpenNestedMix(g_redalertMix, "LOCAL.MIX", &g_localData);
        if (g_localMix) printf("AssetLoader: Opened LOCAL.MIX from REDALERT\n");
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

    g_paletteLoaded = false;
}

ShpFileHandle Assets_LoadSHP(const char* name) {
    if (!name) return nullptr;

    // Search order: CONQUER.MIX (vehicles/buildings), HIRES.MIX (infantry), then top-level
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

    return nullptr;
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

    // Search all archives
    MixFileHandle searchOrder[] = { g_localMix, g_conquerMix, g_hiresMix, g_soundsMix, g_mainMix, g_redalertMix, nullptr };

    for (int i = 0; searchOrder[i]; i++) {
        if (Mix_FileExists(searchOrder[i], name)) {
            return Mix_AllocReadFile(searchOrder[i], name, outSize);
        }
    }

    return nullptr;
}
