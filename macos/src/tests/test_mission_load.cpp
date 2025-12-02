/**
 * Test: Load mission from INI file and display parsed data
 * Standalone test - uses INI parser directly
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../game/ini.h"

// Cell number to X/Y conversion
#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

int main(int argc, char** argv) {
    const char* missionFile = argc > 1 ? argv[1] : "/tmp/ra_extract/SCG01EA.INI";

    printf("=== Mission Loader Test ===\n");
    printf("Loading: %s\n\n", missionFile);

    INIClass ini;
    if (!ini.Load(missionFile)) {
        printf("ERROR: Failed to load INI file\n");
        return 1;
    }

    // [Basic] section
    printf("=== [Basic] Section ===\n");
    char name[64], player[32], brief[32], win[32], lose[32];
    ini.GetString("Basic", "Name", "Unknown", name, sizeof(name));
    ini.GetString("Basic", "Player", "Greece", player, sizeof(player));
    ini.GetString("Basic", "Brief", "", brief, sizeof(brief));
    ini.GetString("Basic", "Win", "", win, sizeof(win));
    ini.GetString("Basic", "Lose", "", lose, sizeof(lose));

    printf("Name:        %s\n", name);
    printf("Player:      %s\n", player);
    printf("Brief Video: %s\n", brief);
    printf("Win Video:   %s\n", win);
    printf("Lose Video:  %s\n", lose);

    // [Map] section
    printf("\n=== [Map] Section ===\n");
    char theater[32];
    ini.GetString("Map", "Theater", "TEMPERATE", theater, sizeof(theater));
    int mapX = ini.GetInt("Map", "X", 0);
    int mapY = ini.GetInt("Map", "Y", 0);
    int mapW = ini.GetInt("Map", "Width", 64);
    int mapH = ini.GetInt("Map", "Height", 64);

    printf("Theater:     %s\n", theater);
    printf("Position:    %d, %d\n", mapX, mapY);
    printf("Size:        %d x %d\n", mapW, mapH);

    // [Briefing] section
    printf("\n=== [Briefing] Section ===\n");
    char briefText[512] = "";
    for (int i = 1; i <= 10; i++) {
        char key[8], line[256];
        snprintf(key, sizeof(key), "%d", i);
        ini.GetString("Briefing", key, "", line, sizeof(line));
        if (strlen(line) > 0) {
            if (strlen(briefText) > 0) strcat(briefText, " ");
            strcat(briefText, line);
        }
    }
    printf("%s\n", briefText);

    // [UNITS] section
    printf("\n=== [UNITS] Section ===\n");
    int unitCount = ini.EntryCount("UNITS");
    printf("Total: %d vehicle units\n", unitCount);
    for (int i = 0; i < unitCount && i < 10; i++) {
        const char* entry = ini.GetEntry("UNITS", i);
        if (!entry) continue;

        char value[128];
        ini.GetString("UNITS", entry, "", value, sizeof(value));

        char house[32], type[32];
        int health, cell, facing;
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d", house, type, &health, &cell, &facing) >= 4) {
            printf("  [%2d] %s %s @ cell %d (%d,%d)\n",
                   i, house, type, cell, CELL_TO_X(cell), CELL_TO_Y(cell));
        }
    }
    if (unitCount > 10) {
        printf("  ... and %d more\n", unitCount - 10);
    }

    // [INFANTRY] section
    printf("\n=== [INFANTRY] Section ===\n");
    int infCount = ini.EntryCount("INFANTRY");
    printf("Total: %d infantry units\n", infCount);
    for (int i = 0; i < infCount && i < 10; i++) {
        const char* entry = ini.GetEntry("INFANTRY", i);
        if (!entry) continue;

        char value[128];
        ini.GetString("INFANTRY", entry, "", value, sizeof(value));

        char house[32], type[32];
        int health, cell, subCell;
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d", house, type, &health, &cell, &subCell) >= 4) {
            printf("  [%2d] %s %s @ cell %d (%d,%d)\n",
                   i, house, type, cell, CELL_TO_X(cell), CELL_TO_Y(cell));
        }
    }
    if (infCount > 10) {
        printf("  ... and %d more\n", infCount - 10);
    }

    // [STRUCTURES] section
    printf("\n=== [STRUCTURES] Section ===\n");
    int bldCount = ini.EntryCount("STRUCTURES");
    printf("Total: %d buildings\n", bldCount);
    for (int i = 0; i < bldCount; i++) {
        const char* entry = ini.GetEntry("STRUCTURES", i);
        if (!entry) continue;

        char value[128];
        ini.GetString("STRUCTURES", entry, "", value, sizeof(value));

        char house[32], type[32];
        int health, cell, facing;
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d", house, type, &health, &cell, &facing) >= 4) {
            printf("  [%2d] %s %s @ cell %d (%d,%d)\n",
                   i, house, type, cell, CELL_TO_X(cell), CELL_TO_Y(cell));
        }
    }

    printf("\n=== Done ===\n");
    return 0;
}
