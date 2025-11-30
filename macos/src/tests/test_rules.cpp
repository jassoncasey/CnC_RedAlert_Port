/**
 * Red Alert macOS Port - RULES.INI Test
 *
 * Tests loading and parsing of RULES.INI
 */

#include "../game/rules.h"
#include <cstdio>
#include <cstring>
#include <cmath>

static int testCount = 0;
static int passCount = 0;

#define TEST(name) do { \
    testCount++; \
    printf("  Test: %s... ", name); \
} while(0)

#define PASS() do { \
    passCount++; \
    printf("PASS\n"); \
} while(0)

#define FAIL(msg) do { \
    printf("FAIL (%s)\n", msg); \
} while(0)

void TestLoading() {
    printf("\n=== Loading Tests ===\n");

    TEST("Load RULES.INI");
    bool loaded = Rules.Load("../resources/RULES.INI");
    if (loaded && Rules.IsLoaded()) {
        PASS();
    } else {
        FAIL("Could not load RULES.INI");
        printf("    Note: Run this test from the build directory\n");
        return;
    }
}

void TestGeneral() {
    printf("\n=== General Settings Tests ===\n");
    const GeneralRules& g = Rules.General();

    TEST("Crate minimum");
    if (g.crateMinimum == 1) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1, got %d", g.crateMinimum);
        FAIL(msg);
    }

    TEST("Crate maximum");
    if (g.crateMaximum == 255) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 255, got %d", g.crateMaximum);
        FAIL(msg);
    }

    TEST("Crate radius");
    if (fabs(g.crateRadius - 3.0f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 3.0, got %.2f", g.crateRadius);
        FAIL(msg);
    }

    TEST("Chrono duration");
    if (fabs(g.chronoDuration - 3.0f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 3.0, got %.2f", g.chronoDuration);
        FAIL(msg);
    }

    TEST("Chrono kill cargo");
    if (g.chronoKillCargo == true) {
        PASS();
    } else {
        FAIL("Expected true");
    }

    TEST("Gap radius");
    if (g.gapRadius == 10) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 10, got %d", g.gapRadius);
        FAIL(msg);
    }

    TEST("Build speed");
    if (fabs(g.buildSpeed - 0.8f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 0.8, got %.2f", g.buildSpeed);
        FAIL(msg);
    }

    TEST("Gold value");
    if (g.goldValue == 25) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 25, got %d", g.goldValue);
        FAIL(msg);
    }

    TEST("Gem value");
    if (g.gemValue == 50) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 50, got %d", g.gemValue);
        FAIL(msg);
    }

    TEST("Ore grows");
    if (g.oreGrows == true) {
        PASS();
    } else {
        FAIL("Expected true");
    }

    TEST("Ore spreads");
    if (g.oreSpreads == true) {
        PASS();
    } else {
        FAIL("Expected true");
    }

    TEST("Gravity");
    if (g.gravity == 3) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 3, got %d", g.gravity);
        FAIL(msg);
    }

    TEST("Atom damage");
    if (g.atomDamage == 1000) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1000, got %d", g.atomDamage);
        FAIL(msg);
    }

    TEST("Refund percent");
    if (fabs(g.refundPercent - 0.5f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 0.5, got %.2f", g.refundPercent);
        FAIL(msg);
    }
}

void TestIQ() {
    printf("\n=== IQ Settings Tests ===\n");
    const IQSettings& iq = Rules.IQ();

    TEST("Max IQ levels");
    if (iq.maxLevels == 5) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 5, got %d", iq.maxLevels);
        FAIL(msg);
    }

    TEST("Super weapons IQ");
    if (iq.superWeapons == 4) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 4, got %d", iq.superWeapons);
        FAIL(msg);
    }

    TEST("Production IQ");
    if (iq.production == 5) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 5, got %d", iq.production);
        FAIL(msg);
    }

    TEST("Harvester IQ");
    if (iq.harvester == 2) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 2, got %d", iq.harvester);
        FAIL(msg);
    }
}

void TestDifficulty() {
    printf("\n=== Difficulty Settings Tests ===\n");

    TEST("Easy firepower");
    const DifficultySettings& easy = Rules.GetDifficulty(0);
    if (fabs(easy.firepower - 1.2f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1.2, got %.2f", easy.firepower);
        FAIL(msg);
    }

    TEST("Easy cost");
    if (fabs(easy.cost - 0.8f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 0.8, got %.2f", easy.cost);
        FAIL(msg);
    }

    TEST("Normal firepower");
    const DifficultySettings& normal = Rules.GetDifficulty(1);
    if (fabs(normal.firepower - 1.0f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1.0, got %.2f", normal.firepower);
        FAIL(msg);
    }
}

void TestCountries() {
    printf("\n=== Country Settings Tests ===\n");

    TEST("England exists");
    const CountrySettings* england = Rules.GetCountry("England");
    if (england != nullptr) {
        PASS();
    } else {
        FAIL("England not found");
        return;
    }

    TEST("England armor bonus");
    if (fabs(england->armor - 1.1f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1.1, got %.2f", england->armor);
        FAIL(msg);
    }

    TEST("Germany exists");
    const CountrySettings* germany = Rules.GetCountry("Germany");
    if (germany != nullptr) {
        PASS();
    } else {
        FAIL("Germany not found");
        return;
    }

    TEST("Germany firepower bonus");
    if (fabs(germany->firepower - 1.1f) < 0.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 1.1, got %.2f", germany->firepower);
        FAIL(msg);
    }

    TEST("USSR cost bonus");
    const CountrySettings* ussr = Rules.GetCountry("USSR");
    if (ussr != nullptr && fabs(ussr->cost - 0.9f) < 0.01f) {
        PASS();
    } else {
        if (ussr == nullptr) {
            FAIL("USSR not found");
        } else {
            char msg[64];
            snprintf(msg, sizeof(msg), "Expected 0.9, got %.2f", ussr->cost);
            FAIL(msg);
        }
    }
}

int main() {
    printf("RULES.INI Parser Test\n");
    printf("=====================\n");

    TestLoading();

    if (Rules.IsLoaded()) {
        TestGeneral();
        TestIQ();
        TestDifficulty();
        TestCountries();
    }

    printf("\n=====================\n");
    printf("Results: %d/%d tests passed\n", passCount, testCount);

    return (passCount == testCount) ? 0 : 1;
}
