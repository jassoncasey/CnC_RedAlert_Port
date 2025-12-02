/**
 * Red Alert macOS Port - Unit/Entity System Implementation
 */

#include "units.h"
#include "map.h"
#include "mission.h"
#include "sprites.h"
#include "sounds.h"
#include "graphics/metal/renderer.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>
#include <queue>
#include <vector>
#include <algorithm>

// Forward declarations
static BOOL IsCellPassable(int cellX, int cellY, BOOL isNaval);

// Unit type definitions
struct UnitTypeDef {
    int16_t maxHealth;
    int16_t speed;
    int16_t attackRange;
    int16_t attackDamage;
    int16_t attackRate;
    int16_t sightRange;     // Sight range in cells (fog of war)
    uint8_t size;           // Visual size in pixels
    uint8_t color;          // Base color
    BOOL isInfantry;
    BOOL isNaval;
};

// Unit type definitions - MUST match enum order in units.h exactly!
// Format: { maxHealth, speed, attackRange, attackDamage, attackRate,
//           sightRange, size, color, isInfantry, isNaval }
static const UnitTypeDef g_unitTypes[UNIT_TYPE_COUNT] = {
    // 0: UNIT_NONE
    { 0, 0, 0, 0, 0, 0, 0, 0, FALSE, FALSE },
    // Infantry - Military
    // 1: UNIT_RIFLE (E1)
    { 50, 2, 64, 8, 30, 5, 6, 15, TRUE, FALSE },
    // 2: UNIT_GRENADIER (E2)
    { 60, 2, 96, 20, 45, 5, 8, 15, TRUE, FALSE },
    // 3: UNIT_ROCKET (E3)
    { 45, 2, 128, 30, 60, 6, 8, 15, TRUE, FALSE },
    // 4: UNIT_FLAMETHROWER (E4)
    { 70, 2, 48, 25, 25, 4, 8, 15, TRUE, FALSE },
    // 5: UNIT_ENGINEER (E6)
    { 25, 2, 0, 0, 0, 4, 6, 15, TRUE, FALSE },
    // 6: UNIT_TANYA (E7)
    { 100, 3, 80, 40, 15, 6, 8, 15, TRUE, FALSE },
    // 7: UNIT_DOG
    { 25, 4, 16, 100, 20, 5, 6, 8, TRUE, FALSE },
    // 8: UNIT_SPY (E5)
    { 25, 2, 0, 0, 0, 5, 6, 15, TRUE, FALSE },
    // 9: UNIT_MEDIC
    { 80, 2, 64, -30, 30, 5, 6, 15, TRUE, FALSE },  // Negative = heal
    // 10: UNIT_THIEF
    { 25, 3, 0, 0, 0, 5, 6, 15, TRUE, FALSE },
    // 11: UNIT_SHOCK
    { 110, 2, 96, 50, 35, 5, 8, 15, TRUE, FALSE },
    // 12: UNIT_GENERAL
    { 100, 2, 0, 0, 0, 5, 8, 15, TRUE, FALSE },
    // Infantry - Civilians (13-23)
    // 13: UNIT_CIVILIAN_1 (C1)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 14: UNIT_CIVILIAN_2 (C2)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 15: UNIT_CIVILIAN_3 (C3)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 16: UNIT_CIVILIAN_4 (C4)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 17: UNIT_CIVILIAN_5 (C5)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 18: UNIT_CIVILIAN_6 (C6)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 19: UNIT_CIVILIAN_7 (C7)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 20: UNIT_CIVILIAN_8 (C8 - Einstein)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 21: UNIT_CIVILIAN_9 (C9)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 22: UNIT_CIVILIAN_10 (C10)
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // 23: UNIT_CHAN
    { 25, 2, 0, 0, 0, 3, 6, 6, TRUE, FALSE },
    // Vehicles (24-37)
    // 24: UNIT_HARVESTER
    { 200, 3, 0, 0, 0, 4, 18, 14, FALSE, FALSE },
    // 25: UNIT_TANK_LIGHT (1TNK)
    { 150, 5, 96, 25, 30, 6, 14, 7, FALSE, FALSE },
    // 26: UNIT_TANK_MEDIUM (2TNK)
    { 250, 4, 112, 40, 35, 6, 16, 7, FALSE, FALSE },
    // 27: UNIT_TANK_HEAVY (3TNK)
    { 500, 3, 128, 60, 40, 7, 20, 7, FALSE, FALSE },
    // 28: UNIT_TANK_MAMMOTH (4TNK)
    { 600, 2, 128, 80, 45, 8, 22, 9, FALSE, FALSE },
    // 29: UNIT_APC
    { 150, 6, 48, 10, 20, 6, 14, 7, FALSE, FALSE },
    // 30: UNIT_ARTILLERY
    { 100, 3, 192, 50, 60, 8, 16, 7, FALSE, FALSE },
    // 31: UNIT_JEEP
    { 100, 7, 80, 15, 20, 6, 12, 7, FALSE, FALSE },
    // 32: UNIT_MCV
    { 400, 2, 0, 0, 0, 5, 20, 7, FALSE, FALSE },
    // 33: UNIT_V2RL
    { 125, 3, 256, 100, 90, 6, 16, 9, FALSE, FALSE },
    // 34: UNIT_MINELAYER
    { 100, 4, 0, 0, 0, 5, 14, 7, FALSE, FALSE },
    // 35: UNIT_TRUCK
    { 100, 5, 0, 0, 0, 4, 14, 7, FALSE, FALSE },
    // 36: UNIT_CHRONO
    { 150, 4, 96, 30, 30, 5, 14, 7, FALSE, FALSE },
    // 37: UNIT_MOBILE_GAP
    { 150, 3, 0, 0, 0, 6, 16, 7, FALSE, FALSE },
    // 38: UNIT_MOBILE_RADAR
    { 100, 4, 0, 0, 0, 8, 14, 7, FALSE, FALSE },
    // Naval (39-44)
    // 39: UNIT_GUNBOAT
    { 200, 4, 96, 20, 30, 7, 16, 1, FALSE, TRUE },
    // 40: UNIT_DESTROYER
    { 350, 5, 128, 40, 35, 8, 20, 1, FALSE, TRUE },
    // 41: UNIT_SUBMARINE
    { 200, 4, 160, 50, 50, 6, 16, 9, FALSE, TRUE },
    // 42: UNIT_CRUISER
    { 500, 3, 192, 80, 45, 9, 24, 1, FALSE, TRUE },
    // 43: UNIT_TRANSPORT
    { 250, 3, 0, 0, 0, 5, 20, 1, FALSE, TRUE },
    // 44: UNIT_PT_BOAT
    { 150, 6, 64, 15, 20, 6, 14, 1, FALSE, TRUE },
    // Aircraft (45-49)
    // 45: UNIT_HIND
    { 150, 6, 96, 30, 25, 7, 16, 9, FALSE, FALSE },
    // 46: UNIT_LONGBOW
    { 120, 7, 128, 40, 30, 8, 16, 7, FALSE, FALSE },
    // 47: UNIT_CHINOOK
    { 150, 5, 0, 0, 0, 6, 18, 7, FALSE, FALSE },
    // 48: UNIT_YAK
    { 100, 8, 80, 25, 20, 7, 14, 9, FALSE, FALSE },
    // 49: UNIT_MIG
    { 100, 9, 96, 50, 25, 8, 14, 9, FALSE, FALSE },
};

// Building type definitions
struct BuildingTypeDef {
    int16_t maxHealth;
    uint8_t width;
    uint8_t height;
    uint8_t color;
    BOOL canAttack;
    int16_t attackRange;
    int16_t attackDamage;
    int16_t attackRate;
    int16_t sightRange;     // Sight range in cells (fog of war)
};

