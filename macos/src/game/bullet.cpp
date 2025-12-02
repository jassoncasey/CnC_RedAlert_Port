/**
 * Red Alert macOS Port - Bullet Class Implementation
 *
 * Based on original BULLET.CPP (~1000 lines)
 */

#include "bullet.h"
#include "combat.h"
#include "mapclass.h"
#include <cmath>
#include <cstdlib>

//===========================================================================
// Global Bullet Pool
//===========================================================================
ObjectPool<BulletClass, BULLET_MAX> Bullets;

//===========================================================================
// Construction
//===========================================================================

BulletClass::BulletClass()
    : ObjectClass()
    , type_(BulletType::NONE)
    , payback_(nullptr)
    , warhead_(WarheadType::NONE)
    , damage_(0)
    , tarCom_(0)
    , targetCoord_(0)
    , facing_(DirType::N)
    , speed_(0)
    , maxSpeed_(0)
    , sourceCoord_(0)
    , state_(BulletState::IDLE)
    , flightTime_(0)
    , armingDelay_(0)
    , fuelRemaining_(-1)
    , isInaccurate_(false)
    , isHoming_(false)
    , isArcing_(false)
    , isHighAltitude_(false)
    , arcPeak_(0)
    , arcProgress_(0)
    , frame_(0)
{
    rtti_ = RTTIType::BULLET;
}

BulletClass::BulletClass(BulletType type, TechnoClass* source, int32_t target,
                         int damage, WarheadType warhead)
    : BulletClass()
{
    Init(type, source, target, damage, warhead);
}

void BulletClass::Init(BulletType type, TechnoClass* source, int32_t target,
                       int damage, WarheadType warhead) {
    type_ = type;
    payback_ = source;
    targetCoord_ = target;
    damage_ = static_cast<int16_t>(damage);
    warhead_ = warhead;

    const BulletTypeData* typeData = TypeClass();
    if (typeData) {
        isInaccurate_ = typeData->isInaccurate;
        isHoming_ = (typeData->rotationStages > 0);
        isArcing_ = typeData->isArcing;
        isHighAltitude_ = typeData->isHigh;

        // Default speed based on bullet type
        maxSpeed_ = 40;  // Default speed in leptons/tick

        // Arming delay - prevents immediate detonation
        armingDelay_ = 2;

        // Fuel for fueled projectiles
        if (typeData->isFueled) {
            fuelRemaining_ = 100;  // ~100 ticks of fuel
        } else {
            fuelRemaining_ = -1;
        }
    }

    // Apply scatter if inaccurate
    if (isInaccurate_) {
        targetCoord_ = ApplyScatter(target);
    }

    state_ = BulletState::IDLE;
    flightTime_ = 0;
}

//===========================================================================
// Type Queries
//===========================================================================

const BulletTypeData* BulletClass::TypeClass() const {
    return GetBulletType(type_);
}

const char* BulletClass::Name() const {
    const BulletTypeData* typeData = TypeClass();
    if (typeData) {
        return typeData->iniName;
    }
    return "BULLET";
}

bool BulletClass::IsInstantHit() const {
    const BulletTypeData* typeData = TypeClass();
    return typeData ? typeData->isInvisible : false;
}

bool BulletClass::IsHoming() const {
    return isHoming_;
}

bool BulletClass::IsAntiAircraft() const {
    const BulletTypeData* typeData = TypeClass();
    return typeData ? typeData->isAntiAircraft : true;
}

bool BulletClass::IsAntiGround() const {
    const BulletTypeData* typeData = TypeClass();
    return typeData ? typeData->isAntiGround : true;
}

//===========================================================================
// Position and Movement
//===========================================================================

