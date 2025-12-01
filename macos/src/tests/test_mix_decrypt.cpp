/**
 * Test MIX file decryption
 *
 * Tests that we can open encrypted MIX files and read files from them.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "assets/mixfile.h"

int main(int argc, char* argv[]) {
    // Default path relative to build/ directory in macos/
    const char* mixPath = "../../assets/REDALERT.MIX";

    if (argc > 1) {
        mixPath = argv[1];
    }

    printf("MIX Decryption Test\n");
    printf("===================\n\n");

    printf("Opening: %s\n", mixPath);

    MixFileHandle mix = Mix_Open(mixPath);
    if (!mix) {
        printf("FAILED: Could not open MIX file\n");
        printf("This could mean:\n");
        printf("  - File doesn't exist at the path\n");
        printf("  - RSA decryption failed (wrong key?)\n");
        printf("  - Blowfish decryption failed\n");
        printf("  - Header format not recognized\n");
        return 1;
    }

    printf("SUCCESS: MIX file opened!\n\n");

    int fileCount = Mix_GetFileCount(mix);
    printf("File count: %d\n\n", fileCount);

    // Try to find some known files that should be in REDALERT.MIX
    // REDALERT.MIX contains sub-MIX archives, not individual files like RULES.INI
    const char* testFiles[] = {
        "LOCAL.MIX",
        "HIRES.MIX",
        "LORES.MIX",
        "NCHIRES.MIX",
        "CONQUER.MIX",
        "GENERAL.MIX",
        "MOVIES1.MIX",
        "MOVIES2.MIX",
        "SCORES.MIX",
        nullptr
    };

    printf("Checking for known files:\n");
    for (int i = 0; testFiles[i]; i++) {
        uint32_t crc = Mix_CalculateCRC(testFiles[i]);
        bool exists = Mix_FileExists(mix, testFiles[i]);
        uint32_t size = Mix_GetFileSize(mix, testFiles[i]);

        printf("  %-16s CRC=%08X  ", testFiles[i], crc);
        if (exists) {
            printf("FOUND  size=%u bytes\n", size);
        } else {
            printf("NOT FOUND\n");
        }
    }

    printf("\n");

    // Try to read LOCAL.MIX and open it as nested MIX
    printf("\nAttempting to read LOCAL.MIX...\n");
    uint32_t localSize = 0;
    void* localData = Mix_AllocReadFile(mix, "LOCAL.MIX", &localSize);

    if (localData && localSize > 0) {
        printf("SUCCESS: Read %u bytes from LOCAL.MIX\n", localSize);

        // Verify it looks like a MIX file (check first bytes)
        uint8_t* data = (uint8_t*)localData;
        printf("First 16 bytes: ");
        for (int i = 0; i < 16 && i < (int)localSize; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");

        // Try to open LOCAL.MIX from memory (it's also encrypted!)
        printf("\nOpening LOCAL.MIX from memory...\n");
        MixFileHandle localMix = Mix_OpenMemory(localData, localSize, TRUE);  // TRUE = takes ownership
        localData = nullptr;  // localMix now owns the data

        if (localMix) {
            printf("SUCCESS: Opened nested LOCAL.MIX with %d files\n", Mix_GetFileCount(localMix));

            // Look for RULES.INI in LOCAL.MIX
            printf("\nLooking for RULES.INI...\n");
            uint32_t rulesCrc = Mix_CalculateCRC("RULES.INI");
            printf("  RULES.INI CRC=%08X  ", rulesCrc);
            if (Mix_FileExists(localMix, "RULES.INI")) {
                uint32_t rulesSize = Mix_GetFileSize(localMix, "RULES.INI");
                printf("FOUND  size=%u bytes\n", rulesSize);

                // Read first 200 bytes
                char rulesData[201];
                uint32_t bytesRead = Mix_ReadFile(localMix, "RULES.INI", rulesData, 200);
                rulesData[bytesRead] = '\0';
                printf("\nFirst %u bytes of RULES.INI:\n%s\n", bytesRead, rulesData);
            } else {
                printf("NOT FOUND\n");
            }

            // Try other common files
            const char* localFiles[] = {"CONQUER.ENG", "TUTORIAL.INI", "MISSION.INI", nullptr};
            printf("\nChecking other files in LOCAL.MIX:\n");
            for (int i = 0; localFiles[i]; i++) {
                printf("  %-20s ", localFiles[i]);
                if (Mix_FileExists(localMix, localFiles[i])) {
                    printf("FOUND  size=%u\n", Mix_GetFileSize(localMix, localFiles[i]));
                } else {
                    printf("NOT FOUND\n");
                }
            }

            Mix_Close(localMix);
        } else {
            printf("FAILED: Could not open nested LOCAL.MIX\n");
            free(localData);
        }
    } else {
        printf("FAILED: Could not read LOCAL.MIX\n");
    }

    // Try to open HIRES.MIX and look for palettes
    printf("\n--- Checking HIRES.MIX for graphics ---\n");
    uint32_t hiresSize = 0;
    void* hiresData = Mix_AllocReadFile(mix, "HIRES.MIX", &hiresSize);
    if (hiresData) {
        MixFileHandle hiresMix = Mix_OpenMemory(hiresData, hiresSize, TRUE);
        if (hiresMix) {
            printf("Opened HIRES.MIX with %d files\n", Mix_GetFileCount(hiresMix));

            // Look for common game files
            const char* hiresFiles[] = {
                "TEMPERAT.PAL", "SNOW.PAL", "INTERIOR.PAL",
                "UNITS.SHP", "INFANTRY.SHP", "CONQUER.SHP",
                "MOUSE.SHP", "HIRES.PAL", nullptr
            };
            for (int i = 0; hiresFiles[i]; i++) {
                printf("  %-20s ", hiresFiles[i]);
                if (Mix_FileExists(hiresMix, hiresFiles[i])) {
                    printf("FOUND  size=%u\n", Mix_GetFileSize(hiresMix, hiresFiles[i]));
                } else {
                    printf("NOT FOUND\n");
                }
            }
            Mix_Close(hiresMix);
        } else {
            free(hiresData);
        }
    }

    Mix_Close(mix);

    // Try MAIN_ALLIED.MIX from assets folder
    printf("\n--- Checking MAIN_ALLIED.MIX ---\n");
    MixFileHandle mainMix = Mix_Open("../../assets/MAIN_ALLIED.MIX");
    if (mainMix) {
        printf("Opened MAIN_ALLIED.MIX with %d files\n", Mix_GetFileCount(mainMix));

        // Look for conquer.mix and other archives
        const char* mainFiles[] = {
            "CONQUER.MIX", "GENERAL.MIX", "SCORES.MIX", "MOVIES1.MIX",
            "SCG01EA.MIX", "SCG02EA.MIX", nullptr  // Campaign missions
        };
        for (int i = 0; mainFiles[i]; i++) {
            printf("  %-20s ", mainFiles[i]);
            if (Mix_FileExists(mainMix, mainFiles[i])) {
                printf("FOUND  size=%u\n", Mix_GetFileSize(mainMix, mainFiles[i]));
            } else {
                printf("NOT FOUND\n");
            }
        }

        // Try to get CONQUER.MIX to find game sprites
        printf("\n--- Opening CONQUER.MIX from MAIN ---\n");
        uint32_t conquerSize = 0;
        void* conquerData = Mix_AllocReadFile(mainMix, "CONQUER.MIX", &conquerSize);
        if (conquerData) {
            MixFileHandle conquerMix = Mix_OpenMemory(conquerData, conquerSize, TRUE);
            if (conquerMix) {
                printf("Opened CONQUER.MIX with %d files\n", Mix_GetFileCount(conquerMix));

                // Look for sprites (RA1 naming)
                const char* spriteFiles[] = {
                    // Tanks - found!
                    "1TNK.SHP", "2TNK.SHP", "3TNK.SHP", "4TNK.SHP",
                    // Vehicles
                    "HARV.SHP", "MCV.SHP", "MNLY.SHP", "JEEP.SHP", "APC.SHP",
                    // Aircraft
                    "HELI.SHP", "HIND.SHP", "MIG.SHP", "YAK.SHP",
                    // Buildings
                    "FACT.SHP", "POWR.SHP", "PROC.SHP", "WEAP.SHP",
                    // Infantry (RA1 uses different names)
                    "E1.SHP", "RIFLE.SHP", "GRENADE.SHP", "ROCKET.SHP",
                    // Palettes
                    "TEMPERAT.PAL", "SNOW.PAL", "INTERIOR.PAL",
                    nullptr
                };
                for (int i = 0; spriteFiles[i]; i++) {
                    printf("  %-20s ", spriteFiles[i]);
                    if (Mix_FileExists(conquerMix, spriteFiles[i])) {
                        printf("FOUND  size=%u\n", Mix_GetFileSize(conquerMix, spriteFiles[i]));
                    } else {
                        printf("NOT FOUND\n");
                    }
                }
                Mix_Close(conquerMix);
            } else {
                free(conquerData);
            }
        }

        // Check GENERAL.MIX for palettes
        printf("\n--- Opening GENERAL.MIX ---\n");
        uint32_t generalSize = 0;
        void* generalData = Mix_AllocReadFile(mainMix, "GENERAL.MIX", &generalSize);
        if (generalData) {
            MixFileHandle generalMix = Mix_OpenMemory(generalData, generalSize, TRUE);
            if (generalMix) {
                printf("Opened GENERAL.MIX with %d files\n", Mix_GetFileCount(generalMix));

                const char* generalFiles[] = {
                    "TEMPERAT.PAL", "SNOW.PAL", "INTERIOR.PAL", "DESERT.PAL",
                    "CONQUER.PAL", "UNITS.PAL", nullptr
                };
                for (int i = 0; generalFiles[i]; i++) {
                    printf("  %-20s ", generalFiles[i]);
                    if (Mix_FileExists(generalMix, generalFiles[i])) {
                        printf("FOUND  size=%u\n", Mix_GetFileSize(generalMix, generalFiles[i]));
                    } else {
                        printf("NOT FOUND\n");
                    }
                }
                Mix_Close(generalMix);
            } else {
                free(generalData);
                printf("Could not open GENERAL.MIX\n");
            }
        } else {
            printf("GENERAL.MIX not found, checking other archives...\n");
        }

        Mix_Close(mainMix);
    } else {
        printf("Could not open MAIN_ALLIED.MIX\n");
    }

    // Palettes might be in REDALERT.MIX itself - check all nested archives
    printf("\n--- Searching for palettes in all archives ---\n");
    MixFileHandle redAlertMix = Mix_Open("../../assets/REDALERT.MIX");
    if (redAlertMix) {
        // NCHIRES.MIX might have palettes for non-cached hires
        uint32_t nchiresSize = 0;
        void* nchiresData = Mix_AllocReadFile(redAlertMix, "NCHIRES.MIX", &nchiresSize);
        if (nchiresData) {
            MixFileHandle nchiresMix = Mix_OpenMemory(nchiresData, nchiresSize, TRUE);
            if (nchiresMix) {
                printf("NCHIRES.MIX has %d files\n", Mix_GetFileCount(nchiresMix));
                const char* palFiles[] = {"TEMPERAT.PAL", "SNOW.PAL", "INTERIOR.PAL", nullptr};
                for (int i = 0; palFiles[i]; i++) {
                    if (Mix_FileExists(nchiresMix, palFiles[i])) {
                        printf("  FOUND: %s (%u bytes)\n", palFiles[i], Mix_GetFileSize(nchiresMix, palFiles[i]));
                    }
                }
                Mix_Close(nchiresMix);
            } else {
                free(nchiresData);
            }
        }
        Mix_Close(redAlertMix);
    }

    printf("\n===================\n");
    printf("Test complete!\n");

    return 0;
}
