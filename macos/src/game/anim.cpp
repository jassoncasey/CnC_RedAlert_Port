/**
 * Red Alert macOS Port - Animation System Implementation
 *
 * Based on original ANIM.CPP and ADATA.CPP
 */

#include "anim.h"
#include <cstring>
#include <cstdio>
#include <new>

//===========================================================================
// Animation Type Data
//===========================================================================

// Damage values in 8.8 fixed point (divide by 256 per tick)
// Original: fixed(1,32) = 8, fixed(1,16) = 16, fixed(1,8) = 32
constexpr int16_t DMG_NONE = 0;
constexpr int16_t DMG_TINY = 8;      // 1/32 per tick
constexpr int16_t DMG_SMALL = 16;    // 1/16 per tick
constexpr int16_t DMG_MEDIUM = 32;   // 1/8 per tick

// Animation type data table - macros for brevity
#define AT AnimType
#define LA AnimLayerType::AIR
#define LG AnimLayerType::GROUND
#define LS AnimLayerType::SURFACE

static AnimTypeClass s_animTypes[] = {
    // Explosions - type, name, graphic, frames, delay, loops, layer
    { AT::FBALL1,       "FBALL1",   "FBALL1",   22, 1, 1, LA },
    { AT::FBALL_FADE,   "FB2",      "FB2",       6, 2, 1, LA },
    { AT::FRAG1,        "FRAG1",    "FRAG1",    14, 1, 1, LA },
    { AT::VEH_HIT1,     "VEH-HIT1", "VEH-HIT1",  8, 2, 1, LA },
    { AT::VEH_HIT2,     "VEH-HIT2", "VEH-HIT2", 11, 1, 1, LA },
    { AT::VEH_HIT3,     "VEH-HIT3", "VEH-HIT3",  6, 2, 1, LA },
    { AT::ART_EXP1,     "ART-EXP1", "ART-EXP1", 19, 1, 1, LA },

    // Napalm/Fire
    { AT::NAPALM1,      "NAPALM1",    "NAPALM1",    14, 2, 1, LA },
    { AT::NAPALM2,      "NAPALM2",    "NAPALM2",    14, 2, 1, LA },
    { AT::NAPALM3,      "NAPALM3",    "NAPALM3",    14, 2, 1, LA },
    { AT::FIRE_SMALL,   "FIRE1",      "FIRE1",      15, 2, 2, LA },
    { AT::FIRE_MED,     "FIRE2",      "FIRE2",      15, 2, 3, LA },
    { AT::FIRE_MED2,    "FIRE3",      "FIRE3",      15, 2, 3, LA },
    { AT::FIRE_TINY,    "FIRE4",      "FIRE4",      7,  3, 2, LA },
    { AT::BURN_SMALL,   "BURN-S",     "BURN-S",     30, 2, 1, LS },
    { AT::BURN_MED,     "BURN-M",     "BURN-M",     30, 2, 1, LS },
    { AT::BURN_BIG,     "BURN-L",     "BURN-L",     62, 2, 1, LS },
    { AT::ON_FIRE_SMALL,"SMOKEY",     "SMOKEY",     8,  3, 0, LA },
    { AT::ON_FIRE_MED,  "BURNS",      "BURNS",      8,  3, 0, LA },
    { AT::ON_FIRE_BIG,  "BURNL",      "BURNL",      8,  3, 0, LA },

    // Smoke/Vapor
    { AT::SMOKE_PUFF,   "SMOKEY",     "SMOKEY",     8,  2, 1, LA },
    { AT::SMOKE_M,      "SMOKE_M",    "SMOKE_M",    91, 2, 1, LA },
    { AT::LZ_SMOKE,     "LZSMOKE",    "LZSMOKE",    91, 3, 1, LG },

    // Weapon effects
    { AT::PIFF,         "PIFF",       "PIFF",       5,  1, 1, LA },
    { AT::PIFFPIFF,     "PIFFPIFF",   "PIFFPIFF",   8,  1, 1, LA },
    { AT::MUZZLE_FLASH, "GUNFIRE",    "GUNFIRE",    10, 1, 1, LA },

    // SAM site animations (8 directions)
    { AT::SAM_N,        "SAM-N",      "SAM",        4,  2, 1, LA },
    { AT::SAM_NE,       "SAM-NE",     "SAM",        4,  2, 1, LA },
    { AT::SAM_E,        "SAM-E",      "SAM",        4,  2, 1, LA },
    { AT::SAM_SE,       "SAM-SE",     "SAM",        4,  2, 1, LA },
    { AT::SAM_S,        "SAM-S",      "SAM",        4,  2, 1, LA },
    { AT::SAM_SW,       "SAM-SW",     "SAM",        4,  2, 1, LA },
    { AT::SAM_W,        "SAM-W",      "SAM",        4,  2, 1, LA },
    { AT::SAM_NW,       "SAM-NW",     "SAM",        4,  2, 1, LA },

    // Gun turret animations (8 directions)
    { AT::GUN_N,        "GUN-N",      "GUN",        6,  2, 1, LA },
    { AT::GUN_NE,       "GUN-NE",     "GUN",        6,  2, 1, LA },
    { AT::GUN_E,        "GUN-E",      "GUN",        6,  2, 1, LA },
    { AT::GUN_SE,       "GUN-SE",     "GUN",        6,  2, 1, LA },
    { AT::GUN_S,        "GUN-S",      "GUN",        6,  2, 1, LA },
    { AT::GUN_SW,       "GUN-SW",     "GUN",        6,  2, 1, LA },
    { AT::GUN_W,        "GUN-W",      "GUN",        6,  2, 1, LA },
    { AT::GUN_NW,       "GUN-NW",     "GUN",        6,  2, 1, LA },

    // Crate effects
    { AT::CRATE_DEVIATOR,"DEVIATOR",  "DEVIATOR",   8,  3, 1, LA },
    { AT::CRATE_DOLLAR, "DOLLAR",     "DOLLAR",     8,  3, 1, LA },
    { AT::CRATE_EARTH,  "EARTH",      "EARTH",      8,  3, 1, LA },
    { AT::CRATE_EMPULSE,"EMPULSE",    "EMPULSE",    12, 2, 1, LA },
    { AT::CRATE_INVUN,  "INVUN",      "INVUN",      8,  3, 1, LA },
    { AT::CRATE_MINE,   "MINE",       "MINE",       8,  3, 1, LA },
    { AT::CRATE_RAPID,  "RAPID",      "RAPID",      8,  3, 1, LA },
    { AT::CRATE_STEALTH,"STEALTH2",   "STEALTH2",   8,  3, 1, LA },
    { AT::CRATE_MISSILE,"MISSILE2",   "MISSILE2",   8,  3, 1, LA },
    { AT::CRATE_ARMOR,  "ARMOR",      "ARMOR",      8,  3, 1, LA },
    { AT::CRATE_SPEED,  "SPEED",      "SPEED",      8,  3, 1, LA },
    { AT::CRATE_FPOWER, "FPOWER",     "FPOWER",     8,  3, 1, LA },
    { AT::CRATE_TQUAKE, "TQUAKE",     "TQUAKE",     8,  3, 1, LA },

    // Water effects
    { AT::WATER_EXP1,   "H2O_EXP1",   "H2O_EXP1",   10, 2, 1, LA },
    { AT::WATER_EXP2,   "H2O_EXP2",   "H2O_EXP2",   10, 2, 1, LA },
    { AT::WATER_EXP3,   "H2O_EXP3",   "H2O_EXP3",   10, 2, 1, LA },

    // Infantry/Units
    { AT::ELECT_DIE,    "ELECTRO",    "ELECTRO",    10, 1, 1, LA },
    { AT::DOG_ELECT_DIE,"ELECTDOG",   "ELECTDOG",   10, 1, 1, LA },
    { AT::CORPSE1,      "CORPSE1",    "CORPSE1",    1,  1, 1, LG },
    { AT::CORPSE2,      "CORPSE2",    "CORPSE2",    1,  1, 1, LG },
    { AT::CORPSE3,      "CORPSE3",    "CORPSE3",    1,  1, 1, LG },
    { AT::PARACHUTE,    "PARACH",     "PARACH",     7,  4, 0, LA },
    { AT::PARA_BOMB,    "PARABOMB",   "PARABOMB",   8,  4, 0, LA },

    // Miscellaneous
    { AT::MOVE_FLASH,   "MOVEFLSH",   "MOVEFLSH",   3,  1, 1, LA },
    { AT::SPUTDOOR,     "SPUTDOOR",   "SPUTDOOR",   6,  2, 1, LS },
    { AT::ATOM_BLAST,   "ATOMSFX",    "ATOMSFX",    19, 1, 1, LA },
    { AT::OILFIELD_BURN,"OILFIRE",    "OILFIRE",    15, 2, 0, LA },
    { AT::CHRONO_BOX,   "CHRONBOX",   "CHRONBOX",   16, 2, 1, LA },
    { AT::GPS_BOX,      "GPSBOX",     "GPSBOX",     16, 2, 1, LA },
    { AT::INVUL_BOX,    "INVULBOX",   "INVULBOX",   16, 2, 1, LA },
    { AT::PARA_BOX,     "PARABOX",    "PARABOX",    16, 2, 1, LA },
    { AT::SONAR_BOX,    "SONARBOX",   "SONARBOX",   16, 2, 1, LA },
    { AT::TWINKLE1,     "TWINKLE1",   "TWINKLE1",   4,  2, 1, LA },
    { AT::TWINKLE2,     "TWINKLE2",   "TWINKLE2",   4,  2, 1, LA },
    { AT::TWINKLE3,     "TWINKLE3",   "TWINKLE3",   4,  2, 1, LA },
    { AT::FLAK,         "FLAK",       "FLAK",       7,  1, 1, LA },
    { AT::MINE_EXP1,    "MINEXP1",    "MINEXP1",    12, 1, 1, LA },
};