static const BuildingTypeDef g_buildingTypes[BUILDING_TYPE_COUNT] = {
    // BUILDING_NONE                                        sight
    { 0, 0, 0, 0, FALSE, 0, 0, 0, 0 },
    // BUILDING_CONSTRUCTION
    { 500, 3, 3, 7, FALSE, 0, 0, 0, 6 },
    // BUILDING_POWER
    { 300, 2, 2, 14, FALSE, 0, 0, 0, 4 },
    // BUILDING_REFINERY
    { 400, 3, 2, 14, FALSE, 0, 0, 0, 5 },
    // BUILDING_BARRACKS
    { 350, 2, 2, 7, FALSE, 0, 0, 0, 5 },
    // BUILDING_FACTORY
    { 400, 3, 3, 7, FALSE, 0, 0, 0, 5 },
    // BUILDING_RADAR
    { 300, 2, 2, 7, FALSE, 0, 0, 0, 10 },  // Radar has long sight
    // BUILDING_TURRET
    { 200, 1, 1, 8, TRUE, 128, 30, 25, 6 },
    // BUILDING_SAM
    { 250, 2, 1, 8, TRUE, 160, 40, 40, 7 },
};

// Global state
static Unit g_units[MAX_UNITS];
static Building g_buildings[MAX_BUILDINGS];

// Team colors
static const uint8_t g_teamColors[TEAM_COUNT] = {
    7,  // TEAM_NEUTRAL - gray
    9,  // TEAM_PLAYER - light blue (Allies)
    4,  // TEAM_ENEMY - red (Soviet)
};

void Units_Init(void) {
    Units_Clear();
}

void Units_Shutdown(void) {
    // Nothing to free
}

void Units_Clear(void) {
    memset(g_units, 0, sizeof(g_units));
    memset(g_buildings, 0, sizeof(g_buildings));
}

// Check if a cell is occupied by another unit
// moverTeam: team of the unit trying to move (-1 to block on all)
// canCrush: if true, don't block on enemy infantry (vehicle can crush them)
static BOOL IsCellOccupiedForTeam(int cellX, int cellY, int excludeId,
                                  int moverTeam, BOOL canCrush) {
    for (int i = 0; i < MAX_UNITS; i++) {
        if (i == excludeId) continue;
        if (!g_units[i].active) continue;
        if (g_units[i].state == STATE_DYING) continue;

        int unitCellX = g_units[i].worldX / CELL_SIZE;
        int unitCellY = g_units[i].worldY / CELL_SIZE;

        if (unitCellX == cellX && unitCellY == cellY) {
            // Check if we can ignore this unit
            if (canCrush && g_units[i].team != moverTeam) {
                // Can crush enemy infantry - check if it's infantry
                const UnitTypeDef* def = &g_unitTypes[g_units[i].type];
                if (def->isInfantry) {
                    continue;  // Don't block, we can crush them
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

// Simple version that blocks on all units
static BOOL IsCellOccupied(int cellX, int cellY, int excludeUnitId = -1) {
    return IsCellOccupiedForTeam(cellX, cellY, excludeUnitId, -1, FALSE);
}

// Update cell occupancy for a unit
static void UpdateCellOccupancy(int unitId, int oldCellX, int oldCellY,
                                int newCellX, int newCellY) {
    // Clear old cell
    if (oldCellX >= 0 && oldCellY >= 0) {
        MapCell* oldCell = Map_GetCell(oldCellX, oldCellY);
        if (oldCell && oldCell->unitId == unitId) {
            oldCell->unitId = -1;
        }
    }
    // Mark new cell
    if (newCellX >= 0 && newCellY >= 0) {
        MapCell* newCell = Map_GetCell(newCellX, newCellY);
        if (newCell) {
            newCell->unitId = unitId;
        }
    }
}

// Find a valid spawn position near the requested location
static BOOL FindValidSpawnPosition(int* worldX, int* worldY, BOOL isNaval) {
    int cellX, cellY;
    Map_WorldToCell(*worldX, *worldY, &cellX, &cellY);

    // Check if original position is valid and unoccupied
    bool passable = IsCellPassable(cellX, cellY, isNaval);
    if (passable && !IsCellOccupied(cellX, cellY)) {
        return TRUE;
    }

    // Search in expanding squares for a valid unoccupied cell
    for (int radius = 1; radius <= 10; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Only check perimeter
                if (abs(dx) != radius && abs(dy) != radius) continue;

                int testX = cellX + dx;
                int testY = cellY + dy;
                bool ok = IsCellPassable(testX, testY, isNaval);
                if (ok && !IsCellOccupied(testX, testY)) {
                    Map_CellToWorld(testX, testY, worldX, worldY);
                    return TRUE;
                }
            }
        }
    }

    return FALSE; // No valid position found
}

int Units_Spawn(UnitType type, Team team, int worldX, int worldY) {
    if (type <= UNIT_NONE || type >= UNIT_TYPE_COUNT) return -1;

    // Find free slot
    int id = -1;
    for (int i = 0; i < MAX_UNITS; i++) {
        if (!g_units[i].active) {
            id = i;
            break;
        }
    }
    if (id < 0) return -1;

    const UnitTypeDef* def = &g_unitTypes[type];

    // Validate and adjust spawn position
    int spawnX = worldX;
    int spawnY = worldY;
    if (!FindValidSpawnPosition(&spawnX, &spawnY, def->isNaval)) {
        // Can't find a valid position - spawn anyway but log warning
        // (In a real scenario we might want to fail here)
    }

    Unit* unit = &g_units[id];
    memset(unit, 0, sizeof(Unit));
    unit->active = 1;
    unit->type = (uint8_t)type;
    unit->team = (uint8_t)team;
    unit->state = STATE_IDLE;
    unit->facing = 0;
    unit->maxHealth = def->maxHealth;
    unit->health = def->maxHealth;
    unit->worldX = spawnX;
    unit->worldY = spawnY;
    unit->targetX = spawnX;
    unit->targetY = spawnY;
    unit->targetUnit = -1;
    unit->speed = def->speed;
    unit->attackRange = def->attackRange;
    unit->attackDamage = def->attackDamage;
    unit->attackRate = def->attackRate;
    unit->attackCooldown = 0;
    unit->sightRange = def->sightRange;
    // Harvester-specific
    unit->cargo = 0;
    unit->homeRefinery = -1;
    unit->harvestTimer = 0;
    // Combat behavior
    unit->lastAttacker = -1;
    unit->scatterTimer = 0;

    // Mark the spawn cell as occupied
    int cellX, cellY;
    Map_WorldToCell(spawnX, spawnY, &cellX, &cellY);
    UpdateCellOccupancy(id, -1, -1, cellX, cellY);

    return id;
}

void Units_Remove(int unitId) {
    if (unitId >= 0 && unitId < MAX_UNITS) {
        Unit* unit = &g_units[unitId];
        if (unit->active) {
            // Clear cell occupancy
            int cellX, cellY;
            Map_WorldToCell(unit->worldX, unit->worldY, &cellX, &cellY);
            UpdateCellOccupancy(unitId, cellX, cellY, -1, -1);
        }
        unit->active = 0;
    }
}

Unit* Units_Get(int unitId) {
    if (unitId >= 0 && unitId < MAX_UNITS && g_units[unitId].active) {
        return &g_units[unitId];
    }
    return nullptr;
}

int Units_CountByTeam(Team team) {
    int count = 0;
    for (int i = 0; i < MAX_UNITS; i++) {
        if (g_units[i].active && g_units[i].team == team) {
            count++;
        }
    }
    return count;
}

int Buildings_Spawn(BuildingType type, Team team, int cellX, int cellY) {
    if (type <= BUILDING_NONE || type >= BUILDING_TYPE_COUNT) return -1;

    // Find free slot
    int id = -1;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (!g_buildings[i].active) {
            id = i;
            break;
        }
    }
    if (id < 0) return -1;

    const BuildingTypeDef* def = &g_buildingTypes[type];

    Building* bld = &g_buildings[id];
    memset(bld, 0, sizeof(Building));
    bld->active = 1;
    bld->type = (uint8_t)type;
    bld->team = (uint8_t)team;
    bld->maxHealth = def->maxHealth;
    bld->health = def->maxHealth;
    bld->cellX = (int16_t)cellX;
    bld->cellY = (int16_t)cellY;
    bld->width = def->width;
    bld->height = def->height;
    bld->sightRange = def->sightRange;

    // Mark cells as occupied
    for (int dy = 0; dy < def->height; dy++) {
        for (int dx = 0; dx < def->width; dx++) {
            MapCell* cell = Map_GetCell(cellX + dx, cellY + dy);
            if (cell) {
                cell->terrain = TERRAIN_BUILDING;
                cell->buildingId = (int16_t)id;
            }
        }
    }

    return id;
}

void Buildings_Remove(int buildingId) {
    if (buildingId >= 0 && buildingId < MAX_BUILDINGS) {
        Building* bld = &g_buildings[buildingId];
        if (bld->active) {
            // Clear cells
            for (int dy = 0; dy < bld->height; dy++) {
                for (int dx = 0; dx < bld->width; dx++) {
                    int cx = bld->cellX + dx, cy = bld->cellY + dy;
                    MapCell* cell = Map_GetCell(cx, cy);
                    if (cell) {
                        cell->terrain = TERRAIN_CLEAR;
                        cell->buildingId = -1;
                    }
                }
            }
            bld->active = 0;
        }
    }
}

Building* Buildings_Get(int buildingId) {
    if (buildingId < 0 || buildingId >= MAX_BUILDINGS) return nullptr;
    if (!g_buildings[buildingId].active) return nullptr;
    return &g_buildings[buildingId];
}

void Units_CommandMove(int unitId, int worldX, int worldY) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    unit->targetX = worldX;
    unit->targetY = worldY;
    unit->targetUnit = -1;
    unit->state = STATE_MOVING;

    // Clear existing path - will be calculated on next update
    unit->pathLength = 0;
    unit->pathIndex = 0;

    // Calculate facing direction (initial)
    int dx = worldX - unit->worldX;
    int dy = worldY - unit->worldY;
    if (dx != 0 || dy != 0) {
        double angle = atan2((double)dy, (double)dx);
        int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
        unit->facing = (uint8_t)((facing + 2) % 8); // Adjust for N=0
    }
}

void Units_CommandAttack(int unitId, int targetUnitId) {
    Unit* unit = Units_Get(unitId);
    Unit* target = Units_Get(targetUnitId);
    if (!unit || !target) return;

    unit->targetUnit = (int16_t)targetUnitId;
    unit->state = STATE_ATTACKING;
}

void Units_CommandStop(int unitId) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    unit->targetX = unit->worldX;
    unit->targetY = unit->worldY;
    unit->targetUnit = -1;
    unit->state = STATE_IDLE;
    unit->pathLength = 0;
}

