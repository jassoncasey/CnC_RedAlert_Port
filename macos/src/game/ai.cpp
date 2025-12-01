/**
 * Red Alert macOS Port - Basic AI System Implementation
 */

#include "ai.h"
#include "units.h"
#include "map.h"
#include <cstdlib>
#include <cmath>

//===========================================================================
// AI Configuration
//===========================================================================

// Build order for AI (in sequence)
static const BuildingType AI_BUILD_ORDER[] = {
    BUILDING_POWER,
    BUILDING_REFINERY,
    BUILDING_BARRACKS,
    BUILDING_POWER,      // Extra power
    BUILDING_FACTORY,
    BUILDING_RADAR,
};
static const int AI_BUILD_ORDER_COUNT = 6;

// Unit production weights
struct UnitWeight {
    UnitType type;
    int weight;         // Higher = more likely to build
    int minCount;       // Minimum before attacking
};

static const UnitWeight AI_UNIT_WEIGHTS[] = {
    { UNIT_RIFLE,       40, 4 },
    { UNIT_GRENADIER,   20, 2 },
    { UNIT_ROCKET,      30, 2 },
    { UNIT_TANK_LIGHT,  50, 2 },
    { UNIT_TANK_MEDIUM, 60, 2 },
};
static const int AI_UNIT_WEIGHT_COUNT = 5;

// Cost table (must match game_ui.cpp)
static const int AI_BUILDING_COSTS[] = {
    0,      // BUILDING_NONE
    0,      // BUILDING_CONSTRUCTION (free/starting)
    300,    // BUILDING_POWER
    2000,   // BUILDING_REFINERY
    500,    // BUILDING_BARRACKS
    2000,   // BUILDING_FACTORY
    1000,   // BUILDING_RADAR
    600,    // BUILDING_TURRET
    750,    // BUILDING_SAM
};

static const int AI_UNIT_COSTS[] = {
    0,      // UNIT_NONE
    100,    // UNIT_RIFLE
    160,    // UNIT_GRENADIER
    300,    // UNIT_ROCKET
    500,    // UNIT_ENGINEER
    1400,   // UNIT_HARVESTER
    700,    // UNIT_TANK_LIGHT
    800,    // UNIT_TANK_MEDIUM
    1500,   // UNIT_TANK_HEAVY
    800,    // UNIT_APC
    600,    // UNIT_ARTILLERY
    500,    // UNIT_GUNBOAT
    1000,   // UNIT_DESTROYER
};

//===========================================================================
// AI State
//===========================================================================

static AIDifficulty g_aiDifficulty = AI_MEDIUM;
static AIState g_aiState = AI_STATE_BUILDING;
static int g_aiCredits = 5000;
static int g_aiBuildOrderIndex = 0;
static int g_aiBuildTimer = 0;
static int g_aiProductionTimer = 0;
static int g_aiAttackTimer = 0;
static int g_aiHarvesterCount = 0;

// Timings based on difficulty (in game ticks, ~15 FPS)
static int g_buildDelay = 300;      // 20 seconds between buildings
static int g_productionDelay = 150; // 10 seconds between units
static int g_attackDelay = 900;     // 60 seconds between attacks
static int g_incomeRate = 50;       // Credits per tick (simulated harvester income)

//===========================================================================
// Helper Functions
//===========================================================================

// Find AI's construction yard position
static bool FindAIConYard(int* outCellX, int* outCellY) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_ENEMY) continue;
        if (bld->type == BUILDING_CONSTRUCTION) {
            *outCellX = bld->cellX;
            *outCellY = bld->cellY;
            return true;
        }
    }
    return false;
}

