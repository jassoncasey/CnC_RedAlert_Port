/**
 * Red Alert macOS Port - Sprite Manager Implementation
 */

#include "sprites.h"
#include "assets/assetloader.h"
#include "assets/shpfile.h"
#include "graphics/metal/renderer.h"
#include <cstdio>
#include <cstring>

// Unit type to SHP filename mapping
// Names from Red Alert original data files (CONQUER.MIX and HIRES.MIX)
static const char* g_unitSpriteNames[UNIT_TYPE_COUNT] = {
    nullptr,        // UNIT_NONE
    "E1.SHP",       // UNIT_RIFLE - Rifle infantry
    "E2.SHP",       // UNIT_GRENADIER - Grenade soldier
    "E3.SHP",       // UNIT_ROCKET - Rocket soldier
    "E7.SHP",       // UNIT_ENGINEER
    "HARV.SHP",     // UNIT_HARVESTER - may also try MCV.SHP
    "1TNK.SHP",     // UNIT_TANK_LIGHT - Light tank
    "2TNK.SHP",     // UNIT_TANK_MEDIUM - Medium tank
    "3TNK.SHP",     // UNIT_TANK_HEAVY - Heavy tank (Soviet)
    "APC.SHP",      // UNIT_APC - or try TRUK.SHP
    "ARTY.SHP",     // UNIT_ARTILLERY - or try V2RL.SHP for V2 launcher
    "PT.SHP",       // UNIT_GUNBOAT - Patrol boat
    "DD.SHP",       // UNIT_DESTROYER
};

// Building type to SHP filename mapping
// Buildings are in CONQUER.MIX
static const char* g_buildingSpriteNames[BUILDING_TYPE_COUNT] = {
    nullptr,        // BUILDING_NONE
    "FACT.SHP",     // BUILDING_CONSTRUCTION - Construction yard
    "POWR.SHP",     // BUILDING_POWER - Power plant
    "PROC.SHP",     // BUILDING_REFINERY - Ore refinery/processor
    "TENT.SHP",     // BUILDING_BARRACKS - Allied barracks (BARR for Soviet)
    "WEAP.SHP",     // BUILDING_FACTORY - War factory
    "DOME.SHP",     // BUILDING_RADAR - Radar dome
    "PBOX.SHP",     // BUILDING_TURRET - Pillbox (GUN might be different)
    "SAM.SHP",      // BUILDING_SAM - SAM site
};

// Loaded sprites
static ShpFileHandle g_unitSprites[UNIT_TYPE_COUNT] = {nullptr};
static ShpFileHandle g_buildingSprites[BUILDING_TYPE_COUNT] = {nullptr};
static bool g_spritesInitialized = false;
static int g_spritesLoaded = 0;

// Infantry have 8 facings with multiple frames per facing
// Vehicles have 32 facings (8 base * animation)
static int GetUnitFrameForFacing(UnitType type, int facing, int animFrame) {
    if (type >= UNIT_RIFLE && type <= UNIT_ENGINEER) {
        // Infantry: 8 facings, multiple animation frames per facing
        // Typical infantry layout: stand, walk cycles per facing
        // Each facing has ~16 frames (stand + walk cycle)
        int framesPerFacing = 16;
        return (facing * framesPerFacing) + (animFrame % framesPerFacing);
    } else {
        // Vehicles: 32 facings for smooth rotation
        // Map 8 game facings to 32 sprite facings (multiply by 4)
        return facing * 4;
    }
}

