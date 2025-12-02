/**
 * Test terrain rendering from MapPack data
 */

#include <cstdio>
#include "game/terrain.h"
#include "game/mission.h"
#include "assets/assetloader.h"

// Stubs for mission functions we don't need
extern "C" void Map_Init() {}
extern "C" void Map_GenerateDemo() {}
extern "C" void Units_Init() {}
extern "C" void AI_Init() {}
int Units_Spawn(UnitType, Team, int, int) { return -1; }
int Buildings_Spawn(BuildingType, Team, int, int) { return -1; }
int Units_CountByTeam(Team) { return 0; }
Building* Buildings_Get(int) { return nullptr; }

// Stub for renderer (we just want to test template loading)
extern "C" void Renderer_Blit(const uint8_t*, int, int, int, int, int) {}

int main(int argc, char* argv[]) {
    const char* iniPath = "/tmp/ra_extract/SCG01EA.INI";
    if (argc > 1) {
        iniPath = argv[1];
    }

    printf("Testing terrain rendering from: %s\n\n", iniPath);

    // Initialize assets
    if (!Assets_Init()) {
        printf("FAIL: Could not initialize assets\n");
        return 1;
    }

    // Load mission
    MissionData mission;
    Mission_Init(&mission);

    if (!Mission_LoadFromINI(&mission, iniPath)) {
        printf("FAIL: Could not load mission INI\n");
        return 1;
    }

    printf("Mission: %s\n", mission.name);
    printf("Theater: %d\n", mission.theater);
    printf("Map: %d,%d size %dx%d\n",
           mission.mapX, mission.mapY,
           mission.mapWidth, mission.mapHeight);

    if (!mission.terrainType || !mission.terrainIcon) {
        printf("FAIL: No terrain data loaded\n");
        Mission_Free(&mission);
        return 1;
    }

    // Set theater
    Terrain_SetTheater(mission.theater);
    Assets_SetTheater((TheaterType)mission.theater);
    printf("Set theater to %d\n\n", mission.theater);

    // Initialize terrain
    if (!Terrain_Init()) {
        printf("FAIL: Could not initialize terrain\n");
        Mission_Free(&mission);
        return 1;
    }

    // Test rendering a sample of tiles from the map
    printf("Testing terrain rendering (no visual output, just loading):\n");

    int loadedCount = 0;
    int failedCount = 0;

    // Sample 100 cells from the map area
    for (int dy = 0; dy < 10; dy++) {
        for (int dx = 0; dx < 10; dx++) {
            int cellX = mission.mapX + dx * (mission.mapWidth / 10);
            int cellY = mission.mapY + dy * (mission.mapHeight / 10);
            int cell = cellY * MAP_CELL_W + cellX;

            if (cell < 0 || cell >= MAP_CELL_TOTAL) continue;

            int templateID = mission.terrainType[cell];
            int tileIndex = mission.terrainIcon[cell];

            // Skip clear terrain
            if (templateID == 255 || templateID == 0xFF) continue;

            // Try to render
            if (Terrain_RenderByID(templateID, tileIndex, 0, 0)) {
                loadedCount++;
            } else {
                failedCount++;
                printf("  FAIL: Template %d (0x%02X), tile %d\n",
                       templateID, templateID, tileIndex);
            }
        }
    }

    printf("\nLoaded: %d, Failed: %d\n", loadedCount, failedCount);

    // Show unique template IDs in the map
    printf("\nUnique template IDs in map area:\n");
    int templateCounts[256] = {0};
    for (int y = mission.mapY; y < mission.mapY + mission.mapHeight; y++) {
        for (int x = mission.mapX; x < mission.mapX + mission.mapWidth; x++) {
            int cell = y * MAP_CELL_W + x;
            if (cell >= 0 && cell < MAP_CELL_TOTAL) {
                templateCounts[mission.terrainType[cell]]++;
            }
        }
    }

    for (int i = 0; i < 256; i++) {
        if (templateCounts[i] > 0) {
            printf("  Template %3d (0x%02X): %d cells\n", i, i, templateCounts[i]);
        }
    }

    Mission_Free(&mission);
    Terrain_Shutdown();
    Assets_Shutdown();

    printf("\nTest complete.\n");
    return failedCount > 0 ? 1 : 0;
}
