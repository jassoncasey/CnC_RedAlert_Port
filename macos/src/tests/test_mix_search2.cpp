/**
 * Test: Search for actual sprite CRCs in MIX files
 */
#include "assets/mixfile.h"
#include <cstdio>

// Complete list of sprites from OpenRA sequences - all lowercase
const char* sprites[] = {
    // Vehicles
    "mcv.shp", "truk.shp", "harv.shp", "harvempty.shp", "harvhalf.shp",
    "1tnk.shp", "2tnk.shp", "3tnk.shp", "4tnk.shp",
    "v2rl.shp", "arty.shp", "jeep.shp", "apc.shp",
    "mnly.shp", "mrj.shp", "mgg.shp", "ttnk.shp", 
    "ftrk.shp", "dtrk.shp", "ctnk.shp", "qtnk.shp", "stnk.shp",
    // Ships
    "pt.shp", "dd.shp", "ca.shp", "ss.shp", "msub.shp", "lst.shp",
    // Aircraft  
    "heli.shp", "orca.shp", "hind.shp", "tran.shp",
    "mig.shp", "yak.shp", "badr.shp", "u2.shp",
    // Buildings
    "fact.shp", "nuke.shp", "powr.shp", "apwr.shp",
    "proc.shp", "silo.shp", "tent.shp", "barr.shp",
    "weap.shp", "fix.shp", "dome.shp", "atek.shp", "stek.shp",
    "pbox.shp", "hbox.shp", "gun.shp", "agun.shp",
    "sam.shp", "tsla.shp", "gap.shp", "iron.shp",
    "pdox.shp", "mslo.shp", "afld.shp", "spen.shp", "syrd.shp",
    "ftur.shp", "kenn.shp", "fcom.shp", "brik.shp", "sbag.shp",
    "barb.shp", "wood.shp", "cycl.shp",
    // Infantry (usually in hires.mix)
    "e1.shp", "e2.shp", "e3.shp", "e4.shp", "e6.shp", "e7.shp",
    "spy.shp", "thf.shp", "medi.shp", "dog.shp",
    "tany.shp", "shok.shp", "c1.shp", "c2.shp", "c3.shp",
    "chan.shp", "delphi.shp", "gnrl.shp", "einstein.shp",
    nullptr
};

int main() {
    printf("=== Searching for sprites in MIX files ===\n\n");

    MixFileHandle conquerMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix");
    MixFileHandle hiresMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/hires.mix");

    int foundConquer = 0, foundHires = 0, missing = 0;
    
    printf("In conquer.mix:\n");
    for (int i = 0; sprites[i]; i++) {
        if (conquerMix && Mix_FileExists(conquerMix, sprites[i])) {
            printf("  %-16s %6u bytes\n", sprites[i], Mix_GetFileSize(conquerMix, sprites[i]));
            foundConquer++;
        }
    }
    
    printf("\nIn hires.mix:\n");
    for (int i = 0; sprites[i]; i++) {
        if (hiresMix && Mix_FileExists(hiresMix, sprites[i])) {
            printf("  %-16s %6u bytes\n", sprites[i], Mix_GetFileSize(hiresMix, sprites[i]));
            foundHires++;
        }
    }
    
    printf("\nMISSING from both:\n");
    for (int i = 0; sprites[i]; i++) {
        bool found = (conquerMix && Mix_FileExists(conquerMix, sprites[i])) ||
                     (hiresMix && Mix_FileExists(hiresMix, sprites[i]));
        if (!found) {
            printf("  %s (CRC: 0x%08X)\n", sprites[i], Mix_CalculateCRC(sprites[i]));
            missing++;
        }
    }

    printf("\n=== Summary ===\n");
    printf("Found in conquer.mix: %d\n", foundConquer);
    printf("Found in hires.mix: %d\n", foundHires);
    printf("Missing from both: %d\n", missing);

    if (conquerMix) Mix_Close(conquerMix);
    if (hiresMix) Mix_Close(hiresMix);

    return 0;
}