// Find valid placement near construction yard
static bool FindBuildingPlacement(int conYardX, int conYardY, int width, int height, int* outX, int* outY) {
    // Search in expanding rings around construction yard
    for (int radius = 3; radius < 15; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int cx = conYardX + dx;
                int cy = conYardY + dy;

                // Check all cells for validity
                bool valid = true;
                for (int by = 0; by < height && valid; by++) {
                    for (int bx = 0; bx < width && valid; bx++) {
                        MapCell* cell = Map_GetCell(cx + bx, cy + by);
                        if (!cell) { valid = false; break; }
                        if (cell->terrain != TERRAIN_CLEAR) valid = false;
                        if (cell->buildingId >= 0) valid = false;
                        if (cell->unitId >= 0) valid = false;
                    }
                }

                if (valid) {
                    *outX = cx;
                    *outY = cy;
                    return true;
                }
            }
        }
    }
    return false;
}

// Find spawn position near a building
static bool FindUnitSpawnPosition(int nearX, int nearY, int* outX, int* outY) {
    for (int radius = 2; radius < 10; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int cx = nearX + dx;
                int cy = nearY + dy;
                MapCell* cell = Map_GetCell(cx, cy);
                if (!cell) continue;
                if (cell->terrain == TERRAIN_CLEAR && cell->unitId < 0 && cell->buildingId < 0) {
                    int worldX, worldY;
                    Map_CellToWorld(cx, cy, &worldX, &worldY);
                    *outX = worldX;
                    *outY = worldY;
                    return true;
                }
            }
        }
    }
    return false;
}

// Get building dimensions
static void GetBuildingSize(BuildingType type, int* width, int* height) {
    switch (type) {
        case BUILDING_POWER:       *width = 2; *height = 2; break;
        case BUILDING_REFINERY:    *width = 3; *height = 3; break;
        case BUILDING_BARRACKS:    *width = 2; *height = 2; break;
        case BUILDING_FACTORY:     *width = 3; *height = 3; break;
        case BUILDING_RADAR:       *width = 2; *height = 2; break;
        case BUILDING_TURRET:      *width = 1; *height = 1; break;
        case BUILDING_SAM:         *width = 2; *height = 1; break;
        default:                   *width = 2; *height = 2; break;
    }
}

//===========================================================================
// AI Logic
//===========================================================================

void AI_Init(void) {
    g_aiState = AI_STATE_BUILDING;
    g_aiCredits = 5000;
    g_aiBuildOrderIndex = 0;
    g_aiBuildTimer = 0;
    g_aiProductionTimer = 0;
    g_aiAttackTimer = 0;
    g_aiHarvesterCount = 0;
}

void AI_Shutdown(void) {
    // Nothing to clean up
}

void AI_SetDifficulty(AIDifficulty difficulty) {
    g_aiDifficulty = difficulty;
    switch (difficulty) {
        case AI_EASY:
            g_buildDelay = 450;      // 30 seconds
            g_productionDelay = 225; // 15 seconds
            g_attackDelay = 1350;    // 90 seconds
            g_incomeRate = 30;
            break;
        case AI_MEDIUM:
            g_buildDelay = 300;
            g_productionDelay = 150;
            g_attackDelay = 900;
            g_incomeRate = 50;
            break;
        case AI_HARD:
            g_buildDelay = 200;      // 13 seconds
            g_productionDelay = 100; // 7 seconds
            g_attackDelay = 600;     // 40 seconds
            g_incomeRate = 80;
            break;
    }
}

void AI_SetCredits(int credits) {
    g_aiCredits = credits;
}

int AI_GetCredits(void) {
    return g_aiCredits;
}

BOOL AI_HasBuilding(BuildingType type) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_ENEMY) continue;
        if (bld->type == type) return TRUE;
    }
    return FALSE;
}

int AI_CountUnits(UnitType type) {
    int count = 0;
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_ENEMY) continue;
        if (type == UNIT_NONE || unit->type == type) {
            count++;
        }
    }
    return count;
}

