/**
 * Red Alert macOS Port - Mission Loader Implementation
 */

#include "mission.h"
#include "ini.h"
#include "map.h"
#include "units.h"
#include "ai.h"
#include "terrain.h"
#include "../assets/lcw.h"
#include "../assets/assetloader.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Parsed trigger storage (simplified - stores raw INI data)
// These will be used to create proper trigger instances when type systems merge
struct ParsedTrigger {
    char name[24];
    int persist;        // 0=volatile, 1=semi, 2=persistent
    int house;          // Country number
    int eventControl;   // 0=only, 1=and, 2=or, 3=linked
    int actionControl;  // 0=only, 1=and
    int event1, e1p1, e1p2;
    int event2, e2p1, e2p2;
    int action1, a1p1, a1p2, a1p3;
    int action2, a2p1, a2p2, a2p3;
    bool active;
};

#define MAX_PARSED_TRIGGERS 80
static ParsedTrigger g_parsedTriggers[MAX_PARSED_TRIGGERS];
static int g_parsedTriggerCount = 0;

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
    mission->baseHouse = -1;    // No base
    mission->baseCount = 0;
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
    if (strcasecmp(str, "E1") == 0) return UNIT_RIFLE;
    if (strcasecmp(str, "E2") == 0) return UNIT_GRENADIER;
    if (strcasecmp(str, "E3") == 0) return UNIT_ROCKET;
    if (strcasecmp(str, "E6") == 0) return UNIT_ENGINEER;
    if (strcasecmp(str, "E5") == 0 || strcasecmp(str, "SPY") == 0) return UNIT_SPY;
    if (strcasecmp(str, "DOG") == 0) return UNIT_DOG;
    if (strcasecmp(str, "MEDI") == 0) return UNIT_MEDIC;
    if (strcasecmp(str, "THF") == 0) return UNIT_THIEF;
    if (strcasecmp(str, "SHOK") == 0) return UNIT_SHOCK;

    // Vehicles
    if (strcasecmp(str, "HARV") == 0) return UNIT_HARVESTER;
    if (strcasecmp(str, "1TNK") == 0) return UNIT_TANK_LIGHT;
    if (strcasecmp(str, "2TNK") == 0) return UNIT_TANK_MEDIUM;
    if (strcasecmp(str, "3TNK") == 0) return UNIT_TANK_HEAVY;
    if (strcasecmp(str, "4TNK") == 0) return UNIT_TANK_MAMMOTH;
    if (strcasecmp(str, "APC") == 0) return UNIT_APC;
    if (strcasecmp(str, "ARTY") == 0) return UNIT_ARTILLERY;
    if (strcasecmp(str, "JEEP") == 0) return UNIT_JEEP;
    if (strcasecmp(str, "MCV") == 0) return UNIT_MCV;
    if (strcasecmp(str, "V2RL") == 0) return UNIT_V2RL;
    if (strcasecmp(str, "MNLY") == 0) return UNIT_MINELAYER;
    if (strcasecmp(str, "TRUK") == 0) return UNIT_TRUCK;
    if (strcasecmp(str, "CTNK") == 0) return UNIT_CHRONO;

    // Naval
    if (strcasecmp(str, "GNBT") == 0) return UNIT_GUNBOAT;
    if (strcasecmp(str, "DD") == 0) return UNIT_DESTROYER;
    if (strcasecmp(str, "SS") == 0) return UNIT_SUBMARINE;
    if (strcasecmp(str, "CA") == 0) return UNIT_CRUISER;
    if (strcasecmp(str, "LST") == 0) return UNIT_TRANSPORT;
    if (strcasecmp(str, "PT") == 0) return UNIT_PT_BOAT;

    // Aircraft
    if (strcasecmp(str, "HIND") == 0) return UNIT_HIND;
    if (strcasecmp(str, "HELI") == 0) return UNIT_LONGBOW;
    if (strcasecmp(str, "TRAN") == 0) return UNIT_CHINOOK;
    if (strcasecmp(str, "YAK") == 0) return UNIT_YAK;
    if (strcasecmp(str, "MIG") == 0) return UNIT_MIG;

    return UNIT_NONE;
}

