/**
 * Test: Enumerate files in MIX using the proper Mix_Open API
 * which handles decryption.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../assets/mixfile.h"

// Comprehensive list of known filenames from Red Alert
static const char* knownFiles[] = {
    // Core files
    "RULES.INI", "REDALERT.INI", "AI.INI", "ART.INI", "SOUND.INI",
    "TUTORIAL.INI", "GAME.DAT", "SPEECH.MIX", "SOUNDS.MIX",

    // MIX file references (nested archives)
    "GENERAL.MIX", "CONQUER.MIX", "LOCAL.MIX", "HIRES.MIX", "LORES.MIX",
    "MOVIES.MIX", "ALLIES.MIX", "SOVIET.MIX",
    "SNOW.MIX", "TEMPERAT.MIX", "DESERT.MIX", "INTERIOR.MIX",
    "MAIN.MIX", "INSTALL.MIX", "SETUP.MIX", "SCORES.MIX",
    "DATA.MIX", "BRIEFING.MIX", "CAMPAIGN.MIX",
    "EXPAND.MIX", "EXPAND2.MIX", "HIRES1.MIX", "LORES1.MIX",

    // Allied missions (SCG = Allied)
    "SCG01EA.INI", "SCG02EA.INI", "SCG03EA.INI", "SCG04EA.INI",
    "SCG05EA.INI", "SCG06EA.INI", "SCG07EA.INI", "SCG08EA.INI",
    "SCG09EA.INI", "SCG10EA.INI", "SCG11EA.INI", "SCG12EA.INI",
    "SCG13EA.INI", "SCG14EA.INI",
    // Without EA suffix
    "SCG01.INI", "SCG02.INI", "SCG03.INI", "SCG04.INI",
    "SCG05.INI", "SCG06.INI", "SCG07.INI", "SCG08.INI",

    // Soviet missions (SCU = Soviet)
    "SCU01EA.INI", "SCU02EA.INI", "SCU03EA.INI", "SCU04EA.INI",
    "SCU05EA.INI", "SCU06EA.INI", "SCU07EA.INI", "SCU08EA.INI",
    "SCU09EA.INI", "SCU10EA.INI", "SCU11EA.INI", "SCU12EA.INI",
    "SCU13EA.INI", "SCU14EA.INI",

    // Alternate naming patterns
    "SCEN01.INI", "SCEN02.INI", "SCENARIO01.INI",
    "MISSION1.INI", "MISSION01.INI",

    // Videos
    "INTRO.VQA", "ALLY1.VQA", "ALLY2.VQA", "ALLY3.VQA",
    "SOV1.VQA", "SOV2.VQA", "SOV3.VQA",
    "ALLYEND.VQA", "SOVEND.VQA", "PROLOG.VQA",

    // Palettes
    "TEMPERAT.PAL", "SNOW.PAL", "DESERT.PAL", "INTERIOR.PAL",
    "CONQUER.PAL", "GAME.PAL",

    // Other data files
    "CONQUER.ENG", "THEME.INI", "MISSION.INI",
    "STRINGS.ENG", "TUTORIAL.INI", "CREDITS.TXT",
    nullptr
};

void searchMix(MixFileHandle mix, const char* mixName) {
    printf("\n=== Contents of %s ===\n", mixName);
    printf("File count: %d\n", Mix_GetFileCount(mix));

    int found = 0;
    for (int i = 0; knownFiles[i] != nullptr; i++) {
        uint32_t size = Mix_GetFileSize(mix, knownFiles[i]);
        if (size > 0) {
            printf("  [%3d] %-20s %10u bytes\n", found++, knownFiles[i], size);
        }
    }

    if (found == 0) {
        printf("  (no known files found - archive may contain nested MIX files)\n");
    }
}

void extractFile(MixFileHandle mix, const char* filename, const char* outDir) {
    uint32_t size;
    void* data = Mix_AllocReadFile(mix, filename, &size);
    if (!data) return;

    char outPath[512];
    snprintf(outPath, sizeof(outPath), "%s/%s", outDir, filename);

    FILE* f = fopen(outPath, "wb");
    if (f) {
        fwrite(data, 1, size, f);
        fclose(f);
        printf("Extracted: %s (%u bytes)\n", filename, size);
    }

    free(data);
}

int main(int argc, char** argv) {
    system("mkdir -p /tmp/ra_extract");

    // List of MIX files to examine
    const char* mixFiles[] = {
        "/Volumes/CD1/MAIN.MIX",
        "/Volumes/CD1/INSTALL/REDALERT.MIX",
        nullptr
    };

    for (int m = 0; mixFiles[m] != nullptr; m++) {
        MixFileHandle mix = Mix_Open(mixFiles[m]);
        if (!mix) {
            printf("\nCould not open: %s\n", mixFiles[m]);
            continue;
        }

        searchMix(mix, mixFiles[m]);

        // Extract any found files
        printf("\nExtracting found files:\n");
        for (int i = 0; knownFiles[i] != nullptr; i++) {
            if (Mix_GetFileSize(mix, knownFiles[i]) > 0) {
                extractFile(mix, knownFiles[i], "/tmp/ra_extract");
            }
        }

        // Check for nested MIX files and extract them
        const char* nestedMixes[] = {
            "GENERAL.MIX", "CONQUER.MIX", "LOCAL.MIX",
            "HIRES.MIX", "LORES.MIX", "ALLIES.MIX", "SOVIET.MIX",
            "SNOW.MIX", "TEMPERAT.MIX", "SCORES.MIX",
            nullptr
        };

        printf("\nChecking for nested MIX archives:\n");
        for (int i = 0; nestedMixes[i] != nullptr; i++) {
            uint32_t size = Mix_GetFileSize(mix, nestedMixes[i]);
            if (size > 0) {
                printf("  Found nested: %s (%u bytes)\n", nestedMixes[i], size);
                extractFile(mix, nestedMixes[i], "/tmp/ra_extract");
            }
        }

        Mix_Close(mix);
    }

    // Now try to open extracted nested MIX files
    printf("\n\n=== Searching extracted nested MIX files ===\n");

    const char* extractedMixes[] = {
        "/tmp/ra_extract/GENERAL.MIX",
        "/tmp/ra_extract/CONQUER.MIX",
        "/tmp/ra_extract/LOCAL.MIX",
        "/tmp/ra_extract/ALLIES.MIX",
        "/tmp/ra_extract/SOVIET.MIX",
        nullptr
    };

    for (int m = 0; extractedMixes[m] != nullptr; m++) {
        MixFileHandle mix = Mix_Open(extractedMixes[m]);
        if (!mix) continue;

        searchMix(mix, extractedMixes[m]);

        // Extract mission files if found
        for (int i = 0; knownFiles[i] != nullptr; i++) {
            if (Mix_GetFileSize(mix, knownFiles[i]) > 0) {
                extractFile(mix, knownFiles[i], "/tmp/ra_extract");
            }
        }

        Mix_Close(mix);
    }

    printf("\n=== Done ===\n");
    printf("Extracted files: ls -la /tmp/ra_extract/\n");
    system("ls -la /tmp/ra_extract/");

    return 0;
}
