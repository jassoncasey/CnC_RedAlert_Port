/**
 * Red Alert macOS Port - Animation System
 *
 * Provides visual effects like explosions, fire, smoke, and special effects.
 * Based on original ANIM.CPP (~3000 lines)
 *
 * Architecture:
 *   - AnimTypeClass: Static type data (frame count, timing, effects)
 *   - AnimClass: Instance of an animation in the world
 *   - Object pool for efficient memory management
 *   - Stage-based frame advancement with configurable timing
 */

#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include "types.h"
#include <cstdint>

//===========================================================================
// Constants
//===========================================================================

// Maximum simultaneous animations
constexpr int ANIM_MAX = 256;

// Animation layer types
enum class AnimLayerType : int {
    GROUND = 0,     // Render with ground/terrain
    SURFACE,        // Render with buildings/units
    AIR,            // Render above everything
    COUNT
};

// Forward declarations for Anims namespace functions (friends)
namespace Anims {
    void Update_All();
    void Render_Layer(AnimLayerType layer, int screenOffsetX, int screenOffsetY);
    void Detach_All(void* target);
}

// Sound types for animations (simplified from original VocType)
enum class AnimSoundType : int {
    SOUND_NONE = -1,
    SOUND_KABOOM1 = 0,      // Big explosion
    SOUND_KABOOM22,         // Medium explosion
    SOUND_KABOOM25,         // Small explosion
    SOUND_FIRE_EXPLODE,     // Napalm
    SOUND_MINEBLOW,         // Mine explosion
    SOUND_SPLASH,           // Water splash
    SOUND_COUNT
};

//===========================================================================
// Animation Types Enum
//===========================================================================

enum class AnimType : int {
    NONE = -1,
    FIRST = 0,

    // Explosions
    FBALL1 = 0,         // Large fireball explosion
    FBALL_FADE,         // Fading fireball puff
    FRAG1,              // Medium fragment explosion
    VEH_HIT1,           // Small fireball (vehicle hit)
    VEH_HIT2,           // Small fragment explosion
    VEH_HIT3,           // Small burn/explosion
    ART_EXP1,           // Large fragment explosion

    // Napalm/Fire
    NAPALM1,            // Small napalm burn
    NAPALM2,            // Medium napalm burn
    NAPALM3,            // Large napalm burn
    FIRE_SMALL,         // Small flame
    FIRE_MED,           // Medium flame
    FIRE_MED2,          // Medium flame (oranger)
    FIRE_TINY,          // Very tiny flames
    BURN_SMALL,         // Small combustible fire
    BURN_MED,           // Medium combustible fire
    BURN_BIG,           // Large combustible fire
    ON_FIRE_SMALL,      // Building burning (small)
    ON_FIRE_MED,        // Building burning (medium)
    ON_FIRE_BIG,        // Building burning (large)

    // Smoke/Vapor
    SMOKE_PUFF,         // Small rocket smoke trail
    SMOKE_M,            // Smoke rising from ground
    LZ_SMOKE,           // Landing zone smoke marker

    // Weapon effects
    PIFF,               // Machine gun impact
    PIFFPIFF,           // Chaingun impact
    MUZZLE_FLASH,       // Big cannon flash

    // SAM site directional (8 directions)
    SAM_N,
    SAM_NE,
    SAM_E,
    SAM_SE,
    SAM_S,
    SAM_SW,
    SAM_W,
    SAM_NW,

    // Gun turret directional (8 directions)
    GUN_N,
    GUN_NE,
    GUN_E,
    GUN_SE,
    GUN_S,
    GUN_SW,
    GUN_W,
    GUN_NW,

    // Crate effects
    CRATE_DEVIATOR,     // Red finned missile
    CRATE_DOLLAR,       // Dollar sign
    CRATE_EARTH,        // Cracked Earth
    CRATE_EMPULSE,      // Plasma ball
    CRATE_INVUN,        // Orange sphere
    CRATE_MINE,         // Spiked mine
    CRATE_RAPID,        // Red skull
    CRATE_STEALTH,      // Cloaking sphere
    CRATE_MISSILE,      // Green finned missile
    CRATE_ARMOR,        // Armor upgrade
    CRATE_SPEED,        // Speed upgrade
    CRATE_FPOWER,       // Firepower upgrade
    CRATE_TQUAKE,       // Terrain quake

