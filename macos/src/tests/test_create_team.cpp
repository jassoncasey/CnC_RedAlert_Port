/**
 * Test: CREATE_TEAM trigger action - parsing verification
 *
 * Verifies:
 * 1. Team types are parsed from INI
 * 2. Team members and missions are correctly loaded
 * 3. Waypoints are parsed for spawn locations
 *
 * Note: Full spawn testing requires the game running
 */

#include "../game/ini.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Cell coordinate macros (from mission.cpp)
#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

#define MAX_TEAM_MEMBERS 5
#define MAX_TEAM_MISSIONS 20
#define MAX_MISSION_WAYPOINTS 100

// Team member (unit type and quantity)
struct TeamMember {
    char unitType[8];
    int quantity;
};

// Team mission (action and data)
struct TeamMission {
    int mission;
    int data;
};

// Team type definition
struct TeamType {
    char name[24];
    int house;
    int flags;
    int recruitPriority;
    int initNum;
    int maxAllowed;
    int origin;
    int trigger;
    TeamMember members[MAX_TEAM_MEMBERS];
    int memberCount;
    TeamMission missions[MAX_TEAM_MISSIONS];
    int missionCount;
};

// Waypoint
struct Waypoint {
    int cell;
    int cellX;
    int cellY;
};

// Parse single team member
static bool ParseTeamMember(char** ptr, TeamMember* member) {
    char* colon = strchr(*ptr, ':');
    if (!colon) return false;

    int typeLen = (int)(colon - *ptr);
    if (typeLen > 7) typeLen = 7;
    strncpy(member->unitType, *ptr, typeLen);
    member->unitType[typeLen] = '\0';

    *ptr = colon + 1;
    char* next;
    member->quantity = strtol(*ptr, &next, 10);
    *ptr = (*next == ',') ? next + 1 : next;
    return true;
}

// Parse single team mission
static bool ParseTeamMission(char** ptr, TeamMission* tmission) {
    char* colon = strchr(*ptr, ':');
    if (!colon) return false;

    char* next;
    tmission->mission = strtol(*ptr, &colon, 10);
    *ptr = colon + 1;
    tmission->data = strtol(*ptr, &next, 10);
    *ptr = (*next == ',') ? next + 1 : next;
    return true;
}

