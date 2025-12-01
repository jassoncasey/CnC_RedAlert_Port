/**
 * Red Alert macOS Port - Mission Loader Implementation
 */

#include "mission.h"
#include "ini.h"
#include "map.h"
#include "units.h"
#include "ai.h"
#include <cstring>
#include <cstdlib>

void Mission_Init(MissionData* mission) {
    if (!mission) return;

    memset(mission, 0, sizeof(MissionData));
    strcpy(mission->name, "Untitled");
    strcpy(mission->description, "No description");
    mission->theater = 0;  // Temperate
    mission->mapWidth = 64;
    mission->mapHeight = 64;
    mission->playerTeam = TEAM_PLAYER;
    mission->startCredits = 5000;
    mission->winCondition = 0;  // Destroy all enemies
    mission->loseCondition = 0; // Lose all units
    mission->timeLimit = 0;     // Unlimited
}

// Parse unit type from string
static UnitType ParseUnitType(const char* str) {
    if (!str) return UNIT_NONE;

    // Infantry
    if (strcasecmp(str, "E1") == 0 || strcasecmp(str, "RIFLE") == 0) return UNIT_RIFLE;
    if (strcasecmp(str, "E2") == 0 || strcasecmp(str, "GRENADIER") == 0) return UNIT_GRENADIER;
    if (strcasecmp(str, "E3") == 0 || strcasecmp(str, "ROCKET") == 0) return UNIT_ROCKET;
    if (strcasecmp(str, "E6") == 0 || strcasecmp(str, "ENGINEER") == 0) return UNIT_ENGINEER;

    // Vehicles
    if (strcasecmp(str, "HARV") == 0 || strcasecmp(str, "HARVESTER") == 0) return UNIT_HARVESTER;
    if (strcasecmp(str, "1TNK") == 0 || strcasecmp(str, "LTANK") == 0) return UNIT_TANK_LIGHT;
    if (strcasecmp(str, "2TNK") == 0 || strcasecmp(str, "MTANK") == 0) return UNIT_TANK_MEDIUM;
    if (strcasecmp(str, "3TNK") == 0 || strcasecmp(str, "HTANK") == 0) return UNIT_TANK_HEAVY;
    if (strcasecmp(str, "APC") == 0) return UNIT_APC;
    if (strcasecmp(str, "ARTY") == 0 || strcasecmp(str, "ARTILLERY") == 0) return UNIT_ARTILLERY;

    // Naval
    if (strcasecmp(str, "GNBT") == 0 || strcasecmp(str, "GUNBOAT") == 0) return UNIT_GUNBOAT;
    if (strcasecmp(str, "DD") == 0 || strcasecmp(str, "DESTROYER") == 0) return UNIT_DESTROYER;

    return UNIT_NONE;
}

// Parse building type from string
static BuildingType ParseBuildingType(const char* str) {
    if (!str) return BUILDING_NONE;

    if (strcasecmp(str, "FACT") == 0 || strcasecmp(str, "CONSTRUCTION") == 0) return BUILDING_CONSTRUCTION;
    if (strcasecmp(str, "POWR") == 0 || strcasecmp(str, "POWER") == 0) return BUILDING_POWER;
    if (strcasecmp(str, "PROC") == 0 || strcasecmp(str, "REFINERY") == 0) return BUILDING_REFINERY;
    if (strcasecmp(str, "TENT") == 0 || strcasecmp(str, "BARR") == 0 || strcasecmp(str, "BARRACKS") == 0) return BUILDING_BARRACKS;
    if (strcasecmp(str, "WEAP") == 0 || strcasecmp(str, "FACTORY") == 0) return BUILDING_FACTORY;
    if (strcasecmp(str, "DOME") == 0 || strcasecmp(str, "RADAR") == 0) return BUILDING_RADAR;
    if (strcasecmp(str, "GUN") == 0 || strcasecmp(str, "TURRET") == 0) return BUILDING_TURRET;
    if (strcasecmp(str, "SAM") == 0) return BUILDING_SAM;

    return BUILDING_NONE;
}