// Parse building type from string
static BuildingType ParseBuildingType(const char* str) {
    if (!str) return BUILDING_NONE;

    // Core structures
    if (strcasecmp(str, "FACT") == 0) return BUILDING_CONSTRUCTION;
    if (strcasecmp(str, "POWR") == 0) return BUILDING_POWER;
    if (strcasecmp(str, "APWR") == 0) return BUILDING_ADV_POWER;
    if (strcasecmp(str, "PROC") == 0) return BUILDING_REFINERY;
    if (strcasecmp(str, "SILO") == 0) return BUILDING_SILO;

    // Production
    if (strcasecmp(str, "TENT") == 0 || strcasecmp(str, "BARR") == 0) return BUILDING_BARRACKS;
    if (strcasecmp(str, "WEAP") == 0) return BUILDING_FACTORY;
    if (strcasecmp(str, "AFLD") == 0) return BUILDING_AIRFIELD;
    if (strcasecmp(str, "HPAD") == 0) return BUILDING_HELIPAD;
    if (strcasecmp(str, "SYRD") == 0) return BUILDING_SHIPYARD;
    if (strcasecmp(str, "SPEN") == 0) return BUILDING_SUB_PEN;

    // Tech
    if (strcasecmp(str, "DOME") == 0) return BUILDING_RADAR;
    if (strcasecmp(str, "ATEK") == 0 || strcasecmp(str, "STEK") == 0) return BUILDING_TECH_CENTER;
    if (strcasecmp(str, "KENN") == 0) return BUILDING_KENNEL;

    // Defense
    if (strcasecmp(str, "GUN") == 0) return BUILDING_TURRET;
    if (strcasecmp(str, "SAM") == 0) return BUILDING_SAM;
    if (strcasecmp(str, "TSLA") == 0) return BUILDING_TESLA;
    if (strcasecmp(str, "AGUN") == 0) return BUILDING_AA_GUN;
    if (strcasecmp(str, "PBOX") == 0) return BUILDING_PILLBOX;
    if (strcasecmp(str, "HBOX") == 0) return BUILDING_CAMO_PILLBOX;
    if (strcasecmp(str, "FTUR") == 0) return BUILDING_FLAME_TOWER;
    if (strcasecmp(str, "GAP") == 0) return BUILDING_GAP;

    // Special
    if (strcasecmp(str, "FIX") == 0) return BUILDING_FIX;
    if (strcasecmp(str, "IRON") == 0) return BUILDING_IRON_CURTAIN;
    if (strcasecmp(str, "PDOX") == 0) return BUILDING_CHRONOSPHERE;
    if (strcasecmp(str, "MSLO") == 0) return BUILDING_MISSILE_SILO;

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

// Parse mission type from string
static MissionType ParseMissionType(const char* str) {
    if (!str) return MISSION_GUARD;

    if (strcasecmp(str, "Guard") == 0) return MISSION_GUARD;
    if (strcasecmp(str, "Hunt") == 0) return MISSION_HUNT;
    if (strcasecmp(str, "Sleep") == 0) return MISSION_SLEEP;
    if (strcasecmp(str, "Harvest") == 0) return MISSION_HARVEST;
    if (strcasecmp(str, "Attack") == 0) return MISSION_ATTACK;
    if (strcasecmp(str, "Area Guard") == 0) return MISSION_GUARD_AREA;
    if (strcasecmp(str, "Retreat") == 0) return MISSION_RETREAT;
    if (strcasecmp(str, "None") == 0) return MISSION_NONE;
    if (strcasecmp(str, "Stop") == 0) return MISSION_NONE;

    return MISSION_GUARD;  // Default
}

// Parse house name to house number (for TeamTypes, Base sections)
// House numbers: 0=Spain, 1=Greece, 2=USSR, 3=England, 4=Ukraine, 5=Germany, 6=France, 7=Turkey
static int ParseHouseName(const char* str) {
    if (!str) return -1;

    if (strcasecmp(str, "Spain") == 0) return 0;
    if (strcasecmp(str, "Greece") == 0) return 1;
    if (strcasecmp(str, "USSR") == 0) return 2;
    if (strcasecmp(str, "England") == 0) return 3;
    if (strcasecmp(str, "Ukraine") == 0) return 4;
    if (strcasecmp(str, "Germany") == 0) return 5;
    if (strcasecmp(str, "France") == 0) return 6;
    if (strcasecmp(str, "Turkey") == 0) return 7;

    // Aliases
    if (strcasecmp(str, "GoodGuy") == 0) return 1;  // Greece
    if (strcasecmp(str, "BadGuy") == 0) return 2;   // USSR
    if (strcasecmp(str, "Neutral") == 0) return 0;  // Spain (often neutral)
    if (strcasecmp(str, "Special") == 0) return 0;

    return -1;
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
        missionStr[0] = '\0';
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing, missionStr, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);
            unit->health = (int16_t)health;
            unit->facing = (int16_t)facing;
            unit->mission = ParseMissionType(missionStr);
            unit->subCell = 0;  // Vehicles don't have sub-cells

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
        int health, cell, facing, sellable = 1, rebuild = 0;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%d",
                   house, type, &health, &cell, &facing, trigger, &sellable, &rebuild) >= 4) {
            MissionBuilding* bld = &mission->buildings[mission->buildingCount];
            bld->type = ParseBuildingType(type);
            bld->team = ParseTeam(house);
            bld->cellX = CELL_TO_X(cell);
            bld->cellY = CELL_TO_Y(cell);
            bld->health = (int16_t)health;
            bld->facing = (int16_t)facing;
            bld->sellable = (int8_t)sellable;
            bld->rebuild = (int8_t)rebuild;

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
        int health, cell, subCell, facing = 0;
        missionStr[0] = '\0';
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                   house, type, &health, &cell, &subCell, missionStr, &facing, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);
            unit->health = (int16_t)health;
            unit->facing = (int16_t)facing;
            unit->mission = ParseMissionType(missionStr);
            unit->subCell = (int16_t)subCell;

            if (unit->type != UNIT_NONE) {
                mission->unitCount++;
            }
        }
    }

    // [Trigs] section - trigger definitions
    // Format: name=persist,house,event_control,action_control,
    //         event1,e1p1,e1p2,event2,e2p1,e2p2,
    //         action1,a1p1,a1p2,a1p3,action2,a2p1,a2p2,a2p3
    count = ini->EntryCount("Trigs");
    g_parsedTriggerCount = 0;  // Clear existing triggers
    for (int i = 0; i < count && g_parsedTriggerCount < MAX_PARSED_TRIGGERS; i++) {
        const char* trigName = ini->GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini->GetString("Trigs", trigName, "", value, sizeof(value));

        // Parse trigger definition
        int persist, house, eventCtrl, actionCtrl;
        int event1, e1p1, e1p2, event2, e2p1, e2p2;
        int action1, a1p1, a1p2, a1p3, action2, a2p1, a2p2, a2p3;

        int parsed = sscanf(value,
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            &persist, &house, &eventCtrl, &actionCtrl,
            &event1, &e1p1, &e1p2, &event2, &e2p1, &e2p2,
            &action1, &a1p1, &a1p2, &a1p3, &action2, &a2p1, &a2p2, &a2p3);

        if (parsed >= 11) {
            ParsedTrigger* trig = &g_parsedTriggers[g_parsedTriggerCount];
            memset(trig, 0, sizeof(ParsedTrigger));

            // Name (max 23 chars)
            strncpy(trig->name, trigName, 23);
            trig->name[23] = '\0';
            trig->active = true;

            // Store all parsed values
            trig->persist = persist;
            trig->house = house;
            trig->eventControl = eventCtrl;
            trig->actionControl = actionCtrl;
            trig->event1 = event1;
            trig->e1p1 = e1p1;
            trig->e1p2 = e1p2;
            trig->event2 = event2;
            trig->e2p1 = e2p1;
            trig->e2p2 = e2p2;
            trig->action1 = action1;
            trig->a1p1 = a1p1;
            trig->a1p2 = a1p2;
            trig->a1p3 = a1p3;

            if (parsed >= 18) {
                trig->action2 = action2;
                trig->a2p1 = a2p1;
                trig->a2p2 = a2p2;
                trig->a2p3 = a2p3;
            }

            g_parsedTriggerCount++;
        }
    }

    // [Waypoints] section - spawn points, movement targets
    // Format: waypoint_number=cell_number
    count = ini->EntryCount("Waypoints");
    mission->waypointCount = 0;
    // Initialize all waypoints to invalid (-1)
    for (int i = 0; i < MAX_MISSION_WAYPOINTS; i++) {
        mission->waypoints[i].cell = -1;
        mission->waypoints[i].cellX = -1;
        mission->waypoints[i].cellY = -1;
    }
    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("Waypoints", i);
        if (!entry) continue;

        int wpNum = atoi(entry);
        if (wpNum < 0 || wpNum >= MAX_MISSION_WAYPOINTS) continue;

        int cell = ini->GetInt("Waypoints", entry, -1);
        if (cell < 0) continue;

        mission->waypoints[wpNum].cell = cell;
        mission->waypoints[wpNum].cellX = CELL_TO_X(cell);
        mission->waypoints[wpNum].cellY = CELL_TO_Y(cell);

        if (wpNum >= mission->waypointCount) {
            mission->waypointCount = wpNum + 1;
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

    // [TeamTypes] section - AI team definitions
    // Format: name=house,flags,recruitPriority,initNum,maxAllowed,origin,trigger,
    //         numMembers,memberType1:qty1,memberType2:qty2,...,
    //         numMissions,mission1:data1,mission2:data2,...
    count = ini->EntryCount("TeamTypes");
    mission->teamTypeCount = 0;
    for (int i = 0; i < count && mission->teamTypeCount < MAX_TEAM_TYPES; i++) {
        const char* teamName = ini->GetEntry("TeamTypes", i);
        if (!teamName) continue;

        char value[512];
        ini->GetString("TeamTypes", teamName, "", value, sizeof(value));
        if (value[0] == '\0') continue;

        MissionTeamType* team = &mission->teamTypes[mission->teamTypeCount];
        memset(team, 0, sizeof(MissionTeamType));

        // Store team name
        strncpy(team->name, teamName, sizeof(team->name) - 1);

        // Parse comma-separated values
        char* ptr = value;
        char* next;

        // house (0-7)
        team->house = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // flags (packed: roundabout, suicide, autocreate, etc.)
        team->flags = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // recruitPriority
        team->recruitPriority = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // initNum
        team->initNum = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // maxAllowed
        team->maxAllowed = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // origin waypoint
        team->origin = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // trigger ID (or -1 for none)
        team->trigger = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // numMembers
        int numMembers = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1; else continue;

        // Parse member entries (type:qty)
        team->memberCount = 0;
        for (int m = 0; m < numMembers && m < MAX_TEAM_MEMBERS; m++) {
            // Find colon separating type:qty
            char* colon = strchr(ptr, ':');
            if (!colon) break;

            // Extract type name
            int typeLen = (int)(colon - ptr);
            if (typeLen > 7) typeLen = 7;
            strncpy(team->members[m].unitType, ptr, typeLen);
            team->members[m].unitType[typeLen] = '\0';

            // Extract quantity
            ptr = colon + 1;
            team->members[m].quantity = strtol(ptr, &next, 10);
            team->memberCount++;

            if (*next == ',') ptr = next + 1;
            else break;
        }

        // numMissions
        int numMissions = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1;

        // Parse mission entries (mission:data)
        team->missionCount = 0;
        for (int m = 0; m < numMissions && m < MAX_TEAM_MISSIONS; m++) {
            // Find colon separating mission:data
            char* colon = strchr(ptr, ':');
            if (!colon) break;

            // Extract mission type
            team->missions[m].mission = strtol(ptr, &colon, 10);

            // Extract data
            ptr = colon + 1;
            team->missions[m].data = strtol(ptr, &next, 10);
            team->missionCount++;

            if (*next == ',') ptr = next + 1;
            else break;
        }

        mission->teamTypeCount++;
    }

    // [Base] section - AI base rebuild info
    // Format: Player=<house>, Count=<number>
    char basePlayer[32];
    ini->GetString("Base", "Player", "", basePlayer, sizeof(basePlayer));
    if (basePlayer[0] != '\0') {
        // Map house name to number
        mission->baseHouse = ParseHouseName(basePlayer);
    }
    mission->baseCount = ini->GetInt("Base", "Count", 0);

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

    // Set theater for correct palette and terrain tiles
    // mission->theater: 0=temperate, 1=snow, 2=interior, 3=desert
    TheaterType theater = THEATER_SNOW;  // Default
    switch (mission->theater) {
        case 0: theater = THEATER_TEMPERATE; break;
        case 1: theater = THEATER_SNOW; break;
        case 2: theater = THEATER_INTERIOR; break;
        case 3: theater = THEATER_DESERT; break;
    }
    Assets_SetTheater(theater);
    Terrain_SetTheater(mission->theater);
    fprintf(stderr, "Mission: Set theater to %d (%s)\n", mission->theater,
            theater == THEATER_TEMPERATE ? "TEMPERATE" :
            theater == THEATER_SNOW ? "SNOW" :
            theater == THEATER_INTERIOR ? "INTERIOR" : "DESERT");

    // Initialize systems
    Map_Init();
    Units_Init();
    AI_Init();

    // Load map from mission data if available, otherwise fall back to demo
    if (mission->terrainType && mission->terrainIcon) {
        // Use real mission terrain data (with overlay for ore/gems)
        Map_LoadFromMission(mission->terrainType, mission->terrainIcon,
                           mission->overlayType, mission->overlayData,
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

    // Log parsed triggers (from [Trigs] section)
    if (g_parsedTriggerCount > 0) {
        fprintf(stderr, "  Loaded %d triggers from mission INI\n",
                g_parsedTriggerCount);
        for (int i = 0; i < g_parsedTriggerCount && i < 5; i++) {
            ParsedTrigger* trig = &g_parsedTriggers[i];
            fprintf(stderr, "    Trigger '%s': event1=%d action1=%d\n",
                    trig->name, trig->event1, trig->action1);
        }
        if (g_parsedTriggerCount > 5) {
            fprintf(stderr, "    ... and %d more\n",
                    g_parsedTriggerCount - 5);
        }
    }

    // Log waypoints
    if (mission->waypointCount > 0) {
        fprintf(stderr, "  Loaded %d waypoints\n", mission->waypointCount);
        int shown = 0;
        for (int i = 0; i < mission->waypointCount && shown < 5; i++) {
            if (mission->waypoints[i].cell >= 0) {
                fprintf(stderr, "    Waypoint %d: cell=%d (%d,%d)\n",
                        i, mission->waypoints[i].cell,
                        mission->waypoints[i].cellX,
                        mission->waypoints[i].cellY);
                shown++;
            }
        }
    }

    // Log team types
    if (mission->teamTypeCount > 0) {
        fprintf(stderr, "  Loaded %d team types\n", mission->teamTypeCount);
        for (int i = 0; i < mission->teamTypeCount && i < 5; i++) {
            const MissionTeamType* team = &mission->teamTypes[i];
            fprintf(stderr, "    Team '%s': house=%d members=%d missions=%d\n",
                    team->name, team->house, team->memberCount, team->missionCount);
        }
        if (mission->teamTypeCount > 5) {
            fprintf(stderr, "    ... and %d more\n", mission->teamTypeCount - 5);
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

    // Player buildings (type, team, cellX, cellY, health, facing, sellable, rebuild)
    mission->buildings[mission->buildingCount++] = {BUILDING_CONSTRUCTION, TEAM_PLAYER, 2, 15, 256, 0, 1, 0};
    mission->buildings[mission->buildingCount++] = {BUILDING_POWER, TEAM_PLAYER, 6, 16, 256, 0, 1, 0};
    mission->buildings[mission->buildingCount++] = {BUILDING_BARRACKS, TEAM_PLAYER, 2, 19, 256, 0, 1, 0};
    mission->buildings[mission->buildingCount++] = {BUILDING_REFINERY, TEAM_PLAYER, 6, 19, 256, 0, 1, 0};

    // Enemy buildings
    mission->buildings[mission->buildingCount++] = {BUILDING_CONSTRUCTION, TEAM_ENEMY, 55, 10, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_POWER, TEAM_ENEMY, 52, 10, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_BARRACKS, TEAM_ENEMY, 55, 6, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_FACTORY, TEAM_ENEMY, 52, 6, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_TURRET, TEAM_ENEMY, 50, 12, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_TURRET, TEAM_ENEMY, 58, 12, 256, 0, 0, 1};
    mission->buildings[mission->buildingCount++] = {BUILDING_REFINERY, TEAM_ENEMY, 58, 8, 256, 0, 0, 1};

    // Player units (type, team, cellX, cellY, health, facing, mission, subCell)
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_PLAYER, 4, 16, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_PLAYER, 5, 17, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_TANK_LIGHT, TEAM_PLAYER, 7, 16, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_TANK_LIGHT, TEAM_PLAYER, 7, 18, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 3, 18, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 4, 18, 256, 64, MISSION_GUARD, 1};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_PLAYER, 5, 18, 256, 64, MISSION_GUARD, 2};
    mission->units[mission->unitCount++] = {UNIT_ROCKET, TEAM_PLAYER, 2, 17, 256, 64, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_HARVESTER, TEAM_PLAYER, 8, 20, 256, 64, MISSION_HARVEST, 0};

    // Enemy units
    mission->units[mission->unitCount++] = {UNIT_TANK_HEAVY, TEAM_ENEMY, 54, 12, 256, 192, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_ENEMY, 52, 13, 256, 192, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_TANK_MEDIUM, TEAM_ENEMY, 56, 13, 256, 192, MISSION_GUARD, 0};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 50, 14, 256, 192, MISSION_HUNT, 0};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 51, 14, 256, 192, MISSION_HUNT, 1};
    mission->units[mission->unitCount++] = {UNIT_RIFLE, TEAM_ENEMY, 52, 14, 256, 192, MISSION_HUNT, 2};
    mission->units[mission->unitCount++] = {UNIT_ROCKET, TEAM_ENEMY, 54, 10, 256, 192, MISSION_GUARD, 0};
}