int main(int argc, char** argv) {
    const char* missionFile = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) missionFile = argv[1];

    printf("=== Test: CREATE_TEAM Parsing ===\n\n");

    INIClass ini;
    if (!ini.Load(missionFile)) {
        printf("FAIL: Could not load %s\n", missionFile);
        return 1;
    }
    printf("Loaded: %s\n\n", missionFile);

    // Parse TeamTypes
    printf("=== Parsing [TeamTypes] ===\n");
    int teamCount = ini.EntryCount("TeamTypes");
    printf("Team type count: %d\n\n", teamCount);

    if (teamCount == 0) {
        printf("FAIL: No team types found\n");
        return 1;
    }

    TeamType teams[32];
    int parsedTeams = 0;

    for (int i = 0; i < teamCount && parsedTeams < 32; i++) {
        const char* teamName = ini.GetEntry("TeamTypes", i);
        if (!teamName) continue;

        char value[512];
        ini.GetString("TeamTypes", teamName, "", value, sizeof(value));
        if (value[0] == '\0') continue;

        TeamType* team = &teams[parsedTeams];
        memset(team, 0, sizeof(TeamType));
        strncpy(team->name, teamName, sizeof(team->name) - 1);

        char* ptr = value;
        char* next;

        // Parse: house,flags,recruit,init,max,origin,trigger,numMembers,...
        team->house = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->flags = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->recruitPriority = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->initNum = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->maxAllowed = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->origin = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->trigger = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;

        // numMembers
        int numMembers = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1;

        // Parse members
        team->memberCount = 0;
        for (int m = 0; m < numMembers && m < MAX_TEAM_MEMBERS; m++) {
            if (!ParseTeamMember(&ptr, &team->members[m])) break;
            team->memberCount++;
        }

        // numMissions
        int numMissions = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1;

        // Parse missions
        team->missionCount = 0;
        for (int m = 0; m < numMissions && m < MAX_TEAM_MISSIONS; m++) {
            if (!ParseTeamMission(&ptr, &team->missions[m])) break;
            team->missionCount++;
        }

        parsedTeams++;
    }

    printf("Parsed %d teams successfully\n\n", parsedTeams);

    // Print team details
    printf("%-12s %5s %6s %4s %4s\n", "Name", "House", "Origin", "Mem", "Miss");
    printf("%-12s %5s %6s %4s %4s\n", "----", "-----", "------", "---", "----");

    int totalUnits = 0;
    for (int i = 0; i < parsedTeams; i++) {
        TeamType* team = &teams[i];
        printf("%-12s %5d %6d %4d %4d\n",
               team->name, team->house, team->origin,
               team->memberCount, team->missionCount);

        // Print members
        for (int m = 0; m < team->memberCount; m++) {
            printf("    %s x%d\n", team->members[m].unitType,
                   team->members[m].quantity);
            totalUnits += team->members[m].quantity;
        }
    }

    printf("\nTotal units across all teams: %d\n", totalUnits);

    // Parse Waypoints
    printf("\n=== Parsing [Waypoints] ===\n");
    Waypoint waypoints[MAX_MISSION_WAYPOINTS];
    for (int i = 0; i < MAX_MISSION_WAYPOINTS; i++) {
        waypoints[i].cell = -1;
    }

    int wpCount = ini.EntryCount("Waypoints");
    printf("Waypoint entries: %d\n", wpCount);

    for (int i = 0; i < wpCount; i++) {
        const char* entry = ini.GetEntry("Waypoints", i);
        if (!entry) continue;

        int wpNum = atoi(entry);
        if (wpNum < 0 || wpNum >= MAX_MISSION_WAYPOINTS) continue;

        int cell = ini.GetInt("Waypoints", entry, -1);
        if (cell < 0) continue;

        waypoints[wpNum].cell = cell;
        waypoints[wpNum].cellX = CELL_TO_X(cell);
        waypoints[wpNum].cellY = CELL_TO_Y(cell);
    }

    // Verify team origin waypoints
    printf("\n=== Verifying Team Origin Waypoints ===\n");
    int validOrigins = 0;
    int invalidOrigins = 0;

    for (int i = 0; i < parsedTeams; i++) {
        TeamType* team = &teams[i];
        int wp = team->origin;

        if (wp >= 0 && wp < MAX_MISSION_WAYPOINTS && waypoints[wp].cell >= 0) {
            validOrigins++;
            printf("  '%s': wp%d -> cell %d (%d,%d)\n",
                   team->name, wp, waypoints[wp].cell,
                   waypoints[wp].cellX, waypoints[wp].cellY);
        } else if (wp >= 0) {
            invalidOrigins++;
            printf("  '%s': wp%d -> INVALID (cell=%d)\n",
                   team->name, wp,
                   (wp < MAX_MISSION_WAYPOINTS) ? waypoints[wp].cell : -999);
        }
    }

    printf("\nValid origins: %d/%d\n", validOrigins, parsedTeams);
    if (invalidOrigins > 0) {
        printf("WARN: %d teams have invalid origin waypoints\n", invalidOrigins);
    }

    // Parse triggers for CREATE_TEAM/REINFORCE
    printf("\n=== Scanning Triggers for Team Actions ===\n");
    int trigCount = ini.EntryCount("Trigs");
    int createTeamCount = 0;
    int reinforceCount = 0;

    for (int i = 0; i < trigCount; i++) {
        const char* trigName = ini.GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini.GetString("Trigs", trigName, "", value, sizeof(value));

        // Parse fields
        int fields[20] = {0};
        int fieldCount = 0;
        char* ptr = value;
        while (*ptr && fieldCount < 20) {
            fields[fieldCount++] = strtol(ptr, &ptr, 10);
            if (*ptr == ',') ptr++;
        }

        // Check action1 (field 10) and action2 (field 14)
        // Action 4 = CREATE_TEAM, Action 7 = REINFORCE
        if (fieldCount >= 12) {
            int action1 = fields[10];
            int a1p1 = fields[11];

            if (action1 == 4) {
                const char* teamName = (a1p1 >= 0 && a1p1 < parsedTeams) ?
                    teams[a1p1].name : "???";
                printf("  '%s': CREATE_TEAM team=%d (%s)\n",
                       trigName, a1p1, teamName);
                createTeamCount++;
            } else if (action1 == 7) {
                const char* teamName = (a1p1 >= 0 && a1p1 < parsedTeams) ?
                    teams[a1p1].name : "???";
                printf("  '%s': REINFORCE team=%d (%s)\n",
                       trigName, a1p1, teamName);
                reinforceCount++;
            }
        }

        if (fieldCount >= 16) {
            int action2 = fields[14];
            int a2p1 = fields[15];

            if (action2 == 4) {
                const char* teamName = (a2p1 >= 0 && a2p1 < parsedTeams) ?
                    teams[a2p1].name : "???";
                printf("  '%s': CREATE_TEAM (action2) team=%d (%s)\n",
                       trigName, a2p1, teamName);
                createTeamCount++;
            } else if (action2 == 7) {
                const char* teamName = (a2p1 >= 0 && a2p1 < parsedTeams) ?
                    teams[a2p1].name : "???";
                printf("  '%s': REINFORCE (action2) team=%d (%s)\n",
                       trigName, a2p1, teamName);
                reinforceCount++;
            }
        }
    }

    printf("\nTriggers using CREATE_TEAM: %d\n", createTeamCount);
    printf("Triggers using REINFORCE: %d\n", reinforceCount);

    // Summary
    printf("\n=== Summary ===\n");
    printf("Team types parsed: %d\n", parsedTeams);
    printf("Total units in teams: %d\n", totalUnits);
    printf("Teams with valid waypoints: %d\n", validOrigins);
    printf("CREATE_TEAM triggers: %d\n", createTeamCount);
    printf("REINFORCE triggers: %d\n", reinforceCount);

    if (parsedTeams > 0 && validOrigins > 0 &&
        (createTeamCount > 0 || reinforceCount > 0)) {
        printf("\n=== PASS ===\n");
        return 0;
    } else {
        printf("\n=== FAIL: Missing data ===\n");
        return 1;
    }
}
