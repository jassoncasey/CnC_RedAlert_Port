/**
 * Red Alert macOS Port - Voice Type Implementation
 *
 * Filename mappings and response arrays for all game sounds.
 * Ported from original AUDIO.CPP
 */

#include "voice_types.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

//===========================================================================
// Sound Effect Definitions - Filename and priority mappings
// Order matches VocType enum exactly
//===========================================================================
static const SoundEffectDef g_soundEffects[] = {
    // Civilian responses
    {"GIRLOKAY", 5, false},   // GIRL_OKAY
    {"GIRLYEAH", 5, false},   // GIRL_YEAH
    {"GUYOKAY1", 5, false},   // GUY_OKAY
    {"GUYYEAH1", 5, false},   // GUY_YEAH

    // Mine layer
    {"MINELAY1", 5, false},   // MINELAY1

    // Generic unit responses (with house variants)
    {"ACKNO",    10, true},   // ACKNOWL
    {"AFFIRM1",  10, true},   // AFFIRM
    {"AWAIT1",   10, true},   // AWAIT
    {"EAFFIRM1", 10, false},  // ENG_AFFIRM
    {"EENGIN1",  10, false},  // ENG_ENG
    {"NOPROB",   10, true},   // NO_PROB
    {"READY",    10, true},   // READY
    {"REPORT1",  10, true},   // REPORT
    {"RITAWAY",  10, true},   // RIGHT_AWAY
    {"ROGER",    10, true},   // ROGER
    {"UGOTIT",   10, true},   // UGOTIT
    {"VEHIC1",   10, true},   // VEHIC
    {"YESSIR1",  10, true},   // YESSIR

    // Death screams
    {"DEDMAN1",  8, false},   // SCREAM1
    {"DEDMAN2",  8, false},   // SCREAM3
    {"DEDMAN3",  8, false},   // SCREAM4
    {"DEDMAN4",  8, false},   // SCREAM5
    {"DEDMAN5",  8, false},   // SCREAM6
    {"DEDMAN6",  8, false},   // SCREAM7
    {"DEDMAN7",  8, false},   // SCREAM10
    {"DEDMAN8",  8, false},   // SCREAM11
    {"DEDMAN10", 8, false},   // YELL1

    // Special effects
    {"CHRONO2",  15, false},  // CHRONO
    {"CANNON1",  12, false},  // CANNON1
    {"CANNON2",  12, false},  // CANNON2
    {"IRONCUR9", 15, false},  // IRON1
    {"EMOVOUT1", 10, false},  // ENG_MOVEOUT
    {"SONPULSE", 8, false},   // SONAR
    {"SANDBAG2", 5, false},   // SANDBAG
    {"MINEBLO1", 12, false},  // MINEBLOW
    {"CHUTE1",   8, false},   // CHUTE1

    // Dog sounds
    {"DOGY1",    8, false},   // DOG_BARK
    {"DOGW5",    8, false},   // DOG_WHINE
    {"DOGG5P",   8, false},   // DOG_GROWL2

    // Fire effects
    {"FIREBL3",  10, false},  // FIRE_LAUNCH
    {"FIRETRT1", 10, false},  // FIRE_EXPLODE

    // Weapon sounds
    {"GRENADE1", 10, false},  // GRENADE_TOSS
    {"GUN11",    8, false},   // GUN_5
    {"GUN13",    8, false},   // GUN_7
    {"EYESSIR1", 10, false},  // ENG_YES
    {"GUN27",    8, false},   // GUN_RIFLE
    {"HEAL2",    8, false},   // HEAL
    {"HYDROD1",  5, false},   // DOOR
    {"INVUL2",   15, false},  // INVULNERABLE

    // Explosions
    {"KABOOM1",  12, false},  // KABOOM1
    {"KABOOM12", 12, false},  // KABOOM12
    {"KABOOM15", 12, false},  // KABOOM15
    {"SPLASH9",  8, false},   // SPLASH
    {"KABOOM22", 12, false},  // KABOOM22
    {"AACANON3", 10, false},  // AACANON3

    // Tanya death
    {"TANDETH1", 15, false},  // TANYA_DIE

    // More weapons
    {"MGUNINF1", 8, false},   // GUN_5F
    {"MISSILE1", 10, false},  // MISSILE_1
    {"MISSILE6", 10, false},  // MISSILE_2
    {"MISSILE7", 10, false},  // MISSILE_3
    {nullptr,    0, false},   // GUN_5R (unused)

    // UI and building sounds
    {"PILLBOX1", 8, false},   // BEEP
    {"RABEEP1",  5, false},   // CLICK
    {"SILENCER", 8, false},   // SILENCER
    {"TANK5",    12, false},  // CANNON6
    {"TANK6",    12, false},  // CANNON7
    {"TORPEDO1", 10, false},  // TORPEDO
    {"TURRET1",  10, false},  // CANNON8
    {"TSLACHG2", 15, false},  // TESLA_POWER_UP
    {"TESLA1",   15, false},  // TESLA_ZAP
    {"SQUISHY2", 8, false},   // SQUISH
    {"SCOLDY1",  8, false},   // SCOLD
    {"RADARON2", 10, false},  // RADAR_ON
    {"RADARDN1", 10, false},  // RADAR_OFF
    {"PLACBLDG", 8, false},   // PLACE_BUILDING_DOWN
    {"KABOOM30", 12, false},  // KABOOM30
    {"KABOOM25", 12, false},  // KABOOM25

    // Dog responses
    {nullptr,    0, false},   // DOG_HURT (unused)
    {"DOGW7",    8, false},   // DOG_YES
    {"DOGW3PX",  8, false},   // CRUMBLE

    // Money sounds
    {"CRMBLE2",  8, false},   // MONEY_UP
    {"CASHUP1",  8, false},   // MONEY_DOWN
    {"CASHDN1",  8, false},   // CONSTRUCTION

    // Network sounds
    {"BUILD5",   10, false},  // GAME_CLOSED
    {"BLEEP9",   5, false},   // INCOMING_MESSAGE
    {"BLEEP6",   5, false},   // SYS_ERROR
    {"BLEEP5",   5, false},   // OPTIONS_CHANGED
    {"BLEEP17",  5, false},   // GAME_FORMING
    {"BLEEP13",  5, false},   // PLAYER_LEFT
    {"BLEEP12",  5, false},   // PLAYER_JOINED
    {"BLEEP11",  8, false},   // DEPTH_CHARGE
    {"H2OBOMB2", 10, false},  // CASHTURN

    // Tanya voice responses
    {"CASHTURN", 15, false},  // TANYA_CHEW - "Chew on this!"
    {"TUFFGUY1", 15, false},  // TANYA_ROCK - "Let's rock!"
    {"ROKROLL1", 15, false},  // TANYA_LAUGH - Laugh
    {"LAUGH1",   15, false},  // TANYA_SHAKE - "Shake it baby"
    {"CMON1",    15, false},  // TANYA_CHING - "Cha-ching!"
    {"BOMBIT1",  15, false},  // TANYA_GOT - "I got it"
    {"GOTIT1",   15, false},  // TANYA_KISS - "Kiss it bye-bye"
    {"KEEPEM1",  15, false},  // TANYA_THERE - "I'm there"
    {"ONIT1",    15, false},  // TANYA_GIVE - "Give it to me"
    {"LEFTY1",   15, false},  // TANYA_YEA - "Yea baby"
    {"YEAH1",    15, false},  // TANYA_YES - "Yes sir?"
    {"YES1",     15, false},  // TANYA_WHATS - "What's up?"

    // Misc
    {"YO1",      8, false},   // WALLKILL2
    {"WALLKIL2", 8, false},   // TRIPLE_SHOT
    {nullptr,    0, false},   // SUBSHOW (unused)

    // Einstein
    {"GUN5",     8, false},   // E_AH
    {"SUBSHOW1", 10, false},  // E_OK
    {"EINAH1",   10, false},  // E_YES
    {"EINOK1",   8, false},   // TRIP_MINE

    // Spy responses
    {"EINYES1",  10, false},  // SPY_COMMANDER
    {"MINE1",    10, false},  // SPY_YESSIR
    {"SCOMND1",  10, false},  // SPY_INDEED
    {"SYESSIR1", 10, false},  // SPY_ONWAY
    {"SINDEED1", 10, false},  // SPY_KING

    // Medic responses
    {"SONWAY1",  10, false},  // MED_REPORTING
    {"SKING1",   10, false},  // MED_YESSIR
    {"MRESPON1", 10, false},  // MED_AFFIRM
    {"MYESSIR1", 10, false},  // MED_MOVEOUT

    // Selection beep
    {"MAFFIRM1", 5, false},   // BEEP_SELECT

    // Thief responses
    {"MMOVOUT1", 10, false},  // THIEF_YEA
    {"BEEPSLCT", 10, false},  // THIEF_MOVEOUT
    {"SYEAH1",   10, false},  // THIEF_OKAY
    {"ANTDIE",   10, false},  // THIEF_WHAT
    {"ANTBITE",  10, false},  // THIEF_AFFIRM

    // Stavros
    {"SMOUT1",   10, false},  // STAVCMDR
    {"SOKAY1",   10, false},  // STAVCRSE
    {nullptr,    0, false},   // STAVYES (unused)
    {"SWHAT1",   10, false},  // STAVMOV

    // Ant
    {"SAFFIRM1", 8, false},   // BUZZY1

    // Rambo/Commando
    {"STAVCMDR", 10, false},  // RAMBO1
    {"STAVCRSE", 10, false},  // RAMBO2
    {"STAVYES",  10, false},  // RAMBO3

    // Mechanic
    {"STAVMOV",  10, false},  // MECHYES1
    {"BUZZY1",   10, false},  // MECHHOWDY1
    {"RAMBO1",   10, false},  // MECHRISE1
    {"RAMBO2",   10, false},  // MECHHUH1
    {"RAMBO3",   10, false},  // MECHHEAR1
    {"MYES1",    10, false},  // MECHLAFF1
    {"MHOWDY1",  10, false},  // MECHBOSS1
    {"MRISE1",   10, false},  // MECHYEEHAW1
    {"MHUH1",    10, false},  // MECHHOTDIG1
    {"MHEAR1",   10, false},  // MECHWRENCH1

    // Shock trooper
    {"MLAFF1",   10, false},  // STBURN1
    {"MBOSS1",   10, false},  // STCHRGE1
    {"MYEEHAW1", 10, false},  // STCRISP1
    {"MHOTDIG1", 10, false},  // STDANCE1
    {"MWRENCH1", 10, false},  // STJUICE1
    {"JBURN1",   10, false},  // STJUMP1
    {"JCHRGE1",  10, false},  // STLIGHT1
    {"JCRISP1",  10, false},  // STPOWER1
    {"JDANCE1",  10, false},  // STSHOCK1
    {"JJUICE1",  10, false},  // STYES1

    // Chrono tank
    {"JJUMP1",   15, false},  // CHRONOTANK1

    // Mechanic fix
    {"JLIGHT1",  10, false},  // MECH_FIXIT1

    // MAD tank
    {"JPOWER1",  15, false},  // MAD_CHARGE
    {"JSHOCK1",  15, false},  // MAD_EXPLODE

    // Shock trooper
    {"JYES1",    10, false},  // SHOCK_TROOP1

    // Beacon
    {"CHROTNK1", 15, false},  // BEACON
};

