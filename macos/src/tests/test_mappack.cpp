/**
 * Test MapPack decoder
 * Verifies base64 -> LCW -> terrain data pipeline
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../game/mission.h"
#include "../game/ini.h"

// Stubs for functions we don't need in this test
extern "C" void Map_Init() {}
extern "C" void Map_GenerateDemo() {}
extern "C" void Units_Init() {}
extern "C" void AI_Init() {}
int Units_Spawn(UnitType, Team, int, int) { return -1; }
int Buildings_Spawn(BuildingType, Team, int, int) { return -1; }
int Units_CountByTeam(Team) { return 0; }
Building* Buildings_Get(int) { return nullptr; }

int main(int argc, char* argv[]) {
    const char* iniPath = "/tmp/ra_extract/SCG01EA.INI";
    if (argc > 1) {
        iniPath = argv[1];
    }

    printf("Testing MapPack decoder with: %s\n", iniPath);

    MissionData mission;
    Mission_Init(&mission);

    if (!Mission_LoadFromINI(&mission, iniPath)) {
        printf("FAIL: Could not load mission INI\n");
        return 1;
    }

    printf("\n=== Mission Info ===\n");
    printf("Name: %s\n", mission.name);
    printf("Theater: %d (0=temp, 1=snow, 2=int, 3=desert)\n",
           mission.theater);
    printf("Map: %d,%d size %dx%d\n",
           mission.mapX, mission.mapY,
           mission.mapWidth, mission.mapHeight);
    printf("Units: %d\n", mission.unitCount);
    printf("Buildings: %d\n", mission.buildingCount);
    printf("Brief video: %s\n", mission.briefVideo);

    printf("\n=== MapPack Data ===\n");
    if (mission.terrainType && mission.terrainIcon) {
        printf("TerrainType: allocated\n");
        printf("TerrainIcon: allocated\n");

        // Count non-zero terrain types
        int nonZeroTypes = 0;
        int nonZeroIcons = 0;
        for (int i = 0; i < MAP_CELL_TOTAL; i++) {
            if (mission.terrainType[i] != 0) nonZeroTypes++;
            if (mission.terrainIcon[i] != 0) nonZeroIcons++;
        }
        printf("Non-zero terrain types: %d / %d cells\n",
               nonZeroTypes, MAP_CELL_TOTAL);
        printf("Non-zero terrain icons: %d / %d cells\n",
               nonZeroIcons, MAP_CELL_TOTAL);

        // Show sample data from map area
        printf("\nSample terrain at map origin (%d,%d):\n",
               mission.mapX, mission.mapY);
        for (int dy = 0; dy < 5; dy++) {
            printf("  ");
            for (int dx = 0; dx < 10; dx++) {
                int cell = (mission.mapY + dy) * MAP_CELL_W +
                           (mission.mapX + dx);
                if (cell >= 0 && cell < MAP_CELL_TOTAL) {
                    printf("%02X/%02X ",
                           mission.terrainType[cell],
                           mission.terrainIcon[cell]);
                }
            }
            printf("\n");
        }
    } else {
        printf("TerrainType: NULL\n");
        printf("TerrainIcon: NULL\n");
    }

    printf("\n=== OverlayPack Data ===\n");
    if (mission.overlayType) {
        printf("OverlayType: allocated\n");
        int nonZero = 0;
        for (int i = 0; i < MAP_CELL_TOTAL; i++) {
            if (mission.overlayType[i] != 0 &&
                mission.overlayType[i] != 0xFF) {
                nonZero++;
            }
        }
        printf("Non-default overlay cells: %d\n", nonZero);
    } else {
        printf("OverlayType: NULL\n");
    }

    Mission_Free(&mission);

    printf("\n=== Result ===\n");
    if (mission.terrainType == nullptr &&
        mission.terrainIcon == nullptr) {
        printf("PASS: Memory freed correctly\n");
    }

    printf("Test complete.\n");
    return 0;
}