// ============================================================================
// Trigger Processing
// ============================================================================

// Red Alert INI event types (from original game)
// Note: These match the INI file format, not our internal enum
enum {
    RA_EVENT_NONE = 0,
    RA_EVENT_ENTERED = 1,        // Player entered cell/zone
    RA_EVENT_SPIED = 2,          // Building spied upon
    RA_EVENT_THIEVED = 3,        // Thief stole vehicle
    RA_EVENT_DISCOVERED = 4,     // Object discovered
    RA_EVENT_HOUSE_DISC = 5,     // House discovered
    RA_EVENT_ATTACKED = 6,       // Object attacked
    RA_EVENT_DESTROYED = 7,      // Object destroyed
    RA_EVENT_ANY = 8,            // Any event
    RA_EVENT_UNITS_DESTR = 9,    // All units destroyed
    RA_EVENT_BLDGS_DESTR = 10,   // All buildings destroyed
    RA_EVENT_ALL_DESTR = 11,     // All units+buildings destroyed
    RA_EVENT_CREDITS = 12,       // Credits reach amount
    RA_EVENT_TIME = 13,          // Time elapsed
    RA_EVENT_TIMER_EXP = 14,     // Mission timer expired
    RA_EVENT_NOBLDGS = 15,       // No buildings left
    RA_EVENT_CIVEVAC = 16,       // Civilian evacuated
    RA_EVENT_OBJBUILT = 17,      // Object built
    RA_EVENT_LEAVES = 18,        // Team leaves map
    RA_EVENT_ZONE_ENT = 19,      // Zone entered
    RA_EVENT_HORZ_CROSS = 20,    // Crosses horizontal line
    RA_EVENT_VERT_CROSS = 21,    // Crosses vertical line
    RA_EVENT_GLOBAL_SET = 22,    // Global variable set
    RA_EVENT_GLOBAL_CLR = 23,    // Global variable cleared
    RA_EVENT_FAKES_DESTR = 24,   // Fake buildings destroyed
    RA_EVENT_LOW_POWER = 25,     // Low power
    RA_EVENT_BRIDGE_DESTR = 26,  // All bridges destroyed
    RA_EVENT_BUILDING_EXISTS = 27, // Specific building exists
};