//===========================================================================
// Response Arrays - Unit voice responses by type
//===========================================================================

// Generic infantry (E1 Rifle Infantry, etc.)
static const VocType g_infantrySelect[] = {
    VocType::AWAIT, VocType::REPORT, VocType::YESSIR, VocType::READY
};
static const VocType g_infantryMove[] = {
    VocType::AFFIRM, VocType::RIGHT_AWAY, VocType::ROGER, VocType::UGOTIT
};
static const VocType g_infantryAttack[] = {
    VocType::ACKNOWL, VocType::NO_PROB, VocType::AFFIRM
};

// Tanya responses
static const VocType g_tanyaSelect[] = {
    VocType::TANYA_YES, VocType::TANYA_WHATS, VocType::TANYA_YEA
};
static const VocType g_tanyaMove[] = {
    VocType::TANYA_THERE, VocType::TANYA_GIVE, VocType::TANYA_GOT
};
static const VocType g_tanyaAttack[] = {
    VocType::TANYA_ROCK, VocType::TANYA_CHEW, VocType::TANYA_SHAKE,
    VocType::TANYA_CHING, VocType::TANYA_KISS
};

// Engineer responses
static const VocType g_engineerSelect[] = {
    VocType::ENG_ENG, VocType::ENG_AFFIRM
};
static const VocType g_engineerMove[] = {
    VocType::ENG_MOVEOUT, VocType::ENG_AFFIRM
};
static const VocType g_engineerAttack[] = {
    VocType::ENG_YES, VocType::ENG_AFFIRM
};

