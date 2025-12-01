/**
 * Red Alert macOS Port - Unit/Entity System Implementation
 */

#include "units.h"
#include "map.h"
#include "sprites.h"
#include "sounds.h"
#include "graphics/metal/renderer.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

// Unit type definitions
struct UnitTypeDef {
    int16_t maxHealth;
    int16_t speed;
    int16_t attackRange;
    int16_t attackDamage;
    int16_t attackRate;
    uint8_t size;           // Visual size in pixels
    uint8_t color;          // Base color
    BOOL isInfantry;
    BOOL isNaval;
};

static const UnitTypeDef g_unitTypes[UNIT_TYPE_COUNT] = {
    // UNIT_NONE
    { 0, 0, 0, 0, 0, 0, 0, FALSE, FALSE },
    // UNIT_RIFLE
    { 50, 2, 64, 8, 30, 6, 15, TRUE, FALSE },
    // UNIT_GRENADIER
    { 60, 2, 96, 20, 45, 8, 15, TRUE, FALSE },
    // UNIT_ROCKET
    { 45, 2, 128, 30, 60, 8, 15, TRUE, FALSE },
    // UNIT_ENGINEER
    { 25, 2, 0, 0, 0, 6, 15, TRUE, FALSE },
    // UNIT_HARVESTER
    { 200, 3, 0, 0, 0, 18, 14, FALSE, FALSE },
    // UNIT_TANK_LIGHT
    { 150, 5, 96, 25, 30, 14, 7, FALSE, FALSE },
    // UNIT_TANK_MEDIUM
    { 250, 4, 112, 40, 35, 16, 7, FALSE, FALSE },
    // UNIT_TANK_HEAVY
    { 500, 3, 128, 60, 40, 20, 7, FALSE, FALSE },
    // UNIT_APC
    { 150, 6, 48, 10, 20, 14, 7, FALSE, FALSE },
    // UNIT_ARTILLERY
    { 100, 3, 192, 50, 60, 16, 7, FALSE, FALSE },
    // UNIT_GUNBOAT
    { 200, 4, 96, 20, 30, 16, 1, FALSE, TRUE },
    // UNIT_DESTROYER
    { 350, 5, 128, 40, 35, 20, 1, FALSE, TRUE },
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
};

static const BuildingTypeDef g_buildingTypes[BUILDING_TYPE_COUNT] = {
    // BUILDING_NONE
    { 0, 0, 0, 0, FALSE, 0, 0, 0 },
    // BUILDING_CONSTRUCTION
    { 500, 3, 3, 7, FALSE, 0, 0, 0 },
    // BUILDING_POWER
    { 300, 2, 2, 14, FALSE, 0, 0, 0 },
    // BUILDING_REFINERY
    { 400, 3, 2, 14, FALSE, 0, 0, 0 },
    // BUILDING_BARRACKS
    { 350, 2, 2, 7, FALSE, 0, 0, 0 },
    // BUILDING_FACTORY
    { 400, 3, 3, 7, FALSE, 0, 0, 0 },
    // BUILDING_RADAR
    { 300, 2, 2, 7, FALSE, 0, 0, 0 },
    // BUILDING_TURRET
    { 200, 1, 1, 8, TRUE, 128, 30, 25 },
    // BUILDING_SAM
    { 250, 2, 1, 8, TRUE, 160, 40, 40 },
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

    Unit* unit = &g_units[id];
    memset(unit, 0, sizeof(Unit));
    unit->active = 1;
    unit->type = (uint8_t)type;
    unit->team = (uint8_t)team;
    unit->state = STATE_IDLE;
    unit->facing = 0;
    unit->maxHealth = def->maxHealth;
    unit->health = def->maxHealth;
    unit->worldX = worldX;
    unit->worldY = worldY;
    unit->targetX = worldX;
    unit->targetY = worldY;
    unit->targetUnit = -1;
    unit->speed = def->speed;
    unit->attackRange = def->attackRange;
    unit->attackDamage = def->attackDamage;
    unit->attackRate = def->attackRate;
    unit->attackCooldown = 0;

    return id;
}

void Units_Remove(int unitId) {
    if (unitId >= 0 && unitId < MAX_UNITS) {
        g_units[unitId].active = 0;
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
                    MapCell* cell = Map_GetCell(bld->cellX + dx, bld->cellY + dy);
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
    if (buildingId >= 0 && buildingId < MAX_BUILDINGS && g_buildings[buildingId].active) {
        return &g_buildings[buildingId];
    }
    return nullptr;
}

void Units_CommandMove(int unitId, int worldX, int worldY) {
    Unit* unit = Units_Get(unitId);
    if (!unit) return;

    unit->targetX = worldX;
    unit->targetY = worldY;
    unit->targetUnit = -1;
    unit->state = STATE_MOVING;

    // Calculate facing direction
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

        if (worldX >= unit->worldX - halfSize && worldX <= unit->worldX + halfSize &&
            worldY >= unit->worldY - halfSize && worldY <= unit->worldY + halfSize) {
            return i;
        }
    }

    return -1;
}

static void UpdateUnitMovement(Unit* unit) {
    if (unit->state != STATE_MOVING) return;

    int dx = unit->targetX - unit->worldX;
    int dy = unit->targetY - unit->worldY;
    int dist = (int)sqrt((double)(dx * dx + dy * dy));

    if (dist <= unit->speed) {
        // Arrived
        unit->worldX = unit->targetX;
        unit->worldY = unit->targetY;
        unit->state = STATE_IDLE;
    } else {
        // Move toward target
        unit->worldX += (dx * unit->speed) / dist;
        unit->worldY += (dy * unit->speed) / dist;

        // Update facing
        double angle = atan2((double)dy, (double)dx);
        int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
        unit->facing = (uint8_t)((facing + 2) % 8);
    }
}

