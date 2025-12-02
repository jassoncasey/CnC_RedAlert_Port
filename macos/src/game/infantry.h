/**
 * Red Alert macOS Port - Infantry Class
 *
 * Infantry units (soldiers) that can move, fight, and occupy buildings.
 * Extends FootClass with infantry-specific behaviors.
 *
 * Based on original INFANTRY.H
 */

#ifndef GAME_INFANTRY_H
#define GAME_INFANTRY_H

#include "object.h"
#include "cell.h"
#include "infantry_types.h"

// Maximum infantry units in game
constexpr int INFANTRY_MAX = 500;

// Fear levels (determines behavior)
constexpr int FEAR_NONE = 0;
constexpr int FEAR_ANXIOUS = 10;      // Getting worried
constexpr int FEAR_NERVOUS = 100;     // Very nervous
constexpr int FEAR_SCARED = 200;      // Quite scared
constexpr int FEAR_PANIC = 255;       // Full panic

// Sub-cell position types for infantry - defined in cell.h
// Forward reference - SpotType is declared in cell.h

/**
 * InfantryClass - Individual infantry unit
 *
 * Infantry are foot soldiers that:
 * - Move between cells using sub-cell positions (5 positions per cell)
 * - Can go prone to reduce damage and improve accuracy
 * - Have a fear system that affects behavior
 * - Can enter buildings and transports
 * - Support animation states for walk, fire, idle, death, etc.
 */
class InfantryClass : public FootClass {
public:
    //-----------------------------------------------------------------------
    // Infantry-specific state
    //-----------------------------------------------------------------------

    // Type reference
    InfantryType type_;

    // Current action being performed
    DoType doing_;

    // Fear level (0-255, higher = more scared)
    uint8_t fear_;

    // Sub-cell position within current cell
    SpotType spot_;
    SpotType spotTarget_;   // Target spot we're moving to

    // Prone state
    bool isProne_;          // Currently prone
    bool isTechnician_;     // Technician ability (captures on touch)
    bool isStoked_;         // Adrenaline boost (temporary speed boost)
    bool isStopping_;       // In process of stopping

    // Animation frame tracking
    int16_t frame_;         // Current animation frame
    int16_t stageCount_;    // Frame counter for animation
    int16_t idleTimer_;     // Time until next idle animation

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    InfantryClass();
    InfantryClass(InfantryType type, HousesType house);
    virtual ~InfantryClass() = default;

    // Initialize from type
    void Init(InfantryType type, HousesType house);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------

    const InfantryTypeData* TypeClass() const;
    const char* Name() const override;

    // Is this infantry type a dog?
    bool IsDog() const;

    // Can this infantry capture buildings?
    bool CanCapture() const;

    // Is this a civilian?
    bool IsCivilian() const;

    //-----------------------------------------------------------------------
    // Position and movement
    //-----------------------------------------------------------------------

    // Get current spot index
    SpotType CurrentSpot() const { return spot_; }

    // Assign movement to specific spot in cell
    bool AssignSpot(CELL cell, SpotType spot);

    // Get coordinate for a sub-cell spot
    static int32_t SpotCoord(CELL cell, SpotType spot);

    // Override movement for infantry-specific handling
    virtual bool StartDrive(int32_t destination) override;
    virtual bool StopDrive() override;
    virtual MoveType CanEnterCell(int16_t cell,
                                     FacingType facing = FacingType::NORTH
                                     ) const override;
    virtual int TopSpeed() const override;

    //-----------------------------------------------------------------------
    // Combat
    //-----------------------------------------------------------------------

    // Fear management
    void Afraid() { fear_ = FEAR_PANIC; }
    void Calm() { fear_ = FEAR_NONE; }
    bool IsPanicked() const { return fear_ >= FEAR_PANIC; }
    bool IsScared() const { return fear_ >= FEAR_SCARED; }

    // Go prone or stand up
    void GoProne();
    void StandUp();
    bool IsProne() const { return isProne_; }

    // Take damage (override for prone bonus)
    virtual ResultType TakeDamage(int& damage, int distance,
                                   WarheadType warhead,
                                   TechnoClass* source = nullptr,
                                   bool forced = false) override;

    // Fire weapon
    virtual bool CanFire() const override;
    virtual int WeaponRange(int weapon = 0) const override;
    virtual int RearmTime(int weapon = 0) const override;

    //-----------------------------------------------------------------------
    // Animation
    //-----------------------------------------------------------------------

    // Set current action/animation
    void SetDoType(DoType doing);
    DoType GetDoType() const { return doing_; }

    // Get current animation frame for rendering
    int ShapeNumber() const override;

    // Animation control data for current type
    const DoInfoStruct* DoControls() const;

    //-----------------------------------------------------------------------
    // Mission handlers (override from MissionClass)
    //-----------------------------------------------------------------------
    virtual int MissionAttack() override;
    virtual int MissionGuard() override;
    virtual int MissionMove() override;
    virtual int MissionHunt() override;
    virtual int MissionCapture() override;
    virtual int MissionEnter() override;
    virtual int MissionRetreat() override;

    //-----------------------------------------------------------------------
    // AI processing
    //-----------------------------------------------------------------------
    virtual void AI() override;

    // Per-cell callback when moving between cells
    virtual void PerCellProcess(PCPType pcp) override;

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------
    virtual void DrawIt(int x, int y, int window) const override;

    //-----------------------------------------------------------------------
    // Limbo/Unlimbo
    //-----------------------------------------------------------------------
    virtual bool Limbo() override;
    virtual bool Unlimbo(int32_t coord, DirType facing = DirType::N) override;

private:
    // Fear decay per frame
    void DecayFear();

    // Start appropriate animation for current state
    void StartAnimation();

    // Process animation frames
    void AnimateFrame();

    // Check if should scatter from threat
    void CheckScatter();
};

//===========================================================================
// Infantry Pool - Global container for all infantry
//===========================================================================
extern ObjectPool<InfantryClass, INFANTRY_MAX> Infantry;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Create new infantry unit of given type
 */
InfantryClass* CreateInfantry(InfantryType type, HousesType house, CELL cell);

/**
 * Find closest free spot in cell for infantry
 */
SpotType FindFreeSpot(CELL cell);

#endif // GAME_INFANTRY_H