void Units_CommandAttackMove(int unitId, int worldX, int worldY) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    unit->targetX = worldX;
    unit->targetY = worldY;
    unit->targetUnit = -1;
    unit->state = STATE_ATTACK_MOVE;

    // Clear existing path - will be calculated on next update
    unit->pathLength = 0;
    unit->pathIndex = 0;

    // Calculate facing direction
    int dx = worldX - unit->worldX;
    int dy = worldY - unit->worldY;
    if (dx != 0 || dy != 0) {
        double angle = atan2((double)dy, (double)dx);
        int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
        unit->facing = (uint8_t)((facing + 2) % 8);
    }
}

void Units_CommandGuard(int unitId) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    // Guard at current position
    unit->targetX = unit->worldX;
    unit->targetY = unit->worldY;
    unit->targetUnit = -1;
    unit->state = STATE_GUARDING;
    unit->pathLength = 0;
}

void Units_CommandForceAttack(int unitId, int worldX, int worldY) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    // Check if there's a unit at the target position (any team)
    int targetId = -1;
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* target = &g_units[i];
        if (!target->active || i == unitId) continue;

        const UnitTypeDef* tdef = &g_unitTypes[target->type];
        int halfSize = tdef->size / 2;

        int tx = target->worldX, ty = target->worldY;
        if (worldX >= tx - halfSize && worldX <= tx + halfSize &&
            worldY >= ty - halfSize && worldY <= ty + halfSize) {
            targetId = i;
            break;
        }
    }

    if (targetId >= 0) {
        // Force attack this unit (even if friendly)
        unit->targetUnit = (int16_t)targetId;
        unit->state = STATE_ATTACKING;
    } else {
        // Attack ground (move there)
        unit->targetX = worldX;
        unit->targetY = worldY;
        unit->targetUnit = -1;
        unit->state = STATE_MOVING;
        unit->pathLength = 0;
    }
}

void Units_NotifyAttacked(int victimId, int attackerId) {
    Unit* victim = Units_Get(victimId);
    if (!victim) return;

    // Record who attacked us
    victim->lastAttacker = (int16_t)attackerId;
}

void Units_ScatterInfantryNear(int worldX, int worldY, int radius) {
    int radiusSq = radius * radius;

    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;

        const UnitTypeDef* def = &g_unitTypes[unit->type];
        if (!def->isInfantry) continue;  // Only infantry scatter

        // Check if in scatter radius
        int dx = unit->worldX - worldX;
        int dy = unit->worldY - worldY;
        int distSq = dx * dx + dy * dy;

        if (distSq > radiusSq) continue;
        if (distSq == 0) continue;  // Don't scatter unit at explosion center

        // Scatter cooldown to prevent constant movement
        if (unit->scatterTimer > 0) continue;

        // Calculate scatter direction (away from explosion)
        int dist = (int)sqrt((double)distSq);
        int scatterDist = CELL_SIZE + (rand() % CELL_SIZE);  // 1-2 cells away

        int newX = unit->worldX + (dx * scatterDist) / dist;
        int newY = unit->worldY + (dy * scatterDist) / dist;

        // Validate position
        int cellX, cellY;
        Map_WorldToCell(newX, newY, &cellX, &cellY);
        if (Map_IsPassable(cellX, cellY)) {
            // Command move to scatter position
            Units_CommandMove(i, newX, newY);
            unit->scatterTimer = 30;  // Cooldown before can scatter again
        }
    }
}

