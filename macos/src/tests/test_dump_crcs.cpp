/**
 * Dump all CRCs from conquer.mix to understand what's there
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <vector>
#include <algorithm>

// All known SHP filenames from OpenRA sequences
const char* knownNames[] = {
    // Vehicles
    "mcv.shp", "truk.shp", "harv.shp", "harvempty.shp", "harvhalf.shp",
    "1tnk.shp", "2tnk.shp", "3tnk.shp", "4tnk.shp",
    "v2rl.shp", "arty.shp", "jeep.shp", "apc.shp", "mnly.shp",
    "mrj.shp", "mgg.shp", "ttnk.shp", "ftrk.shp", "dtrk.shp",
    "ctnk.shp", "qtnk.shp", "stnk.shp",
    // Ships
    "pt.shp", "dd.shp", "ca.shp", "ss.shp", "msub.shp", "lst.shp",
    // Aircraft
    "heli.shp", "orca.shp", "hind.shp", "tran.shp", "mig.shp",
    "yak.shp", "badr.shp", "u2.shp",
    // Buildings  
    "fact.shp", "nuke.shp", "powr.shp", "apwr.shp", "proc.shp",
    "silo.shp", "silo2.shp", "tent.shp", "barr.shp", "weap.shp",
    "fix.shp", "dome.shp", "atek.shp", "stek.shp", "pbox.shp",
    "hbox.shp", "gun.shp", "agun.shp", "sam.shp", "tsla.shp",
    "gap.shp", "iron.shp", "pdox.shp", "mslo.shp", "afld.shp",
    "spen.shp", "syrd.shp", "ftur.shp", "kenn.shp", "fcom.shp",
    // Infantry
    "e1.shp", "e2.shp", "e3.shp", "e4.shp", "e6.shp", "e7.shp",
    "spy.shp", "thf.shp", "medi.shp", "dog.shp", "mech.shp",
    "tany.shp", "shok.shp",
    // Walls
    "brik.shp", "sbag.shp", "barb.shp", "wood.shp", "cycl.shp",
    // Misc  
    "v2.shp", "turr.shp", "ssam.shp", "minigun.shp", "gunfire.shp",
    "gunfire2.shp", "smoke.shp", "fire1.shp", "fire2.shp", "fire3.shp",
    "bomblet.shp", "atom.shp", "frag1.shp", "fball1.shp",
    "oildrm.shp", "oilb.shp", "bio.shp", "hosp.shp",
    // Make files
    "factmake.shp", "procmake.shp", "powrmake.shp", "weapmake.shp",
    "tentmake.shp", "barrmake.shp", "fixmake.shp", "domemake.shp",
    nullptr
};

int main() {
    printf("=== Dumping CRCs from conquer.mix ===\n\n");

    MixFileHandle mix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix");
    if (!mix) {
        printf("ERROR: Cannot open conquer.mix\n");
        return 1;
    }
    
    printf("conquer.mix has %d files\n\n", Mix_GetFileCount(mix));
    
    // Build hash of known CRCs
    printf("Checking known filenames:\n");
    int found = 0;
    for (int i = 0; knownNames[i]; i++) {
        if (Mix_FileExists(mix, knownNames[i])) {
            uint32_t crc = Mix_CalculateCRC(knownNames[i]);
            printf("  %-20s CRC 0x%08X  %6u bytes\n", 
                   knownNames[i], crc, Mix_GetFileSize(mix, knownNames[i]));
            found++;
        }
    }
    
    printf("\nFound %d out of %d known names\n", found, Mix_GetFileCount(mix));
    printf("Missing %d files with unknown CRCs\n", Mix_GetFileCount(mix) - found);

    Mix_Close(mix);
    return 0;
}