BOOL Sprites_Init(void) {
    if (g_spritesInitialized) return TRUE;

    fprintf(stderr, "Sprites: Loading unit sprites...\n");

    // Load unit sprites
    for (int i = 1; i < UNIT_TYPE_COUNT; i++) {
        if (g_unitSpriteNames[i]) {
            g_unitSprites[i] = Assets_LoadSHP(g_unitSpriteNames[i]);
            if (g_unitSprites[i]) {
                g_spritesLoaded++;
                fprintf(stderr, "  Loaded %s (%d frames)\n",
                       g_unitSpriteNames[i], Shp_GetFrameCount(g_unitSprites[i]));
            } else {
                fprintf(stderr, "  MISSING: %s\n", g_unitSpriteNames[i]);
            }
        }
    }

    // Load building sprites
    fprintf(stderr, "Sprites: Loading building sprites...\n");
    for (int i = 1; i < BUILDING_TYPE_COUNT; i++) {
        if (g_buildingSpriteNames[i]) {
            g_buildingSprites[i] = Assets_LoadSHP(g_buildingSpriteNames[i]);
            if (g_buildingSprites[i]) {
                g_spritesLoaded++;
                fprintf(stderr, "  Loaded %s (%d frames)\n",
                       g_buildingSpriteNames[i], Shp_GetFrameCount(g_buildingSprites[i]));
            } else {
                fprintf(stderr, "  MISSING: %s\n", g_buildingSpriteNames[i]);
            }
        }
    }

    g_spritesInitialized = true;
    fprintf(stderr, "Sprites: Loaded %d sprites total\n", g_spritesLoaded);
    return TRUE; // Return TRUE even with 0 sprites (fallback rendering works)
}

void Sprites_Shutdown(void) {
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        if (g_unitSprites[i]) {
            Shp_Free(g_unitSprites[i]);
            g_unitSprites[i] = nullptr;
        }
    }

    for (int i = 0; i < BUILDING_TYPE_COUNT; i++) {
        if (g_buildingSprites[i]) {
            Shp_Free(g_buildingSprites[i]);
            g_buildingSprites[i] = nullptr;
        }
    }

    g_spritesInitialized = false;
    g_spritesLoaded = 0;
}

BOOL Sprites_Available(void) {
    return g_spritesInitialized && g_spritesLoaded > 0;
}

BOOL Sprites_RenderUnit(UnitType type, int facing, int frame,
                        int screenX, int screenY, uint8_t teamColor) {
    (void)teamColor; // TODO: Color remapping

    if (type <= UNIT_NONE || type >= UNIT_TYPE_COUNT) return FALSE;

    ShpFileHandle shp = g_unitSprites[type];
    if (!shp) return FALSE;

    int frameCount = Shp_GetFrameCount(shp);
    if (frameCount == 0) return FALSE;

    // Calculate frame index based on facing and animation
    int frameIndex = GetUnitFrameForFacing(type, facing, frame);

    // Clamp to valid range
    if (frameIndex >= frameCount) {
        frameIndex = frameIndex % frameCount;
    }

    const ShpFrame* shpFrame = Shp_GetFrame(shp, frameIndex);
    if (!shpFrame || !shpFrame->pixels) return FALSE;

    // Render centered on position
    Renderer_BlitSprite(shpFrame->pixels, shpFrame->width, shpFrame->height,
                        screenX, screenY,
                        shpFrame->width / 2, shpFrame->height / 2,
                        TRUE);

    return TRUE;
}

BOOL Sprites_RenderBuilding(BuildingType type, int frame,
                            int screenX, int screenY, uint8_t teamColor) {
    (void)teamColor; // TODO: Color remapping

    if (type <= BUILDING_NONE || type >= BUILDING_TYPE_COUNT) return FALSE;

    ShpFileHandle shp = g_buildingSprites[type];
    if (!shp) return FALSE;

    int frameCount = Shp_GetFrameCount(shp);
    if (frameCount == 0) return FALSE;

    // Clamp frame to valid range
    int frameIndex = frame % frameCount;

    const ShpFrame* shpFrame = Shp_GetFrame(shp, frameIndex);
    if (!shpFrame || !shpFrame->pixels) return FALSE;

    // Render at top-left position (buildings are not centered)
    Renderer_Blit(shpFrame->pixels, shpFrame->width, shpFrame->height,
                  screenX, screenY, TRUE);

    return TRUE;
}

int Sprites_GetUnitFrameCount(UnitType type) {
    if (type <= UNIT_NONE || type >= UNIT_TYPE_COUNT) return 0;
    if (!g_unitSprites[type]) return 0;
    return Shp_GetFrameCount(g_unitSprites[type]);
}

int Sprites_GetBuildingFrameCount(BuildingType type) {
    if (type <= BUILDING_NONE || type >= BUILDING_TYPE_COUNT) return 0;
    if (!g_buildingSprites[type]) return 0;
    return Shp_GetFrameCount(g_buildingSprites[type]);
}