int Units_CommandAllHunt(Team team) {
    int count = 0;
    Team enemyTeam = (team == TEAM_PLAYER) ? TEAM_ENEMY : TEAM_PLAYER;

    // Find nearest enemy unit to use as attack target for all hunters
    int nearestEnemy = -1;
    int nearestDist = INT_MAX;
    int centerX = 0, centerY = 0;
    int teamUnitCount = 0;

    // Calculate center of our units
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active || unit->team != team) continue;
        if (unit->state == STATE_DYING) continue;
        centerX += unit->worldX;
        centerY += unit->worldY;
        teamUnitCount++;
    }

    if (teamUnitCount > 0) {
        centerX /= teamUnitCount;
        centerY /= teamUnitCount;
    }

    // Find nearest enemy to our center
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active || unit->team != enemyTeam) continue;
        if (unit->state == STATE_DYING) continue;

        int dx = unit->worldX - centerX;
        int dy = unit->worldY - centerY;
        int dist = dx * dx + dy * dy;

        if (dist < nearestDist) {
            nearestDist = dist;
            nearestEnemy = i;
        }
    }

    // Also check buildings for nearest enemy
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = &g_buildings[i];
        if (!bld->active || bld->team != enemyTeam) continue;

        int bx = bld->cellX * CELL_SIZE + (bld->width * CELL_SIZE / 2);
        int by = bld->cellY * CELL_SIZE + (bld->height * CELL_SIZE / 2);
        int dx = bx - centerX;
        int dy = by - centerY;
        int dist = dx * dx + dy * dy;

        if (dist < nearestDist) {
            nearestDist = dist;
            // Use building center as target
            nearestEnemy = -1;  // No unit, use position
            centerX = bx;
            centerY = by;
        }
    }

    // Get target position
    int targetX, targetY;
    if (nearestEnemy >= 0) {
        targetX = g_units[nearestEnemy].worldX;
        targetY = g_units[nearestEnemy].worldY;
    } else if (nearestDist < INT_MAX) {
        // Building was nearest
        targetX = centerX;
        targetY = centerY;
    } else {
        // No enemy found - move toward map center
        targetX = 64 * CELL_SIZE;
        targetY = 64 * CELL_SIZE;
    }

    // Command all team units to attack-move toward target
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active || unit->team != team) continue;
        if (unit->state == STATE_DYING) continue;

        // Skip harvesters - they should keep harvesting
        if (unit->type == UNIT_HARVESTER) continue;

        Units_CommandAttackMove(i, targetX, targetY);
        count++;
    }

    return count;
}

void Units_Select(int unitId, BOOL addToSelection) {
    if (!addToSelection) {
        Units_DeselectAll();
    }

    Unit* unit = Units_Get(unitId);
    if (unit && unit->team == TEAM_PLAYER) {
        unit->selected = 1;
        // Play selection sound
        Sounds_PlayAt(SFX_UNIT_SELECT, unit->worldX, unit->worldY, 150);
    }
}

void Units_DeselectAll(void) {
    for (int i = 0; i < MAX_UNITS; i++) {
        g_units[i].selected = 0;
    }
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        g_buildings[i].selected = 0;
    }
}

int Units_GetFirstSelected(void) {
    for (int i = 0; i < MAX_UNITS; i++) {
        if (g_units[i].active && g_units[i].selected) {
            return i;
        }
    }
    return -1;
}

int Units_GetSelectedCount(void) {
    int count = 0;
    for (int i = 0; i < MAX_UNITS; i++) {
        if (g_units[i].active && g_units[i].selected) {
            count++;
        }
    }
    return count;
}

void Units_SelectInRect(int x1, int y1, int x2, int y2, Team team) {
    // Convert to world coordinates
    int wx1, wy1, wx2, wy2;
    Map_ScreenToWorld(x1, y1, &wx1, &wy1);
    Map_ScreenToWorld(x2, y2, &wx2, &wy2);

    // Normalize rect
    if (wx1 > wx2) { int t = wx1; wx1 = wx2; wx2 = t; }
    if (wy1 > wy2) { int t = wy1; wy1 = wy2; wy2 = t; }

    Units_DeselectAll();

    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active || unit->team != team) continue;

        if (unit->worldX >= wx1 && unit->worldX <= wx2 &&
            unit->worldY >= wy1 && unit->worldY <= wy2) {
            unit->selected = 1;
        }
    }
}

int Units_GetAtScreen(int screenX, int screenY) {
    int worldX, worldY;
    Map_ScreenToWorld(screenX, screenY, &worldX, &worldY);

    // Check units (prefer smaller/infantry first)
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;

        const UnitTypeDef* def = &g_unitTypes[unit->type];
        int halfSize = def->size / 2;

        int ux = unit->worldX, uy = unit->worldY;
        if (worldX >= ux - halfSize && worldX <= ux + halfSize &&
            worldY >= uy - halfSize && worldY <= uy + halfSize) {
            return i;
        }
    }

    return -1;
}

//===========================================================================
// Pathfinding System (A* algorithm)
//===========================================================================

// Check if a cell is passable for this unit type
static BOOL IsCellPassable(int cellX, int cellY, BOOL isNaval) {
    if (isNaval) {
        return Map_IsWaterPassable(cellX, cellY);
    }
    return Map_IsPassable(cellX, cellY);
}

// Check if unit can move to the target cell (used by combat/targeting)
static BOOL CanMoveTo(Unit* unit, int worldX, int worldY) {
    int cellX, cellY;
    Map_WorldToCell(worldX, worldY, &cellX, &cellY);
    const UnitTypeDef* def = &g_unitTypes[unit->type];
    return IsCellPassable(cellX, cellY, def->isNaval);
}
// Suppress unused warning - function available for future use
__attribute__((unused))
static BOOL (*CanMoveToRef)(Unit*, int, int) = CanMoveTo;

// Direction offsets for 8-way movement (N, NE, E, SE, S, SW, W, NW)
static const int DIR_DX[8] = { 0,  1, 1, 1, 0, -1, -1, -1 };
static const int DIR_DY[8] = { -1, -1, 0, 1, 1,  1,  0, -1 };
static const int DIR_COST[8] = { 10, 14, 10, 14, 10, 14, 10, 14 };

// A* node for pathfinding
struct PathNode {
    int16_t cellX, cellY;
    int g, h, f;
    int16_t parentX, parentY;

    bool operator>(const PathNode& other) const { return f > other.f; }
};

// Calculate heuristic (Manhattan distance * 10)
static int Heuristic(int x1, int y1, int x2, int y2) {
    return (abs(x2 - x1) + abs(y2 - y1)) * 10;
}

// Find path from start cell to target cell using A*
// Returns true if path found, fills unit->pathCells and unit->pathLength
static BOOL FindPath(Unit* unit, int startCellX, int startCellY,
                     int targetCellX, int targetCellY) {
    const UnitTypeDef* def = &g_unitTypes[unit->type];
    BOOL isNaval = def->isNaval;

    // Clear path
    unit->pathLength = 0;
    unit->pathIndex = 0;

    // Check if target is reachable
    if (!IsCellPassable(targetCellX, targetCellY, isNaval)) {
        return FALSE;
    }

    // Already at target?
    if (startCellX == targetCellX && startCellY == targetCellY) {
        return TRUE;
    }

    int mapW = Map_GetWidth();
    int mapH = Map_GetHeight();

    // Open set (priority queue)
    std::priority_queue<PathNode, std::vector<PathNode>,
                        std::greater<PathNode>> openSet;

    // Closed set and g-scores (simple 2D array for small maps)
    std::vector<bool> closed(mapW * mapH, false);
    std::vector<int> gScore(mapW * mapH, 0x7FFF);
    std::vector<int16_t> parentX(mapW * mapH, -1);
    std::vector<int16_t> parentY(mapW * mapH, -1);

    // Start node
    PathNode start;
    start.cellX = startCellX;
    start.cellY = startCellY;
    start.g = 0;
    start.h = Heuristic(startCellX, startCellY, targetCellX, targetCellY);
    start.f = start.h;
    start.parentX = -1;
    start.parentY = -1;

    openSet.push(start);
    gScore[startCellY * mapW + startCellX] = 0;

    int iterations = 0;
    const int MAX_ITERATIONS = 2000;  // Prevent infinite loops

    while (!openSet.empty() && iterations < MAX_ITERATIONS) {
        iterations++;

        PathNode current = openSet.top();
        openSet.pop();

        int idx = current.cellY * mapW + current.cellX;

        // Skip if already processed
        if (closed[idx]) continue;
        closed[idx] = true;

        // Found target?
        if (current.cellX == targetCellX && current.cellY == targetCellY) {
            // Reconstruct path (backwards)
            std::vector<int16_t> pathReverse;
            int cx = targetCellX, cy = targetCellY;

            while (cx != startCellX || cy != startCellY) {
                int cidx = cy * mapW + cx;
                pathReverse.push_back(cy * mapW + cx);

                int px = parentX[cidx];
                int py = parentY[cidx];
                if (px < 0 || py < 0) break;  // Safety
                cx = px;
                cy = py;

                // Safety limit
                if ((int)pathReverse.size() > MAX_PATH_WAYPOINTS * 2)
                    break;
            }

            // Copy path (reversed) to unit
            int pathLen = (int)pathReverse.size();
            if (pathLen > MAX_PATH_WAYPOINTS) pathLen = MAX_PATH_WAYPOINTS;
            unit->pathLength = pathLen;
            for (int i = 0; i < pathLen; i++) {
                unit->pathCells[i] = pathReverse[pathLen - 1 - i];
            }

            return TRUE;
        }

        // Explore neighbors
        for (int dir = 0; dir < 8; dir++) {
            int nx = current.cellX + DIR_DX[dir];
            int ny = current.cellY + DIR_DY[dir];

            // Bounds check
            if (nx < 0 || nx >= mapW || ny < 0 || ny >= mapH) continue;

            int nidx = ny * mapW + nx;

            // Already closed?
            if (closed[nidx]) continue;

            // Passable?
            if (!IsCellPassable(nx, ny, isNaval)) continue;

            // Calculate cost
            int newG = current.g + DIR_COST[dir];

            // Better path?
            if (newG < gScore[nidx]) {
                gScore[nidx] = newG;
                parentX[nidx] = current.cellX;
                parentY[nidx] = current.cellY;

                PathNode neighbor;
                neighbor.cellX = nx;
                neighbor.cellY = ny;
                neighbor.g = newG;
                neighbor.h = Heuristic(nx, ny, targetCellX, targetCellY);
                neighbor.f = neighbor.g + neighbor.h;
                neighbor.parentX = current.cellX;
                neighbor.parentY = current.cellY;

                openSet.push(neighbor);
            }
        }
    }

    return FALSE;  // No path found
}