    // Water effects
    WATER_EXP1,         // Water explosion 1
    WATER_EXP2,         // Water explosion 2
    WATER_EXP3,         // Water explosion 3

    // Infantry/Units
    ELECT_DIE,          // Electrocution death
    DOG_ELECT_DIE,      // Dog electrocution
    CORPSE1,            // Corpse 1
    CORPSE2,            // Corpse 2
    CORPSE3,            // Corpse 3
    PARACHUTE,          // Parachute (attaches to units)
    PARA_BOMB,          // Paradrop bomb

    // Miscellaneous
    MOVE_FLASH,         // Movement effect
    SPUTDOOR,           // Sputnik door
    ATOM_BLAST,         // Atom bomb blast
    OILFIELD_BURN,      // Oil field burning
    CHRONO_BOX,         // Chronosphere effect
    GPS_BOX,            // GPS satellite effect
    INVUL_BOX,          // Invulnerability crate
    PARA_BOX,           // Parachute crate
    SONAR_BOX,          // Sonar effect
    TWINKLE1,           // Sparkle 1
    TWINKLE2,           // Sparkle 2
    TWINKLE3,           // Sparkle 3
    FLAK,               // Flak explosion
    MINE_EXP1,          // Mine explosion

    COUNT
};

// Total animation types
constexpr int ANIM_TYPE_COUNT = static_cast<int>(AnimType::COUNT);

//===========================================================================
// Animation Type Class - Static type data
//===========================================================================

class AnimTypeClass {
public:
    AnimType type_;             // Type identifier
    const char* name_;          // Internal name (e.g., "FBALL1")
    const char* graphicName_;   // Graphic file name

    // Frame information
    int frameCount_;            // Total frames in animation
    int startFrame_;            // Starting frame number
    int loopStart_;             // Frame to loop back to (-1 = no loop)
    int loopEnd_;               // Frame to end loop at (-1 = last frame)
    int biggestFrame_;          // Frame where effect is largest (for effects)

    // Timing
    int frameDelay_;            // Game ticks between frame advances
    int startDelay_;            // Delay before animation starts
    bool isNormalized_;         // Constant rate regardless of game speed

    // Looping
    int defaultLoops_;          // Default loop count (0 = infinite, 1 = once)

    // Effects
    int16_t damage_;            // Damage per tick (fixed 8.8, /256 per tick)
    AnimSoundType sound_;       // Sound to play at start
    AnimType chainTo_;          // Animation to chain into
    bool isScorcher_;           // Leaves scorch marks
    bool isCraterForming_;      // Forms craters
    bool isSticky_;             // Attaches to units/buildings

    // Rendering
    AnimLayerType layer_;       // Rendering layer
    bool isTranslucent_;        // Use alpha blending
    int size_;                  // Max dimension for cell refresh (pixels)

    // Constructor
    AnimTypeClass(
        AnimType type,
        const char* name,
        const char* graphicName,
        int frameCount,
        int frameDelay,
        int defaultLoops = 1,
        AnimLayerType layer = AnimLayerType::AIR
    );

    // Get type data by enum
    static const AnimTypeClass* Find(AnimType type);

    // Initialize all type data
    static void Init();
};

//===========================================================================
// Animation Class - Instance of an animation
//===========================================================================

class AnimClass {
public:
    // Construction/Destruction
    AnimClass();
    AnimClass(AnimType type, int x, int y, int delay = 0, int loops = 1);
    ~AnimClass();

    // Type identification
    RTTIType What_Am_I() const { return RTTIType::ANIMATION; }
    bool Is_Active() const { return isActive_; }

    // Animation control
    void Start();
    void Stop();
    void Pause(bool pause);
    bool IsPaused() const { return isPaused_; }

    // Update and render
    void AI();
    void Render(int screenX, int screenY);