// Ensure we have all types defined
static_assert(
    sizeof(s_animTypes) / sizeof(s_animTypes[0]) == ANIM_TYPE_COUNT,
    "Animation type count mismatch"
);

//===========================================================================
// AnimTypeClass Implementation
//===========================================================================

AnimTypeClass::AnimTypeClass(
    AnimType type,
    const char* name,
    const char* graphicName,
    int frameCount,
    int frameDelay,
    int defaultLoops,
    AnimLayerType layer
) :
    type_(type),
    name_(name),
    graphicName_(graphicName),
    frameCount_(frameCount),
    startFrame_(0),
    loopStart_(-1),
    loopEnd_(-1),
    biggestFrame_(frameCount / 2),
    frameDelay_(frameDelay),
    startDelay_(0),
    isNormalized_(false),
    defaultLoops_(defaultLoops),
    damage_(DMG_NONE),
    sound_(AnimSoundType::SOUND_NONE),
    chainTo_(AT::NONE),
    isScorcher_(false),
    isCraterForming_(false),
    isSticky_(false),
    layer_(layer),
    isTranslucent_(false),
    size_(24)
{
}

const AnimTypeClass* AnimTypeClass::Find(AnimType type) {
    int index = static_cast<int>(type);
    if (index >= 0 && index < ANIM_TYPE_COUNT) {
        return &s_animTypes[index];
    }
    return nullptr;
}