// Set up the next waypoint from the path
static void SetNextWaypoint(Unit* unit) {
    if (unit->pathIndex >= unit->pathLength) {
        // Path complete
        unit->state = STATE_IDLE;
        return;
    }

    int mapW = Map_GetWidth();
    int cellIdx = unit->pathCells[unit->pathIndex];
    int cellX = cellIdx % mapW;
    int cellY = cellIdx / mapW;

    Map_CellToWorld(cellX, cellY, &unit->nextWaypointX, &unit->nextWaypointY);
    unit->pathIndex++;
}

// Check if unit can crush infantry (tanks only)
static BOOL CanCrushInfantry(Unit* unit) {
    // Only non-infantry ground vehicles can crush
    const UnitTypeDef* def = &g_unitTypes[unit->type];
    if (def->isInfantry) return FALSE;
    if (def->isNaval) return FALSE;

    // Harvesters can't crush (too slow/not military)
    if (unit->type == UNIT_HARVESTER) return FALSE;

    return TRUE;
}

// Check for and crush any enemy infantry at the given position
// Only crushes ENEMY infantry - friendly units are avoided
static void TryCrushInfantry(Unit* crusher, int unitId,
                             int worldX, int worldY) {
    if (!CanCrushInfantry(crusher)) return;

    const UnitTypeDef* crusherDef = &g_unitTypes[crusher->type];
    int crushRadius = crusherDef->size / 2;

    for (int i = 0; i < MAX_UNITS; i++) {
        if (i == unitId) continue;
        Unit* target = &g_units[i];
        if (!target->active) continue;
        if (target->state == STATE_DYING) continue;

        // ONLY crush ENEMY infantry - not friendlies!
        if (target->team == crusher->team) continue;

        const UnitTypeDef* targetDef = &g_unitTypes[target->type];
        if (!targetDef->isInfantry) continue;  // Can only crush infantry

        // Check distance
        int dx = target->worldX - worldX;
        int dy = target->worldY - worldY;
        int dist = (int)sqrt((double)(dx * dx + dy * dy));

        if (dist < crushRadius + targetDef->size / 2) {
            // Crush! Enemy infantry dies instantly
            target->health = 0;
            target->state = STATE_DYING;

            // Fire DESTROYED trigger if target has one attached
            if (target->triggerName[0] != '\0') {
                Mission_TriggerDestroyed(target->triggerName);
            }

            // Play squish sound
            Sounds_PlayAt(SFX_EXPLOSION_SM, target->worldX,
                          target->worldY, 120);
        }
    }
}

static void UpdateUnitMovement(Unit* unit, int unitId) {
    // Handle both MOVING and ATTACK_MOVE states
    if (unit->state != STATE_MOVING && unit->state != STATE_ATTACK_MOVE) return;

    // Track current cell for occupancy updates
    int oldCellX, oldCellY;
    Map_WorldToCell(unit->worldX, unit->worldY, &oldCellX, &oldCellY);

    // If no path, try to find one
    if (unit->pathLength == 0) {
        int startCellX, startCellY, targetCellX, targetCellY;
        Map_WorldToCell(unit->worldX, unit->worldY, &startCellX, &startCellY);
        Map_WorldToCell(unit->targetX, unit->targetY,
                        &targetCellX, &targetCellY);

        if (!FindPath(unit, startCellX, startCellY, targetCellX, targetCellY)) {
            // No path available
            unit->state = STATE_IDLE;
            return;
        }

        // Initialize waypoint
        unit->pathIndex = 0;
        SetNextWaypoint(unit);
    }

    // Check if next waypoint is occupied (by another unit)
    int waypointCellX, waypointCellY;
    Map_WorldToCell(unit->nextWaypointX, unit->nextWaypointY,
                    &waypointCellX, &waypointCellY);

    // Vehicles that can crush don't block on enemy infantry
    BOOL canCrush = CanCrushInfantry(unit);
    int wcx = waypointCellX, wcy = waypointCellY;
    if (IsCellOccupiedForTeam(wcx, wcy, unitId, unit->team, canCrush)) {
        // Cell occupied - wait or recalculate path
        // For now, just stop and clear path to recalculate next frame
        unit->pathLength = 0;
        return;
    }

    // Move toward current waypoint
    int dx = unit->nextWaypointX - unit->worldX;
    int dy = unit->nextWaypointY - unit->worldY;
    int dist = (int)sqrt((double)(dx * dx + dy * dy));

    if (dist <= unit->speed) {
        // Reached waypoint
        unit->worldX = unit->nextWaypointX;
        unit->worldY = unit->nextWaypointY;

        // Try to crush any infantry at this position
        TryCrushInfantry(unit, unitId, unit->worldX, unit->worldY);

        // Update cell occupancy if we changed cells
        int newCellX, newCellY;
        Map_WorldToCell(unit->worldX, unit->worldY, &newCellX, &newCellY);
        if (newCellX != oldCellX || newCellY != oldCellY) {
            UpdateCellOccupancy(unitId, oldCellX, oldCellY, newCellX, newCellY);
        }

        // Move to next waypoint
        if (unit->pathIndex < unit->pathLength) {
            SetNextWaypoint(unit);
        } else {
            // Final destination reached
            unit->state = STATE_IDLE;
        }
    } else {
        // Move toward waypoint
        unit->worldX += (dx * unit->speed) / dist;
        unit->worldY += (dy * unit->speed) / dist;

        // Try to crush any infantry at this position
        TryCrushInfantry(unit, unitId, unit->worldX, unit->worldY);

        // Update cell occupancy if we changed cells
        int newCellX, newCellY;
        Map_WorldToCell(unit->worldX, unit->worldY, &newCellX, &newCellY);
        if (newCellX != oldCellX || newCellY != oldCellY) {
            UpdateCellOccupancy(unitId, oldCellX, oldCellY, newCellX, newCellY);
        }

        // Update facing based on movement direction
        double angle = atan2((double)dy, (double)dx);
        int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
        unit->facing = (uint8_t)((facing + 2) % 8);  // Adjust for N=0
    }
}