// Parse team from string
static Team ParseTeam(const char* str) {
    if (!str) return TEAM_NEUTRAL;

    // Player side names (Allies)
    if (strcasecmp(str, "Greece") == 0 ||
        strcasecmp(str, "England") == 0 ||
        strcasecmp(str, "France") == 0 ||
        strcasecmp(str, "Germany") == 0 ||
        strcasecmp(str, "Spain") == 0 ||
        strcasecmp(str, "GoodGuy") == 0 ||
        strcasecmp(str, "Player") == 0 ||
        strcasecmp(str, "Allies") == 0) {
        return TEAM_PLAYER;
    }

    // Enemy side names (Soviet)
    if (strcasecmp(str, "USSR") == 0 ||
        strcasecmp(str, "Ukraine") == 0 ||
        strcasecmp(str, "BadGuy") == 0 ||
        strcasecmp(str, "Enemy") == 0 ||
        strcasecmp(str, "Soviet") == 0) {
        return TEAM_ENEMY;
    }

    return TEAM_NEUTRAL;
}

int Mission_LoadFromINI(MissionData* mission, const char* filename) {
    if (!mission || !filename) return 0;

    INIClass ini;
    if (!ini.Load(filename)) {
        return 0;
    }

    // Initialize defaults
    Mission_Init(mission);

    // [Basic] section
    ini.GetString("Basic", "Name", "Mission", mission->name, sizeof(mission->name));
    ini.GetString("Basic", "Brief", "Complete the mission.", mission->description, sizeof(mission->description));

    // Theater
    char theaterStr[32];
    ini.GetString("Basic", "Theater", "TEMPERATE", theaterStr, sizeof(theaterStr));
    if (strcasecmp(theaterStr, "SNOW") == 0) mission->theater = 1;
    else if (strcasecmp(theaterStr, "INTERIOR") == 0) mission->theater = 2;
    else mission->theater = 0;

    // Player
    char playerStr[32];
    ini.GetString("Basic", "Player", "Greece", playerStr, sizeof(playerStr));
    mission->playerTeam = ParseTeam(playerStr);

    // Credits
    mission->startCredits = ini.GetInt("Basic", "Credits", 5000);

    // Map size
    mission->mapWidth = ini.GetInt("Map", "Width", 64);
    mission->mapHeight = ini.GetInt("Map", "Height", 64);

    // Win/lose conditions
    mission->winCondition = ini.GetInt("Basic", "Win", 0);
    mission->loseCondition = ini.GetInt("Basic", "Lose", 0);
    mission->timeLimit = ini.GetInt("Basic", "TimeLimit", 0);

    // [UNITS] section - Format: ID=TYPE,OWNER,HEALTH,CELLX,CELLY,FACING
    int sectionIdx = -1;
    for (int i = 0; i < ini.SectionCount(); i++) {
        if (strcasecmp(ini.GetSectionName(i), "UNITS") == 0) {
            sectionIdx = i;
            break;
        }
    }

    if (sectionIdx >= 0) {
        int count = ini.EntryCount("UNITS");
        for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
            const char* entry = ini.GetEntry("UNITS", i);
            if (!entry) continue;

            char value[128];
            ini.GetString("UNITS", entry, "", value, sizeof(value));

            // Parse: TYPE,OWNER,HEALTH,CELLX,CELLY,FACING
            char type[32], owner[32];
            int health, cellX, cellY, facing;

            if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%d",
                       type, owner, &health, &cellX, &cellY, &facing) >= 4) {
                MissionUnit* unit = &mission->units[mission->unitCount];
                unit->type = ParseUnitType(type);
                unit->team = ParseTeam(owner);
                unit->cellX = cellX;
                unit->cellY = cellY;

                if (unit->type != UNIT_NONE) {
                    mission->unitCount++;
                }
            }
        }
    }

    // [STRUCTURES] section - Format: ID=TYPE,OWNER,HEALTH,CELLX,CELLY
    sectionIdx = -1;
    for (int i = 0; i < ini.SectionCount(); i++) {
        if (strcasecmp(ini.GetSectionName(i), "STRUCTURES") == 0) {
            sectionIdx = i;
            break;
        }
    }

    if (sectionIdx >= 0) {
        int count = ini.EntryCount("STRUCTURES");
        for (int i = 0; i < count && mission->buildingCount < MAX_MISSION_BUILDINGS; i++) {
            const char* entry = ini.GetEntry("STRUCTURES", i);
            if (!entry) continue;

            char value[128];
            ini.GetString("STRUCTURES", entry, "", value, sizeof(value));

            // Parse: TYPE,OWNER,HEALTH,CELLX,CELLY
            char type[32], owner[32];
            int health, cellX, cellY;

            if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d",
                       type, owner, &health, &cellX, &cellY) >= 4) {
                MissionBuilding* bld = &mission->buildings[mission->buildingCount];
                bld->type = ParseBuildingType(type);
                bld->team = ParseTeam(owner);
                bld->cellX = cellX;
                bld->cellY = cellY;

                if (bld->type != BUILDING_NONE) {
                    mission->buildingCount++;
                }
            }
        }
    }

    // [INFANTRY] section - same format as UNITS
    sectionIdx = -1;
    for (int i = 0; i < ini.SectionCount(); i++) {
        if (strcasecmp(ini.GetSectionName(i), "INFANTRY") == 0) {
            sectionIdx = i;
            break;
        }
    }

    if (sectionIdx >= 0) {
        int count = ini.EntryCount("INFANTRY");
        for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
            const char* entry = ini.GetEntry("INFANTRY", i);
            if (!entry) continue;

            char value[128];
            ini.GetString("INFANTRY", entry, "", value, sizeof(value));

            // Parse: TYPE,OWNER,HEALTH,CELLX,CELLY,FACING,ACTION,TRIGGER
            char type[32], owner[32];
            int health, cellX, cellY;

            if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d",
                       type, owner, &health, &cellX, &cellY) >= 4) {
                MissionUnit* unit = &mission->units[mission->unitCount];
                unit->type = ParseUnitType(type);
                unit->team = ParseTeam(owner);
                unit->cellX = cellX;
                unit->cellY = cellY;

                if (unit->type != UNIT_NONE) {
                    mission->unitCount++;
                }
            }
        }
    }

    return 1;
}

