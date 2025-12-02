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
// Order MUST match UnitType enum in units.h
static const char* g_unitSpriteNames[UNIT_TYPE_COUNT] = {
    nullptr,        // UNIT_NONE
    // Infantry - Military
    "E1.SHP",       // UNIT_RIFLE - Rifle infantry
    "E2.SHP",       // UNIT_GRENADIER - Grenade soldier
    "E3.SHP",       // UNIT_ROCKET - Rocket soldier
    "E4.SHP",       // UNIT_FLAMETHROWER - Flamethrower infantry
    "E6.SHP",       // UNIT_ENGINEER
    "E7.SHP",       // UNIT_TANYA
    "DOG.SHP",      // UNIT_DOG - Attack dog
    "SPY.SHP",      // UNIT_SPY
    "MEDI.SHP",     // UNIT_MEDIC
    "THF.SHP",      // UNIT_THIEF
    "SHOK.SHP",     // UNIT_SHOCK - Shock trooper
    "GNRL.SHP",     // UNIT_GENERAL
    // Infantry - Civilians
    "C1.SHP",       // UNIT_CIVILIAN_1
    "C2.SHP",       // UNIT_CIVILIAN_2
    "C3.SHP",       // UNIT_CIVILIAN_3
    "C4.SHP",       // UNIT_CIVILIAN_4
    "C5.SHP",       // UNIT_CIVILIAN_5
    "C6.SHP",       // UNIT_CIVILIAN_6
    "C7.SHP",       // UNIT_CIVILIAN_7 - Technician
    "C8.SHP",       // UNIT_CIVILIAN_8 - Einstein
    "C9.SHP",       // UNIT_CIVILIAN_9
    "C10.SHP",      // UNIT_CIVILIAN_10
    "CHAN.SHP",     // UNIT_CHAN
    // Vehicles
    "HARV.SHP",     // UNIT_HARVESTER
    "1TNK.SHP",     // UNIT_TANK_LIGHT - Light tank
    "2TNK.SHP",     // UNIT_TANK_MEDIUM - Medium tank
    "3TNK.SHP",     // UNIT_TANK_HEAVY - Heavy tank
    "4TNK.SHP",     // UNIT_TANK_MAMMOTH - Mammoth tank
    "APC.SHP",      // UNIT_APC
    "ARTY.SHP",     // UNIT_ARTILLERY
    "JEEP.SHP",     // UNIT_JEEP
    "MCV.SHP",      // UNIT_MCV
    "V2RL.SHP",     // UNIT_V2RL
    "MNLY.SHP",     // UNIT_MINELAYER
    "TRUK.SHP",     // UNIT_TRUCK
    "CTNK.SHP",     // UNIT_CHRONO - Chrono tank
    "MGG.SHP",      // UNIT_MOBILE_GAP
    "MRJ.SHP",      // UNIT_MOBILE_RADAR
    // Naval
    "GNBT.SHP",     // UNIT_GUNBOAT - Gunboat
    "DD.SHP",       // UNIT_DESTROYER
    "SS.SHP",       // UNIT_SUBMARINE
    "CA.SHP",       // UNIT_CRUISER
    "LST.SHP",      // UNIT_TRANSPORT
    "PT.SHP",       // UNIT_PT_BOAT
    // Aircraft
    "HIND.SHP",     // UNIT_HIND
    "HELI.SHP",     // UNIT_LONGBOW
    "TRAN.SHP",     // UNIT_CHINOOK
    "YAK.SHP",      // UNIT_YAK
    "MIG.SHP",      // UNIT_MIG
};

