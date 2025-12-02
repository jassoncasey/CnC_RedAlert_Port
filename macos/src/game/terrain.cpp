/**
 * Red Alert macOS Port - Terrain Tile Manager Implementation
 */

#include "terrain.h"
#include "assets/assetloader.h"
#include "assets/tmpfile.h"
#include "graphics/metal/renderer.h"
#include <cstdio>
#include <cstring>

// Terrain template names for SNOW tileset
// These are the .sno files based on OpenRA's snow.yaml
static const char* g_terrainTemplates[] = {
    "clear1.sno",   // Clear terrain (basic ground) - 20 tiles
    "w1.sno",       // Water - main water tiles
    "w2.sno",       // Water - additional water
    "sh01.sno",     // Shore tiles
    "sh02.sno",
    "sh03.sno",
    "sh04.sno",
    "sh05.sno",
    "sh06.sno",
    "sh07.sno",
    "sh08.sno",
    "d01.sno",      // Debris/rough
    "d02.sno",
    "d03.sno",
    "d04.sno",
    "s01.sno",      // Road/cliffs
    "s02.sno",
    "rv01.sno",     // River
    "rv02.sno",
    "br1.sno",      // Bridge
    "br2.sno",
    nullptr
};

// Fallback: try .tem for temperate tileset
static const char* g_temperateTemplates[] = {
    "clear1.tem",
    "w1.tem",
    "w2.tem",
    "sh01.tem",
    "sh02.tem",
    "d01.tem",
    "d02.tem",
    "s01.tem",
    "s02.tem",
    "rv01.tem",
    "br1.tem",
    nullptr
};

#define MAX_TERRAIN_TEMPLATES 32
#define MAX_TILES_PER_TEMPLATE 64

// Loaded terrain tiles
static TmpFileHandle g_terrainTmp[MAX_TERRAIN_TEMPLATES] = {nullptr};
static int g_terrainTemplateCount = 0;
static bool g_terrainInitialized = false;

// Cache of tile pixels for quick access
static uint8_t* g_clearTile = nullptr;  // Default clear tile
static int g_tileSize = 24;  // Red Alert uses 24x24 tiles

BOOL Terrain_Init(void) {
    if (g_terrainInitialized) return TRUE;

    fprintf(stderr, "Terrain: Loading terrain tiles...\n");

    // Try to load terrain templates
    // First try SNOW tileset, then TEMPERATE
    int loadedTiles = 0;
    bool notFull = g_terrainTemplateCount < MAX_TERRAIN_TEMPLATES;
    for (int i = 0; g_terrainTemplates[i] && notFull; i++) {
        uint32_t size = 0;
        void* data = Assets_LoadRaw(g_terrainTemplates[i], &size);
        if (data && size > 0) {
            TmpFileHandle tmp = Tmp_Load(data, size);
            g_terrainTmp[g_terrainTemplateCount] = tmp;
            if (tmp) {
                loadedTiles += Tmp_GetTileCount(tmp);
                g_terrainTemplateCount++;
            }
            free(data);
        }
        notFull = g_terrainTemplateCount < MAX_TERRAIN_TEMPLATES;
    }

    // Try temperate if snow didn't work
    if (g_terrainTemplateCount == 0) {
        notFull = g_terrainTemplateCount < MAX_TERRAIN_TEMPLATES;
        for (int i = 0; g_temperateTemplates[i] && notFull; i++) {
            uint32_t size = 0;
            void* data = Assets_LoadRaw(g_temperateTemplates[i], &size);
            if (data && size > 0) {
                TmpFileHandle tmp = Tmp_Load(data, size);
                g_terrainTmp[g_terrainTemplateCount] = tmp;
                if (tmp) {
                    loadedTiles += Tmp_GetTileCount(tmp);
                    g_terrainTemplateCount++;
                }
                free(data);
            }
            notFull = g_terrainTemplateCount < MAX_TERRAIN_TEMPLATES;
        }
    }

    // Create a default clear tile if we loaded any terrain
    if (g_terrainTemplateCount > 0 && g_terrainTmp[0]) {
        const TmpTile* tile = Tmp_GetTile(g_terrainTmp[0], 0);
        if (tile && tile->pixels) {
            g_tileSize = tile->width;
            g_clearTile = new uint8_t[g_tileSize * g_tileSize];
            memcpy(g_clearTile, tile->pixels, g_tileSize * g_tileSize);
        }
    }

    // If no tiles loaded, create a procedural clear tile
    if (!g_clearTile) {
        g_clearTile = new uint8_t[g_tileSize * g_tileSize];
        // Fill with palette index 141 (light gray/snow in SNOW.PAL)
        memset(g_clearTile, 141, g_tileSize * g_tileSize);
        // Add some subtle variation
        for (int i = 0; i < g_tileSize * g_tileSize; i++) {
            g_clearTile[i] = 140 + (rand() % 4);  // Subtle noise
        }
    }

    g_terrainInitialized = true;
    fprintf(stderr, "Terrain: Loaded %d templates (%d tiles)\n",
            g_terrainTemplateCount, loadedTiles);
    return TRUE;
}

