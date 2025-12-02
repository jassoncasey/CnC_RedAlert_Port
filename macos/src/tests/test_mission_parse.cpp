/**
 * Test: Verify mission INI parsing (no spawning, just parse)
 */

#include "../game/ini.h"
#include <cstdio>
#include <cstring>

// Simplified cell coordinate macros
#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

int main(int argc, char** argv) {
    const char* missionFile = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) missionFile = argv[1];

    printf("=== Testing Mission Parse ===\n");
    printf("File: %s\n\n", missionFile);

    INIClass ini;
    if (!ini.Load(missionFile)) {
        printf("FAIL: Could not load INI file\n");
        return 1;
    }

    // [Basic] section
    char name[64], player[32], theater[32];
    ini.GetString("Basic", "Name", "Unknown", name, sizeof(name));
    ini.GetString("Basic", "Player", "Greece", player, sizeof(player));
    ini.GetString("Map", "Theater", "TEMPERATE", theater, sizeof(theater));

    printf("[Basic]\n");
    printf("  Name: %s\n", name);
    printf("  Player: %s\n", player);
    printf("\n");

    // [Map] section
    int x = ini.GetInt("Map", "X", 0);
    int y = ini.GetInt("Map", "Y", 0);
    int w = ini.GetInt("Map", "Width", 64);
    int h = ini.GetInt("Map", "Height", 64);
    printf("[Map]\n");
    printf("  Theater: %s\n", theater);
    printf("  Position: (%d, %d)\n", x, y);
    printf("  Size: %d x %d\n", w, h);
    printf("\n");

    // Count entities
    int infantryCount = ini.EntryCount("INFANTRY");
    int unitCount = ini.EntryCount("UNITS");
    int structCount = ini.EntryCount("STRUCTURES");
    int waypointCount = ini.EntryCount("Waypoints");
    int trigCount = ini.EntryCount("Trigs");
    int teamCount = ini.EntryCount("TeamTypes");

    printf("Entity Counts:\n");
    printf("  [INFANTRY]: %d\n", infantryCount);
    printf("  [UNITS]: %d\n", unitCount);
    printf("  [STRUCTURES]: %d\n", structCount);
    printf("  [Waypoints]: %d\n", waypointCount);
    printf("  [Trigs]: %d\n", trigCount);
    printf("  [TeamTypes]: %d\n", teamCount);
    printf("\n");

    // Parse a few infantry entries
    if (infantryCount > 0) {
        printf("First 5 Infantry:\n");
        for (int i = 0; i < infantryCount && i < 5; i++) {
            const char* entry = ini.GetEntry("INFANTRY", i);
            if (!entry) continue;
            char value[128];
            ini.GetString("INFANTRY", entry, "", value, sizeof(value));

            char house[32], type[32], mission[32], trigger[32];
            int health, cell, subCell, facing;
            if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                       house, type, &health, &cell, &subCell,
                       mission, &facing, trigger) >= 5) {
                printf("  %s: %s %s at (%d,%d) %s\n",
                       entry, house, type,
                       CELL_TO_X(cell), CELL_TO_Y(cell), mission);
            }
        }
        printf("\n");
    }

    // Parse a few structure entries
    if (structCount > 0) {
        printf("First 5 Structures:\n");
        for (int i = 0; i < structCount && i < 5; i++) {
            const char* entry = ini.GetEntry("STRUCTURES", i);
            if (!entry) continue;
            char value[128];
            ini.GetString("STRUCTURES", entry, "", value, sizeof(value));

            char house[32], type[32], trigger[32];
            int health, cell, facing;
            int sellable, rebuild;
            if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%d,%31s",
                       house, type, &health, &cell, &facing,
                       &sellable, trigger) >= 4) {
                printf("  %s: %s %s at (%d,%d)\n",
                       entry, house, type,
                       CELL_TO_X(cell), CELL_TO_Y(cell));
            }
        }
        printf("\n");
    }

    // Check for MapPack
    char mappack[64];
    ini.GetString("MapPack", "1", "", mappack, sizeof(mappack));
    printf("[MapPack]: %s\n", mappack[0] ? "YES (has data)" : "NO");

    // Check for OverlayPack
    char overlay[64];
    ini.GetString("OverlayPack", "1", "", overlay, sizeof(overlay));
    printf("[OverlayPack]: %s\n", overlay[0] ? "YES (has data)" : "NO");

    // Briefing
    char brief[256];
    ini.GetString("Briefing", "1", "", brief, sizeof(brief));
    printf("\n[Briefing]:\n");
    if (brief[0]) {
        printf("  %s...\n", brief);
    } else {
        printf("  (empty)\n");
    }

    printf("\n=== PASS ===\n");
    return 0;
}