int Mission_LoadFromBuffer(MissionData* mission, const char* buffer, int size) {
    if (!mission || !buffer || size <= 0) return 0;

    INIClass ini;
    if (!ini.LoadFromBuffer(buffer, size)) {
        return 0;
    }

    // Use same parsing logic as file version (simplified - reparse from buffer)
    Mission_Init(mission);

    // [Basic] section
    ini.GetString("Basic", "Name", "Mission", mission->name, sizeof(mission->name));
    mission->startCredits = ini.GetInt("Basic", "Credits", 5000);

    // ... (same parsing as above, but simpler for buffer)

    return 1;
}

void Mission_Start(const MissionData* mission) {
    if (!mission) return;

    // Initialize systems
    Map_Init();
    Units_Init();
    AI_Init();

    // Generate map based on theater
    Map_GenerateDemo();  // TODO: Load actual map data

    // Set player credits
    // (Credits are managed by GameUI, this is just for reference)

    // Spawn buildings first (they affect map passability)
    for (int i = 0; i < mission->buildingCount; i++) {
        const MissionBuilding* bld = &mission->buildings[i];
        Buildings_Spawn(bld->type, bld->team, bld->cellX, bld->cellY);
    }

    // Spawn units
    for (int i = 0; i < mission->unitCount; i++) {
        const MissionUnit* unit = &mission->units[i];
        int worldX = unit->cellX * CELL_SIZE + CELL_SIZE / 2;
        int worldY = unit->cellY * CELL_SIZE + CELL_SIZE / 2;
        Units_Spawn(unit->type, unit->team, worldX, worldY);
    }
}

