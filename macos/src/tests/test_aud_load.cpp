/**
 * Test AUD Loading from Game Archives
 *
 * Loads real game sounds and verifies the AUD parser works correctly.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "assets/mixfile.h"
#include "assets/audfile.h"

// Helper to find and load a file from the game archives
static void* LoadFromArchives(const char* filename, uint32_t* outSize) {
    *outSize = 0;

    // Archive search order - try multiple nested levels
    const char* topLevelArchives[] = {
        "/Volumes/CD1/MAIN.MIX",         // Full game CD if mounted
        "../../assets/MAIN_ALLIED.MIX",
        "../../assets/REDALERT.MIX",
        nullptr
    };

    const char* nestedArchives[] = {
        "SOUNDS.MIX",
        "LOCAL.MIX",
        "CONQUER.MIX",
        nullptr
    };

    for (int t = 0; topLevelArchives[t]; t++) {
        MixFileHandle topMix = Mix_Open(topLevelArchives[t]);
        if (!topMix) continue;

        // Try nested archives first
        for (int n = 0; nestedArchives[n]; n++) {
            uint32_t nestedSize = 0;
            void* nestedData = Mix_AllocReadFile(topMix, nestedArchives[n], &nestedSize);
            if (nestedData) {
                MixFileHandle nestedMix = Mix_OpenMemory(nestedData, nestedSize, TRUE);
                if (nestedMix) {
                    if (Mix_FileExists(nestedMix, filename)) {
                        void* result = Mix_AllocReadFile(nestedMix, filename, outSize);
                        Mix_Close(nestedMix);
                        Mix_Close(topMix);
                        return result;
                    }
                    Mix_Close(nestedMix);
                }
            }
        }

        // Try directly in top-level
        if (Mix_FileExists(topMix, filename)) {
            void* result = Mix_AllocReadFile(topMix, filename, outSize);
            Mix_Close(topMix);
            return result;
        }

        Mix_Close(topMix);
    }

    return nullptr;
}

// Helper to scan for AUD files in MIX archives
static void ScanForAudFiles(MixFileHandle mix, const char* mixName) {
    // Red Alert sound effect names - much more comprehensive list
    const char* sounds[] = {
        // Unit responses
        "ACKNO1.AUD", "AFFIRM1.AUD", "AWAIT1.AUD", "CHRONO2.AUD",
        "CHRONO4.AUD", "MCOMND1.AUD", "MREADY1.AUD", "MCOURSE1.AUD",
        "MYESSIR1.AUD", "MYES1.AUD", "MOVOUT1.AUD", "NUYELL1.AUD",
        "REPORT1.AUD", "READY.AUD", "UDESTROYR.AUD", "UHUH.AUD",
        "YESSIR1.AUD",
        // Weapons
        "CANNON1.AUD", "CANNON2.AUD", "GUN5.AUD", "GUN8.AUD",
        "MGUN2.AUD", "MGUN3.AUD", "MGUN11.AUD", "MGUN4.AUD",
        "RIFLE.AUD", "SILENCER.AUD", "TSLACHG2.AUD",
        // Explosions
        "EXPLO1.AUD", "EXPLO2.AUD", "EXPLO3.AUD", "EXPLO4.AUD",
        "EXPLOS.AUD", "EXPLODE.AUD", "BOMBIT1.AUD",
        // Buildings
        "BUILD5.AUD", "CLOCK1.AUD", "CASHUP1.AUD", "KACHING1.AUD",
        "KEYSTROK.AUD", "RADAR1.AUD", "SELLBLBG.AUD",
        // EVA announcements
        "BLDGPRG1.AUD", "MISNLST1.AUD", "MISNWON1.AUD", "NAVYLST1.AUD",
        "NEWOPT1.AUD", "NOBUILD1.AUD", "NODEPLY1.AUD", "NOFUNDS1.AUD",
        "OUTMAP1.AUD", "POWRDN1.AUD", "PRIMRYB1.AUD", "REINFOR1.AUD",
        "SIRONE1.AUD", "SLDEST1.AUD", "STRCKIL1.AUD", "SUBSURF1.AUD",
        "TIRONE1.AUD", "TITRONE1.AUD", "UNITREDY.AUD", "UNITLST1.AUD",
        nullptr
    };

    printf("Scanning %s for AUD files:\n", mixName);
    int found = 0;
    for (int i = 0; sounds[i]; i++) {
        if (Mix_FileExists(mix, sounds[i])) {
            uint32_t size = Mix_GetFileSize(mix, sounds[i]);
            printf("  Found: %s (%u bytes)\n", sounds[i], size);
            found++;
            if (found >= 20) {
                printf("  ... (showing first 20)\n");
                break;
            }
        }
    }
    if (found == 0) {
        printf("  (no common sounds found)\n");
    }
}

int main() {
    printf("===========================================\n");
    printf("AUD Loading Test\n");
    printf("===========================================\n\n");

    // First, scan the archives to find what sounds are available
    printf("Scanning archives for AUD files...\n\n");

    const char* archives[] = {
        "../../assets/REDALERT.MIX",
        "../../assets/MAIN_ALLIED.MIX",
        "/Volumes/CD1/MAIN.MIX",        // Full game CD if mounted
        nullptr
    };

    for (int i = 0; archives[i]; i++) {
        MixFileHandle topMix = Mix_Open(archives[i]);
        if (!topMix) continue;

        printf("=== %s ===\n", archives[i]);
        ScanForAudFiles(topMix, archives[i]);

        // Check nested SOUNDS.MIX
        uint32_t soundsSize = 0;
        void* soundsData = Mix_AllocReadFile(topMix, "SOUNDS.MIX", &soundsSize);
        if (soundsData) {
            MixFileHandle soundsMix = Mix_OpenMemory(soundsData, soundsSize, TRUE);
            if (soundsMix) {
                ScanForAudFiles(soundsMix, "SOUNDS.MIX");
                Mix_Close(soundsMix);
            }
        }

        Mix_Close(topMix);
        printf("\n");
    }

    // Test loading a specific sound
    printf("===========================================\n");
    printf("Loading specific sounds...\n");
    printf("===========================================\n\n");

    const char* testSounds[] = {
        "CANNON1.AUD",   // Weapon sound
        "CANNON2.AUD",   // Weapon sound
        "CHRONO2.AUD",   // Chronosphere effect
        "GUN5.AUD",      // Gunfire
        "TSLACHG2.AUD",  // Tesla charging
        "BUILD5.AUD",    // Building sound
        nullptr
    };

    for (int i = 0; testSounds[i]; i++) {
        printf("Test: Loading %s...\n", testSounds[i]);
        uint32_t size = 0;
        void* data = LoadFromArchives(testSounds[i], &size);

        if (!data) {
            printf("  Not found in archives\n\n");
            continue;
        }

        printf("  Found: %u bytes\n", size);

        AudData* aud = Aud_Load(data, size);
        free(data);

        if (!aud) {
            printf("  FAILED: Could not parse AUD\n\n");
            continue;
        }

        printf("  SUCCESS:\n");
        printf("    Sample rate: %u Hz\n", aud->sampleRate);
        printf("    Channels: %d\n", aud->channels);
        printf("    Samples: %u\n", aud->sampleCount);
        printf("    Duration: %.2f seconds\n",
               (float)aud->sampleCount / aud->sampleRate);

        // Check for non-zero samples
        int nonZero = 0;
        for (uint32_t s = 0; s < aud->sampleCount && s < 1000; s++) {
            if (aud->samples[s] != 0) nonZero++;
        }
        printf("    Non-zero samples (first 1000): %d\n", nonZero);

        Aud_Free(aud);
        printf("\n");
    }

    printf("===========================================\n");
    printf("Test Complete\n");
    printf("===========================================\n");

    return 0;
}