bool BulletClass::Launch(int32_t sourceCoord) {
    sourceCoord_ = sourceCoord;
    coord_ = sourceCoord;

    // Calculate initial facing toward target
    facing_ = static_cast<DirType>(DirectionTo(targetCoord_));

    // Set initial speed
    speed_ = maxSpeed_;

    // Calculate arc parameters if arcing
    if (isArcing_) {
        int distance = DistanceTo(targetCoord_);
        // Peak at 1/4 distance height
        arcPeak_ = static_cast<int16_t>(distance / 4);
        arcProgress_ = 0;
    }

    // Enter active state
    state_ = BulletState::FLYING;
    flightTime_ = 0;

    return Unlimbo(sourceCoord, facing_);
}

void BulletClass::UpdateFlight() {
    if (state_ != BulletState::FLYING) return;

    flightTime_++;

    // Consume fuel
    if (fuelRemaining_ > 0) {
        fuelRemaining_--;
        if (fuelRemaining_ == 0) {
            // Out of fuel - detonate immediately
            state_ = BulletState::DETONATING;
            return;
        }
    }

    // Update homing if applicable
    if (isHoming_) {
        UpdateHoming();
    }

    // Calculate movement delta
    int dx = 0, dy = 0;

    // Direction to target
    int targetX = Coord_X(targetCoord_);
    int targetY = Coord_Y(targetCoord_);
    int currentX = Coord_X(coord_);
    int currentY = Coord_Y(coord_);

    int deltaX = targetX - currentX;
    int deltaY = targetY - currentY;
    int distance = DistanceToTarget();

    if (distance > 0) {
        // Normalize movement by speed
        if (distance <= speed_) {
            // Will reach target this tick
            dx = deltaX;
            dy = deltaY;
        } else {
            // Move toward target at speed
            dx = (deltaX * speed_) / distance;
            dy = (deltaY * speed_) / distance;
        }
    }

    // Apply movement
    int newX = currentX + dx;
    int newY = currentY + dy;

    // Update arc progress
    if (isArcing_) {
        int totalDist = DistanceTo(sourceCoord_) + DistanceToTarget();
        if (totalDist > 0) {
            int srcDist = DistanceTo(sourceCoord_);
            int prog = (srcDist * 256) / totalDist;
            arcProgress_ = static_cast<int16_t>(prog);
        }
        height_ = CalculateArcHeight();
    }

    coord_ = XY_Coord(newX, newY);

    // Update facing for animated bullets
    facing_ = static_cast<DirType>(DirectionTo(targetCoord_));
}

int32_t BulletClass::ArcingPosition() const {
    if (!isArcing_) return coord_;

    // Add height offset to coordinate for rendering
    int16_t arcHeight = CalculateArcHeight();
    return XY_Coord(Coord_X(coord_), Coord_Y(coord_) - arcHeight);
}

bool BulletClass::HasReachedTarget() const {
    return DistanceToTarget() <= speed_;
}

int BulletClass::DistanceToTarget() const {
    return DistanceTo(targetCoord_);
}

int32_t BulletClass::ApplyScatter(int32_t target) const {
    // Add random scatter to target
    int scatter = 64;  // ~1/4 cell scatter
    int offsetX = (rand() % (scatter * 2 + 1)) - scatter;
    int offsetY = (rand() % (scatter * 2 + 1)) - scatter;

    return XY_Coord(Coord_X(target) + offsetX, Coord_Y(target) + offsetY);
}

void BulletClass::UpdateHoming() {
    // Recalculate facing toward target each tick
    // For homing missiles, they gradually turn toward target
    uint8_t desiredDir = DirectionTo(targetCoord_);
    uint8_t currentDir = static_cast<uint8_t>(facing_);

    // Calculate shortest turn direction
    int diff = desiredDir - currentDir;
    if (diff > 128) diff -= 256;
    if (diff < -128) diff += 256;

    // Limit turn rate (ROT from bullet type)
    const BulletTypeData* typeData = TypeClass();
    int maxTurn = typeData ? typeData->rotationStages * 4 : 8;

    if (diff > maxTurn) diff = maxTurn;
    if (diff < -maxTurn) diff = -maxTurn;

    facing_ = static_cast<DirType>((currentDir + diff) & 0xFF);
}

