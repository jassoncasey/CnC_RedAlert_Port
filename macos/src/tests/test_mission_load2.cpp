/**
 * Test: Verify mission loading from actual campaign INI files
 */

#include "../game/mission.h"
#include "../game/ini.h"
#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    const char* missionFile = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) missionFile = argv[1];

    printf("=== Testing Mission Load ===\n");
    printf("File: %s\n\n", missionFile);

    MissionData mission;
    if (!Mission_LoadFromINI(&mission, missionFile)) {
        printf("FAIL: Could not load mission\n");
        return 1;
    }

    printf("Mission loaded successfully!\n\n");
    printf("Name: %s\n", mission.name);
    printf("Theater: %d\n", mission.theater);
    printf("Map: %dx%d at (%d,%d)\n",
           mission.mapWidth, mission.mapHeight,
           mission.mapX, mission.mapY);
    printf("Credits: %d\n", mission.startCredits);
    printf("\n");

    printf("Terrain data: %s\n",
           mission.terrainType ? "YES" : "NO");
    printf("Overlay data: %s\n",
           mission.overlayType ? "YES" : "NO");
    printf("\n");

    printf("Units: %d\n", mission.unitCount);
    for (int i = 0; i < mission.unitCount && i < 10; i++) {
        MissionUnit* u = &mission.units[i];
        printf("  [%d] type=%d team=%d cell=(%d,%d)\n",
               i, u->type, u->team, u->cellX, u->cellY);
    }
    if (mission.unitCount > 10)
        printf("  ... and %d more\n", mission.unitCount - 10);
    printf("\n");

    printf("Buildings: %d\n", mission.buildingCount);
    for (int i = 0; i < mission.buildingCount && i < 10; i++) {
        MissionBuilding* b = &mission.buildings[i];
        printf("  [%d] type=%d team=%d cell=(%d,%d)\n",
               i, b->type, b->team, b->cellX, b->cellY);
    }
    if (mission.buildingCount > 10)
        printf("  ... and %d more\n", mission.buildingCount - 10);
    printf("\n");

    printf("Waypoints: %d\n", mission.waypointCount);
    int shown = 0;
    for (int i = 0; i < MAX_MISSION_WAYPOINTS && shown < 5; i++) {
        if (mission.waypoints[i].cell >= 0) {
            printf("  WP%d: cell=%d (%d,%d)\n",
                   i, mission.waypoints[i].cell,
                   mission.waypoints[i].cellX,
                   mission.waypoints[i].cellY);
            shown++;
        }
    }
    printf("\n");

    printf("Team Types: %d\n", mission.teamTypeCount);
    for (int i = 0; i < mission.teamTypeCount && i < 5; i++) {
        MissionTeamType* t = &mission.teamTypes[i];
        printf("  '%s': house=%d members=%d missions=%d\n",
               t->name, t->house, t->memberCount, t->missionCount);
    }
    printf("\n");

    printf("Briefing video: %s\n",
           mission.briefVideo[0] ? mission.briefVideo : "(none)");
    printf("Win video: %s\n",
           mission.winVideo[0] ? mission.winVideo : "(none)");
    printf("Lose video: %s\n",
           mission.loseVideo[0] ? mission.loseVideo : "(none)");
    printf("\n");

    printf("Description:\n%s\n", mission.description);

    Mission_Free(&mission);

    printf("\n=== PASS ===\n");
    return 0;
}