void Terrain_Shutdown(void) {
    for (int i = 0; i < g_terrainTemplateCount; i++) {
        if (g_terrainTmp[i]) {
            Tmp_Free(g_terrainTmp[i]);
            g_terrainTmp[i] = nullptr;
        }
    }
    g_terrainTemplateCount = 0;

    if (g_clearTile) {
        delete[] g_clearTile;
        g_clearTile = nullptr;
    }

    g_terrainInitialized = false;
}

BOOL Terrain_Available(void) {
    return g_terrainInitialized;
}

// Map TerrainType enum to terrain template index
// Template indices from g_terrainTemplates array:
//   0: clear1.sno, 1-2: water, 3-10: shore, 11-14: debris/rock
//   15-16: roads, 17-18: river, 19-20: bridge
static int GetTemplateForTerrain(int terrainType, int variant) {
    switch (terrainType) {
        case 0:  // TERRAIN_CLEAR
            return 0;  // clear1.sno
        case 1:  // TERRAIN_WATER
            return 1 + (variant % 2);  // w1.sno or w2.sno
        case 2:  // TERRAIN_ROCK
            return 11 + (variant % 4);  // d01-d04.sno (debris)
        case 3:  // TERRAIN_TREE
            return 0;  // Use clear (trees drawn separately)
        case 4:  // TERRAIN_ROAD
            return 15 + (variant % 2);  // s01-s02.sno
        case 5:  // TERRAIN_BRIDGE
            return 19 + (variant % 2);  // br1-br2.sno
        case 6:  // TERRAIN_BUILDING
            return 0;  // Use clear under buildings
        case 7:  // TERRAIN_ORE
            return 0;  // Use clear (ore drawn as overlay)
        case 8:  // TERRAIN_GEM
            return 0;  // Use clear (gems drawn as overlay)
        default:
            return 0;  // Default to clear
    }
}

BOOL Terrain_RenderTile(int terrainType, int variant,
                        int screenX, int screenY) {
    if (!g_terrainInitialized) return FALSE;

    const uint8_t* pixels = nullptr;
    int width = g_tileSize;
    int height = g_tileSize;

    // Map terrain type to appropriate template index
    int templateIdx = GetTemplateForTerrain(terrainType, variant);

    // Try to get tile from loaded templates
    if (templateIdx < g_terrainTemplateCount && g_terrainTmp[templateIdx]) {
        int tileCount = Tmp_GetTileCount(g_terrainTmp[templateIdx]);
        int tileIdx = variant % tileCount;
        const TmpTile* tile = Tmp_GetTile(g_terrainTmp[templateIdx], tileIdx);
        if (tile && tile->pixels) {
            pixels = tile->pixels;
            width = tile->width;
            height = tile->height;
        }
    }

    // Fallback to clear tile
    if (!pixels && g_clearTile) {
        pixels = g_clearTile;
    }

    if (!pixels) return FALSE;

    // Render the tile
    Renderer_Blit(pixels, width, height, screenX, screenY, FALSE);
    return TRUE;
}

int Terrain_GetTileSize(void) {
    return g_tileSize;
}

int Terrain_GetLoadedCount(void) {
    return g_terrainTemplateCount;
}

// Template ID to filename mapping (from OpenRA snow.yaml / temperat.yaml)
// Based on actual OpenRA YAML definitions for Red Alert tilesets
// Returns the template filename for a given ID, or nullptr if unknown
static const char* GetTemplateFilename(int templateID, const char* ext) {
    static char filename[32];

    // Clear terrain (255, 65535, 0)
    if (templateID == 255 || templateID == 0xFFFF || templateID == 0) {
        snprintf(filename, sizeof(filename), "clear1%s", ext);
        return filename;
    }

    // Water (1-2)
    if (templateID >= 1 && templateID <= 2) {
        snprintf(filename, sizeof(filename), "w%d%s", templateID, ext);
        return filename;
    }

    // Shore/Beach (3-58) → sh01.sno - sh56.sno
    if (templateID >= 3 && templateID <= 58) {
        snprintf(filename, sizeof(filename), "sh%02d%s", templateID - 2, ext);
        return filename;
    }

    // Water cliffs (59-96) → wc01.sno - wc38.sno
    if (templateID >= 59 && templateID <= 96) {
        snprintf(filename, sizeof(filename), "wc%02d%s", templateID - 58, ext);
        return filename;
    }

    // River (112-124) → rv01.sno - rv13.sno
    if (templateID >= 112 && templateID <= 124) {
        snprintf(filename, sizeof(filename), "rv%02d%s", templateID - 111, ext);
        return filename;
    }

    // Roads/slopes (135-172) → s01.sno - s38.sno
    if (templateID >= 135 && templateID <= 172) {
        snprintf(filename, sizeof(filename), "s%02d%s", templateID - 134, ext);
        return filename;
    }

    // Debris (173-215) → d01.sno - d43.sno
    if (templateID >= 173 && templateID <= 215) {
        snprintf(filename, sizeof(filename), "d%02d%s", templateID - 172, ext);
        return filename;
    }

    // Extra debris (227-228) → d44.sno - d45.sno
    if (templateID >= 227 && templateID <= 228) {
        snprintf(filename, sizeof(filename), "d%02d%s", templateID - 183, ext);
        return filename;
    }

    // River extras (229-230) → rv14.sno - rv15.sno
    if (templateID >= 229 && templateID <= 230) {
        snprintf(filename, sizeof(filename), "rv%02d%s", templateID - 215, ext);
        return filename;
    }

    // Road cliffs (231-234) → rc01.sno - rc04.sno
    if (templateID >= 231 && templateID <= 234) {
        snprintf(filename, sizeof(filename), "rc%02d%s", templateID - 230, ext);
        return filename;
    }

    // Bridge (235-244) → br1a.sno, br1b.sno, br1c.sno, br2a.sno, etc.
    if (templateID >= 235 && templateID <= 244) {
        int bridgeNum = (templateID - 235) / 3 + 1;  // 1 or 2
        char suffix = 'a' + ((templateID - 235) % 3);  // a, b, or c
        snprintf(filename, sizeof(filename),
                 "br%d%c%s", bridgeNum, suffix, ext);
        return filename;
    }

    // Unknown - use clear
    snprintf(filename, sizeof(filename), "clear1%s", ext);
    return filename;
}