// Building type to SHP filename mapping
// Buildings are in CONQUER.MIX
// Order MUST match BuildingType enum in units.h
static const char* g_buildingSpriteNames[BUILDING_TYPE_COUNT] = {
    nullptr,        // BUILDING_NONE
    // Core structures
    "FACT.SHP",     // BUILDING_CONSTRUCTION - Construction yard
    "POWR.SHP",     // BUILDING_POWER - Power plant
    "APWR.SHP",     // BUILDING_ADV_POWER - Advanced power plant
    "PROC.SHP",     // BUILDING_REFINERY - Ore refinery
    "SILO.SHP",     // BUILDING_SILO - Ore silo
    // Production
    "TENT.SHP",     // BUILDING_BARRACKS - Allied barracks (BARR for Soviet)
    "WEAP.SHP",     // BUILDING_FACTORY - War factory
    "AFLD.SHP",     // BUILDING_AIRFIELD
    "HPAD.SHP",     // BUILDING_HELIPAD
    "SYRD.SHP",     // BUILDING_SHIPYARD
    "SPEN.SHP",     // BUILDING_SUB_PEN
    // Tech
    "DOME.SHP",     // BUILDING_RADAR - Radar dome
    "ATEK.SHP",     // BUILDING_TECH_CENTER (STEK for Soviet)
    "KENN.SHP",     // BUILDING_KENNEL
    "BIO.SHP",      // BUILDING_BIO_LAB
    "FCOM.SHP",     // BUILDING_FORWARD_COM
    "MISS.SHP",     // BUILDING_MISSION
    // Defense
    "GUN.SHP",      // BUILDING_TURRET
    "SAM.SHP",      // BUILDING_SAM
    "TSLA.SHP",     // BUILDING_TESLA
    "AGUN.SHP",     // BUILDING_AA_GUN
    "PBOX.SHP",     // BUILDING_PILLBOX
    "HBOX.SHP",     // BUILDING_CAMO_PILLBOX
    "FTUR.SHP",     // BUILDING_FLAME_TOWER
    "GAP.SHP",      // BUILDING_GAP
    "MINP.SHP",     // BUILDING_MINE_AP
    "MINV.SHP",     // BUILDING_MINE_AV
    // Special
    "FIX.SHP",      // BUILDING_FIX
    "IRON.SHP",     // BUILDING_IRON_CURTAIN
    "PDOX.SHP",     // BUILDING_CHRONOSPHERE
    "MSLO.SHP",     // BUILDING_MISSILE_SILO
    // Fake structures
    "FACF.SHP",     // BUILDING_FAKE_CONST
    "WEAF.SHP",     // BUILDING_FAKE_FACTORY
    "SYRF.SHP",     // BUILDING_FAKE_SHIPYARD
    "DOMF.SHP",     // BUILDING_FAKE_RADAR
    // Props
    "BARL.SHP",     // BUILDING_BARREL
    "BRL3.SHP",     // BUILDING_BARREL_3
    // Civilian buildings
    "V01.SHP",      // BUILDING_CIV_01 - Church
    "V02.SHP",      // BUILDING_CIV_02 - Han's house
    "V03.SHP",      // BUILDING_CIV_03 - Hewitt house
    "V04.SHP",      // BUILDING_CIV_04 - Ricktor house
    "V05.SHP",      // BUILDING_CIV_05 - Gretchin house
    "V06.SHP",      // BUILDING_CIV_06 - Barn
    "V07.SHP",      // BUILDING_CIV_07 - Windmill
    "V08.SHP",      // BUILDING_CIV_08 - Fenced house
    "V09.SHP",      // BUILDING_CIV_09 - Church 2
    "V10.SHP",      // BUILDING_CIV_10 - Hospital
    "V11.SHP",      // BUILDING_CIV_11 - Grain silo
    "V13.SHP",      // BUILDING_CIV_13 - Water tower
    "V19.SHP",      // BUILDING_CIV_19 - Oil derrick
};

// Loaded sprites
static ShpFileHandle g_unitSprites[UNIT_TYPE_COUNT] = {nullptr};
static ShpFileHandle g_buildingSprites[BUILDING_TYPE_COUNT] = {nullptr};
static bool g_spritesInitialized = false;
static int g_spritesLoaded = 0;

// Infantry have 8 facings with multiple frames per facing
// Vehicles have 32 facings (8 base * animation)
static int GetUnitFrameForFacing(UnitType type, int facing, int animFrame) {
    // Infantry types: UNIT_RIFLE through UNIT_CHAN (all infantry and civilians)
    bool isInfantry = (type >= UNIT_RIFLE && type <= UNIT_CHAN);
    if (isInfantry) {
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
                int frames = Shp_GetFrameCount(g_unitSprites[i]);
                fprintf(stderr, "  Loaded %s (%d frames)\n",
                       g_unitSpriteNames[i], frames);
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
                int frames = Shp_GetFrameCount(g_buildingSprites[i]);
                fprintf(stderr, "  Loaded %s (%d frames)\n",
                       g_buildingSpriteNames[i], frames);
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
