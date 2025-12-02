/**
 * Test: Search for sprite CRCs in MIX files
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstring>
#include <cctype>

// List of sprite names to search for
const char* sprites[] = {
    // Units
    "HARV.SHP", "harv.shp",
    "MCV.SHP", "mcv.shp",
    "APC.SHP", "apc.shp",
    "ARTY.SHP", "arty.shp",
    "V2RL.SHP", "v2rl.shp",
    "TRUK.SHP", "truk.shp",
    "JEEP.SHP", "jeep.shp",
    "TTNK.SHP", "ttnk.shp",  // Tesla tank
    "4TNK.SHP", "4tnk.shp",  // Mammoth tank
    "STNK.SHP", "stnk.shp",  // Stealth tank
    "MSUB.SHP", "msub.shp",  // Missile sub
    "HELI.SHP", "heli.shp",
    "ORCA.SHP", "orca.shp",
    "HIND.SHP", "hind.shp",
    "MIG.SHP", "mig.shp",
    "YAK.SHP", "yak.shp",
    "TRAN.SHP", "tran.shp",  // Transport heli
    // Buildings
    "FACT.SHP", "fact.shp",
    "POWR.SHP", "powr.shp",
    "APWR.SHP", "apwr.shp",  // Advanced power
    "TENT.SHP", "tent.shp",  // Allied barracks
    "BARR.SHP", "barr.shp",  // Soviet barracks
    "WEAP.SHP", "weap.shp",
    "PBOX.SHP", "pbox.shp",  // Pillbox
    "HBOX.SHP", "hbox.shp",  // Camouflaged pillbox
    "GUN.SHP", "gun.shp",    // Turret
    "AGUN.SHP", "agun.shp",  // AA gun
    "SAM.SHP", "sam.shp",
    "GAP.SHP", "gap.shp",    // Gap generator
    "IRON.SHP", "iron.shp",  // Iron curtain
    "TESLA.SHP", "tesla.shp",
    "TSLA.SHP", "tsla.shp",  // Tesla coil
    "FCOM.SHP", "fcom.shp",  // Forward command
    "AFLD.SHP", "afld.shp",  // Airfield
    "SPEN.SHP", "spen.shp",  // Sub pen
    "SYRD.SHP", "syrd.shp",  // Ship yard
    "SILO.SHP", "silo.shp",  // Ore silo
    "FTUR.SHP", "ftur.shp",  // Flame tower
    "KENN.SHP", "kenn.shp",  // Kennel (dogs)
    "FIX.SHP", "fix.shp",    // Repair bay
    "BIO.SHP", "bio.shp",    // Tech center
    "MISS.SHP", "miss.shp",  // Missile silo
    "MINP.SHP", "minp.shp",  // Mine layer
    "MINV.SHP", "minv.shp",  // Mine layer
    // Walls/fences
    "SBAG.SHP", "sbag.shp",  // Sandbags
    "CYCL.SHP", "cycl.shp",  // Chain link
    "BRIK.SHP", "brik.shp",  // Concrete wall
    "BARB.SHP", "barb.shp",  // Barb wire
    "WOOD.SHP", "wood.shp",  // Wood fence
    nullptr
};

int main() {
    printf("=== Searching for sprites in MIX files ===\n\n");

    MixFileHandle conquerMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix");
    MixFileHandle hiresMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/hires.mix");

    if (conquerMix) printf("Opened conquer.mix (%d files)\n", Mix_GetFileCount(conquerMix));
    if (hiresMix) printf("Opened hires.mix (%d files)\n", Mix_GetFileCount(hiresMix));

    printf("\nSearching for sprites:\n");
    for (int i = 0; sprites[i]; i++) {
        bool foundConquer = conquerMix && Mix_FileExists(conquerMix, sprites[i]);
        bool foundHires = hiresMix && Mix_FileExists(hiresMix, sprites[i]);

        if (foundConquer) {
            printf("  %-12s FOUND in conquer.mix (%u bytes)\n", sprites[i],
                   Mix_GetFileSize(conquerMix, sprites[i]));
        }
        if (foundHires) {
            printf("  %-12s FOUND in hires.mix (%u bytes)\n", sprites[i],
                   Mix_GetFileSize(hiresMix, sprites[i]));
        }
    }

    // Print CRCs for missing files
    printf("\nCRCs for missing files:\n");
    const char* missingNames[] = {"HARV.SHP", "APC.SHP", "FACT.SHP", "POWR.SHP", "TENT.SHP", "WEAP.SHP", "PBOX.SHP", "SAM.SHP", "ARTY.SHP", nullptr};
    for (int i = 0; missingNames[i]; i++) {
        uint32_t crc = Mix_CalculateCRC(missingNames[i]);
        printf("  %s -> CRC 0x%08X\n", missingNames[i], crc);
    }

    if (conquerMix) Mix_Close(conquerMix);
    if (hiresMix) Mix_Close(hiresMix);

    return 0;
}