// Template cache - stores loaded templates indexed by ID
#define MAX_CACHED_TEMPLATES 256
static TmpFileHandle g_templateCache[MAX_CACHED_TEMPLATES] = {nullptr};
static int g_currentTheater = 1;  // Default to snow

// Load a template by ID, caching for future use
static TmpFileHandle LoadTemplateByID(int templateID) {
    if (templateID < 0 || templateID >= MAX_CACHED_TEMPLATES) {
        templateID = 255;  // Default to clear
    }

    // Return cached if available
    if (g_templateCache[templateID]) {
        return g_templateCache[templateID];
    }

    // Get theater extension
    const char* ext = ".sno";  // Default to snow
    switch (g_currentTheater) {
        case 0: ext = ".tem"; break;  // Temperate
        case 1: ext = ".sno"; break;  // Snow
        case 2: ext = ".int"; break;  // Interior
        case 3: ext = ".des"; break;  // Desert
    }

    // Get filename and try to load
    const char* filename = GetTemplateFilename(templateID, ext);
    if (!filename) return nullptr;

    uint32_t size = 0;
    void* data = Assets_LoadTemplate(filename, &size);
    if (!data) {
        // Try uppercase
        char upper[32];
        for (int i = 0; filename[i] && i < 31; i++) {
            upper[i] = (filename[i] >= 'a' && filename[i] <= 'z')
                       ? filename[i] - 32 : filename[i];
            upper[i+1] = '\0';
        }
        data = Assets_LoadTemplate(upper, &size);
    }

    if (!data) return nullptr;

    TmpFileHandle tmp = Tmp_Load(data, size);
    free(data);

    if (tmp) {
        g_templateCache[templateID] = tmp;
    }

    return tmp;
}

void Terrain_SetTheater(int theater) {
    if (theater == g_currentTheater) return;

    // Clear template cache
    for (int i = 0; i < MAX_CACHED_TEMPLATES; i++) {
        if (g_templateCache[i]) {
            Tmp_Free(g_templateCache[i]);
            g_templateCache[i] = nullptr;
        }
    }

    g_currentTheater = theater;

    // Pre-load clear terrain template
    LoadTemplateByID(255);
}

BOOL Terrain_RenderByID(int templateID, int tileIndex,
                        int screenX, int screenY) {
    if (!g_terrainInitialized) {
        Terrain_Init();
    }

    // Handle clear/invalid as clear terrain
    if (templateID == 255 || templateID == 0xFFFF || templateID == 0) {
        templateID = 255;
    }

    // Load template (from cache or disk)
    TmpFileHandle tmp = LoadTemplateByID(templateID);
    if (!tmp) {
        // Fallback: try to render as clear
        tmp = LoadTemplateByID(255);
        if (!tmp && g_clearTile) {
            // Use procedural clear tile
            Renderer_Blit(g_clearTile, g_tileSize, g_tileSize,
                          screenX, screenY, FALSE);
            return TRUE;
        }
        if (!tmp) return FALSE;
    }

    // Get tile from template
    int tileCount = Tmp_GetTileCount(tmp);
    if (tileIndex < 0 || tileIndex >= tileCount) {
        tileIndex = 0;  // Default to first tile
    }

    const TmpTile* tile = Tmp_GetTile(tmp, tileIndex);
    if (!tile || !tile->pixels) {
        // Fallback to clear
        if (g_clearTile) {
            Renderer_Blit(g_clearTile, g_tileSize, g_tileSize,
                          screenX, screenY, FALSE);
            return TRUE;
        }
        return FALSE;
    }

    // Render the tile
    Renderer_Blit(tile->pixels, tile->width, tile->height,
                  screenX, screenY, FALSE);
    return TRUE;
}