// Spy responses
static const VocType g_spySelect[] = {
    VocType::SPY_COMMANDER, VocType::SPY_YESSIR
};
static const VocType g_spyMove[] = {
    VocType::SPY_ONWAY, VocType::SPY_INDEED, VocType::SPY_KING
};
static const VocType g_spyAttack[] = {
    VocType::SPY_INDEED, VocType::SPY_ONWAY
};

// Medic responses
static const VocType g_medicSelect[] = {
    VocType::MED_REPORTING, VocType::MED_YESSIR
};
static const VocType g_medicMove[] = {
    VocType::MED_MOVEOUT, VocType::MED_AFFIRM
};
static const VocType g_medicAttack[] = {
    VocType::MED_AFFIRM
};

// Thief responses
static const VocType g_thiefSelect[] = {
    VocType::THIEF_YEA, VocType::THIEF_WHAT
};
static const VocType g_thiefMove[] = {
    VocType::THIEF_MOVEOUT, VocType::THIEF_OKAY, VocType::THIEF_AFFIRM
};
static const VocType g_thiefAttack[] = {
    VocType::THIEF_AFFIRM
};

// Dog responses
static const VocType g_dogSelect[] = {
    VocType::DOG_BARK, VocType::DOG_BARK
};
static const VocType g_dogMove[] = {
    VocType::DOG_YES, VocType::DOG_BARK
};
static const VocType g_dogAttack[] = {
    VocType::DOG_GROWL2, VocType::DOG_BARK
};

