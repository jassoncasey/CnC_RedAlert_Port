/**
 * Red Alert macOS Port - Combat System Implementation
 *
 * Based on original COMBAT.CPP (~400 lines)
 */

#include "combat.h"
#include "object.h"
#include "bullet.h"
#include "mapclass.h"
#include "cell.h"
#include <cmath>
#include <algorithm>

//===========================================================================
// Warhead Helpers
//===========================================================================

// Helper to convert WarheadType to WarheadTypeEnum
static inline WarheadTypeEnum ToWhEnum(WarheadType wh) {
    return static_cast<WarheadTypeEnum>(static_cast<int>(wh));
}

int GetWarheadModifier(WarheadType warhead, ArmorType armor) {
    const WarheadTypeData* whData = GetWarheadType(ToWhEnum(warhead));

    if (!whData) return 256;  // 100% if no data

    // Return appropriate modifier based on armor type
    switch (armor) {
        case ArmorType::NONE:
            return whData->vsNone;
        case ArmorType::WOOD:
            return whData->vsWood;
        case ArmorType::LIGHT:
            return whData->vsLight;
        case ArmorType::HEAVY:
            return whData->vsHeavy;
        case ArmorType::CONCRETE:
            return whData->vsConcrete;
        default:
            return 256;
    }
}

int GetWarheadSpread(WarheadType warhead) {
    const WarheadTypeData* whData = GetWarheadType(ToWhEnum(warhead));

    if (!whData) return DEFAULT_SPREAD;
    return whData->spread;
}

bool CanDestroyWall(WarheadType warhead) {
    const WarheadTypeData* whData = GetWarheadType(ToWhEnum(warhead));
    return whData ? whData->isWallDestroyer : false;
}

bool CanDestroyWood(WarheadType warhead) {
    const WarheadTypeData* whData = GetWarheadType(ToWhEnum(warhead));
    return whData ? whData->isWoodDestroyer : false;
}

bool CanDestroyOre(WarheadType warhead) {
    const WarheadTypeData* whData = GetWarheadType(ToWhEnum(warhead));
    return whData ? whData->isTiberiumDestroyer : false;
}

//===========================================================================
// Core Combat Functions
//===========================================================================

int Modify_Damage(int damage, WarheadType warhead, ArmorType armor,
                  int distance) {
    // No damage if base damage is 0
    if (damage == 0) return 0;

    // Handle healing (negative damage)
    if (damage < 0) {
        // Healing only works at close range
        if (distance > FULL_DAMAGE_DISTANCE) {
            return 0;
        }
        return damage;  // Return negative value for healing
    }

    // Apply warhead vs armor modifier
    // Modifier is fixed-point: 256 = 100%, 128 = 50%, etc.
    int modifier = GetWarheadModifier(warhead, armor);
    damage = (damage * modifier) / 256;

    // Apply distance falloff
    // Damage decreases linearly with distance beyond full damage range
    if (distance > FULL_DAMAGE_DISTANCE) {
        int spread = GetWarheadSpread(warhead);
        if (spread > 0) {
            // Calculate falloff factor
            int effectiveDistance = distance - FULL_DAMAGE_DISTANCE;
            int falloffRange = spread * 2;  // Full falloff over 2x spread

            if (effectiveDistance >= falloffRange) {
                // Beyond falloff range - minimum damage
                damage = MIN_DAMAGE;
            } else {
                // Linear falloff
                int remaining = falloffRange - effectiveDistance;
                damage = damage * remaining / falloffRange;
            }
        }
    }

    // Enforce minimum damage (prevents 0 damage from high armor)
    if (damage < MIN_DAMAGE) {
        damage = MIN_DAMAGE;
    }

    // Cap maximum damage
    if (damage > MAX_DAMAGE) {
        damage = MAX_DAMAGE;
    }

    return damage;
}