int16_t BulletClass::CalculateArcHeight() const {
    // Parabolic arc: height = peak * (1 - (2*progress - 1)^2)
    // At progress=0 or 256, height=0; at progress=128, height=peak
    int normalized = (arcProgress_ * 2) - 256;  // -256 to 256
    int squared = (normalized * normalized) / 256;  // 0 to 256
    int height = (arcPeak_ * (256 - squared)) / 256;
    return static_cast<int16_t>(height > 0 ? height : 0);
}

//===========================================================================
// Detonation
//===========================================================================

bool BulletClass::ShouldDetonate() const {
    if (state_ != BulletState::FLYING) return false;

    // Check arming delay
    if (flightTime_ < armingDelay_) return false;

    // Check if reached target
    if (HasReachedTarget()) return true;

    // Check proximity fuse
    const BulletTypeData* typeData = TypeClass();
    if (typeData && typeData->isProximityFused) {
        if (DistanceToTarget() < LEPTONS_PER_CELL / 2) {
            return true;
        }
    }

    // Out of fuel
    if (fuelRemaining_ == 0) return true;

    return false;
}

void BulletClass::Detonate() {
    if (state_ == BulletState::DETONATING) return;

    state_ = BulletState::DETONATING;
    Impact(coord_);
}

void BulletClass::Impact(int32_t coord) {
    // Apply explosion damage
    Explosion_Damage(coord, damage_, payback_, warhead_);

    // TODO: Create explosion animation based on warhead type

    // Remove bullet from game
    Limbo();
}

//===========================================================================
// AI Processing
//===========================================================================

void BulletClass::AI() {
    ObjectClass::AI();

    switch (state_) {
        case BulletState::IDLE:
            // Waiting to be launched
            break;

        case BulletState::FLYING:
            UpdateFlight();
            if (ShouldDetonate()) {
                Detonate();
            }
            break;

        case BulletState::DETONATING:
            // Cleanup handled in Detonate()
            break;
    }
}

//===========================================================================
// Rendering
//===========================================================================

void BulletClass::DrawIt(int x, int y, int window) const {
    // Would render bullet sprite at given position
    // Invisible bullets don't render
    const BulletTypeData* typeData = TypeClass();
    if (typeData && typeData->isInvisible) return;

    // TODO: Render bullet sprite
    (void)x;
    (void)y;
    (void)window;
}

int BulletClass::ShapeNumber() const {
    const BulletTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Calculate frame from facing if bullet has rotation stages
    int stages = typeData->rotationStages;
    if (stages > 0) {
        int facing = static_cast<int>(facing_) / (256 / stages);
        return facing % stages;
    }

    return 0;
}

//===========================================================================
// Limbo/Unlimbo
//===========================================================================

bool BulletClass::Limbo() {
    state_ = BulletState::IDLE;
    return ObjectClass::Limbo();
}

bool BulletClass::Unlimbo(int32_t coord, DirType facing) {
    if (!ObjectClass::Unlimbo(coord, facing)) return false;

    facing_ = facing;
    return true;
}

//===========================================================================
// Helper Functions
//===========================================================================

BulletClass* CreateBullet(BulletType type, TechnoClass* source,
                          int32_t sourceCoord, int32_t targetCoord,
                          int damage, WarheadType warhead) {
    // Check for instant-hit bullets
    const BulletTypeData* typeData = GetBulletType(type);
    if (typeData && typeData->isInvisible) {
        // Don't create actual bullet - apply damage instantly
        InstantHit(source, targetCoord, damage, warhead);
        return nullptr;
    }

    // Allocate new bullet
    BulletClass* bullet = Bullets.Allocate();
    if (bullet) {
        bullet->Init(type, source, targetCoord, damage, warhead);
        bullet->Launch(sourceCoord);
    }
    return bullet;
}

void InstantHit(TechnoClass* source, int32_t targetCoord,
                int damage, WarheadType warhead) {
    // Apply damage immediately without creating a bullet
    Explosion_Damage(targetCoord, damage, source, warhead);
}