void AnimTypeClass::Init() {
    // Set up additional properties for specific animation types

    // Helper macro for sound assignments
    #define ATSND(t) s_animTypes[static_cast<int>(AT::t)]

    // Explosions with sounds
    ATSND(FBALL1).sound_ = AnimSoundType::SOUND_KABOOM25;
    ATSND(FBALL1).isCraterForming_ = true;
    ATSND(FBALL1).isScorcher_ = true;

    ATSND(FRAG1).sound_ = AnimSoundType::SOUND_KABOOM22;

    ATSND(ART_EXP1).sound_ = AnimSoundType::SOUND_KABOOM1;
    ATSND(ART_EXP1).isCraterForming_ = true;

    // Napalm with sounds and fire effects
    ATSND(NAPALM1).sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    ATSND(NAPALM1).isScorcher_ = true;
    ATSND(NAPALM2).sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    ATSND(NAPALM2).isScorcher_ = true;
    ATSND(NAPALM3).sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    ATSND(NAPALM3).isScorcher_ = true;

    // Fire animations with damage
    ATSND(FIRE_SMALL).damage_ = DMG_TINY;
    ATSND(FIRE_MED).damage_ = DMG_SMALL;
    ATSND(FIRE_MED2).damage_ = DMG_SMALL;

    // Building fire with damage (loops infinitely)
    ATSND(ON_FIRE_SMALL).damage_ = DMG_TINY;
    ATSND(ON_FIRE_MED).damage_ = DMG_SMALL;
    ATSND(ON_FIRE_BIG).damage_ = DMG_MEDIUM;

    // Burn animations with damage
    ATSND(BURN_SMALL).damage_ = DMG_TINY;
    ATSND(BURN_MED).damage_ = DMG_SMALL;
    ATSND(BURN_BIG).damage_ = DMG_SMALL;

    // Sticky animations (attach to units)
    ATSND(PARACHUTE).isSticky_ = true;
    ATSND(ON_FIRE_SMALL).isSticky_ = true;
    ATSND(ON_FIRE_MED).isSticky_ = true;
    ATSND(ON_FIRE_BIG).isSticky_ = true;

    // Atom blast special
    ATSND(ATOM_BLAST).isCraterForming_ = true;
    ATSND(ATOM_BLAST).isScorcher_ = true;
    ATSND(ATOM_BLAST).size_ = 128;

    // Mine explosion
    ATSND(MINE_EXP1).sound_ = AnimSoundType::SOUND_MINEBLOW;
    ATSND(MINE_EXP1).isCraterForming_ = true;

    // Translucent effects
    ATSND(MUZZLE_FLASH).isTranslucent_ = true;
    ATSND(SMOKE_PUFF).isTranslucent_ = true;
    ATSND(SMOKE_M).isTranslucent_ = true;

    // Electrocution chains to fire
    ATSND(ELECT_DIE).chainTo_ = AT::FIRE_MED;
    ATSND(DOG_ELECT_DIE).chainTo_ = AT::FIRE_SMALL;

    // Water explosions
    ATSND(WATER_EXP1).sound_ = AnimSoundType::SOUND_SPLASH;
    ATSND(WATER_EXP2).sound_ = AnimSoundType::SOUND_SPLASH;
    ATSND(WATER_EXP3).sound_ = AnimSoundType::SOUND_SPLASH;

    // Corpses are ground layer and static
    ATSND(CORPSE1).layer_ = LG;
    ATSND(CORPSE2).layer_ = LG;
    ATSND(CORPSE3).layer_ = LG;
}