// Red Alert INI action types (from original game)
enum {
    RA_ACTION_NONE = 0,
    RA_ACTION_WIN = 1,           // Player wins
    RA_ACTION_LOSE = 2,          // Player loses
    RA_ACTION_BEGIN_PROD = 3,    // Begin production
    RA_ACTION_CREATE_TEAM = 4,   // Create team
    RA_ACTION_DESTROY_TEAM = 5,  // Destroy team
    RA_ACTION_ALL_HUNT = 6,      // All hunt
    RA_ACTION_REINFORCE = 7,     // Reinforcements arrive
    RA_ACTION_DZ = 8,            // Drop zone marker
    RA_ACTION_FIRE_SALE = 9,     // Fire sale
    RA_ACTION_PLAY_MOVIE = 10,   // Play movie
    RA_ACTION_TEXT = 11,         // Display text
    RA_ACTION_DESTR_TRIG = 12,   // Destroy trigger
    RA_ACTION_AUTOCREATE = 13,   // Auto create teams
    RA_ACTION_WINLOSE = 14,      // Win/lose based on object
    RA_ACTION_ALLOWWIN = 15,     // Allow win (same as win?)
    RA_ACTION_REVEAL_ALL = 16,   // Reveal entire map
    RA_ACTION_REVEAL_SOME = 17,  // Reveal around waypoint
    RA_ACTION_REVEAL_ZONE = 18,  // Reveal zone
    RA_ACTION_PLAY_SOUND = 19,   // Play sound
    RA_ACTION_PLAY_MUSIC = 20,   // Play music
    RA_ACTION_PLAY_SPEECH = 21,  // Play speech
    RA_ACTION_FORCE_TRIG = 22,   // Force trigger
    RA_ACTION_START_TIMER = 23,  // Start mission timer
    RA_ACTION_STOP_TIMER = 24,   // Stop mission timer
    RA_ACTION_ADD_TIMER = 25,    // Add time
    RA_ACTION_SUB_TIMER = 26,    // Subtract time
    RA_ACTION_SET_TIMER = 27,    // Set timer
    RA_ACTION_SET_GLOBAL = 28,   // Set global flag
    RA_ACTION_CLEAR_GLOBAL = 29, // Clear global flag
    RA_ACTION_BASE_BUILDING = 30, // Auto base building
    RA_ACTION_GROW_SHROUD = 31,  // Grow shroud one step
    RA_ACTION_DESTROY_OBJ = 32,  // Destroy attached object
    RA_ACTION_1_SPECIAL = 33,    // One-time special
    RA_ACTION_FULL_SPECIAL = 34, // Full special
    RA_ACTION_PREF_TARGET = 35,  // Preferred target
    RA_ACTION_LAUNCH_NUKES = 36, // Launch fake nukes
};