// Try to build next building in order
static void AI_TryBuildStructure(void) {
    if (g_aiBuildOrderIndex >= AI_BUILD_ORDER_COUNT) {
        return; // Build order complete
    }

    BuildingType toBuild = AI_BUILD_ORDER[g_aiBuildOrderIndex];
    int cost = AI_BUILDING_COSTS[toBuild];

    // Check if we can afford it
    if (g_aiCredits < cost) {
        return;
    }

    // Find construction yard
    int conYardX, conYardY;
    if (!FindAIConYard(&conYardX, &conYardY)) {
        return; // No construction yard
    }

    // Find placement
    int width, height;
    GetBuildingSize(toBuild, &width, &height);

    int placeX, placeY;
    if (!FindBuildingPlacement(conYardX, conYardY, width, height, &placeX, &placeY)) {
        return; // No valid placement
    }

    // Build it
    int id = Buildings_Spawn(toBuild, TEAM_ENEMY, placeX, placeY);
    if (id >= 0) {
        g_aiCredits -= cost;
        g_aiBuildOrderIndex++;

        // Mark cells as building terrain
        for (int by = 0; by < height; by++) {
            for (int bx = 0; bx < width; bx++) {
                MapCell* cell = Map_GetCell(placeX + bx, placeY + by);
                if (cell) {
                    cell->terrain = TERRAIN_BUILDING;
                    cell->buildingId = (int16_t)id;
                }
            }
        }
    }
}

// Try to produce a unit
static void AI_TryProduceUnit(void) {
    // Need barracks or factory
    bool hasBarracks = AI_HasBuilding(BUILDING_BARRACKS);
    bool hasFactory = AI_HasBuilding(BUILDING_FACTORY);

    if (!hasBarracks && !hasFactory) {
        return;
    }

    // Pick a random unit weighted by preference
    int totalWeight = 0;
    for (int i = 0; i < AI_UNIT_WEIGHT_COUNT; i++) {
        UnitType type = AI_UNIT_WEIGHTS[i].type;
        // Check prerequisites
        bool canBuild = false;
        if (type >= UNIT_RIFLE && type <= UNIT_ENGINEER) {
            canBuild = hasBarracks;
        } else {
            canBuild = hasFactory;
        }
        if (canBuild && g_aiCredits >= AI_UNIT_COSTS[type]) {
            totalWeight += AI_UNIT_WEIGHTS[i].weight;
        }
    }

    if (totalWeight == 0) return;

    int roll = rand() % totalWeight;
    int cumulative = 0;
    UnitType toBuild = UNIT_NONE;

    for (int i = 0; i < AI_UNIT_WEIGHT_COUNT; i++) {
        UnitType type = AI_UNIT_WEIGHTS[i].type;
        bool canBuild = false;
        if (type >= UNIT_RIFLE && type <= UNIT_ENGINEER) {
            canBuild = hasBarracks;
        } else {
            canBuild = hasFactory;
        }
        if (canBuild && g_aiCredits >= AI_UNIT_COSTS[type]) {
            cumulative += AI_UNIT_WEIGHTS[i].weight;
            if (roll < cumulative) {
                toBuild = type;
                break;
            }
        }
    }

    if (toBuild == UNIT_NONE) return;

    // Find spawn location near production building
    int spawnNearX = 0, spawnNearY = 0;
    bool foundBuilding = false;

    // Find relevant production building
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_ENEMY) continue;

        if ((toBuild >= UNIT_RIFLE && toBuild <= UNIT_ENGINEER && bld->type == BUILDING_BARRACKS) ||
            (toBuild >= UNIT_TANK_LIGHT && bld->type == BUILDING_FACTORY)) {
            spawnNearX = bld->cellX;
            spawnNearY = bld->cellY + bld->height;
            foundBuilding = true;
            break;
        }
    }

    if (!foundBuilding) return;

    int spawnX, spawnY;
    if (!FindUnitSpawnPosition(spawnNearX, spawnNearY, &spawnX, &spawnY)) {
        return;
    }

    // Spawn unit
    int id = Units_Spawn(toBuild, TEAM_ENEMY, spawnX, spawnY);
    if (id >= 0) {
        g_aiCredits -= AI_UNIT_COSTS[toBuild];
    }
}