static int FindNearestEnemy(Unit* unit, int maxRange) {
    int closestDist = maxRange + 1;
    int closestEnemy = -1;

    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* target = &g_units[i];
        if (!target->active) continue;
        if (target->team == unit->team) continue;
        if (target->team == TEAM_NEUTRAL) continue;
        if (target->health <= 0) continue;

        int dx = target->worldX - unit->worldX;
        int dy = target->worldY - unit->worldY;
        int dist = (int)sqrt((double)(dx * dx + dy * dy));

        if (dist < closestDist) {
            closestDist = dist;
            closestEnemy = i;
        }
    }

    return closestEnemy;
}

static void UpdateUnitCombat(Unit* unit, int unitId) {
    if (unit->attackCooldown > 0) {
        unit->attackCooldown--;
    }

    // Scatter timer countdown
    if (unit->scatterTimer > 0) {
        unit->scatterTimer--;
    }

    // Return fire: if idle/moving and we were attacked, fight back
    if ((unit->state == STATE_IDLE || unit->state == STATE_MOVING) &&
        unit->attackRange > 0 && unit->lastAttacker >= 0) {
        Unit* attacker = Units_Get(unit->lastAttacker);
        if (attacker && attacker->health > 0 && attacker->team != unit->team) {
            // Return fire!
            unit->targetUnit = unit->lastAttacker;
            unit->state = STATE_ATTACKING;
        }
        unit->lastAttacker = -1;  // Clear after processing
    }

    // Auto-engage: if idle and has attack capability, look for enemies
    if (unit->state == STATE_IDLE && unit->attackRange > 0) {
        // Scan wider than attack range
        int enemyId = FindNearestEnemy(unit, unit->attackRange * 2);
        if (enemyId >= 0) {
            unit->targetUnit = (int16_t)enemyId;
            unit->state = STATE_ATTACKING;
        }
    }

    // Attack-move: while moving, look for enemies in range and attack them
    if (unit->state == STATE_ATTACK_MOVE && unit->attackRange > 0) {
        int enemyId = FindNearestEnemy(unit, unit->attackRange);
        if (enemyId >= 0) {
            // Found enemy - attack them, but remember we're attack-moving
            unit->targetUnit = (int16_t)enemyId;
            // Stay in ATTACK_MOVE state so we continue after killing target
        }
    }

    // Guard mode: attack any enemies that come in range
    if (unit->state == STATE_GUARDING && unit->attackRange > 0) {
        int enemyId = FindNearestEnemy(unit, unit->attackRange * 2);
        if (enemyId >= 0) {
            unit->targetUnit = (int16_t)enemyId;
            // Stay in GUARDING state
        }
    }

    // Handle combat (ATTACKING, ATTACK_MOVE with target, GUARDING with target)
    bool inCombat = (unit->state == STATE_ATTACKING ||
                     unit->state == STATE_ATTACK_MOVE ||
                     unit->state == STATE_GUARDING);
    if (inCombat && unit->targetUnit >= 0) {
        Unit* target = Units_Get(unit->targetUnit);
        if (!target || target->health <= 0) {
            // Target dead - clear target, preserve state for attack-move
            unit->targetUnit = -1;
            if (unit->state == STATE_ATTACKING) {
                unit->state = STATE_IDLE;
            }
            // ATTACK_MOVE and GUARDING look for new targets
            return;
        }

        // Calculate distance to target
        int dx = target->worldX - unit->worldX;
        int dy = target->worldY - unit->worldY;
        int dist = (int)sqrt((double)(dx * dx + dy * dy));

        if (dist > unit->attackRange) {
            // Move closer
            Units_CommandMove(unitId, target->worldX, target->worldY);
            unit->targetUnit = (int16_t)unit->targetUnit; // Keep target
            unit->state = STATE_ATTACKING;
        } else if (unit->attackCooldown == 0) {
            // Attack!
            target->health -= unit->attackDamage;
            unit->attackCooldown = unit->attackRate;

            // Fire ATTACKED trigger if target has one attached
            if (target->triggerName[0] != '\0') {
                Mission_TriggerAttacked(target->triggerName);
            }

            // Notify target they were attacked (for return fire)
            Units_NotifyAttacked(unit->targetUnit, unitId);

            // Face target
            double angle = atan2((double)dy, (double)dx);
            int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
            unit->facing = (uint8_t)((facing + 2) % 8);

            // Play attack sound based on unit type
            const UnitTypeDef* def = &g_unitTypes[unit->type];
            SoundEffect sfx = SFX_GUN_SHOT;
            BOOL isExplosive = FALSE;
            if (unit->type == UNIT_ROCKET) {
                sfx = SFX_ROCKET;
                isExplosive = TRUE;
            } else if (unit->type == UNIT_GRENADIER) {
                sfx = SFX_EXPLOSION_SM;
                isExplosive = TRUE;
            } else if (!def->isInfantry) {
                sfx = SFX_CANNON;
                isExplosive = TRUE;
            }
            Sounds_PlayAt(sfx, unit->worldX, unit->worldY, 200);

            // Infantry scatter from explosive attacks
            if (isExplosive) {
                Units_ScatterInfantryNear(target->worldX, target->worldY,
                                         CELL_SIZE * 2);
            }

            // Check if target dies
            if (target->health <= 0) {
                target->state = STATE_DYING;

                // Fire DESTROYED trigger if target has one attached
                if (target->triggerName[0] != '\0') {
                    Mission_TriggerDestroyed(target->triggerName);
                }

                // Play death sound
                int tx = target->worldX, ty = target->worldY;
                Sounds_PlayAt(SFX_EXPLOSION_SM, tx, ty, 180);
                // Scatter infantry near the explosion
                Units_ScatterInfantryNear(tx, ty, CELL_SIZE * 3);
            }
        }
    }
}

//===========================================================================
// Harvester AI
//===========================================================================

// Player credits (shared with game_ui.cpp via extern)
static int* g_pPlayerCredits = nullptr;

void Units_SetCreditsPtr(int* creditsPtr) {
    g_pPlayerCredits = creditsPtr;
}

// House/team mapping functions
Team House_ToTeam(HouseType house) {
    // Soviet houses
    if (house == HOUSE_USSR || house == HOUSE_UKRAINE) {
        return TEAM_ENEMY;
    }
    // Allied houses (Spain, Greece, England, Germany, France, Turkey)
    if (house >= HOUSE_SPAIN && house <= HOUSE_TURKEY &&
        house != HOUSE_USSR && house != HOUSE_UKRAINE) {
        return TEAM_PLAYER;
    }
    return TEAM_NEUTRAL;
}

int House_IsAlly(HouseType h1, HouseType h2) {
    // Same house is always allied
    if (h1 == h2) return 1;

    // Both Soviet -> allied
    int h1Soviet = (h1 == HOUSE_USSR || h1 == HOUSE_UKRAINE);
    int h2Soviet = (h2 == HOUSE_USSR || h2 == HOUSE_UKRAINE);
    if (h1Soviet && h2Soviet) return 1;

    // Both Allied -> allied (neither is Soviet)
    if (!h1Soviet && !h2Soviet &&
        h1 >= HOUSE_SPAIN && h1 < HOUSE_COUNT &&
        h2 >= HOUSE_SPAIN && h2 < HOUSE_COUNT) {
        return 1;
    }

    return 0;  // Not allied
}

