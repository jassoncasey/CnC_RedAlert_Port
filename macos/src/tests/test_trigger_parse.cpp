/**
 * Test trigger and waypoint parsing from mission INI files
 * Just tests INI parsing, no game systems
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "../game/ini.h"

#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

int main(int argc, char* argv[]) {
    const char* missionPath = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) {
        missionPath = argv[1];
    }

    printf("Testing trigger/waypoint parsing from: %s\n\n", missionPath);

    // Load INI
    INIClass ini;
    if (!ini.Load(missionPath)) {
        printf("ERROR: Failed to load INI file\n");
        return 1;
    }

    // Count triggers in [Trigs] section
    int trigCount = ini.EntryCount("Trigs");
    printf("=== TRIGGERS ===\n");
    printf("Found %d triggers in [Trigs] section\n\n", trigCount);

    // Parse first 5 triggers
    for (int i = 0; i < trigCount && i < 5; i++) {
        const char* trigName = ini.GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini.GetString("Trigs", trigName, "", value, sizeof(value));

        printf("Trigger '%s': ", trigName);

        // Parse trigger definition
        int persist, house, eventCtrl, actionCtrl;
        int event1, e1p1, e1p2, event2, e2p1, e2p2;
        int action1, a1p1, a1p2, a1p3;

        int parsed = sscanf(value,
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            &persist, &house, &eventCtrl, &actionCtrl,
            &event1, &e1p1, &e1p2, &event2, &e2p1, &e2p2,
            &action1, &a1p1, &a1p2, &a1p3);

        if (parsed >= 11) {
            printf("event1=%d action1=%d\n", event1, action1);
        } else {
            printf("parse error\n");
        }
    }
    if (trigCount > 5) {
        printf("... and %d more\n", trigCount - 5);
    }

    // Count waypoints in [Waypoints] section
    int wpCount = ini.EntryCount("Waypoints");
    printf("\n=== WAYPOINTS ===\n");
    printf("Found %d waypoints in [Waypoints] section\n\n", wpCount);

    // Parse first 10 waypoints
    for (int i = 0; i < wpCount && i < 10; i++) {
        const char* entry = ini.GetEntry("Waypoints", i);
        if (!entry) continue;

        int wpNum = atoi(entry);
        int cell = ini.GetInt("Waypoints", entry, -1);
        if (cell >= 0) {
            printf("Waypoint %d: cell=%d -> (%d,%d)\n",
                   wpNum, cell, CELL_TO_X(cell), CELL_TO_Y(cell));
        }
    }
    if (wpCount > 10) {
        printf("... and %d more\n", wpCount - 10);
    }

    printf("\n--- Parsing test complete ---\n");
    return 0;
}