//===========================================================================
// AnimClass Static Members
//===========================================================================

AnimClass AnimClass::pool_[ANIM_MAX];
AnimClass* AnimClass::poolHead_ = nullptr;
bool AnimClass::poolInitialized_ = false;

//===========================================================================
// AnimClass Implementation
//===========================================================================

AnimClass::AnimClass() :
    type_(AT::NONE),
    typeClass_(nullptr),
    x_(0),
    y_(0),
    currentFrame_(0),
    frameTimer_(0),
    frameRate_(1),
    loopsRemaining_(1),
    startDelay_(0),
    hasStarted_(false),
    isActive_(false),
    isPaused_(false),
    isVisible_(true),
    attachedTo_(nullptr),
    attachOffsetX_(0),
    attachOffsetY_(0),
    attachedAlive_(false),
    ownerHouse_(HousesType::NONE),
    damageAccum_(0),
    middleCalled_(false),
    poolNext_(nullptr)
{
}

AnimClass::AnimClass(AnimType type, int x, int y, int delay, int loops) :
    type_(type),
    typeClass_(AnimTypeClass::Find(type)),
    x_(x),
    y_(y),
    currentFrame_(0),
    frameTimer_(0),
    frameRate_(1),
    loopsRemaining_(loops),
    startDelay_(delay),
    hasStarted_(false),
    isActive_(true),
    isPaused_(false),
    isVisible_(true),
    attachedTo_(nullptr),
    attachOffsetX_(0),
    attachOffsetY_(0),
    attachedAlive_(false),
    ownerHouse_(HousesType::NONE),
    damageAccum_(0),
    middleCalled_(false),
    poolNext_(nullptr)
{
    if (typeClass_) {
        frameRate_ = typeClass_->frameDelay_;
        frameTimer_ = frameRate_;

        // Use type's default loops if not specified
        if (loops <= 0) {
            loopsRemaining_ = typeClass_->defaultLoops_;
        }

        // Add type's start delay
        startDelay_ += typeClass_->startDelay_;

        // Start immediately if no delay
        if (startDelay_ == 0) {
            Start();
        }
    }
}

AnimClass::~AnimClass() {
    Detach();
}

