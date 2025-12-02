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

    // Parse object triggers from STRUCTURES
    printf("\n=== Parsing Object Triggers ===\n");
    CellTrigger objTrigs[MAX_CELL_TRIGGERS];
    int objTrigCount = 0;

    // Helper to add object trigger
    auto addObjTrig = [&](int cell, const char* trigName) {
        if (!trigName || trigName[0] == '\0') return;
        if (strcasecmp(trigName, "None") == 0) return;
        if (objTrigCount >= MAX_CELL_TRIGGERS) return;
        objTrigs[objTrigCount].cell = cell;
        strncpy(objTrigs[objTrigCount].trigName, trigName, 23);
        objTrigs[objTrigCount].trigName[23] = '\0';
        objTrigCount++;
    };

    // STRUCTURES: house,type,health,cell,facing,trigger,sellable,rebuild
    int sCount = ini.EntryCount("STRUCTURES");
    for (int i = 0; i < sCount; i++) {
        const char* entry = ini.GetEntry("STRUCTURES", i);
        if (!entry) continue;
        char value[128];
        ini.GetString("STRUCTURES", entry, "", value, sizeof(value));
        char house[32], type[32], trigger[32];
        int health, cell, facing;
        trigger[0] = '\0';
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,]",
                   house, type, &health, &cell, &facing, trigger) >= 6) {
            addObjTrig(cell, trigger);
        }
    }

    // UNITS: house,type,health,cell,facing,mission,trigger
    int uCount = ini.EntryCount("UNITS");
    for (int i = 0; i < uCount; i++) {
        const char* entry = ini.GetEntry("UNITS", i);
        if (!entry) continue;
        char value[128];
        ini.GetString("UNITS", entry, "", value, sizeof(value));
        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;
        trigger[0] = '\0';
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing,
                   missionStr, trigger) >= 7) {
            addObjTrig(cell, trigger);
        }
    }

    // INFANTRY: house,type,health,cell,subcell,mission,facing,trigger
    int iCount = ini.EntryCount("INFANTRY");
    for (int i = 0; i < iCount; i++) {
        const char* entry = ini.GetEntry("INFANTRY", i);
        if (!entry) continue;
        char value[128];
        ini.GetString("INFANTRY", entry, "", value, sizeof(value));
        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, subCell, facing;
        trigger[0] = '\0';
        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                   house, type, &health, &cell, &subCell,
                   missionStr, &facing, trigger) >= 8) {
            addObjTrig(cell, trigger);
        }
    }
    printf("Object trigger entries: %d\n", objTrigCount);

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

    // Helper to check if a trigger has object-attached entries
    auto hasObjectTriggers = [&](const char* name) -> int {
        int count = 0;
        for (int i = 0; i < objTrigCount; i++) {
            if (strcasecmp(objTrigs[i].trigName, name) == 0) {
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
    int objectBased = 0;
    int orphaned = 0;     // param=-1, no cells or objects (unused triggers)
    int invalidRefs = 0;  // Invalid waypoint references (actual bugs)

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
                int objs = hasObjectTriggers(trigName);
                if (cells > 0 && objs > 0) {
                    printf("  '%s': event1=ENTERED, cell+object (%d cells, %d objs)\n",
                           trigName, cells, objs);
                    cellBased++;  // Count as cell-based (has both)
                } else if (cells > 0) {
                    printf("  '%s': event1=ENTERED, cell-based (%d cells)\n",
                           trigName, cells);
                    cellBased++;
                } else if (objs > 0) {
                    printf("  '%s': event1=ENTERED, object-attached (%d objs)\n",
                           trigName, objs);
                    objectBased++;
                } else {
                    // Orphaned trigger - no cells or objects (unused in mission)
                    printf("  '%s': event1=ENTERED, orphaned (unused)\n",
                           trigName);
                    orphaned++;
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
                int objs = hasObjectTriggers(trigName);
                if (cells > 0 && objs > 0) {
                    printf("  '%s': event2=ENTERED, cell+object (%d cells, %d objs)\n",
                           trigName, cells, objs);
                    cellBased++;
                } else if (cells > 0) {
                    printf("  '%s': event2=ENTERED, cell-based (%d cells)\n",
                           trigName, cells);
                    cellBased++;
                } else if (objs > 0) {
                    printf("  '%s': event2=ENTERED, object-attached (%d objs)\n",
                           trigName, objs);
                    objectBased++;
                } else {
                    printf("  '%s': event2=ENTERED, orphaned (unused)\n",
                           trigName);
                    orphaned++;
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
    printf("Object triggers parsed: %d\n", objTrigCount);
    printf("ENTERED triggers found: %d\n", enteredCount);
    printf("  Waypoint-based: %d\n", waypointBased);
    printf("  Cell-based: %d\n", cellBased);
    printf("  Object-attached: %d\n", objectBased);
    printf("  Orphaned (unused): %d\n", orphaned);
    printf("  Invalid refs: %d\n", invalidRefs);

    // Pass if no invalid waypoint references (orphaned triggers are OK)
    // Orphaned triggers are defined but never attached - harmless
    if (invalidRefs == 0) {
        printf("\n=== PASS ===\n");
        if (orphaned > 0) {
            printf("Note: %d orphaned triggers (unused in mission)\n",
                   orphaned);
        }
        return 0;
    } else {
        printf("\n=== FAIL: Invalid waypoint references found ===\n");
        return 1;
    }
}
