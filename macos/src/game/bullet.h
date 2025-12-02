/**
 * Red Alert macOS Port - Bullet Class
 *
 * Projectiles that travel from shooter to target.
 * Based on original BULLET.H
 */

#ifndef GAME_BULLET_H
#define GAME_BULLET_H

#include "object.h"
#include "weapon_types.h"

// Maximum bullets in game
constexpr int BULLET_MAX = 500;

// Bullet states
enum class BulletState : int8_t {
    IDLE = 0,
    FLYING,
    DETONATING
};

/**
 * BulletClass - Flying projectile
 *
 * Bullets:
 * - Travel from source to target coordinate
 * - Apply damage on impact via warhead
 * - Can be homing (missiles) or dumb (shells)
 * - Some are invisible (instant hit weapons)
 */
class BulletClass : public ObjectClass {
public:
    //-----------------------------------------------------------------------
    // Bullet-specific state
    //-----------------------------------------------------------------------

    // Type reference
    BulletType type_;

    // Source (who fired this) - for kill credit
    TechnoClass* payback_;

    // Weapon data
    WarheadType warhead_;
    int16_t damage_;                // Base damage to inflict

    // Target
    uint32_t tarCom_;               // Target (object or cell)
    int32_t targetCoord_;           // Target coordinate

    // Movement
    DirType facing_;                // Direction of travel
    int16_t speed_;                 // Current speed (leptons per tick)
    int16_t maxSpeed_;              // Maximum speed
    int32_t sourceCoord_;           // Where bullet originated

    // Flight state
    BulletState state_;
    int16_t flightTime_;            // Ticks in flight
    int16_t armingDelay_;           // Ticks before can detonate
    int16_t fuelRemaining_;         // Fuel for fueled missiles (-1 = unlimited)

    // Flags
    bool isInaccurate_ : 1;         // Has scatter
    bool isHoming_ : 1;             // Homes in on target
    bool isArcing_ : 1;             // Ballistic arc trajectory
    bool isHighAltitude_ : 1;       // Flying high (immune to ground fire)

    // Arcing trajectory data
    int16_t arcPeak_;               // Peak height of arc
    int16_t arcProgress_;           // Progress along arc (0-256)

    // Animation
    int16_t frame_;                 // Current animation frame

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    BulletClass();
    BulletClass(BulletType type, TechnoClass* source,
                int32_t target, int damage, WarheadType warhead);
    virtual ~BulletClass() = default;

    // Initialize
    void Init(BulletType type, TechnoClass* source,
              int32_t target, int damage, WarheadType warhead);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------

    const BulletTypeData* TypeClass() const;
    const char* Name() const override;

    // Is this an instant-hit bullet (invisible)?
    bool IsInstantHit() const;

    // Is this a homing missile?
    bool IsHoming() const;

    // Can this hit aircraft?
    bool IsAntiAircraft() const;

    // Can this hit ground units?
    bool IsAntiGround() const;

    //-----------------------------------------------------------------------
    // Position and movement
    //-----------------------------------------------------------------------

    // Launch the bullet from a coordinate
    bool Launch(int32_t sourceCoord);

    // Update bullet position
    void UpdateFlight();

    // Calculate position for arcing trajectory
    int32_t ArcingPosition() const;

    // Check if bullet has reached target
    bool HasReachedTarget() const;

    // Get remaining distance to target
    int DistanceToTarget() const;

    //-----------------------------------------------------------------------
    // Detonation
    //-----------------------------------------------------------------------

    // Check if bullet should explode
    bool ShouldDetonate() const;

    // Explode and apply damage
    void Detonate();

    // Called when bullet hits something
    void Impact(int32_t coord);

    //-----------------------------------------------------------------------
    // AI processing
    //-----------------------------------------------------------------------
    virtual void AI() override;

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------
    virtual void DrawIt(int x, int y, int window) const override;
    int ShapeNumber() const;

    //-----------------------------------------------------------------------
    // Limbo/Unlimbo
    //-----------------------------------------------------------------------
    virtual bool Limbo() override;
    virtual bool Unlimbo(int32_t coord, DirType facing = DirType::N) override;

private:
    // Apply scatter to target coordinate
    int32_t ApplyScatter(int32_t target) const;

    // Update homing behavior
    void UpdateHoming();

    // Calculate arc height at current progress
    int16_t CalculateArcHeight() const;
};

//===========================================================================
// Bullet Pool - Global container for all bullets
//===========================================================================
extern ObjectPool<BulletClass, BULLET_MAX> Bullets;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Create and launch a new bullet
 */
BulletClass* CreateBullet(BulletType type, TechnoClass* source,
                          int32_t sourceCoord, int32_t targetCoord,
                          int damage, WarheadType warhead);

/**
 * Create instant-hit effect (no actual bullet travel)
 */
void InstantHit(TechnoClass* source, int32_t targetCoord,
                int damage, WarheadType warhead);

#endif // GAME_BULLET_H