void AnimClass::Start() {
    if (hasStarted_ || !typeClass_) return;

    hasStarted_ = true;
    currentFrame_ = typeClass_->startFrame_;
    frameTimer_ = frameRate_;
    middleCalled_ = false;

    // Play start sound
    if (typeClass_->sound_ != AnimSoundType::SOUND_NONE) {
        // Sound_Effect(typeClass_->sound_, x_, y_);
        // TODO: Integrate with audio system
    }

    // Call Middle() if no "biggest" frame defined
    if (typeClass_->biggestFrame_ <= 0) {
        Middle();
    }
}

void AnimClass::Stop() {
    isActive_ = false;
    Detach();
}

void AnimClass::Pause(bool pause) {
    isPaused_ = pause;
}

void AnimClass::AI() {
    if (!isActive_ || isPaused_ || !typeClass_) return;

    // Handle start delay
    if (!hasStarted_) {
        if (startDelay_ > 0) {
            startDelay_--;
            return;
        }
        Start();
    }

    // Update attached position
    // Note: External code must call Update_Attached_Position() each frame
    // If attached object died, attachedAlive_ will be false
    if (attachedTo_ && !attachedAlive_) {
        Detach();
    }

    // Apply damage if applicable
    if (typeClass_->damage_ != 0 && attachedTo_) {
        Apply_Damage();
    }

    // Check for "biggest" frame for effects
    if (!middleCalled_ && currentFrame_ >= typeClass_->biggestFrame_) {
        Middle();
    }

    // Advance frame
    if (!Advance_Frame()) {
        // Animation ended
        if (typeClass_->chainTo_ != AT::NONE) {
            // Chain to next animation
            type_ = typeClass_->chainTo_;
            typeClass_ = AnimTypeClass::Find(type_);
            if (typeClass_) {
                currentFrame_ = typeClass_->startFrame_;
                frameRate_ = typeClass_->frameDelay_;
                frameTimer_ = frameRate_;
                middleCalled_ = false;
                // Keep loops and damage state
            } else {
                Stop();
            }
        } else {
            Stop();
        }
    }
}

void AnimClass::Render(int screenX, int screenY) {
    if (!isActive_ || !isVisible_ || !typeClass_) return;

    // Calculate screen position
    int drawX = x_ - screenX;
    int drawY = y_ - screenY;

    // For now, render a placeholder colored rectangle
    // In the future, this will use SHP graphics
    int size = typeClass_->size_;
    int halfSize = size / 2;

    // Choose color based on animation type category
    uint8_t r = 255, g = 128, b = 0;  // Default orange (explosion)

    if (type_ >= AT::FIRE_SMALL && type_ <= AT::ON_FIRE_BIG) {
        r = 255; g = 64; b = 0;  // Red-orange (fire)
    } else if (type_ >= AT::SMOKE_PUFF && type_ <= AT::LZ_SMOKE) {
        r = 128; g = 128; b = 128;  // Gray (smoke)
    } else if (type_ >= AT::WATER_EXP1 && type_ <= AT::WATER_EXP3) {
        r = 64; g = 128; b = 255;  // Blue (water)
    } else if (type_ >= AT::CRATE_DEVIATOR && type_ <= AT::CRATE_TQUAKE) {
        r = 255; g = 255; b = 0;  // Yellow (crate effects)
    } else if (type_ == AT::ELECT_DIE || type_ == AT::DOG_ELECT_DIE) {
        r = 128; g = 128; b = 255;  // Light blue (electric)
    }

    // Animate alpha based on frame position
    float progress = (float)currentFrame_ / (float)typeClass_->frameCount_;
    uint8_t alpha = 255;
    if (progress > 0.5f) {
        alpha = (uint8_t)(255 * (1.0f - (progress - 0.5f) * 2.0f));
    }

    // Scale size based on frame (grow then shrink for explosions)
    float scale = 1.0f;
    if (progress < 0.3f) {
        scale = progress / 0.3f;  // Grow
    } else if (progress > 0.7f) {
        scale = 1.0f - (progress - 0.7f) / 0.3f;  // Shrink
    }

    int drawSize = (int)(size * scale);
    int drawHalf = drawSize / 2;

    // TODO: Actual rendering through Metal/graphics system
    // Graphics_Draw_Rect(x-h, y-h, size, size, r, g, b, alpha);

    (void)drawX;
    (void)drawY;
    (void)halfSize;
    (void)drawHalf;
    (void)r;
    (void)g;
    (void)b;
    (void)alpha;
}

