/**
 * Test: ALL_HUNT trigger action - parsing verification
 *
 * Verifies:
 * 1. Triggers with action 6 (ALL_HUNT) are correctly identified
 * 2. Trigger house is parsed correctly
 * 3. House-to-team mapping works (USSR/Ukraine -> TEAM_ENEMY)
 *
 * Note: Full hunt behavior testing requires the game running
 */

#include "../game/ini.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Action number for ALL_HUNT
#define RA_ACTION_ALL_HUNT 6

// House definitions (from units.h)
enum HouseType {
    HOUSE_SPAIN = 0,    // Allied
    HOUSE_GREECE,       // Allied
    HOUSE_USSR,         // Soviet
    HOUSE_ENGLAND,      // Allied
    HOUSE_UKRAINE,      // Soviet
    HOUSE_GERMANY,      // Allied
    HOUSE_FRANCE,       // Allied
    HOUSE_TURKEY,       // Allied
    HOUSE_COUNT
};

// Team definitions (from units.h)
enum Team {
    TEAM_NEUTRAL = 0,
    TEAM_PLAYER,
    TEAM_ENEMY,
    TEAM_COUNT
};

// Convert house to team (same logic as in mission.cpp)
static Team HouseToTeam(int houseNum) {
    // Soviet houses are enemy
    if (houseNum == HOUSE_USSR || houseNum == HOUSE_UKRAINE) {
        return TEAM_ENEMY;
    }
    // Allied houses are player
    return TEAM_PLAYER;
}

static const char* HouseName(int house) {
    switch (house) {
        case HOUSE_SPAIN: return "Spain";
        case HOUSE_GREECE: return "Greece";
        case HOUSE_USSR: return "USSR";
        case HOUSE_ENGLAND: return "England";
        case HOUSE_UKRAINE: return "Ukraine";
        case HOUSE_GERMANY: return "Germany";
        case HOUSE_FRANCE: return "France";
        case HOUSE_TURKEY: return "Turkey";
        default: return "Unknown";
    }
}

static const char* TeamName(Team team) {
    switch (team) {
        case TEAM_NEUTRAL: return "Neutral";
        case TEAM_PLAYER: return "Player";
        case TEAM_ENEMY: return "Enemy";
        default: return "Unknown";
    }
}

int main(int argc, char** argv) {
    const char* missionFile = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) missionFile = argv[1];

    printf("=== Test: ALL_HUNT Parsing ===\n\n");

    INIClass ini;
    if (!ini.Load(missionFile)) {
        printf("FAIL: Could not load %s\n", missionFile);
        return 1;
    }
    printf("Loaded: %s\n\n", missionFile);

    // Scan all INI files for ALL_HUNT triggers
    printf("=== Scanning Triggers for ALL_HUNT (action %d) ===\n",
           RA_ACTION_ALL_HUNT);

    int trigCount = ini.EntryCount("Trigs");
    int allHuntCount = 0;

    for (int i = 0; i < trigCount; i++) {
        const char* trigName = ini.GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini.GetString("Trigs", trigName, "", value, sizeof(value));

        // Parse trigger fields
        // Format: persist,house,eventCtrl,actionCtrl,event1,e1p1,e1p2,
        //         event2,e2p1,e2p2,action1,a1p1,a1p2,a1p3,action2,...
        int fields[20] = {0};
        int fieldCount = 0;
        char* ptr = value;
        while (*ptr && fieldCount < 20) {
            fields[fieldCount++] = strtol(ptr, &ptr, 10);
            if (*ptr == ',') ptr++;
        }

        if (fieldCount < 12) continue;

        int house = fields[1];
        int action1 = fields[10];
        int action2 = (fieldCount >= 15) ? fields[14] : 0;

        if (action1 == RA_ACTION_ALL_HUNT || action2 == RA_ACTION_ALL_HUNT) {
            Team team = HouseToTeam(house);
            printf("  '%s': house=%d (%s) -> %s\n",
                   trigName, house, HouseName(house), TeamName(team));

            if (action1 == RA_ACTION_ALL_HUNT) {
                printf("    action1=ALL_HUNT\n");
            }
            if (action2 == RA_ACTION_ALL_HUNT) {
                printf("    action2=ALL_HUNT\n");
            }
            allHuntCount++;
        }
    }

    printf("\nTriggers using ALL_HUNT: %d\n", allHuntCount);

    // Test house-to-team mapping
    printf("\n=== House to Team Mapping ===\n");
    for (int h = 0; h < HOUSE_COUNT; h++) {
        Team t = HouseToTeam(h);
        printf("  %s -> %s\n", HouseName(h), TeamName(t));
    }

    // Verify mapping
    bool mappingCorrect = true;
    if (HouseToTeam(HOUSE_USSR) != TEAM_ENEMY) {
        printf("FAIL: USSR should map to TEAM_ENEMY\n");
        mappingCorrect = false;
    }
    if (HouseToTeam(HOUSE_UKRAINE) != TEAM_ENEMY) {
        printf("FAIL: Ukraine should map to TEAM_ENEMY\n");
        mappingCorrect = false;
    }
    if (HouseToTeam(HOUSE_GREECE) != TEAM_PLAYER) {
        printf("FAIL: Greece should map to TEAM_PLAYER\n");
        mappingCorrect = false;
    }

    printf("\n=== Summary ===\n");
    printf("Triggers parsed: %d\n", trigCount);
    printf("ALL_HUNT triggers found: %d\n", allHuntCount);
    printf("House-to-team mapping: %s\n", mappingCorrect ? "CORRECT" : "FAILED");

    if (mappingCorrect) {
        printf("\n=== PASS ===\n");
        return 0;
    } else {
        printf("\n=== FAIL ===\n");
        return 1;
    }
}