const char* House_GetName(HouseType house) {
    static const char* names[] = {
        "Spain", "Greece", "USSR", "England",
        "Ukraine", "Germany", "France", "Turkey"
    };
    if (house >= HOUSE_SPAIN && house < HOUSE_COUNT) {
        return names[house];
    }
    return "Unknown";
}

// Find nearest ore cell to the given position
static bool FindNearestOre(int fromX, int fromY, int* outCellX, int* outCellY) {
    int mapW = Map_GetWidth();
    int mapH = Map_GetHeight();
    int bestDist = 999999;
    bool found = false;

    int startCellX, startCellY;
    Map_WorldToCell(fromX, fromY, &startCellX, &startCellY);

    // Search in expanding rings
    for (int radius = 1; radius < 30; radius++) {
        int minY = startCellY - radius, maxY = startCellY + radius;
        int minX = startCellX - radius, maxX = startCellX + radius;
        for (int cy = minY; cy <= maxY; cy++) {
            for (int cx = minX; cx <= maxX; cx++) {
                if (cx < 0 || cy < 0 || cx >= mapW || cy >= mapH) continue;

                MapCell* cell = Map_GetCell(cx, cy);
                if (!cell) continue;

                if (cell->terrain == TERRAIN_ORE && cell->oreAmount > 0) {
                    int dx = cx - startCellX;
                    int dy = cy - startCellY;
                    int dist = dx * dx + dy * dy;
                    if (dist < bestDist) {
                        bestDist = dist;
                        *outCellX = cx;
                        *outCellY = cy;
                        found = true;
                    }
                }
            }
        }
        if (found) break;  // Found ore in this ring
    }
    return found;
}

// Find nearest refinery for the given team
static int FindNearestRefinery(int fromX, int fromY, Team team) {
    int bestDist = 999999;
    int bestId = -1;

    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->type != BUILDING_REFINERY) continue;
        if (bld->team != team) continue;

        int bldWorldX, bldWorldY;
        int cx = bld->cellX + bld->width / 2;
        int cy = bld->cellY + bld->height / 2;
        Map_CellToWorld(cx, cy, &bldWorldX, &bldWorldY);

        int dx = bldWorldX - fromX;
        int dy = bldWorldY - fromY;
        int dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            bestId = i;
        }
    }
    return bestId;
}

// Update harvester behavior
static void UpdateHarvester(Unit* unit, int unitId) {
    if (unit->type != UNIT_HARVESTER) return;

    int cellX, cellY;
    Map_WorldToCell(unit->worldX, unit->worldY, &cellX, &cellY);
    MapCell* currentCell = Map_GetCell(cellX, cellY);

    switch (unit->state) {
        case STATE_IDLE: {
            // Find ore and start harvesting
            if (unit->cargo >= HARVESTER_MAX_CARGO) {
                // Full - return to refinery
                unit->state = STATE_RETURNING;
            } else {
                // Look for ore
                int oreCellX, oreCellY;
                int wx = unit->worldX, wy = unit->worldY;
                if (FindNearestOre(wx, wy, &oreCellX, &oreCellY)) {
                    int oreWorldX, oreWorldY;
                    Map_CellToWorld(oreCellX, oreCellY, &oreWorldX, &oreWorldY);
                    Units_CommandMove(unitId, oreWorldX, oreWorldY);
                    // After moving to ore, state will be checked again
                }
            }
            break;
        }

        case STATE_MOVING: {
            // Check if we've arrived at ore
            bool hasOre = currentCell && currentCell->terrain == TERRAIN_ORE;
            if (hasOre && currentCell->oreAmount > 0) {
                // Start harvesting
                unit->state = STATE_HARVESTING;
                unit->harvestTimer = 30;  // Harvest every 30 ticks
            }
            // Otherwise movement continues via UpdateUnitMovement
            break;
        }

        case STATE_HARVESTING: {
            // Harvest ore from current cell
            bool noOre = !currentCell || currentCell->terrain != TERRAIN_ORE;
            if (noOre || currentCell->oreAmount == 0) {
                // No more ore here - find more or return
                if (unit->cargo >= HARVESTER_MAX_CARGO * 3 / 4) {
                    // Mostly full - return
                    unit->state = STATE_RETURNING;
                } else {
                    // Look for more ore
                    unit->state = STATE_IDLE;
                }
                break;
            }

            // Harvest tick
            unit->harvestTimer--;
            if (unit->harvestTimer <= 0) {
                // Extract ore
                int toHarvest = HARVESTER_LOAD_RATE;
                if (toHarvest > currentCell->oreAmount) {
                    toHarvest = currentCell->oreAmount;
                }
                if (unit->cargo + toHarvest > HARVESTER_MAX_CARGO) {
                    toHarvest = HARVESTER_MAX_CARGO - unit->cargo;
                }

                unit->cargo += toHarvest;
                currentCell->oreAmount -= toHarvest;

                // If cell depleted, change terrain back to clear
                if (currentCell->oreAmount == 0) {
                    currentCell->terrain = TERRAIN_CLEAR;
                }

                // Check if full
                if (unit->cargo >= HARVESTER_MAX_CARGO) {
                    unit->state = STATE_RETURNING;
                } else {
                    unit->harvestTimer = 30;  // Continue harvesting
                }
            }
            break;
        }

        case STATE_RETURNING: {
            // Find refinery and return
            if (unit->homeRefinery < 0 || !Buildings_Get(unit->homeRefinery)) {
                int wx = unit->worldX, wy = unit->worldY;
                Team tm = (Team)unit->team;
                unit->homeRefinery = FindNearestRefinery(wx, wy, tm);
            }

            Building* refinery = Buildings_Get(unit->homeRefinery);
            if (!refinery) {
                // No refinery - just idle
                unit->state = STATE_IDLE;
                break;
            }

            // Move toward refinery
            int refX, refY;
            int rx = refinery->cellX + 1;
            int ry = refinery->cellY + refinery->height;
            Map_CellToWorld(rx, ry, &refX, &refY);

            int dx = refX - unit->worldX;
            int dy = refY - unit->worldY;
            int dist = (int)sqrt((double)(dx * dx + dy * dy));

            if (dist < CELL_SIZE * 2) {
                // At refinery - unload
                bool isPlayer = unit->team == TEAM_PLAYER;
                if (unit->cargo > 0 && g_pPlayerCredits && isPlayer) {
                    int credits = (unit->cargo * ORE_VALUE) / 10;  // Scale down
                    *g_pPlayerCredits += credits;
                    unit->cargo = 0;
                }
                // Go back to harvesting
                unit->state = STATE_IDLE;
            } else if (unit->pathLength == 0) {
                // Need to path to refinery
                Units_CommandMove(unitId, refX, refY);
                unit->state = STATE_RETURNING;  // Keep returning state
            }
            break;
        }

        default:
            break;
    }
}

