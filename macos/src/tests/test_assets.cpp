/**
 * Asset Loading Test
 *
 * Tests MIX file reader against real game assets.
 * Run: make test_assets && ./build/test_assets
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../assets/mixfile.h"

#define ASSETS_PATH "../../assets/"

// Test with old-style (unencrypted) MIX file first
static const char* DEFAULT_MIX = ASSETS_PATH "AUD.MIX";

int main(int argc, char* argv[]) {
    const char* mixPath = argc > 1 ? argv[1] : DEFAULT_MIX;

    printf("=== MIX File Reader Test ===\n\n");
    printf("Opening: %s\n", mixPath);

    MixFileHandle mix = Mix_Open(mixPath);
    if (!mix) {
        printf("ERROR: Failed to open MIX file\n");
        printf("Make sure game assets are extracted to assets/ directory\n");
        printf("\nNote: REDALERT.MIX is encrypted and requires PKStraw decryption.\n");
        printf("Use AUD.MIX or SETUP.MIX for testing (unencrypted format).\n");
        return 1;
    }

    int fileCount = Mix_GetFileCount(mix);
    printf("SUCCESS: File opened\n");
    printf("File count: %d\n\n", fileCount);

    if (fileCount <= 0) {
        printf("ERROR: MIX file appears empty or corrupt\n");
        Mix_Close(mix);
        return 1;
    }

    printf("MIX file reader is working correctly!\n");
    printf("\n=== Test Passed ===\n");

    Mix_Close(mix);
    return 0;
}