    // Attachment (for sticky animations) - stores target ID
    void Attach_To(void* target, int targetX, int targetY);
    void Detach();
    void* Get_Attached() const { return attachedTo_; }
    void Update_Attached_Position(int targetX, int targetY);

    // State queries
    AnimType Get_Type() const { return type_; }
    const AnimTypeClass* Get_Type_Class() const { return typeClass_; }
    int Get_Frame() const { return currentFrame_; }
    int Get_Frame_Count() const;
    bool Is_Looping() const { return loopsRemaining_ != 1; }
    AnimLayerType Get_Layer() const;

    // Position
    int Get_X() const { return x_; }
    int Get_Y() const { return y_; }
    void Set_Position(int x, int y) { x_ = x; y_ = y; }

    // Frame control
    void Set_Frame(int frame);
    void Set_Rate(int ticksPerFrame);

    // Owner (for damage attribution)
    void Set_Owner(HousesType house) { ownerHouse_ = house; }
    HousesType Get_Owner() const { return ownerHouse_; }

    // Pool management
    static AnimClass* Allocate();
    static void Free(AnimClass* anim);
    static void Free_All();

    // Get active animations
    static int Get_Count();
    static AnimClass* Get_First();
    static AnimClass* Get_Next(AnimClass* current);

    // Allow Anims namespace to access pool_
    friend void Anims::Update_All();
    friend void Anims::Render_Layer(AnimLayerType, int, int);
    friend void Anims::Detach_All(void*);

private:
    AnimType type_;
    const AnimTypeClass* typeClass_;

    // Position (world coordinates)
    int x_;
    int y_;

    // Frame state
    int currentFrame_;
    int frameTimer_;        // Ticks until next frame
    int frameRate_;         // Ticks per frame (copied from type, can be overridden)

    // Loop state
    int loopsRemaining_;    // Loops left (0 = infinite)

    // Delay state
    int startDelay_;        // Ticks until animation starts
    bool hasStarted_;

    // Control flags
    bool isActive_;
    bool isPaused_;
    bool isVisible_;        // For sync purposes

    // Attachment (void* to avoid ObjectClass dependency)
    void* attachedTo_;
    int attachOffsetX_;
    int attachOffsetY_;
    bool attachedAlive_;  // Set by external code via Update_Attached_Position

    // Damage
    HousesType ownerHouse_;
    int16_t damageAccum_;   // Fractional damage accumulator (8.8 fixed)

    // Middle-frame effect handling
    bool middleCalled_;

    // Called at "biggest" frame for effects
    void Middle();

    // Apply accumulated damage
    void Apply_Damage();

    // Advance to next frame
    bool Advance_Frame();

    // Pool linkage
    AnimClass* poolNext_;
    static AnimClass* poolHead_;
    static AnimClass pool_[ANIM_MAX];
    static bool poolInitialized_;
};

//===========================================================================
// Animation Manager - Global animation control
//===========================================================================

namespace Anims {
    // Initialize animation system
    void Init();

    // Shutdown animation system
    void Shutdown();

    // Create a new animation
    AnimClass* Create(AnimType type, int x, int y, int delay = 0, int loops = 1);

    // Create attached animation (pass target position, not object)
    AnimClass* Create_Attached(AnimType type, void* target, int targetX, int targetY, int delay = 0);

    // Update all animations
    void Update_All();

    // Render all animations in a specific layer
    void Render_Layer(AnimLayerType layer, int screenOffsetX, int screenOffsetY);

    // Remove all animations
    void Clear_All();

    // Remove animations attached to a target
    void Detach_All(void* target);

    // Get animation count
    int Count();

    // Helper: Get explosion animation for a weapon
    AnimType Get_Explosion_Anim(int warheadType);

    // Helper: Get fire animation for damage level
    AnimType Get_Fire_Anim(int damagePercent);
}

//===========================================================================
// Animation Type Data (defined in anim.cpp)
//===========================================================================

extern const AnimTypeClass AnimTypes[];

#endif // GAME_ANIM_H
