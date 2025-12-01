/**
 * Test Asset Pipeline
 *
 * Comprehensive test of loading game assets from MIX archives.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "assets/mixfile.h"
#include "assets/shpfile.h"
#include "assets/palfile.h"

// Known filenames from the original Red Alert game
// We'll compute CRCs and search for these
static const char* KNOWN_FILES[] = {
    // Palettes
    "TEMPERAT.PAL", "SNOW.PAL", "INTERIOR.PAL", "DESERT.PAL",
    "UNITTEM.PAL", "UNITSNO.PAL", "UNITINT.PAL", "UNITDES.PAL",
    // Infantry
    "E1.SHP", "E2.SHP", "E3.SHP", "E4.SHP", "E5.SHP", "E6.SHP", "E7.SHP",
    "SPY.SHP", "THF.SHP", "MEDI.SHP", "MECH.SHP", "DOG.SHP",
    // Vehicles
    "1TNK.SHP", "2TNK.SHP", "3TNK.SHP", "4TNK.SHP",
    "HARV.SHP", "MCV.SHP", "MNLY.SHP", "JEEP.SHP", "APC.SHP",
    "ARTY.SHP", "V2RL.SHP", "MRLS.SHP", "MGG.SHP", "TRUK.SHP",
    // Aircraft
    "HELI.SHP", "HIND.SHP", "MIG.SHP", "YAK.SHP", "TRAN.SHP", "BADR.SHP",
    // Naval
    "SS.SHP", "DD.SHP", "CA.SHP", "LST.SHP", "PT.SHP", "MSUB.SHP",
    // Buildings
    "FACT.SHP", "POWR.SHP", "APWR.SHP", "PROC.SHP", "SILO.SHP",
    "WEAP.SHP", "AGUN.SHP", "SAM.SHP", "DOME.SHP", "PBOX.SHP",
    "HPAD.SHP", "ATEK.SHP", "STEK.SHP", "IRON.SHP", "PDOX.SHP",
    "BARR.SHP", "TENT.SHP", "KENN.SHP", "FIX.SHP", "SPEN.SHP",
    // INI files
    "RULES.INI", "AI.INI", "TUTORIAL.INI", "ART.INI", "SOUND.INI",
    nullptr
};

// Archive search paths
struct ArchivePath {
    const char* path;
    const char* description;
};

static ArchivePath ARCHIVES[] = {
    {"../../assets/REDALERT.MIX", "REDALERT.MIX (installed)"},
    {"../../assets/MAIN_ALLIED.MIX", "MAIN_ALLIED.MIX (CD1)"},
    {"../../assets/MAIN_SOVIET.MIX", "MAIN_SOVIET.MIX (CD2)"},
    {"../../assets/SETUP.MIX", "SETUP.MIX (installed)"},
    {"../../assets/AUD.MIX", "AUD.MIX (installed)"},
    {"/Volumes/CD1/MAIN.MIX", "MAIN.MIX (CD1 mounted)"},
    {"/Volumes/CD1/SETUP/SETUP.MIX", "SETUP.MIX (CD1 mounted)"},
    {"/Volumes/CD1/SETUP/AUD.MIX", "AUD.MIX (CD1 mounted)"},
    {"/Volumes/CD1/INSTALL/REDALERT.MIX", "REDALERT.MIX (CD1 mounted)"},
    {nullptr, nullptr}
};

// Nested archives to search
static const char* NESTED_ARCHIVES[] = {
    "LOCAL.MIX", "HIRES.MIX", "LORES.MIX", "NCHIRES.MIX",
    "CONQUER.MIX", "GENERAL.MIX",
    "TEMPERAT.MIX", "SNOW.MIX", "INTERIOR.MIX",  // Theater MIXes
    "DESERT.MIX", "JUNGLE.MIX", "WINTER.MIX",
    "SPEECH.MIX", "SOUNDS.MIX",
    "ALLIES.MIX", "RUSSIAN.MIX",
    nullptr
};

struct FileLocation {
    const char* filename;
    const char* archive;
    const char* nested;
    uint32_t size;
};

#define MAX_FOUND 500
static FileLocation g_found[MAX_FOUND];
static int g_foundCount = 0;

void RecordFound(const char* filename, const char* archive, const char* nested, uint32_t size) {
    if (g_foundCount < MAX_FOUND) {
        g_found[g_foundCount].filename = filename;
        g_found[g_foundCount].archive = archive;
        g_found[g_foundCount].nested = nested;
        g_found[g_foundCount].size = size;
        g_foundCount++;
    }
}

void SearchInMix(MixFileHandle mix, const char* archiveName, const char* nestedName) {
    if (!mix) return;

    // Search for known files
    for (int i = 0; KNOWN_FILES[i]; i++) {
        if (Mix_FileExists(mix, KNOWN_FILES[i])) {
            uint32_t size = Mix_GetFileSize(mix, KNOWN_FILES[i]);
            RecordFound(KNOWN_FILES[i], archiveName, nestedName, size);
        }
    }

    // Try nested archives
    if (!nestedName) {  // Only recurse one level
        for (int i = 0; NESTED_ARCHIVES[i]; i++) {
            if (Mix_FileExists(mix, NESTED_ARCHIVES[i])) {
                uint32_t size = 0;
                void* data = Mix_AllocReadFile(mix, NESTED_ARCHIVES[i], &size);
                if (data) {
                    MixFileHandle nested = Mix_OpenMemory(data, size, TRUE);
                    if (nested) {
                        SearchInMix(nested, archiveName, NESTED_ARCHIVES[i]);
                        Mix_Close(nested);
                    } else {
                        free(data);
                    }
                }
            }
        }
    }
}

int main() {
    printf("===========================================\n");
    printf("Red Alert Asset Pipeline Test\n");
    printf("===========================================\n\n");

    // Search all archives
    for (int i = 0; ARCHIVES[i].path; i++) {
        printf("Scanning: %s\n", ARCHIVES[i].description);
        MixFileHandle mix = Mix_Open(ARCHIVES[i].path);
        if (mix) {
            printf("  Opened with %d files\n", Mix_GetFileCount(mix));
            SearchInMix(mix, ARCHIVES[i].path, nullptr);
            Mix_Close(mix);
        } else {
            printf("  (not found)\n");
        }
    }

    // Print results by category
    printf("\n===========================================\n");
    printf("Files Found: %d\n", g_foundCount);
    printf("===========================================\n\n");

    printf("--- Palettes ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        if (strstr(g_found[i].filename, ".PAL")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   g_found[i].filename, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    printf("\n--- Infantry ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        const char* name = g_found[i].filename;
        if ((name[0] == 'E' && name[1] >= '1' && name[1] <= '9') ||
            strstr(name, "SPY") || strstr(name, "THF") || strstr(name, "MEDI") ||
            strstr(name, "MECH") || strstr(name, "DOG")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   name, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    printf("\n--- Vehicles ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        const char* name = g_found[i].filename;
        if (strstr(name, "TNK") || strstr(name, "HARV") || strstr(name, "MCV") ||
            strstr(name, "APC") || strstr(name, "JEEP") || strstr(name, "ARTY") ||
            strstr(name, "V2RL") || strstr(name, "MRLS") || strstr(name, "TRUK")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   name, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    printf("\n--- Aircraft ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        const char* name = g_found[i].filename;
        if (strstr(name, "HELI") || strstr(name, "HIND") || strstr(name, "MIG") ||
            strstr(name, "YAK") || strstr(name, "TRAN") || strstr(name, "BADR")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   name, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    printf("\n--- Buildings ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        const char* name = g_found[i].filename;
        if (strstr(name, "FACT") || strstr(name, "POWR") || strstr(name, "PROC") ||
            strstr(name, "WEAP") || strstr(name, "SILO") || strstr(name, "AGUN") ||
            strstr(name, "SAM") || strstr(name, "DOME") || strstr(name, "BARR") ||
            strstr(name, "TENT") || strstr(name, "HPAD")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   name, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    printf("\n--- INI Files ---\n");
    for (int i = 0; i < g_foundCount; i++) {
        if (strstr(g_found[i].filename, ".INI")) {
            printf("  %-20s %8u bytes  %s%s%s\n",
                   g_found[i].filename, g_found[i].size,
                   g_found[i].archive,
                   g_found[i].nested ? " -> " : "",
                   g_found[i].nested ? g_found[i].nested : "");
        }
    }

    // Test: Load a palette and sprite if found
    printf("\n===========================================\n");
    printf("Loading Test Assets\n");
    printf("===========================================\n\n");

    // Find a palette to test
    for (int i = 0; i < g_foundCount; i++) {
        if (strcmp(g_found[i].filename, "TEMPERAT.PAL") == 0) {
            printf("Found TEMPERAT.PAL, attempting to load...\n");
            // TODO: Actually load and verify
            break;
        }
    }

    // Find a sprite to test
    for (int i = 0; i < g_foundCount; i++) {
        if (strcmp(g_found[i].filename, "1TNK.SHP") == 0) {
            printf("Found 1TNK.SHP, attempting to load...\n");
            // TODO: Actually load and verify
            break;
        }
    }

    printf("\n===========================================\n");
    printf("Test Complete\n");
    printf("===========================================\n");

    return 0;
}
