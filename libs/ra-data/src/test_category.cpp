/**
 * Test ra-data category mapping
 */

#include <ra/category.h>
#include <cstdio>

int main() {
    struct Test {
        const char* file;
        ra::AssetCategory expected;
    };

    Test tests[] = {
        // Infantry
        {"E1.SHP", ra::AssetCategory::Infantry},
        {"e2.shp", ra::AssetCategory::Infantry},  // case insensitive
        {"DOG.SHP", ra::AssetCategory::Infantry},
        {"SPY.SHP", ra::AssetCategory::Infantry},
        {"EINSTEIN.SHP", ra::AssetCategory::Infantry},
        {"C1.SHP", ra::AssetCategory::Infantry},

        // Vehicles
        {"1TNK.SHP", ra::AssetCategory::Vehicle},
        {"4TNK.SHP", ra::AssetCategory::Vehicle},
        {"HARV.SHP", ra::AssetCategory::Vehicle},
        {"MCV.SHP", ra::AssetCategory::Vehicle},
        {"ARTY.SHP", ra::AssetCategory::Vehicle},

        // Aircraft
        {"HELI.SHP", ra::AssetCategory::Aircraft},
        {"HIND.SHP", ra::AssetCategory::Aircraft},
        {"MIG.SHP", ra::AssetCategory::Aircraft},

        // Vessels
        {"DD.SHP", ra::AssetCategory::Vessel},
        {"CA.SHP", ra::AssetCategory::Vessel},
        {"SS.SHP", ra::AssetCategory::Vessel},
        {"MSUB.SHP", ra::AssetCategory::Vessel},

        // Buildings
        {"POWR.SHP", ra::AssetCategory::Building},
        {"FACT.SHP", ra::AssetCategory::Building},
        {"BARR.SHP", ra::AssetCategory::Building},
        {"PROC.SHP", ra::AssetCategory::Building},

        // Defense
        {"GUN.SHP", ra::AssetCategory::Defense},
        {"TSLA.SHP", ra::AssetCategory::Defense},
        {"SAM.SHP", ra::AssetCategory::Defense},
        {"SBAG.SHP", ra::AssetCategory::Defense},

        // Smudge
        {"CR1.SHP", ra::AssetCategory::Smudge},
        {"SC4.SHP", ra::AssetCategory::Smudge},
        {"BIB2.SHP", ra::AssetCategory::Smudge},

        // Overlay
        {"GOLD01.SHP", ra::AssetCategory::Overlay},
        {"GEM03.SHP", ra::AssetCategory::Overlay},
        {"WCRATE.SHP", ra::AssetCategory::Overlay},

        // Animation
        {"FIRE1.SHP", ra::AssetCategory::Animation},
        {"FBALL.SHP", ra::AssetCategory::Animation},
        {"NAPALM.SHP", ra::AssetCategory::Animation},

        // Projectile
        {"MISSILE.SHP", ra::AssetCategory::Projectile},
        {"DRAGON.SHP", ra::AssetCategory::Projectile},
        {"BOMB.SHP", ra::AssetCategory::Projectile},

        // Cameo (icons)
        {"1TNKICON.SHP", ra::AssetCategory::Cameo},
        {"E1ICON.SHP", ra::AssetCategory::Cameo},
        {"POWRICON.SHP", ra::AssetCategory::Cameo},

        // Cursor
        {"MOUSE.SHP", ra::AssetCategory::Cursor},

        // Audio - Music
        {"HELL226M.AUD", ra::AssetCategory::Music},
        {"FAC1226M.AUD", ra::AssetCategory::Music},
        {"TWIN.AUD", ra::AssetCategory::Music},

        // Audio - Voice
        {"MISNWON1.AUD", ra::AssetCategory::Voice},
        {"READY.AUD", ra::AssetCategory::Voice},

        // Audio - SFX (default for unknown AUD)
        {"EXPLOD1.AUD", ra::AssetCategory::SoundEffect},
        {"CANNON1.AUD", ra::AssetCategory::SoundEffect},

        // Video
        {"INTRO.VQA", ra::AssetCategory::Cutscene},
        {"ALLY1.VQA", ra::AssetCategory::Cutscene},

        // Data
        {"RULES.INI", ra::AssetCategory::Rules},
        {"SCG01EA.INI", ra::AssetCategory::Rules},

        // Interface
        {"TEMPERAT.PAL", ra::AssetCategory::Interface},
        {"8POINT.FNT", ra::AssetCategory::Interface},
    };

    int passed = 0, failed = 0;

    for (const auto& t : tests) {
        ra::AssetCategory got = ra::categorize(t.file);
        if (got == t.expected) {
            printf("PASS: %-20s -> %s\n", t.file, ra::category_name(got));
            passed++;
        } else {
            printf("FAIL: %-20s -> %s (expected %s)\n",
                   t.file, ra::category_name(got),
                   ra::category_name(t.expected));
            failed++;
        }
    }

    printf("\n%d passed, %d failed\n", passed, failed);

    // Test theater detection
    printf("\nTheater detection:\n");
    printf("  TEMPERAT.MIX -> %s\n",
           ra::theater_name(ra::detect_theater("TEMPERAT.MIX")));
    printf("  SNOW.MIX     -> %s\n",
           ra::theater_name(ra::detect_theater("SNOW.MIX")));
    printf("  INTERIOR.MIX -> %s\n",
           ra::theater_name(ra::detect_theater("INTERIOR.MIX")));
    printf("  CONQUER.MIX  -> %s\n",
           ra::theater_name(ra::detect_theater("CONQUER.MIX")));

    return failed > 0 ? 1 : 0;
}
