/**
 * Test: Extract mission INI files from REDALERT.MIX
 *
 * Mission naming convention:
 * - SCU##EA.INI = Soviet missions (SCU = Soviet Campaign, ## = mission number)
 * - SCG##EA.INI = Allied/Greece missions (SCG = Soviet Campaign Greece? Actually Allied)
 * - EA suffix = maybe "English Allies"?
 *
 * Actually the naming is:
 * - SCG = GDI/Allied campaign
 * - SCU = USSR/Soviet campaign
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../assets/mixfile.h"

// Mission filenames to search for
static const char* alliedMissions[] = {
    "SCG01EA.INI", "SCG02EA.INI", "SCG03EA.INI", "SCG04EA.INI",
    "SCG05EA.INI", "SCG06EA.INI", "SCG07EA.INI", "SCG08EA.INI",
    "SCG09EA.INI", "SCG10EA.INI", "SCG11EA.INI", "SCG12EA.INI",
    "SCG13EA.INI", "SCG14EA.INI",
    nullptr
};

static const char* sovietMissions[] = {
    "SCU01EA.INI", "SCU02EA.INI", "SCU03EA.INI", "SCU04EA.INI",
    "SCU05EA.INI", "SCU06EA.INI", "SCU07EA.INI", "SCU08EA.INI",
    "SCU09EA.INI", "SCU10EA.INI", "SCU11EA.INI", "SCU12EA.INI",
    "SCU13EA.INI", "SCU14EA.INI",
    nullptr
};

// Alternative naming patterns to try
static const char* altPatterns[] = {
    // Might not have EA suffix
    "SCG01.INI", "SCU01.INI",
    // Might be numbered differently
    "SCEN01.INI", "SCENARIO01.INI",
    // Might be in a subdirectory notation
    "MISSIONS/SCG01EA.INI",
    // Expansion missions
    "SCG15EA.INI", "SCU15EA.INI",
    nullptr
};

// Common files that might be in REDALERT.MIX
static const char* commonFiles[] = {
    "RULES.INI",
    "REDALERT.INI",
    "AI.INI",
    "ART.INI",
    "SOUND.INI",
    "TUTORIAL.INI",
    nullptr
};

void searchMix(MixFileHandle mix, const char* mixName, const char** filenames) {
    printf("\nSearching %s:\n", mixName);
    int found = 0;

    for (int i = 0; filenames[i] != nullptr; i++) {
        uint32_t size = Mix_GetFileSize(mix, filenames[i]);
        if (size > 0) {
            printf("  FOUND: %s (%u bytes)\n", filenames[i], size);
            found++;
        }
    }

    if (found == 0) {
        printf("  (none found)\n");
    }
}

void extractFile(MixFileHandle mix, const char* filename, const char* outDir) {
    uint32_t size = Mix_GetFileSize(mix, filename);
    if (size == 0) {
        printf("File not found: %s\n", filename);
        return;
    }

    void* data = Mix_AllocReadFile(mix, filename, &size);
    if (!data) {
        printf("Failed to read: %s\n", filename);
        return;
    }

    // Write to output directory
    char outPath[512];
    snprintf(outPath, sizeof(outPath), "%s/%s", outDir, filename);

    FILE* f = fopen(outPath, "wb");
    if (f) {
        fwrite(data, 1, size, f);
        fclose(f);
        printf("Extracted: %s -> %s (%u bytes)\n", filename, outPath, size);
    } else {
        printf("Failed to write: %s\n", outPath);
    }

    free(data);
}

int main(int argc, char** argv) {
    // Try to open MAIN.MIX (the big archive on CD root)
    const char* mixPaths[] = {
        "/Volumes/CD1/MAIN.MIX",
        "/Volumes/CD2/MAIN.MIX",
        "/Volumes/CD1/INSTALL/REDALERT.MIX",
        "../assets/MAIN.MIX",
        "assets/MAIN.MIX",
        nullptr
    };

    MixFileHandle mix = nullptr;
    const char* usedPath = nullptr;

    for (int i = 0; mixPaths[i] != nullptr; i++) {
        mix = Mix_Open(mixPaths[i]);
        if (mix) {
            usedPath = mixPaths[i];
            break;
        }
    }

    if (!mix) {
        printf("ERROR: Could not open REDALERT.MIX from any location\n");
        printf("Tried:\n");
        for (int i = 0; mixPaths[i] != nullptr; i++) {
            printf("  - %s\n", mixPaths[i]);
        }
        return 1;
    }

    printf("Opened: %s\n", usedPath);
    printf("File count: %d\n", Mix_GetFileCount(mix));

    // Search for known files
    searchMix(mix, "Allied Missions", alliedMissions);
    searchMix(mix, "Soviet Missions", sovietMissions);
    searchMix(mix, "Alternative Patterns", altPatterns);
    searchMix(mix, "Common Files", commonFiles);

    // Try to extract RULES.INI if found
    const char* outDir = "/tmp/ra_extract";
    char mkdirCmd[256];
    snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p %s", outDir);
    system(mkdirCmd);

    // Extract any found files
    printf("\n=== Extracting found files to %s ===\n", outDir);

    for (int i = 0; commonFiles[i] != nullptr; i++) {
        if (Mix_GetFileSize(mix, commonFiles[i]) > 0) {
            extractFile(mix, commonFiles[i], outDir);
        }
    }

    // Extract first Allied mission if found
    for (int i = 0; alliedMissions[i] != nullptr; i++) {
        if (Mix_GetFileSize(mix, alliedMissions[i]) > 0) {
            extractFile(mix, alliedMissions[i], outDir);
        }
    }

    // Extract first Soviet mission if found
    for (int i = 0; sovietMissions[i] != nullptr; i++) {
        if (Mix_GetFileSize(mix, sovietMissions[i]) > 0) {
            extractFile(mix, sovietMissions[i], outDir);
        }
    }

    Mix_Close(mix);

    printf("\n=== Done ===\n");
    printf("Check extracted files in: %s\n", outDir);

    return 0;
}