void Units_Update(void) {
    // === Fog of War: Clear and reveal around player units/buildings ===
    Map_ClearVisibility();

    // Reveal around player units
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;
        if (unit->team != TEAM_PLAYER) continue;

        int cellX, cellY;
        Map_WorldToCell(unit->worldX, unit->worldY, &cellX, &cellY);
        Map_RevealAround(cellX, cellY, unit->sightRange, TEAM_PLAYER);
    }

    // Reveal around player buildings
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = &g_buildings[i];
        if (!bld->active) continue;
        if (bld->team != TEAM_PLAYER) continue;

        // Reveal from center of building
        int centerX = bld->cellX + bld->width / 2;
        int centerY = bld->cellY + bld->height / 2;
        Map_RevealAround(centerX, centerY, bld->sightRange, TEAM_PLAYER);
    }

    // === Update units ===
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;

        // Handle dying state
        if (unit->state == STATE_DYING) {
            // Could add death animation here
            Units_Remove(i);
            continue;
        }

        UpdateUnitMovement(unit, i);
        UpdateUnitCombat(unit, i);
        UpdateHarvester(unit, i);
    }

    // Update building combat (turrets)
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = &g_buildings[i];
        if (!bld->active) continue;

        const BuildingTypeDef* def = &g_buildingTypes[bld->type];
        if (!def->canAttack) continue;

        if (bld->attackCooldown > 0) {
            bld->attackCooldown--;
            continue;
        }

        // Find closest enemy
        int bldWorldX, bldWorldY;
        int cx = bld->cellX + bld->width / 2;
        int cy = bld->cellY + bld->height / 2;
        Map_CellToWorld(cx, cy, &bldWorldX, &bldWorldY);

        int closestDist = def->attackRange + 1;
        int closestEnemy = -1;

        for (int j = 0; j < MAX_UNITS; j++) {
            Unit* target = &g_units[j];
            bool skip = !target->active || target->team == bld->team;
            if (skip || target->team == TEAM_NEUTRAL) continue;

            int dx = target->worldX - bldWorldX;
            int dy = target->worldY - bldWorldY;
            int dist = (int)sqrt((double)(dx * dx + dy * dy));

            if (dist < closestDist) {
                closestDist = dist;
                closestEnemy = j;
            }
        }

        // Attack closest enemy
        if (closestEnemy >= 0) {
            Unit* target = &g_units[closestEnemy];
            target->health -= def->attackDamage;
            bld->attackCooldown = def->attackRate;

            // Fire ATTACKED trigger if target has one attached
            if (target->triggerName[0] != '\0') {
                Mission_TriggerAttacked(target->triggerName);
            }

            // Play turret sound based on building type
            SoundEffect sfx;
            if (bld->type == BUILDING_SAM) {
                sfx = SFX_ROCKET;
            } else if (bld->type == BUILDING_TURRET) {
                sfx = SFX_GUN_SHOT;  // Pillbox uses machine gun sound
            } else {
                sfx = SFX_CANNON;    // Other turrets (AA gun, etc.) use cannon
            }
            Sounds_PlayAt(sfx, bldWorldX, bldWorldY, 200);

            if (target->health <= 0) {
                target->state = STATE_DYING;

                // Fire DESTROYED trigger if target has one attached
                if (target->triggerName[0] != '\0') {
                    Mission_TriggerDestroyed(target->triggerName);
                }

                int tx = target->worldX, ty = target->worldY;
                Sounds_PlayAt(SFX_EXPLOSION_SM, tx, ty, 180);
            }
        }
    }
}

void Units_Render(void) {
    Viewport* vp = Map_GetViewport();

    // Render buildings first (under units)
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = &g_buildings[i];
        if (!bld->active) continue;

        const BuildingTypeDef* def = &g_buildingTypes[bld->type];

        // Calculate screen position
        int worldX = bld->cellX * CELL_SIZE;
        int worldY = bld->cellY * CELL_SIZE;
        int screenX = worldX - vp->x;
        int screenY = worldY - vp->y;

        int pixelWidth = bld->width * CELL_SIZE;
        int pixelHeight = bld->height * CELL_SIZE;

        // Skip if off screen
        if (screenX + pixelWidth < 0 || screenX > vp->width ||
            screenY + pixelHeight < 0 || screenY > vp->height) {
            continue;
        }

        // Hide enemy buildings in fog of war
        if (bld->team != TEAM_PLAYER) {
            int centerX = bld->cellX + bld->width / 2;
            int centerY = bld->cellY + bld->height / 2;
            if (!Map_IsCellVisible(centerX, centerY)) {
                continue;  // Don't render - in fog
            }
        }

        // Get team color
        uint8_t color = g_teamColors[bld->team];

        // Try to render with real sprite first
        BuildingType bt = (BuildingType)bld->type;
        if (!Sprites_RenderBuilding(bt, 0, screenX, screenY, color)) {
            // Fallback to basic shapes
            int x2 = screenX + 2, y2 = screenY + 2;
            int w4 = pixelWidth - 4, h4 = pixelHeight - 4;
            Renderer_FillRect(x2, y2, w4, h4, def->color);
            int x1 = screenX + 1, y1 = screenY + 1;
            int w2 = pixelWidth - 2, h2 = pixelHeight - 2;
            Renderer_DrawRect(x1, y1, w2, h2, color);
        }

        // Draw selection indicator
        if (bld->selected) {
            int sx = screenX - 1, sy = screenY - 1;
            int sw = pixelWidth + 2, sh = pixelHeight + 2;
            Renderer_DrawRect(sx, sy, sw, sh, 15);
        }

        // Draw health bar
        int hw = (pixelWidth - 4) * bld->health / bld->maxHealth;
        int halfHp = bld->maxHealth / 2;
        int quartHp = bld->maxHealth / 4;
        uint8_t hc = (bld->health > halfHp) ? 10 :
                     (bld->health > quartHp) ? 14 : 4;
        Renderer_FillRect(screenX + 2, screenY - 4, hw, 2, hc);
    }

    // Render units
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;

        const UnitTypeDef* def = &g_unitTypes[unit->type];

        // Calculate screen position
        int screenX, screenY;
        Map_WorldToScreen(unit->worldX, unit->worldY, &screenX, &screenY);

        int halfSize = def->size / 2;

        // Skip if off screen
        if (screenX + halfSize < 0 || screenX - halfSize > vp->width ||
            screenY + halfSize < 0 || screenY - halfSize > vp->height) {
            continue;
        }

        // Hide enemy units in fog of war
        if (unit->team != TEAM_PLAYER) {
            int cellX, cellY;
            Map_WorldToCell(unit->worldX, unit->worldY, &cellX, &cellY);
            if (!Map_IsCellVisible(cellX, cellY)) {
                continue;  // Don't render - in fog
            }
        }

        // Get team color
        uint8_t teamColor = g_teamColors[unit->team];

        // Try to render with real sprite first
        UnitType ut = (UnitType)unit->type;
        int sx = screenX, sy = screenY;
        if (!Sprites_RenderUnit(ut, unit->facing, 0, sx, sy, teamColor)) {
            // Fallback to basic shapes
            int ux = screenX - halfSize, uy = screenY - halfSize;
            if (def->isInfantry) {
                // Draw infantry as small circle
                Renderer_FillCircle(screenX, screenY, halfSize, teamColor);
            } else {
                // Draw vehicle as rectangle
                int sz = def->size;
                Renderer_FillRect(ux, uy, sz, sz, def->color);
                // Team color stripe
                Renderer_FillRect(ux, uy, sz, 3, teamColor);
            }

            // Draw facing indicator (gun barrel) - only for fallback
            if (unit->attackRange > 0) {
                static const int facingDx[] = {0, 1, 1, 1, 0, -1, -1, -1};
                static const int facingDy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
                int barrelLen = halfSize + 2;
                int bx = screenX + facingDx[unit->facing] * barrelLen;
                int by = screenY + facingDy[unit->facing] * barrelLen;
                Renderer_DrawLine(screenX, screenY, bx, by, 8);
            }
        }

        // Draw selection indicator
        if (unit->selected) {
            Renderer_DrawCircle(screenX, screenY, halfSize + 2, 15);
        }

        // Draw health bar
        int hw = (def->size) * unit->health / unit->maxHealth;
        int halfHp = unit->maxHealth / 2;
        int quartHp = unit->maxHealth / 4;
        uint8_t hc = (unit->health > halfHp) ? 10 :
                     (unit->health > quartHp) ? 14 : 4;
        int hx = screenX - halfSize, hy = screenY - halfSize - 4;
        Renderer_FillRect(hx, hy, hw, 2, hc);
    }
}
