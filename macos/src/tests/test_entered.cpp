/**
 * Test: ENTERED trigger event - parsing and zone verification
 *
 * Verifies:
 * 1. Triggers with event 1 (ENTERED) are correctly identified
 * 2. Waypoint-based triggers (param1 >= 0) reference valid waypoints
 * 3. Cell-based triggers (param1 < 0) have associated CellTriggers entries
 *
 * Note: Full zone detection testing requires the game running
 */

#include "../game/ini.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Event number for ENTERED
#define RA_EVENT_ENTERED 1

// Map constants
#define MAP_CELL_W 128
#define CELL_TO_X(cell) ((cell) % MAP_CELL_W)
#define CELL_TO_Y(cell) ((cell) / MAP_CELL_W)

#define MAX_WAYPOINTS 100
#define MAX_CELL_TRIGGERS 256

struct Waypoint {
    int cell;
    int cellX;
    int cellY;
};

struct CellTrigger {
    int cell;
    char trigName[24];
};

int main(int argc, char** argv) {
    const char* missionFile = "/tmp/ra_extract/SCU01EA.INI";
    if (argc > 1) missionFile = argv[1];

    printf("=== Test: ENTERED Event Parsing ===\n\n");

    INIClass ini;
    if (!ini.Load(missionFile)) {
        printf("FAIL: Could not load %s\n", missionFile);
        return 1;
    }
    printf("Loaded: %s\n\n", missionFile);

    // Parse waypoints first
    printf("=== Parsing [Waypoints] ===\n");
    Waypoint waypoints[MAX_WAYPOINTS];
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        waypoints[i].cell = -1;
    }

    int wpCount = ini.EntryCount("Waypoints");
    printf("Waypoint entries: %d\n", wpCount);

    for (int i = 0; i < wpCount; i++) {
        const char* entry = ini.GetEntry("Waypoints", i);
        if (!entry) continue;

        int wpNum = atoi(entry);
        if (wpNum < 0 || wpNum >= MAX_WAYPOINTS) continue;

        int cell = ini.GetInt("Waypoints", entry, -1);
        if (cell < 0) continue;

        waypoints[wpNum].cell = cell;
        waypoints[wpNum].cellX = CELL_TO_X(cell);
        waypoints[wpNum].cellY = CELL_TO_Y(cell);
    }

    // Parse cell triggers
    printf("\n=== Parsing [CellTriggers] ===\n");
    CellTrigger cellTrigs[MAX_CELL_TRIGGERS];
    int cellTrigCount = 0;

    int ctCount = ini.EntryCount("CellTriggers");
    printf("CellTrigger entries: %d\n", ctCount);

    for (int i = 0; i < ctCount && cellTrigCount < MAX_CELL_TRIGGERS; i++) {
        const char* entry = ini.GetEntry("CellTriggers", i);
        if (!entry) continue;

        int cell = atoi(entry);
        if (cell < 0) continue;

        char trigName[24];
        ini.GetString("CellTriggers", entry, "", trigName, sizeof(trigName));
        if (trigName[0] == '\0') continue;

        cellTrigs[cellTrigCount].cell = cell;
        strncpy(cellTrigs[cellTrigCount].trigName, trigName, 23);
        cellTrigs[cellTrigCount].trigName[23] = '\0';
        cellTrigCount++;
    }

    // Helper to check if a trigger has cell-based entries
    auto hasCellTriggers = [&](const char* name) -> int {
        int count = 0;
        for (int i = 0; i < cellTrigCount; i++) {
            if (strcasecmp(cellTrigs[i].trigName, name) == 0) {
                count++;
            }
        }
        return count;
    };

    // Scan triggers for ENTERED events
    printf("\n=== Scanning Triggers for ENTERED (event %d) ===\n",
           RA_EVENT_ENTERED);

    int trigCount = ini.EntryCount("Trigs");
    int enteredCount = 0;
    int waypointBased = 0;
    int cellBased = 0;
    int objectBased = 0;  // param=-1, no cells - requires object attachment
    int invalidRefs = 0;

    for (int i = 0; i < trigCount; i++) {
        const char* trigName = ini.GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini.GetString("Trigs", trigName, "", value, sizeof(value));

        // Parse trigger fields
        // Format: persist,house,eventCtrl,actionCtrl,event1,e1p1,e1p2,...
        int fields[20] = {0};
        int fieldCount = 0;
        char* ptr = value;
        while (*ptr && fieldCount < 20) {
            fields[fieldCount++] = strtol(ptr, &ptr, 10);
            if (*ptr == ',') ptr++;
        }

        if (fieldCount < 7) continue;

        int event1 = fields[4];
        int e1p1 = fields[5];  // Waypoint for ENTERED (or -1 for cell-based)
        int event2 = (fieldCount >= 9) ? fields[8] : 0;
        int e2p1 = (fieldCount >= 10) ? fields[9] : 0;

        // Check event1
        if (event1 == RA_EVENT_ENTERED) {
            int wp = e1p1;
            if (wp >= 0 && wp < MAX_WAYPOINTS && waypoints[wp].cell >= 0) {
                // Waypoint-based trigger
                printf("  '%s': event1=ENTERED, waypoint %d -> cell %d (%d,%d)\n",
                       trigName, wp, waypoints[wp].cell,
                       waypoints[wp].cellX, waypoints[wp].cellY);
                waypointBased++;
            } else if (wp < 0) {
                // Cell-based or object-attached trigger
                int cells = hasCellTriggers(trigName);
                if (cells > 0) {
                    printf("  '%s': event1=ENTERED, cell-based (%d cells)\n",
                           trigName, cells);
                    cellBased++;
                } else {
                    // Object-attached: fires when player enters object's cell
                    // (needs object trigger tracking to work)
                    printf("  '%s': event1=ENTERED, object-attached (no cells)\n",
                           trigName);
                    objectBased++;
                }
            } else {
                printf("  '%s': event1=ENTERED, invalid waypoint %d\n",
                       trigName, wp);
                invalidRefs++;
            }
            enteredCount++;
        }

        // Check event2
        if (event2 == RA_EVENT_ENTERED) {
            int wp = e2p1;
            if (wp >= 0 && wp < MAX_WAYPOINTS && waypoints[wp].cell >= 0) {
                printf("  '%s': event2=ENTERED, waypoint %d -> cell %d (%d,%d)\n",
                       trigName, wp, waypoints[wp].cell,
                       waypoints[wp].cellX, waypoints[wp].cellY);
                waypointBased++;
            } else if (wp < 0) {
                int cells = hasCellTriggers(trigName);
                if (cells > 0) {
                    printf("  '%s': event2=ENTERED, cell-based (%d cells)\n",
                           trigName, cells);
                    cellBased++;
                } else {
                    printf("  '%s': event2=ENTERED, object-attached (no cells)\n",
                           trigName);
                    objectBased++;
                }
            } else {
                printf("  '%s': event2=ENTERED, invalid waypoint %d\n",
                       trigName, wp);
                invalidRefs++;
            }
            enteredCount++;
        }
    }

    // Summary
    printf("\n=== Summary ===\n");
    printf("Waypoints parsed: %d\n", wpCount);
    printf("Cell triggers parsed: %d\n", cellTrigCount);
    printf("ENTERED triggers found: %d\n", enteredCount);
    printf("  Waypoint-based: %d\n", waypointBased);
    printf("  Cell-based: %d\n", cellBased);
    printf("  Object-attached: %d\n", objectBased);
    printf("  Invalid references: %d\n", invalidRefs);

    // Pass if parsing works - object-attached triggers are valid but
    // require object trigger tracking (not yet implemented)
    if (invalidRefs == 0) {
        printf("\n=== PASS ===\n");
        return 0;
    } else {
        printf("\n=== FAIL: Invalid references found ===\n");
        return 1;
    }
}
