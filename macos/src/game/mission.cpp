/**
 * Red Alert macOS Port - Mission Loader Implementation
 */

#include "mission.h"
#include "ini.h"
#include "map.h"
#include "units.h"
#include "ai.h"
#include "../assets/lcw.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

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
    mission->terrainType = nullptr;
    mission->terrainIcon = nullptr;
    mission->overlayType = nullptr;
    mission->overlayData = nullptr;
}

void Mission_Free(MissionData* mission) {
    if (!mission) return;

    if (mission->terrainType) {
        free(mission->terrainType);
        mission->terrainType = nullptr;
    }
    if (mission->terrainIcon) {
        free(mission->terrainIcon);
        mission->terrainIcon = nullptr;
    }
    if (mission->overlayType) {
        free(mission->overlayType);
        mission->overlayType = nullptr;
    }
    if (mission->overlayData) {
        free(mission->overlayData);
        mission->overlayData = nullptr;
    }
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
        strcasecmp(str, "Turkey") == 0 ||
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

// Convert cell number to X/Y coordinates (Red Alert uses 128-wide maps internally)
#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

// Parse [MapPack] or [OverlayPack] section
// Format: Base64-encoded, chunked LCW compression
// Each chunk: 4-byte length (& 0xDFFFFFFF) + LCW data -> 8192 bytes
// Returns allocated buffer on success, nullptr on failure
static uint8_t* ParsePackSection(INIClass* ini, const char* section,
                                  int* outSize) {
    if (!ini || !section || !outSize) return nullptr;

    // Count entries and estimate base64 size
    int entryCount = ini->EntryCount(section);
    if (entryCount <= 0) return nullptr;

    // Concatenate all base64 lines
    int maxB64Size = entryCount * 128;
    char* b64Data = (char*)malloc(maxB64Size);
    if (!b64Data) return nullptr;

    int b64Len = 0;
    for (int i = 0; i < entryCount; i++) {
        char key[8];
        snprintf(key, sizeof(key), "%d", i + 1);
        char line[128];
        ini->GetString(section, key, "", line, sizeof(line));
        int lineLen = (int)strlen(line);
        if (b64Len + lineLen < maxB64Size) {
            memcpy(b64Data + b64Len, line, lineLen);
            b64Len += lineLen;
        }
    }
    b64Data[b64Len] = '\0';

    if (b64Len == 0) {
        free(b64Data);
        return nullptr;
    }

    // Decode base64 -> packed chunk data
    int maxPackedSize = (b64Len * 3) / 4 + 16;
    uint8_t* packed = (uint8_t*)malloc(maxPackedSize);
    if (!packed) {
        free(b64Data);
        return nullptr;
    }

    int packedSize = Base64_Decode(b64Data, b64Len, packed, maxPackedSize);
    free(b64Data);

    if (packedSize <= 0) {
        free(packed);
        return nullptr;
    }

    // Allocate output buffer (MapPack = 128*128*3 bytes for RA format)
    // Each chunk decompresses to 8192 bytes
    int maxDecompSize = MAP_CELL_TOTAL * 3;
    uint8_t* decompressed = (uint8_t*)malloc(maxDecompSize);
    if (!decompressed) {
        free(packed);
        return nullptr;
    }

    // Process chunks: [4-byte length][LCW data] -> 8192 bytes each
    int srcIdx = 0;
    int dstIdx = 0;
    const int CHUNK_SIZE = 8192;

    while (srcIdx + 4 <= packedSize) {
        // Read chunk length (little-endian, mask off high bits)
        uint32_t chunkLen = packed[srcIdx] |
                           (packed[srcIdx + 1] << 8) |
                           (packed[srcIdx + 2] << 16) |
                           (packed[srcIdx + 3] << 24);
        chunkLen &= 0x0000FFFF;  // Only low 16 bits are length
        srcIdx += 4;

        if (chunkLen == 0 || srcIdx + (int)chunkLen > packedSize) {
            break;
        }

        // Decompress this chunk to 8192 bytes
        if (dstIdx + CHUNK_SIZE > maxDecompSize) {
            break;
        }

        int decompLen = LCW_Decompress(&packed[srcIdx],
                                        &decompressed[dstIdx],
                                        chunkLen, CHUNK_SIZE);
        if (decompLen <= 0) {
            // Try next chunk anyway
            srcIdx += chunkLen;
            dstIdx += CHUNK_SIZE;
            continue;
        }

        srcIdx += chunkLen;
        dstIdx += CHUNK_SIZE;
    }

    free(packed);

    if (dstIdx <= 0) {
        free(decompressed);
        return nullptr;
    }

    *outSize = dstIdx;
    return decompressed;
}

int Mission_LoadFromINI(MissionData* mission, const char* filename) {
    if (!mission || !filename) return 0;

    INIClass ini;
    if (!ini.Load(filename)) {
        return 0;
    }

    return Mission_LoadFromINIClass(mission, &ini);
}

int Mission_LoadFromINIClass(MissionData* mission, INIClass* ini) {
    if (!mission || !ini) return 0;

    // Initialize defaults
    Mission_Init(mission);

    // [Basic] section
    ini->GetString("Basic", "Name", "Mission", mission->name, sizeof(mission->name));

    // Theater from [Map] section (not [Basic])
    char theaterStr[32];
    ini->GetString("Map", "Theater", "TEMPERATE", theaterStr, sizeof(theaterStr));
    if (strcasecmp(theaterStr, "SNOW") == 0) mission->theater = 1;
    else if (strcasecmp(theaterStr, "INTERIOR") == 0) mission->theater = 2;
    else if (strcasecmp(theaterStr, "DESERT") == 0) mission->theater = 3;
    else mission->theater = 0;

    // Player
    char playerStr[32];
    ini->GetString("Basic", "Player", "Greece", playerStr, sizeof(playerStr));
    mission->playerTeam = ParseTeam(playerStr);

    // Briefing video
    char briefStr[64];
    ini->GetString("Basic", "Brief", "", briefStr, sizeof(briefStr));
    strncpy(mission->briefVideo, briefStr, sizeof(mission->briefVideo) - 1);

    // Win/Lose videos
    char winStr[64], loseStr[64];
    ini->GetString("Basic", "Win", "", winStr, sizeof(winStr));
    ini->GetString("Basic", "Lose", "", loseStr, sizeof(loseStr));
    strncpy(mission->winVideo, winStr, sizeof(mission->winVideo) - 1);
    strncpy(mission->loseVideo, loseStr, sizeof(mission->loseVideo) - 1);

    // [Briefing] section for mission description
    int briefCount = ini->EntryCount("Briefing");
    mission->description[0] = '\0';
    size_t descLen = 0;
    for (int i = 0; i < briefCount && i < 10; i++) {
        char lineKey[8];
        snprintf(lineKey, sizeof(lineKey), "%d", i + 1);
        char line[256];
        ini->GetString("Briefing", lineKey, "", line, sizeof(line));
        if (strlen(line) > 0 && descLen < sizeof(mission->description) - 2) {
            size_t lineLen = strlen(line);
            if (descLen + lineLen + 1 < sizeof(mission->description)) {
                if (descLen > 0) {
                    mission->description[descLen++] = ' ';
                }
                strcpy(mission->description + descLen, line);
                descLen += lineLen;
            }
        }
    }

    // Map dimensions from [Map] section
    mission->mapX = ini->GetInt("Map", "X", 0);
    mission->mapY = ini->GetInt("Map", "Y", 0);
    mission->mapWidth = ini->GetInt("Map", "Width", 64);
    mission->mapHeight = ini->GetInt("Map", "Height", 64);

    // Credits from player's house section
    mission->startCredits = ini->GetInt(playerStr, "Credits", 5000);
    if (mission->startCredits == 0) {
        mission->startCredits = ini->GetInt("Basic", "Credits", 5000);
    }

    // [UNITS] section - Format: ID=House,Type,Health,Cell,Facing,Mission,Trigger
    // Example: 0=Greece,JEEP,256,6463,128,Guard,None
    int count = ini->EntryCount("UNITS");
    for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
        const char* entry = ini->GetEntry("UNITS", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("UNITS", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing, missionStr, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);

            if (unit->type != UNIT_NONE) {
                mission->unitCount++;
            }
        }
    }

    // [STRUCTURES] section - Format: ID=House,Type,Health,Cell,Facing,Trigger,Sellable,Rebuilt
    // Example: 0=USSR,TSLA,256,7623,0,None,1,0
    count = ini->EntryCount("STRUCTURES");
    for (int i = 0; i < count && mission->buildingCount < MAX_MISSION_BUILDINGS; i++) {
        const char* entry = ini->GetEntry("STRUCTURES", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("STRUCTURES", entry, "", value, sizeof(value));

        char house[32], type[32], trigger[32];
        int health, cell, facing, sellable, rebuild;

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%d",
                   house, type, &health, &cell, &facing, trigger, &sellable, &rebuild) >= 4) {
            MissionBuilding* bld = &mission->buildings[mission->buildingCount];
            bld->type = ParseBuildingType(type);
            bld->team = ParseTeam(house);
            bld->cellX = CELL_TO_X(cell);
            bld->cellY = CELL_TO_Y(cell);

            if (bld->type != BUILDING_NONE) {
                mission->buildingCount++;
            }
        }
    }

    // [INFANTRY] section - Format: ID=House,Type,Health,Cell,SubCell,Mission,Facing,Trigger
    // Example: 0=USSR,DOG,256,7615,2,Hunt,0,dwig
    count = ini->EntryCount("INFANTRY");
    for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
        const char* entry = ini->GetEntry("INFANTRY", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("INFANTRY", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, subCell, facing;

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                   house, type, &health, &cell, &subCell, missionStr, &facing, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);

            if (unit->type != UNIT_NONE) {
                mission->unitCount++;
            }
        }
    }

    // [MapPack] section - base64 LCW-compressed terrain data
    // RA format: 16-bit tile IDs (128*128*2=32KB) + 8-bit indices (128*128=16KB)
    // Total: 49152 bytes = MAP_CELL_TOTAL * 3
    int mapPackSize = 0;
    uint8_t* mapPackData = ParsePackSection(ini, "MapPack", &mapPackSize);
    if (mapPackData && mapPackSize >= MAP_CELL_TOTAL * 3) {
        // Allocate arrays - store low byte of 16-bit tile ID as type
        mission->terrainType = (uint8_t*)malloc(MAP_CELL_TOTAL);
        mission->terrainIcon = (uint8_t*)malloc(MAP_CELL_TOTAL);

        if (mission->terrainType && mission->terrainIcon) {
            // Extract low byte of each 16-bit tile ID as terrain type
            // (High byte is rarely used, usually 0 or 0xFF for special)
            for (int i = 0; i < MAP_CELL_TOTAL; i++) {
                uint16_t tileID = mapPackData[i * 2] |
                                  (mapPackData[i * 2 + 1] << 8);
                // 0 and 0xFFFF are "clear" terrain
                mission->terrainType[i] = (tileID == 0 || tileID == 0xFFFF)
                                          ? 0xFF : (uint8_t)(tileID & 0xFF);
            }
            // Copy tile indices from second half
            memcpy(mission->terrainIcon,
                   mapPackData + MAP_CELL_TOTAL * 2,
                   MAP_CELL_TOTAL);
        }
        free(mapPackData);
    }

    // [OverlayPack] section - base64 LCW-compressed overlay data
    int overlayPackSize = 0;
    uint8_t* overlayPackData = ParsePackSection(ini, "OverlayPack",
                                                 &overlayPackSize);
    if (overlayPackData && overlayPackSize >= MAP_CELL_TOTAL) {
        mission->overlayType = (uint8_t*)malloc(MAP_CELL_TOTAL);
        if (mission->overlayType) {
            memcpy(mission->overlayType, overlayPackData, MAP_CELL_TOTAL);
        }
        // Overlay data (variant/frame) if present
        if (overlayPackSize >= MAP_CELL_TOTAL * 2) {
            mission->overlayData = (uint8_t*)malloc(MAP_CELL_TOTAL);
            if (mission->overlayData) {
                memcpy(mission->overlayData, overlayPackData + MAP_CELL_TOTAL,
                       MAP_CELL_TOTAL);
            }
        }
        free(overlayPackData);
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

    // Load map from mission data if available, otherwise fall back to demo
    if (mission->terrainType && mission->terrainIcon) {
        // Use real mission terrain data
        Map_LoadFromMission(mission->terrainType, mission->terrainIcon,
                           mission->mapX, mission->mapY,
                           mission->mapWidth, mission->mapHeight);
    } else {
        // Fall back to procedural demo map
        Map_GenerateDemo();
    }

    // Set player credits
    // (Credits are managed by GameUI, this is just for reference)

    // Spawn buildings first (they affect map passability)
    // Convert from 128x128 map coords to local map coords
    for (int i = 0; i < mission->buildingCount; i++) {
        const MissionBuilding* bld = &mission->buildings[i];
        int localX = bld->cellX - mission->mapX;
        int localY = bld->cellY - mission->mapY;
        // Only spawn if within visible map bounds
        if (localX >= 0 && localX < mission->mapWidth &&
            localY >= 0 && localY < mission->mapHeight) {
            Buildings_Spawn(bld->type, bld->team, localX, localY);
        }
    }

    // Spawn units
    // Convert from 128x128 map coords to local world coords
    for (int i = 0; i < mission->unitCount; i++) {
        const MissionUnit* unit = &mission->units[i];
        int localCellX = unit->cellX - mission->mapX;
        int localCellY = unit->cellY - mission->mapY;
        // Only spawn if within visible map bounds
        if (localCellX >= 0 && localCellX < mission->mapWidth &&
            localCellY >= 0 && localCellY < mission->mapHeight) {
            int worldX = localCellX * CELL_SIZE + CELL_SIZE / 2;
            int worldY = localCellY * CELL_SIZE + CELL_SIZE / 2;
            Units_Spawn(unit->type, unit->team, worldX, worldY);
        }
    }

    // Center viewport on first player unit
    for (int i = 0; i < mission->unitCount; i++) {
        const MissionUnit* unit = &mission->units[i];
        if (unit->team == TEAM_PLAYER) {
            int localCellX = unit->cellX - mission->mapX;
            int localCellY = unit->cellY - mission->mapY;
            if (localCellX >= 0 && localCellX < mission->mapWidth &&
                localCellY >= 0 && localCellY < mission->mapHeight) {
                int worldX = localCellX * CELL_SIZE + CELL_SIZE / 2;
                int worldY = localCellY * CELL_SIZE + CELL_SIZE / 2;
                Map_CenterViewport(worldX, worldY);
                break;
            }
        }
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