// Check if AI has enough units for an attack
static bool AI_HasEnoughForAttack(void) {
    for (int i = 0; i < AI_UNIT_WEIGHT_COUNT; i++) {
        int count = AI_CountUnits(AI_UNIT_WEIGHTS[i].type);
        if (count < AI_UNIT_WEIGHTS[i].minCount) {
            return false;
        }
    }
    return true;
}

// Find player base center
static bool FindPlayerBase(int* outX, int* outY) {
    // Look for player construction yard or any building
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_PLAYER) continue;

        int worldX, worldY;
        Map_CellToWorld(bld->cellX + bld->width / 2, bld->cellY + bld->height / 2, &worldX, &worldY);
        *outX = worldX;
        *outY = worldY;
        return true;
    }

    // Fall back to any player unit
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_PLAYER) continue;
        *outX = unit->worldX;
        *outY = unit->worldY;
        return true;
    }

    return false;
}

// Send attack wave
static void AI_SendAttack(void) {
    int targetX, targetY;
    if (!FindPlayerBase(&targetX, &targetY)) {
        return;
    }

    // Command all idle AI units to attack player base
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_ENEMY) continue;
        if (unit->type == UNIT_HARVESTER) continue; // Don't send harvesters

        // Only send units that are idle or already attacking
        if (unit->state == STATE_IDLE || unit->state == STATE_ATTACKING) {
            // Find nearest enemy and attack, or move toward base
            int closestEnemy = -1;
            int closestDist = 999999;

            for (int j = 0; j < MAX_UNITS; j++) {
                Unit* target = Units_Get(j);
                if (!target || !target->active) continue;
                if (target->team == TEAM_PLAYER) {
                    int dx = target->worldX - unit->worldX;
                    int dy = target->worldY - unit->worldY;
                    int dist = dx * dx + dy * dy;
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestEnemy = j;
                    }
                }
            }

            if (closestEnemy >= 0 && closestDist < 500 * 500) {
                Units_CommandAttack(i, closestEnemy);
            } else {
                Units_CommandMove(i, targetX, targetY);
            }
        }
    }
}

void AI_Update(void) {
    // Simulate income (simplified harvester income)
    if (AI_HasBuilding(BUILDING_REFINERY)) {
        g_aiCredits += g_incomeRate / 15; // Per tick at ~15 FPS
    }

    // Cap credits
    if (g_aiCredits > 50000) g_aiCredits = 50000;

    // Building phase
    g_aiBuildTimer++;
    if (g_aiBuildTimer >= g_buildDelay) {
        AI_TryBuildStructure();
        g_aiBuildTimer = 0;
    }

    // Unit production
    g_aiProductionTimer++;
    if (g_aiProductionTimer >= g_productionDelay) {
        AI_TryProduceUnit();
        g_aiProductionTimer = 0;
    }

    // Attack waves
    g_aiAttackTimer++;
    if (g_aiAttackTimer >= g_attackDelay) {
        if (AI_HasEnoughForAttack() || AI_CountUnits(UNIT_NONE) > 5) {
            AI_SendAttack();
            g_aiState = AI_STATE_ATTACKING;
        }
        g_aiAttackTimer = 0;
    }

    // Auto-acquire for idle AI units (defend/attack nearby enemies)
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_ENEMY) continue;
        if (unit->type == UNIT_HARVESTER) continue;
        if (unit->attackDamage == 0) continue;

        // If idle, look for nearby enemies
        if (unit->state == STATE_IDLE) {
            int closestEnemy = -1;
            int closestDist = unit->attackRange * 3;  // Aggro range
            closestDist *= closestDist;

            for (int j = 0; j < MAX_UNITS; j++) {
                Unit* target = Units_Get(j);
                if (!target || !target->active) continue;
                if (target->team == TEAM_PLAYER) {
                    int dx = target->worldX - unit->worldX;
                    int dy = target->worldY - unit->worldY;
                    int dist = dx * dx + dy * dy;
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestEnemy = j;
                    }
                }
            }

            if (closestEnemy >= 0) {
                Units_CommandAttack(i, closestEnemy);
            }
        }
    }
}