// Helper: Count units by team (enemy house detection)
static int CountUnitsByHouse(int houseNum) {
    // House numbers: 0=Spain, 1=Greece, 2=USSR, 3=England, 4=Ukraine,
    //                5=Germany, 6=France, 7=Turkey
    // USSR (2) and Ukraine (4) are Soviet
    // Others are Allied
    Team team;
    if (houseNum == 2 || houseNum == 4) {
        team = TEAM_ENEMY;  // Soviet houses
    } else {
        team = TEAM_PLAYER; // Allied houses
    }
    return Units_CountByTeam(team);
}

// Helper: Count buildings by team
static int CountBuildingsByHouse(int houseNum) {
    Team team;
    if (houseNum == 2 || houseNum == 4) {
        team = TEAM_ENEMY;
    } else {
        team = TEAM_PLAYER;
    }

    int count = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (bld && bld->team == team) {
            count++;
        }
    }
    return count;
}

// Check if a trigger event is satisfied
static bool CheckTriggerEvent(ParsedTrigger* trig, int eventNum, int param1,
                               int param2, int frameCount) {
    (void)trig;
    (void)param1;

    switch (eventNum) {
        case RA_EVENT_NONE:
            return false;

        case RA_EVENT_ALL_DESTR:  // All units+buildings destroyed
            // param2 is house number
            if (CountUnitsByHouse(param2) == 0 &&
                CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_UNITS_DESTR:  // All units destroyed
            if (CountUnitsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_BLDGS_DESTR:  // All buildings destroyed
            if (CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_NOBLDGS:  // No buildings (same as BLDGS_DESTR?)
            if (CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_TIME:  // Time elapsed
            // param2 is time in 1/10 seconds, frameCount is in frames at 60fps
            // Time in frames = param2 * 6 (for 60fps)
            if (frameCount >= param2 * 6) {
                return true;
            }
            break;

        default:
            // Unsupported event types - don't trigger
            break;
    }

    return false;
}

// Execute a trigger action
// Returns: 1=win, -1=lose, 0=continue
static int ExecuteTriggerAction(ParsedTrigger* trig, int actionNum,
                                 int param1, int param2, int param3,
                                 const MissionData* mission) {
    (void)trig;
    (void)param1;
    (void)param2;
    (void)param3;
    (void)mission;

    switch (actionNum) {
        case RA_ACTION_WIN:
        case RA_ACTION_ALLOWWIN:  // Treat allow-win as win
            fprintf(stderr, "  TRIGGER: Win action executed!\n");
            return 1;

        case RA_ACTION_LOSE:
            fprintf(stderr, "  TRIGGER: Lose action executed!\n");
            return -1;

        case RA_ACTION_REINFORCE:
            // param1 = team type index
            fprintf(stderr, "  TRIGGER: Reinforcement action (team %d)\n",
                    param1);
            // TODO: Spawn reinforcement team at waypoint
            break;

        case RA_ACTION_TEXT:
            // param3 = text ID
            fprintf(stderr, "  TRIGGER: Display text ID %d\n", param3);
            // TODO: Display mission text
            break;

        case RA_ACTION_REVEAL_SOME:
            // param3 = waypoint number
            fprintf(stderr, "  TRIGGER: Reveal around waypoint %d\n", param3);
            // TODO: Reveal fog around waypoint
            break;

        default:
            // Unsupported action - just log it
            if (actionNum != RA_ACTION_NONE) {
                fprintf(stderr, "  TRIGGER: Unsupported action %d\n",
                        actionNum);
            }
            break;
    }

    return 0;
}

int Mission_ProcessTriggers(const MissionData* mission, int frameCount) {
    if (!mission) return 0;

    int result = 0;

    for (int i = 0; i < g_parsedTriggerCount; i++) {
        ParsedTrigger* trig = &g_parsedTriggers[i];
        if (!trig->active) continue;

        // Check event1
        bool event1Fired = CheckTriggerEvent(trig, trig->event1,
                                              trig->e1p1, trig->e1p2,
                                              frameCount);

        // Check event2 if using AND/OR control
        bool event2Fired = false;
        if (trig->eventControl != 0) {  // Not "ONLY"
            event2Fired = CheckTriggerEvent(trig, trig->event2,
                                             trig->e2p1, trig->e2p2,
                                             frameCount);
        }

        // Determine if trigger should fire
        bool shouldFire = false;
        switch (trig->eventControl) {
            case 0:  // ONLY - just event1
                shouldFire = event1Fired;
                break;
            case 1:  // AND - both events
                shouldFire = event1Fired && event2Fired;
                break;
            case 2:  // OR - either event
                shouldFire = event1Fired || event2Fired;
                break;
            case 3:  // LINKED - events linked to actions
                shouldFire = event1Fired || event2Fired;
                break;
        }

        if (!shouldFire) continue;

        fprintf(stderr, "  TRIGGER '%s' fired!\n", trig->name);

        // Execute action1
        int actionResult = ExecuteTriggerAction(trig, trig->action1,
                                                 trig->a1p1, trig->a1p2,
                                                 trig->a1p3, mission);
        if (actionResult != 0) {
            result = actionResult;
        }

        // Execute action2 if using AND control
        if (trig->actionControl != 0) {  // Not "ONLY"
            actionResult = ExecuteTriggerAction(trig, trig->action2,
                                                 trig->a2p1, trig->a2p2,
                                                 trig->a2p3, mission);
            if (actionResult != 0) {
                result = actionResult;
            }
        }

        // Handle persistence
        if (trig->persist == 0) {  // Volatile - fire once
            trig->active = false;
        }
        // Semi-persistent (1) and persistent (2) can fire again
    }

    return result;
}

int Mission_GetWaypoint(const MissionData* mission, int waypointNum,
                         int* outX, int* outY) {
    if (!mission || waypointNum < 0 ||
        waypointNum >= MAX_MISSION_WAYPOINTS) {
        return 0;
    }

    const MissionWaypoint* wp = &mission->waypoints[waypointNum];
    if (wp->cell < 0) {
        return 0;
    }

    // Convert from map cell coords to world coords
    int localCellX = wp->cellX - mission->mapX;
    int localCellY = wp->cellY - mission->mapY;

    if (localCellX < 0 || localCellX >= mission->mapWidth ||
        localCellY < 0 || localCellY >= mission->mapHeight) {
        return 0;
    }

    *outX = localCellX * CELL_SIZE + CELL_SIZE / 2;
    *outY = localCellY * CELL_SIZE + CELL_SIZE / 2;

    return 1;
}