void Explosion_Damage(int32_t coord, int damage, TechnoClass* source,
                      WarheadType warhead) {
    if (damage == 0) return;

    // Get center cell
    CELL centerCell = Coord_Cell(coord);

    // Process center cell and 8 adjacent cells
    static const int cellOffsets[9][2] = {
        { 0,  0}, // Center
        {-1, -1}, {0, -1}, {1, -1},  // Top row
        {-1,  0},          {1,  0},  // Middle row (skip center)
        {-1,  1}, {0,  1}, {1,  1}   // Bottom row
    };

    int centerX = Cell_X(centerCell);
    int centerY = Cell_Y(centerCell);

    for (int i = 0; i < 9; i++) {
        int cellX = centerX + cellOffsets[i][0];
        int cellY = centerY + cellOffsets[i][1];

        // Bounds check
        bool outX = cellX < 0 || cellX >= MAP_CELL_W;
        bool outY = cellY < 0 || cellY >= MAP_CELL_H;
        if (outX || outY) continue;

        CELL cell = XY_Cell(cellX, cellY);
        CellClass& cellRef = Map[cell];

        // Process cell occupier (main building/unit in cell)
        ObjectClass* occupier = cellRef.CellOccupier();
        if (occupier && occupier != source) {
            // Calculate distance from explosion center to object
            int distance = Distance(coord, occupier->CenterCoord());

            // Get object's armor type
            ArmorType armor = ArmorType::NONE;
            if (occupier->IsTechno()) {
                // TechnoClass has armor type
                // For now, use NONE as default
                // TODO: Get armor from type data
            }

            // Calculate modified damage
            int objDamage = Modify_Damage(damage, warhead, armor, distance);

            // Apply damage to object
            if (objDamage > 0) {
                occupier->TakeDamage(objDamage, distance, warhead,
                                     source, false);
            }
        }

        // Process overlapper list (infantry, etc. in corners)
        for (int j = 0; j < MAX_OVERLAPPER; j++) {
            ObjectClass* overlapper = cellRef.overlappers_[j];
            if (!overlapper || overlapper == source || overlapper == occupier) {
                continue;
            }

            int distance = Distance(coord, overlapper->CenterCoord());
            ArmorType armor = ArmorType::NONE;

            int objDamage = Modify_Damage(damage, warhead, armor, distance);
            if (objDamage > 0) {
                overlapper->TakeDamage(objDamage, distance, warhead,
                                       source, false);
            }
        }

        // Handle wall destruction
        if (CanDestroyWall(warhead) && cellRef.IsWall()) {
            // Calculate distance to cell center
            int cellCoord = Cell_Coord(cell);
            int distance = Distance(coord, cellCoord);

            if (distance < GetWarheadSpread(warhead)) {
                cellRef.ReduceWall(damage);
            }
        }

        // Handle ore destruction
        if (CanDestroyOre(warhead) && cellRef.HasOre()) {
            int cellCoord = Cell_Coord(cell);
            int distance = Distance(coord, cellCoord);

            if (distance < GetWarheadSpread(warhead) / 2) {
                cellRef.ReduceOre(1);
            }
        }
    }
}

//===========================================================================
// Combat Animation
//===========================================================================

AnimType Combat_Anim(int damage, WarheadType warhead) {
    // Select explosion animation based on damage and warhead
    // Higher damage = bigger explosion

    if (damage < 10) {
        return AnimType::PIFF;  // Tiny puff
    } else if (damage < 25) {
        return AnimType::PIFFPIFF;  // Small explosion
    } else if (damage < 50) {
        return AnimType::VEH_HIT1;  // Medium explosion
    } else if (damage < 100) {
        return AnimType::VEH_HIT2;  // Large explosion
    } else {
        return AnimType::VEH_HIT3;  // Huge explosion
    }

    // TODO: Consider warhead type for special animations
    // e.g., FIRE warhead should use flame animation
    (void)warhead;
}

//===========================================================================
// Weapon Firing
//===========================================================================

bool Fire_Weapon(TechnoClass* source, WeaponTypeEnum weapon,
                 int32_t targetCoord) {
    if (!source) return false;

    const WeaponTypeData* wpnData = GetWeaponType(weapon);
    if (!wpnData) return false;

    // Get source coordinate
    int32_t sourceCoord = source->CenterCoord();

    // Check range
    if (!In_Range(source, weapon, targetCoord)) {
        return false;
    }

    // Create bullet
    BulletType bulletType = wpnData->bullet;
    int damage = wpnData->damage;
    int whIdx = static_cast<int>(wpnData->warhead);
    WarheadType warhead = static_cast<WarheadType>(whIdx);

    CreateBullet(bulletType, source, sourceCoord, targetCoord,
                 damage, warhead);

    // TODO: Play firing sound
    // TODO: Create muzzle flash animation

    return true;
}

bool In_Range(TechnoClass* source, WeaponTypeEnum weapon, int32_t targetCoord) {
    if (!source) return false;

    int range = GetWeaponRange(weapon);
    if (range <= 0) return false;

    int distance = Distance(source->CenterCoord(), targetCoord);
    return distance <= range;
}

int GetWeaponRange(WeaponTypeEnum weapon) {
    const WeaponTypeData* wpnData = GetWeaponType(weapon);
    if (!wpnData) return 0;
    return wpnData->range;
}

int GetWeaponROF(WeaponTypeEnum weapon) {
    const WeaponTypeData* wpnData = GetWeaponType(weapon);
    if (!wpnData) return 60;  // Default ~1 second
    return wpnData->rateOfFire;
}