static int FindNearestEnemy(Unit* unit, int maxRange) {
    int closestDist = maxRange + 1;
    int closestEnemy = -1;

    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* target = &g_units[i];
        if (!target->active) continue;
        if (target->team == unit->team || target->team == TEAM_NEUTRAL) continue;
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

    // Auto-engage: if idle and has attack capability, look for enemies
    if (unit->state == STATE_IDLE && unit->attackRange > 0) {
        int enemyId = FindNearestEnemy(unit, unit->attackRange * 2);  // Scan wider than attack range
        if (enemyId >= 0) {
            unit->targetUnit = (int16_t)enemyId;
            unit->state = STATE_ATTACKING;
        }
    }

    if (unit->state == STATE_ATTACKING && unit->targetUnit >= 0) {
        Unit* target = Units_Get(unit->targetUnit);
        if (!target || target->health <= 0) {
            // Target dead
            unit->targetUnit = -1;
            unit->state = STATE_IDLE;
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

            // Face target
            double angle = atan2((double)dy, (double)dx);
            int facing = (int)((angle + M_PI) / (M_PI / 4.0)) % 8;
            unit->facing = (uint8_t)((facing + 2) % 8);

            // Play attack sound based on unit type
            const UnitTypeDef* def = &g_unitTypes[unit->type];
            SoundEffect sfx = SFX_GUN_SHOT;
            if (unit->type == UNIT_ROCKET) {
                sfx = SFX_ROCKET;
            } else if (!def->isInfantry) {
                sfx = SFX_CANNON;
            }
            Sounds_PlayAt(sfx, unit->worldX, unit->worldY, 200);

            // Check if target dies
            if (target->health <= 0) {
                target->state = STATE_DYING;
                // Play death sound
                Sounds_PlayAt(SFX_EXPLOSION_SM, target->worldX, target->worldY, 180);
            }
        }
    }
}

void Units_Update(void) {
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = &g_units[i];
        if (!unit->active) continue;

        // Handle dying state
        if (unit->state == STATE_DYING) {
            // Could add death animation here
            Units_Remove(i);
            continue;
        }

        UpdateUnitMovement(unit);
        UpdateUnitCombat(unit, i);
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
        Map_CellToWorld(bld->cellX + bld->width / 2, bld->cellY + bld->height / 2, &bldWorldX, &bldWorldY);

        int closestDist = def->attackRange + 1;
        int closestEnemy = -1;

        for (int j = 0; j < MAX_UNITS; j++) {
            Unit* target = &g_units[j];
            if (!target->active || target->team == bld->team || target->team == TEAM_NEUTRAL) continue;

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

            // Play turret sound
            SoundEffect sfx = (bld->type == BUILDING_SAM) ? SFX_ROCKET : SFX_CANNON;
            Sounds_PlayAt(sfx, bldWorldX, bldWorldY, 200);

            if (target->health <= 0) {
                target->state = STATE_DYING;
                Sounds_PlayAt(SFX_EXPLOSION_SM, target->worldX, target->worldY, 180);
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

        // Get team color
        uint8_t color = g_teamColors[bld->team];

        // Try to render with real sprite first
        if (!Sprites_RenderBuilding((BuildingType)bld->type, 0, screenX, screenY, color)) {
            // Fallback to basic shapes
            Renderer_FillRect(screenX + 2, screenY + 2, pixelWidth - 4, pixelHeight - 4, def->color);
            Renderer_DrawRect(screenX + 1, screenY + 1, pixelWidth - 2, pixelHeight - 2, color);
        }

        // Draw selection indicator
        if (bld->selected) {
            Renderer_DrawRect(screenX - 1, screenY - 1, pixelWidth + 2, pixelHeight + 2, 15);
        }

        // Draw health bar
        int healthWidth = (pixelWidth - 4) * bld->health / bld->maxHealth;
        uint8_t healthColor = (bld->health > bld->maxHealth / 2) ? 10 :
                              (bld->health > bld->maxHealth / 4) ? 14 : 4;
        Renderer_FillRect(screenX + 2, screenY - 4, healthWidth, 2, healthColor);
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

        // Get team color
        uint8_t teamColor = g_teamColors[unit->team];

        // Try to render with real sprite first
        if (!Sprites_RenderUnit((UnitType)unit->type, unit->facing, 0, screenX, screenY, teamColor)) {
            // Fallback to basic shapes
            if (def->isInfantry) {
                // Draw infantry as small circle
                Renderer_FillCircle(screenX, screenY, halfSize, teamColor);
            } else {
                // Draw vehicle as rectangle
                Renderer_FillRect(screenX - halfSize, screenY - halfSize, def->size, def->size, def->color);
                // Team color stripe
                Renderer_FillRect(screenX - halfSize, screenY - halfSize, def->size, 3, teamColor);
            }

            // Draw facing indicator (gun barrel) - only for fallback
            if (unit->attackRange > 0) {
                static const int facingDx[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
                static const int facingDy[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
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
        int healthWidth = (def->size) * unit->health / unit->maxHealth;
        uint8_t healthColor = (unit->health > unit->maxHealth / 2) ? 10 :
                              (unit->health > unit->maxHealth / 4) ? 14 : 4;
        Renderer_FillRect(screenX - halfSize, screenY - halfSize - 4, healthWidth, 2, healthColor);
    }
}