void AnimClass::Attach_To(void* target, int targetX, int targetY) {
    if (!target) return;

    attachedTo_ = target;
    attachedAlive_ = true;

    // Calculate offset from object center
    attachOffsetX_ = x_ - targetX;
    attachOffsetY_ = y_ - targetY;
}

void AnimClass::Update_Attached_Position(int targetX, int targetY) {
    if (!attachedTo_) return;

    attachedAlive_ = true;  // Mark as still alive
    x_ = targetX + attachOffsetX_;
    y_ = targetY + attachOffsetY_;
}

void AnimClass::Detach() {
    attachedTo_ = nullptr;
    attachOffsetX_ = 0;
    attachOffsetY_ = 0;
}

int AnimClass::Get_Frame_Count() const {
    return typeClass_ ? typeClass_->frameCount_ : 0;
}

AnimLayerType AnimClass::Get_Layer() const {
    return typeClass_ ? typeClass_->layer_ : LA;
}

void AnimClass::Set_Frame(int frame) {
    if (typeClass_ && frame >= 0 && frame < typeClass_->frameCount_) {
        currentFrame_ = frame;
    }
}

void AnimClass::Set_Rate(int ticksPerFrame) {
    if (ticksPerFrame > 0) {
        frameRate_ = ticksPerFrame;
    }
}

void AnimClass::Middle() {
    if (middleCalled_ || !typeClass_) return;
    middleCalled_ = true;

    // Create scorch marks
    if (typeClass_->isScorcher_) {
        // TODO: Map_->Add_Smudge(x_, y_, SMUDGE_SCORCH);
    }

    // Create craters
    if (typeClass_->isCraterForming_) {
        // TODO: Map_->Add_Smudge(x_, y_, SMUDGE_CRATER);
    }

    // Special handling for atom blast
    if (type_ == AT::ATOM_BLAST) {
        // TODO: Do_Atom_Damage(ownerHouse_, x_, y_);
        // Screen shake, palette flash, etc.
    }
}

void AnimClass::Apply_Damage() {
    if (!typeClass_ || typeClass_->damage_ == 0 || !attachedTo_) return;

    // Accumulate fractional damage (8.8 fixed point)
    damageAccum_ += typeClass_->damage_;

    // Apply whole damage points
    while (damageAccum_ >= 256) {
        damageAccum_ -= 256;
        // TODO: attachedTo_->Take_Damage(1, 0, WARHEAD_FIRE, 0, house);
    }
}

bool AnimClass::Advance_Frame() {
    if (!typeClass_) return false;

    // Countdown frame timer
    frameTimer_--;
    if (frameTimer_ > 0) {
        return true;  // Still on current frame
    }

    // Reset timer
    frameTimer_ = frameRate_;

    // Advance frame
    currentFrame_++;

    // Check for end of animation
    int loopEnd = typeClass_->loopEnd_;
    int endFrame = (loopEnd >= 0) ? loopEnd : typeClass_->frameCount_;
    int loopStart = typeClass_->loopStart_;
    int startFrame = (loopStart >= 0) ? loopStart : 0;

    if (currentFrame_ >= endFrame) {
        // Check for looping
        if (loopsRemaining_ == 0) {
            // Infinite loop
            currentFrame_ = startFrame;
            middleCalled_ = false;
            return true;
        } else if (loopsRemaining_ > 1) {
            // More loops remaining
            loopsRemaining_--;
            currentFrame_ = startFrame;
            middleCalled_ = false;
            return true;
        } else {
            // Animation complete
            return false;
        }
    }

    return true;
}

//===========================================================================
// AnimClass Pool Management
//===========================================================================

AnimClass* AnimClass::Allocate() {
    // Initialize pool on first use
    if (!poolInitialized_) {
        poolHead_ = nullptr;
        for (int i = ANIM_MAX - 1; i >= 0; i--) {
            pool_[i].isActive_ = false;
            pool_[i].poolNext_ = poolHead_;
            poolHead_ = &pool_[i];
        }
        poolInitialized_ = true;
    }

    // Get from free list
    if (poolHead_) {
        AnimClass* anim = poolHead_;
        poolHead_ = anim->poolNext_;
        anim->poolNext_ = nullptr;
        return anim;
    }

    return nullptr;  // Pool exhausted
}