int Mission_CheckVictory(const MissionData* mission) {
    if (!mission) return 0;

    // Check win condition
    switch (mission->winCondition) {
        case 0:  // Destroy all enemy units and buildings
            if (Units_CountByTeam(TEAM_ENEMY) == 0) {
                // Also check buildings (simplified - count active enemy buildings)
                int enemyBuildings = 0;
                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_ENEMY) {
                        enemyBuildings++;
                    }
                }
                if (enemyBuildings == 0) {
                    return 1;  // Win!
                }
            }
            break;

        case 1:  // Destroy enemy buildings only
            {
                int enemyBuildings = 0;
                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_ENEMY) {
                        enemyBuildings++;
                    }
                }
                if (enemyBuildings == 0) {
                    return 1;  // Win!
                }
            }
            break;
    }

    // Check lose condition
    switch (mission->loseCondition) {
        case 0:  // Lose all units
            if (Units_CountByTeam(TEAM_PLAYER) == 0) {
                // Also check buildings
                int playerBuildings = 0;
                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_PLAYER) {
                        playerBuildings++;
                    }
                }
                if (playerBuildings == 0) {
                    return -1;  // Lose
                }
            }
            break;

        case 1:  // Lose all buildings
            {
                int playerBuildings = 0;
                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_PLAYER) {
                        playerBuildings++;
                    }
                }
                if (playerBuildings == 0) {
                    return -1;  // Lose
                }
            }
            break;
    }

    return 0;  // Game ongoing
}

void Mission_GetDemo(MissionData* mission) {
    if (!mission) return;

    Mission_Init(mission);

    strcpy(mission->name, "Demo Skirmish");
    strcpy(mission->description, "Destroy the enemy base.");
    mission->theater = 1;  // Snow
    mission->mapWidth = 64;
    mission->mapHeight = 64;
    mission->startCredits = 5000;
    mission->winCondition = 0;  // Destroy all
    mission->loseCondition = 0; // Lose all

    // Player buildings
    mission->buildings[mission->buildingCount++] = {BUILDING_CONSTRUCTION, TEAM_PLAYER, 2, 15};
    mission->buildings[mission->buildingCount++] = {BUILDING_POWER, TEAM_PLAYER, 6, 16};
    mission->buildings[mission->buildingCount++] = {BUILDING_BARRACKS, TEAM_PLAYER, 2, 19};
    mission->buildings[mission->buildingCount++] = {BUILDING_REFINERY, TEAM_PLAYER, 6, 19};

    // Enemy buildings
    mission->buildings[mission->buildingCount++] = {BUILDING_CONSTRUCTION, TEAM_ENEMY, 55, 10};
    mission->buildings[mission->buildingCount++] = {BUILDING_POWER, TEAM_ENEMY, 52, 10};
    mission->buildings[mission->buildingCount++] = {BUILDING_BARRACKS, TEAM_ENEMY, 55, 6};
    mission->buildings[mission->buildingCount++] = {BUILDING_FACTORY, TEAM_ENEMY, 52, 6};
    mission->buildings[mission->buildingCount++] = {BUILDING_TURRET, TEAM_ENEMY, 50, 12};
    mission->buildings[mission->buildingCount++] = {BUILDING_TURRET, TEAM_ENEMY, 58, 12};
    mission->buildings[mission->buildingCount++] = {BUILDING_REFINERY, TEAM_ENEMY, 58, 8};

    // Player units
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_PLAYER, 4, 16};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_PLAYER, 5, 17};
    mission->units[mission->unitCount++] = {UNIT_TANK_LIGHT, TEAM_PLAYER, 7, 16};
    mission->units[mission->unitCount++] = {UNIT_TANK_LIGHT, TEAM_PLAYER, 7, 18};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 3, 18};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 4, 18};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 5, 18};
    mission->units[mission->unitCount++] = {UNIT_ROCKET, TEAM_PLAYER, 2, 17};
    mission->units[mission->unitCount++] = {UNIT_HARVESTER, TEAM_PLAYER, 8, 20};

    // Enemy units
    mission->units[mission->unitCount++] = {UNIT_TANK_HEAVY, TEAM_ENEMY, 54, 12};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_ENEMY, 52, 13};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_ENEMY, 56, 13};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 50, 14};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 51, 14};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 52, 14};
    mission->units[mission->unitCount++] = {UNIT_ROCKET, TEAM_ENEMY, 54, 10};
}
