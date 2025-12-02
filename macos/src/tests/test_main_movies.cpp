// Test to find movies in MAIN.MIX
#include <cstdio>
#include <cstring>
#include <cstdint>
#include "assets/mixfile.h"

int main() {
    // Open MAIN.MIX
    MixFileHandle mix = Mix_Open("/Volumes/CD2/MAIN.MIX");
    if (!mix) {
        printf("Failed to open MAIN.MIX\n");
        return 1;
    }

    printf("MAIN.MIX: %d files\n", Mix_GetFileCount(mix));

    // Common VQA names to search for
    const char* vqaNames[] = {
        "INTRO.VQA", "ALLY1.VQA", "ALLY2.VQA", "ALLY3.VQA", "ALLY4.VQA",
        "SOV1.VQA", "SOV2.VQA", "SOV3.VQA", "SOV4.VQA",
        "PROLOG.VQA", "CREDITS.VQA", "WINA.VQA", "WINS.VQA",
        "MOVIE01.VQA", "MOVIE02.VQA", "AAGUN.VQA", "MIG.VQA",
        nullptr
    };

    printf("\nSearching for VQA files:\n");
    for (int i = 0; vqaNames[i]; i++) {
        if (Mix_FileExists(mix, vqaNames[i])) {
            uint32_t size = Mix_GetFileSize(mix, vqaNames[i]);
            printf("  Found: %s (size=%u)\n", vqaNames[i], size);
        }
    }

    // Also check for MOVIES.MIX inside
    const char* subMixNames[] = {
        "MOVIES.MIX", "MOVIES1.MIX", "MOVIES2.MIX",
        "GENERAL.MIX", "LOCAL.MIX", "EXPAND.MIX",
        nullptr
    };

    printf("\nLooking for sub-MIX archives:\n");
    for (int i = 0; subMixNames[i]; i++) {
        if (Mix_FileExists(mix, subMixNames[i])) {
            uint32_t size = Mix_GetFileSize(mix, subMixNames[i]);
            printf("  Found: %s (size=%u)\n", subMixNames[i], size);
        }
    }

    // Open MOVIES2.MIX if found
    if (Mix_FileExists(mix, "MOVIES2.MIX")) {
        uint32_t movies2Size = 0;
        void* movies2Data = Mix_AllocReadFile(mix, "MOVIES2.MIX", &movies2Size);
        if (movies2Data) {
            printf("\nOpening MOVIES2.MIX (%u bytes)...\n", movies2Size);
            MixFileHandle movies2 = Mix_OpenMemory(movies2Data, movies2Size, TRUE);
            if (movies2) {
                printf("MOVIES2.MIX: %d files\n", Mix_GetFileCount(movies2));

                // More VQA names to try
                const char* movieNames[] = {
                    "INTRO.VQA", "PROLOG.VQA", "ALLY1.VQA", "ALLY2.VQA", "ALLY3.VQA",
                    "ALLY4.VQA", "ALLY5.VQA", "ALLY6.VQA", "ALLY7.VQA", "ALLY8.VQA",
                    "ALLY9.VQA", "ALLY10.VQA", "ALLYEND.VQA", "ALLYPARA.VQA",
                    "SOV1.VQA", "SOV2.VQA", "SOV3.VQA", "SOV4.VQA", "SOV5.VQA",
                    "SOV6.VQA", "SOV7.VQA", "SOV8.VQA", "SOV9.VQA", "SOV10.VQA",
                    "SOVEND.VQA", "SOVPARA.VQA", "WINA.VQA", "WINS.VQA",
                    "AAGUN.VQA", "MIG.VQA", "BMAP.VQA", "ENGLISH.VQA",
                    "APTS.VQA", "BRDGTILT.VQA", "CRTEFCT.VQA", "CRONTEST.VQA",
                    "DESGULF.VQA", "DUALITY.VQA", "LANDING.VQA", "MASstrike.VQA",
                    "MCROCO.VQA", "ONTHPRW.VQA", "PROLOG.VQA", "REDINTRO.VQA",
                    "SEARCH.VQA", "SNOWBOMB.VQA", "SOVBOMB.VQA", "SPOTTER.VQA",
                    "TRIGGER.VQA", "TYX.VQA",
                    nullptr
                };

                printf("\nVQA files in MOVIES2.MIX:\n");
                for (int i = 0; movieNames[i]; i++) {
                    if (Mix_FileExists(movies2, movieNames[i])) {
                        uint32_t size = Mix_GetFileSize(movies2, movieNames[i]);
                        printf("  Found: %s (size=%u, %.2f MB)\n", movieNames[i], size, size/1048576.0f);
                    }
                }

                Mix_Close(movies2);
            } else {
                free(movies2Data);
            }
        }
    }

    Mix_Close(mix);
    return 0;
}