void AnimClass::Free(AnimClass* anim) {
    if (!anim) return;

    anim->Stop();
    anim->type_ = AT::NONE;
    anim->typeClass_ = nullptr;
    anim->isActive_ = false;

    // Return to free list
    anim->poolNext_ = poolHead_;
    poolHead_ = anim;
}

void AnimClass::Free_All() {
    // Reset all animations
    poolHead_ = nullptr;
    for (int i = ANIM_MAX - 1; i >= 0; i--) {
        pool_[i].Stop();
        pool_[i].type_ = AT::NONE;
        pool_[i].typeClass_ = nullptr;
        pool_[i].isActive_ = false;
        pool_[i].poolNext_ = poolHead_;
        poolHead_ = &pool_[i];
    }
}

int AnimClass::Get_Count() {
    int count = 0;
    for (int i = 0; i < ANIM_MAX; i++) {
        if (pool_[i].isActive_) {
            count++;
        }
    }
    return count;
}

AnimClass* AnimClass::Get_First() {
    for (int i = 0; i < ANIM_MAX; i++) {
        if (pool_[i].isActive_) {
            return &pool_[i];
        }
    }
    return nullptr;
}

AnimClass* AnimClass::Get_Next(AnimClass* current) {
    if (!current) return nullptr;

    int startIndex = (int)(current - pool_) + 1;
    for (int i = startIndex; i < ANIM_MAX; i++) {
        if (pool_[i].isActive_) {
            return &pool_[i];
        }
    }
    return nullptr;
}

//===========================================================================
// Anims Namespace Implementation
//===========================================================================

namespace Anims {

static bool s_initialized = false;

void Init() {
    if (s_initialized) return;

    AnimTypeClass::Init();
    AnimClass::Free_All();
    s_initialized = true;
}

void Shutdown() {
    AnimClass::Free_All();
    s_initialized = false;
}

AnimClass* Create(AnimType type, int x, int y, int delay, int loops) {
    if (!s_initialized) Init();

    AnimClass* anim = AnimClass::Allocate();
    if (!anim) {
        // Pool exhausted, find oldest animation to recycle
        // For now, just fail
        return nullptr;
    }

    // Construct in-place
    new (anim) AnimClass(type, x, y, delay, loops);
    return anim;
}

AnimClass* Create_Attached(AnimType type, void* target, int targetX,
                           int targetY, int delay) {
    if (!target) return nullptr;

    AnimClass* anim = Create(type, targetX, targetY, delay, 1);
    if (anim) {
        anim->Attach_To(target, targetX, targetY);
    }
    return anim;
}

void Update_All() {
    for (int i = 0; i < ANIM_MAX; i++) {
        if (AnimClass::pool_[i].isActive_) {
            AnimClass::pool_[i].AI();
        }
    }
}

void Render_Layer(AnimLayerType layer, int screenOffsetX, int screenOffsetY) {
    for (int i = 0; i < ANIM_MAX; i++) {
        AnimClass& anim = AnimClass::pool_[i];
        if (anim.isActive_ && anim.Get_Layer() == layer) {
            anim.Render(screenOffsetX, screenOffsetY);
        }
    }
}

void Clear_All() {
    AnimClass::Free_All();
}

void Detach_All(void* target) {
    if (!target) return;

    for (int i = 0; i < ANIM_MAX; i++) {
        AnimClass& a = AnimClass::pool_[i];
        if (a.isActive_ && a.Get_Attached() == target) {
            a.Detach();
        }
    }
}

int Count() {
    return AnimClass::Get_Count();
}

AnimType Get_Explosion_Anim(int warheadType) {
    // Map warhead types to explosion animations
    switch (warheadType) {
        case 0:  // Small arms
            return AT::PIFF;
        case 1:  // High explosive
            return AT::FBALL1;
        case 2:  // Armor piercing
            return AT::VEH_HIT2;
        case 3:  // Fire
            return AT::NAPALM2;
        case 4:  // Special (nuke)
            return AT::ATOM_BLAST;
        default:
            return AT::FRAG1;
    }
}

AnimType Get_Fire_Anim(int damagePercent) {
    // Return appropriate fire animation based on damage level
    if (damagePercent >= 75) {
        return AT::ON_FIRE_BIG;
    } else if (damagePercent >= 50) {
        return AT::ON_FIRE_MED;
    } else if (damagePercent >= 25) {
        return AT::ON_FIRE_SMALL;
    }
    return AT::NONE;
}

} // namespace Anims
