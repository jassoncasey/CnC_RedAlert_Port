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

// Animation type data table
static AnimTypeClass s_animTypes[] = {
    // Explosions
    // AnimType, name, graphic, frames, delay, loops, layer
    { AnimType::FBALL1,       "FBALL1",     "FBALL1",     22, 1, 1, AnimLayerType::AIR },
    { AnimType::FBALL_FADE,   "FB2",        "FB2",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::FRAG1,        "FRAG1",      "FRAG1",      14, 1, 1, AnimLayerType::AIR },
    { AnimType::VEH_HIT1,     "VEH-HIT1",   "VEH-HIT1",   8,  2, 1, AnimLayerType::AIR },
    { AnimType::VEH_HIT2,     "VEH-HIT2",   "VEH-HIT2",   11, 1, 1, AnimLayerType::AIR },
    { AnimType::VEH_HIT3,     "VEH-HIT3",   "VEH-HIT3",   6,  2, 1, AnimLayerType::AIR },
    { AnimType::ART_EXP1,     "ART-EXP1",   "ART-EXP1",   19, 1, 1, AnimLayerType::AIR },

    // Napalm/Fire
    { AnimType::NAPALM1,      "NAPALM1",    "NAPALM1",    14, 2, 1, AnimLayerType::AIR },
    { AnimType::NAPALM2,      "NAPALM2",    "NAPALM2",    14, 2, 1, AnimLayerType::AIR },
    { AnimType::NAPALM3,      "NAPALM3",    "NAPALM3",    14, 2, 1, AnimLayerType::AIR },
    { AnimType::FIRE_SMALL,   "FIRE1",      "FIRE1",      15, 2, 2, AnimLayerType::AIR },
    { AnimType::FIRE_MED,     "FIRE2",      "FIRE2",      15, 2, 3, AnimLayerType::AIR },
    { AnimType::FIRE_MED2,    "FIRE3",      "FIRE3",      15, 2, 3, AnimLayerType::AIR },
    { AnimType::FIRE_TINY,    "FIRE4",      "FIRE4",      7,  3, 2, AnimLayerType::AIR },
    { AnimType::BURN_SMALL,   "BURN-S",     "BURN-S",     30, 2, 1, AnimLayerType::SURFACE },
    { AnimType::BURN_MED,     "BURN-M",     "BURN-M",     30, 2, 1, AnimLayerType::SURFACE },
    { AnimType::BURN_BIG,     "BURN-L",     "BURN-L",     62, 2, 1, AnimLayerType::SURFACE },
    { AnimType::ON_FIRE_SMALL,"SMOKEY",     "SMOKEY",     8,  3, 0, AnimLayerType::AIR },
    { AnimType::ON_FIRE_MED,  "BURNS",      "BURNS",      8,  3, 0, AnimLayerType::AIR },
    { AnimType::ON_FIRE_BIG,  "BURNL",      "BURNL",      8,  3, 0, AnimLayerType::AIR },

    // Smoke/Vapor
    { AnimType::SMOKE_PUFF,   "SMOKEY",     "SMOKEY",     8,  2, 1, AnimLayerType::AIR },
    { AnimType::SMOKE_M,      "SMOKE_M",    "SMOKE_M",    91, 2, 1, AnimLayerType::AIR },
    { AnimType::LZ_SMOKE,     "LZSMOKE",    "LZSMOKE",    91, 3, 1, AnimLayerType::GROUND },

    // Weapon effects
    { AnimType::PIFF,         "PIFF",       "PIFF",       5,  1, 1, AnimLayerType::AIR },
    { AnimType::PIFFPIFF,     "PIFFPIFF",   "PIFFPIFF",   8,  1, 1, AnimLayerType::AIR },
    { AnimType::MUZZLE_FLASH, "GUNFIRE",    "GUNFIRE",    10, 1, 1, AnimLayerType::AIR },

    // SAM site animations (8 directions)
    { AnimType::SAM_N,        "SAM-N",      "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_NE,       "SAM-NE",     "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_E,        "SAM-E",      "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_SE,       "SAM-SE",     "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_S,        "SAM-S",      "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_SW,       "SAM-SW",     "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_W,        "SAM-W",      "SAM",        4,  2, 1, AnimLayerType::AIR },
    { AnimType::SAM_NW,       "SAM-NW",     "SAM",        4,  2, 1, AnimLayerType::AIR },

    // Gun turret animations (8 directions)
    { AnimType::GUN_N,        "GUN-N",      "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_NE,       "GUN-NE",     "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_E,        "GUN-E",      "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_SE,       "GUN-SE",     "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_S,        "GUN-S",      "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_SW,       "GUN-SW",     "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_W,        "GUN-W",      "GUN",        6,  2, 1, AnimLayerType::AIR },
    { AnimType::GUN_NW,       "GUN-NW",     "GUN",        6,  2, 1, AnimLayerType::AIR },

    // Crate effects
    { AnimType::CRATE_DEVIATOR,"DEVIATOR",  "DEVIATOR",   8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_DOLLAR, "DOLLAR",     "DOLLAR",     8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_EARTH,  "EARTH",      "EARTH",      8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_EMPULSE,"EMPULSE",    "EMPULSE",    12, 2, 1, AnimLayerType::AIR },
    { AnimType::CRATE_INVUN,  "INVUN",      "INVUN",      8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_MINE,   "MINE",       "MINE",       8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_RAPID,  "RAPID",      "RAPID",      8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_STEALTH,"STEALTH2",   "STEALTH2",   8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_MISSILE,"MISSILE2",   "MISSILE2",   8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_ARMOR,  "ARMOR",      "ARMOR",      8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_SPEED,  "SPEED",      "SPEED",      8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_FPOWER, "FPOWER",     "FPOWER",     8,  3, 1, AnimLayerType::AIR },
    { AnimType::CRATE_TQUAKE, "TQUAKE",     "TQUAKE",     8,  3, 1, AnimLayerType::AIR },

    // Water effects
    { AnimType::WATER_EXP1,   "H2O_EXP1",   "H2O_EXP1",   10, 2, 1, AnimLayerType::AIR },
    { AnimType::WATER_EXP2,   "H2O_EXP2",   "H2O_EXP2",   10, 2, 1, AnimLayerType::AIR },
    { AnimType::WATER_EXP3,   "H2O_EXP3",   "H2O_EXP3",   10, 2, 1, AnimLayerType::AIR },

    // Infantry/Units
    { AnimType::ELECT_DIE,    "ELECTRO",    "ELECTRO",    10, 1, 1, AnimLayerType::AIR },
    { AnimType::DOG_ELECT_DIE,"ELECTDOG",   "ELECTDOG",   10, 1, 1, AnimLayerType::AIR },
    { AnimType::CORPSE1,      "CORPSE1",    "CORPSE1",    1,  1, 1, AnimLayerType::GROUND },
    { AnimType::CORPSE2,      "CORPSE2",    "CORPSE2",    1,  1, 1, AnimLayerType::GROUND },
    { AnimType::CORPSE3,      "CORPSE3",    "CORPSE3",    1,  1, 1, AnimLayerType::GROUND },
    { AnimType::PARACHUTE,    "PARACH",     "PARACH",     7,  4, 0, AnimLayerType::AIR },
    { AnimType::PARA_BOMB,    "PARABOMB",   "PARABOMB",   8,  4, 0, AnimLayerType::AIR },

    // Miscellaneous
    { AnimType::MOVE_FLASH,   "MOVEFLSH",   "MOVEFLSH",   3,  1, 1, AnimLayerType::AIR },
    { AnimType::SPUTDOOR,     "SPUTDOOR",   "SPUTDOOR",   6,  2, 1, AnimLayerType::SURFACE },
    { AnimType::ATOM_BLAST,   "ATOMSFX",    "ATOMSFX",    19, 1, 1, AnimLayerType::AIR },
    { AnimType::OILFIELD_BURN,"OILFIRE",    "OILFIRE",    15, 2, 0, AnimLayerType::AIR },
    { AnimType::CHRONO_BOX,   "CHRONBOX",   "CHRONBOX",   16, 2, 1, AnimLayerType::AIR },
    { AnimType::GPS_BOX,      "GPSBOX",     "GPSBOX",     16, 2, 1, AnimLayerType::AIR },
    { AnimType::INVUL_BOX,    "INVULBOX",   "INVULBOX",   16, 2, 1, AnimLayerType::AIR },
    { AnimType::PARA_BOX,     "PARABOX",    "PARABOX",    16, 2, 1, AnimLayerType::AIR },
    { AnimType::SONAR_BOX,    "SONARBOX",   "SONARBOX",   16, 2, 1, AnimLayerType::AIR },
    { AnimType::TWINKLE1,     "TWINKLE1",   "TWINKLE1",   4,  2, 1, AnimLayerType::AIR },
    { AnimType::TWINKLE2,     "TWINKLE2",   "TWINKLE2",   4,  2, 1, AnimLayerType::AIR },
    { AnimType::TWINKLE3,     "TWINKLE3",   "TWINKLE3",   4,  2, 1, AnimLayerType::AIR },
    { AnimType::FLAK,         "FLAK",       "FLAK",       7,  1, 1, AnimLayerType::AIR },
    { AnimType::MINE_EXP1,    "MINEXP1",    "MINEXP1",    12, 1, 1, AnimLayerType::AIR },
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
    chainTo_(AnimType::NONE),
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

    // Explosions with sounds
    s_animTypes[static_cast<int>(AnimType::FBALL1)].sound_ = AnimSoundType::SOUND_KABOOM25;
    s_animTypes[static_cast<int>(AnimType::FBALL1)].isCraterForming_ = true;
    s_animTypes[static_cast<int>(AnimType::FBALL1)].isScorcher_ = true;

    s_animTypes[static_cast<int>(AnimType::FRAG1)].sound_ = AnimSoundType::SOUND_KABOOM22;

    s_animTypes[static_cast<int>(AnimType::ART_EXP1)].sound_ = AnimSoundType::SOUND_KABOOM1;
    s_animTypes[static_cast<int>(AnimType::ART_EXP1)].isCraterForming_ = true;

    // Napalm with sounds and fire effects
    s_animTypes[static_cast<int>(AnimType::NAPALM1)].sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    s_animTypes[static_cast<int>(AnimType::NAPALM1)].isScorcher_ = true;
    s_animTypes[static_cast<int>(AnimType::NAPALM2)].sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    s_animTypes[static_cast<int>(AnimType::NAPALM2)].isScorcher_ = true;
    s_animTypes[static_cast<int>(AnimType::NAPALM3)].sound_ = AnimSoundType::SOUND_FIRE_EXPLODE;
    s_animTypes[static_cast<int>(AnimType::NAPALM3)].isScorcher_ = true;

    // Fire animations with damage
    s_animTypes[static_cast<int>(AnimType::FIRE_SMALL)].damage_ = DMG_TINY;
    s_animTypes[static_cast<int>(AnimType::FIRE_MED)].damage_ = DMG_SMALL;
    s_animTypes[static_cast<int>(AnimType::FIRE_MED2)].damage_ = DMG_SMALL;

    // Building fire with damage (loops infinitely)
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_SMALL)].damage_ = DMG_TINY;
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_MED)].damage_ = DMG_SMALL;
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_BIG)].damage_ = DMG_MEDIUM;

    // Burn animations with damage
    s_animTypes[static_cast<int>(AnimType::BURN_SMALL)].damage_ = DMG_TINY;
    s_animTypes[static_cast<int>(AnimType::BURN_MED)].damage_ = DMG_SMALL;
    s_animTypes[static_cast<int>(AnimType::BURN_BIG)].damage_ = DMG_SMALL;

    // Sticky animations (attach to units)
    s_animTypes[static_cast<int>(AnimType::PARACHUTE)].isSticky_ = true;
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_SMALL)].isSticky_ = true;
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_MED)].isSticky_ = true;
    s_animTypes[static_cast<int>(AnimType::ON_FIRE_BIG)].isSticky_ = true;

    // Atom blast special
    s_animTypes[static_cast<int>(AnimType::ATOM_BLAST)].isCraterForming_ = true;
    s_animTypes[static_cast<int>(AnimType::ATOM_BLAST)].isScorcher_ = true;
    s_animTypes[static_cast<int>(AnimType::ATOM_BLAST)].size_ = 128;

    // Mine explosion
    s_animTypes[static_cast<int>(AnimType::MINE_EXP1)].sound_ = AnimSoundType::SOUND_MINEBLOW;
    s_animTypes[static_cast<int>(AnimType::MINE_EXP1)].isCraterForming_ = true;

    // Translucent effects
    s_animTypes[static_cast<int>(AnimType::MUZZLE_FLASH)].isTranslucent_ = true;
    s_animTypes[static_cast<int>(AnimType::SMOKE_PUFF)].isTranslucent_ = true;
    s_animTypes[static_cast<int>(AnimType::SMOKE_M)].isTranslucent_ = true;

    // Electrocution chains to fire
    s_animTypes[static_cast<int>(AnimType::ELECT_DIE)].chainTo_ = AnimType::FIRE_MED;
    s_animTypes[static_cast<int>(AnimType::DOG_ELECT_DIE)].chainTo_ = AnimType::FIRE_SMALL;

    // Water explosions
    s_animTypes[static_cast<int>(AnimType::WATER_EXP1)].sound_ = AnimSoundType::SOUND_SPLASH;
    s_animTypes[static_cast<int>(AnimType::WATER_EXP2)].sound_ = AnimSoundType::SOUND_SPLASH;
    s_animTypes[static_cast<int>(AnimType::WATER_EXP3)].sound_ = AnimSoundType::SOUND_SPLASH;

    // Corpses are ground layer and static
    s_animTypes[static_cast<int>(AnimType::CORPSE1)].layer_ = AnimLayerType::GROUND;
    s_animTypes[static_cast<int>(AnimType::CORPSE2)].layer_ = AnimLayerType::GROUND;
    s_animTypes[static_cast<int>(AnimType::CORPSE3)].layer_ = AnimLayerType::GROUND;
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
    type_(AnimType::NONE),
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
        if (typeClass_->chainTo_ != AnimType::NONE) {
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

    if (type_ >= AnimType::FIRE_SMALL && type_ <= AnimType::ON_FIRE_BIG) {
        r = 255; g = 64; b = 0;  // Red-orange (fire)
    } else if (type_ >= AnimType::SMOKE_PUFF && type_ <= AnimType::LZ_SMOKE) {
        r = 128; g = 128; b = 128;  // Gray (smoke)
    } else if (type_ >= AnimType::WATER_EXP1 && type_ <= AnimType::WATER_EXP3) {
        r = 64; g = 128; b = 255;  // Blue (water)
    } else if (type_ >= AnimType::CRATE_DEVIATOR && type_ <= AnimType::CRATE_TQUAKE) {
        r = 255; g = 255; b = 0;  // Yellow (crate effects)
    } else if (type_ == AnimType::ELECT_DIE || type_ == AnimType::DOG_ELECT_DIE) {
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
    // For now this is a placeholder - the actual drawing would be:
    // Graphics_Draw_Rect(drawX - drawHalf, drawY - drawHalf, drawSize, drawSize, r, g, b, alpha);

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
    return typeClass_ ? typeClass_->layer_ : AnimLayerType::AIR;
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
    if (type_ == AnimType::ATOM_BLAST) {
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
        // TODO: attachedTo_->Take_Damage(1, 0, WARHEAD_FIRE, nullptr, ownerHouse_);
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
    int endFrame = (typeClass_->loopEnd_ >= 0) ? typeClass_->loopEnd_ : typeClass_->frameCount_;

    if (currentFrame_ >= endFrame) {
        // Check for looping
        if (loopsRemaining_ == 0) {
            // Infinite loop
            currentFrame_ = (typeClass_->loopStart_ >= 0) ? typeClass_->loopStart_ : 0;
            middleCalled_ = false;
            return true;
        } else if (loopsRemaining_ > 1) {
            // More loops remaining
            loopsRemaining_--;
            currentFrame_ = (typeClass_->loopStart_ >= 0) ? typeClass_->loopStart_ : 0;
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
    anim->type_ = AnimType::NONE;
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
        pool_[i].type_ = AnimType::NONE;
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

AnimClass* Create_Attached(AnimType type, void* target, int targetX, int targetY, int delay) {
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
        if (AnimClass::pool_[i].isActive_ && AnimClass::pool_[i].Get_Attached() == target) {
            AnimClass::pool_[i].Detach();
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
            return AnimType::PIFF;
        case 1:  // High explosive
            return AnimType::FBALL1;
        case 2:  // Armor piercing
            return AnimType::VEH_HIT2;
        case 3:  // Fire
            return AnimType::NAPALM2;
        case 4:  // Special (nuke)
            return AnimType::ATOM_BLAST;
        default:
            return AnimType::FRAG1;
    }
}

AnimType Get_Fire_Anim(int damagePercent) {
    // Return appropriate fire animation based on damage level
    if (damagePercent >= 75) {
        return AnimType::ON_FIRE_BIG;
    } else if (damagePercent >= 50) {
        return AnimType::ON_FIRE_MED;
    } else if (damagePercent >= 25) {
        return AnimType::ON_FIRE_SMALL;
    }
    return AnimType::NONE;
}

} // namespace Anims