// Civilian responses
static const VocType g_civilianSelect[] = {
    VocType::GUY_OKAY, VocType::GUY_YEAH, VocType::GIRL_OKAY, VocType::GIRL_YEAH
};
static const VocType g_civilianMove[] = {
    VocType::GUY_OKAY, VocType::GUY_YEAH, VocType::GIRL_OKAY, VocType::GIRL_YEAH
};
static const VocType g_civilianAttack[] = {
    VocType::GUY_OKAY
};

// Shock Trooper responses (Aftermath expansion - Soviet)
static const VocType g_shockSelect[] = {
    VocType::STYES1, VocType::STPOWER1, VocType::SHOCK_TROOP1
};
static const VocType g_shockMove[] = {
    VocType::STCHRGE1, VocType::STJUICE1, VocType::STJUMP1
};
static const VocType g_shockAttack[] = {
    VocType::STBURN1, VocType::STCRISP1, VocType::STDANCE1,
    VocType::STLIGHT1, VocType::STSHOCK1
};

// General/Stavros responses (Greek commando)
static const VocType g_generalSelect[] = {
    VocType::STAVCMDR, VocType::STAVCRSE
};
static const VocType g_generalMove[] = {
    VocType::STAVMOV, VocType::STAVCRSE
};
static const VocType g_generalAttack[] = {
    VocType::STAVCRSE, VocType::STAVMOV
};

// Generic vehicle responses
static const VocType g_vehicleSelect[] = {
    VocType::VEHIC, VocType::REPORT, VocType::AWAIT
};
static const VocType g_vehicleMove[] = {
    VocType::AFFIRM, VocType::ROGER, VocType::UGOTIT
};
static const VocType g_vehicleAttack[] = {
    VocType::ACKNOWL, VocType::AFFIRM
};

//===========================================================================
// Response Sets - Grouped by unit type
//===========================================================================

static const VoiceResponseSet g_infantryResponses = {
    g_infantrySelect, 4,
    g_infantryMove, 4,
    g_infantryAttack, 3
};

static const VoiceResponseSet g_tanyaResponses = {
    g_tanyaSelect, 3,
    g_tanyaMove, 3,
    g_tanyaAttack, 5
};

static const VoiceResponseSet g_engineerResponses = {
    g_engineerSelect, 2,
    g_engineerMove, 2,
    g_engineerAttack, 2
};

static const VoiceResponseSet g_spyResponses = {
    g_spySelect, 2,
    g_spyMove, 3,
    g_spyAttack, 2
};

static const VoiceResponseSet g_medicResponses = {
    g_medicSelect, 2,
    g_medicMove, 2,
    g_medicAttack, 1
};

static const VoiceResponseSet g_thiefResponses = {
    g_thiefSelect, 2,
    g_thiefMove, 3,
    g_thiefAttack, 1
};

static const VoiceResponseSet g_dogResponses = {
    g_dogSelect, 2,
    g_dogMove, 2,
    g_dogAttack, 2
};

static const VoiceResponseSet g_civilianResponses = {
    g_civilianSelect, 4,
    g_civilianMove, 4,
    g_civilianAttack, 1
};

static const VoiceResponseSet g_shockResponses = {
    g_shockSelect, 3,
    g_shockMove, 3,
    g_shockAttack, 5
};

static const VoiceResponseSet g_generalResponses = {
    g_generalSelect, 2,
    g_generalMove, 2,
    g_generalAttack, 2
};

static const VoiceResponseSet g_vehicleResponses = {
    g_vehicleSelect, 3,
    g_vehicleMove, 3,
    g_vehicleAttack, 2
};

//===========================================================================
// Public API Implementation
//===========================================================================

const char* Voice_GetFilename(VocType voc) {
    int idx = static_cast<int>(voc);
    if (idx < 0 || idx >= static_cast<int>(VocType::COUNT)) {
        return nullptr;
    }
    return g_soundEffects[idx].filename;
}

const SoundEffectDef* Voice_GetSoundDef(VocType voc) {
    int idx = static_cast<int>(voc);
    if (idx < 0 || idx >= static_cast<int>(VocType::COUNT)) {
        return nullptr;
    }
    return &g_soundEffects[idx];
}

bool Voice_HasVariants(VocType voc) {
    int idx = static_cast<int>(voc);
    if (idx < 0 || idx >= static_cast<int>(VocType::COUNT)) {
        return false;
    }
    return g_soundEffects[idx].hasVariants;
}

const VoiceResponseSet* Voice_GetInfantryResponses(int unitType) {
    // Map unit types (from units.h enum) to response sets
    // Note: These values must match the UnitType enum in units.h, NOT types.h
    // units.h: UNIT_TANYA=6, UNIT_ENGINEER=5, UNIT_SPY=8, etc.

    switch (unitType) {
        case 6:   // UNIT_TANYA
            return &g_tanyaResponses;
        case 5:   // UNIT_ENGINEER
            return &g_engineerResponses;
        case 8:   // UNIT_SPY
            return &g_spyResponses;
        case 9:   // UNIT_MEDIC
            return &g_medicResponses;
        case 10:  // UNIT_THIEF
            return &g_thiefResponses;
        case 7:   // UNIT_DOG
            return &g_dogResponses;
        case 11:  // UNIT_SHOCK - Shock Trooper (Aftermath)
            return &g_shockResponses;
        case 12:  // UNIT_GENERAL - General/Stavros
            return &g_generalResponses;
        case 13:  // UNIT_CIVILIAN_1
        case 14:  // UNIT_CIVILIAN_2
        case 15:  // UNIT_CIVILIAN_3
        case 16:  // UNIT_CIVILIAN_4
        case 17:  // UNIT_CIVILIAN_5
        case 18:  // UNIT_CIVILIAN_6
        case 19:  // UNIT_CIVILIAN_7
        case 20:  // UNIT_CIVILIAN_8 (Einstein)
        case 21:  // UNIT_CIVILIAN_9
        case 22:  // UNIT_CIVILIAN_10
        case 23:  // UNIT_CHAN
            return &g_civilianResponses;
        default:
            // Generic infantry (RIFLE, GRENADIER, ROCKET, FLAMETHROWER, etc.)
            return &g_infantryResponses;
    }
}

const VoiceResponseSet* Voice_GetVehicleResponses(int /*unitType*/) {
    // For now, all vehicles use generic responses
    // Can be extended for special units (Chrono Tank, MAD Tank, etc.)
    return &g_vehicleResponses;
}

VocType Voice_GetResponse(int unitType, bool isInfantry,
                          ResponseType response) {
    const VoiceResponseSet* set = isInfantry
        ? Voice_GetInfantryResponses(unitType)
        : Voice_GetVehicleResponses(unitType);

    if (!set) return VocType::NONE;

    const VocType* responses = nullptr;
    int count = 0;

    switch (response) {
        case ResponseType::SELECT:
            responses = set->selectResponses;
            count = set->selectCount;
            break;
        case ResponseType::MOVE:
            responses = set->moveResponses;
            count = set->moveCount;
            break;
        case ResponseType::ATTACK:
            responses = set->attackResponses;
            count = set->attackCount;
            break;
        default:
            return VocType::NONE;
    }

    if (!responses || count <= 0) return VocType::NONE;

    // Random selection for variety
    int idx = rand() % count;
    return responses[idx];
}

void Voice_BuildFilename(VocType voc, VoiceVariant variant,
                         char* outBuffer, int bufSize) {
    const char* baseName = Voice_GetFilename(voc);
    if (!baseName || bufSize < 16) {
        if (outBuffer && bufSize > 0) outBuffer[0] = '\0';
        return;
    }

    bool hasVariants = Voice_HasVariants(voc);

    if (hasVariants) {
        // Build variant filename (e.g., AWAIT1.V00 or AWAIT1.R00)
        const char* ext = (variant == VoiceVariant::ALLIED) ? ".V" : ".R";
        int variantNum = rand() % 4;  // 00-03
        snprintf(outBuffer, bufSize, "%s%s%02d", baseName, ext, variantNum);
    } else {
        // Standard AUD file
        snprintf(outBuffer, bufSize, "%s.AUD", baseName);
    }
}
