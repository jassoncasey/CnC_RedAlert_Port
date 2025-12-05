/**
 * Red Alert macOS Port - Voice Type Definitions
 *
 * Complete VOC enum and voice response system matching original game.
 * Ported from original AUDIO.CPP/DEFINES.H
 */

#ifndef GAME_VOICE_TYPES_H
#define GAME_VOICE_TYPES_H

#include <cstdint>

//===========================================================================
// VocType - All sound effects in the game
// Matches original Red Alert enum from DEFINES.H
//===========================================================================
enum class VocType : int16_t {
    NONE = -1,

    // Civilian responses
    GIRL_OKAY,      // "GIRLOKAY" - Female civilian okay
    GIRL_YEAH,      // "GIRLYEAH" - Female civilian yeah
    GUY_OKAY,       // "GUYOKAY1" - Male civilian okay
    GUY_YEAH,       // "GUYYEAH1" - Male civilian yeah

    // Mine layer
    MINELAY1,       // "MINELAY1" - Mine laying sound

    // Generic unit responses (house-variant: .V00-.V03, .R00-.R03)
    ACKNOWL,        // "ACKNO"    - Acknowledged
    AFFIRM,         // "AFFIRM1"  - Affirmative
    AWAIT,          // "AWAIT1"   - Awaiting orders
    ENG_AFFIRM,     // "EAFFIRM1" - Engineer affirmative
    ENG_ENG,        // "EENGIN1"  - Engineer ready
    NO_PROB,        // "NOPROB"   - No problem
    READY,          // "READY"    - Ready and waiting
    REPORT,         // "REPORT1"  - Reporting
    RIGHT_AWAY,     // "RITAWAY"  - Right away sir
    ROGER,          // "ROGER"    - Roger
    UGOTIT,         // "UGOTIT"   - You got it
    VEHIC,          // "VEHIC1"   - Vehicle reporting
    YESSIR,         // "YESSIR1"  - Yes sir

    // Death screams
    SCREAM1,        // "DEDMAN1"  - Death scream 1
    SCREAM3,        // "DEDMAN2"  - Death scream 2
    SCREAM4,        // "DEDMAN3"  - Death scream 3
    SCREAM5,        // "DEDMAN4"  - Death scream 4
    SCREAM6,        // "DEDMAN5"  - Death scream 5
    SCREAM7,        // "DEDMAN6"  - Death scream 6
    SCREAM10,       // "DEDMAN7"  - Death scream 7
    SCREAM11,       // "DEDMAN8"  - Death scream 8
    YELL1,          // "DEDMAN10" - Death yell

    // Special effects
    CHRONO,         // "CHRONO2"  - Chronosphere
    CANNON1,        // "CANNON1"  - Cannon fire 1
    CANNON2,        // "CANNON2"  - Cannon fire 2
    IRON1,          // "IRONCUR9" - Iron Curtain
    ENG_MOVEOUT,    // "EMOVOUT1" - Engineer moving out
    SONAR,          // "SONPULSE" - Sonar ping
    SANDBAG,        // "SANDBAG2" - Sandbag placement
    MINEBLOW,       // "MINEBLO1" - Mine explosion
    CHUTE1,         // "CHUTE1"   - Parachute

    // Dog sounds
    DOG_BARK,       // "DOGY1"    - Dog bark
    DOG_WHINE,      // "DOGW5"    - Dog whine
    DOG_GROWL2,     // "DOGG5P"   - Dog growl

    // Fire effects
    FIRE_LAUNCH,    // "FIREBL3"  - Fire launch
    FIRE_EXPLODE,   // "FIRETRT1" - Fire explosion

    // Weapon sounds
    GRENADE_TOSS,   // "GRENADE1" - Grenade throw
    GUN_5,          // "GUN11"    - Gun type 5
    GUN_7,          // "GUN13"    - Gun type 7
    ENG_YES,        // "EYESSIR1" - Engineer yes sir
    GUN_RIFLE,      // "GUN27"    - Rifle
    HEAL,           // "HEAL2"    - Healing
    DOOR,           // "HYDROD1"  - Door
    INVULNERABLE,   // "INVUL2"   - Invulnerability

    // Explosions
    KABOOM1,        // "KABOOM1"  - Explosion 1
    KABOOM12,       // "KABOOM12" - Explosion 12
    KABOOM15,       // "KABOOM15" - Explosion 15
    SPLASH,         // "SPLASH9"  - Water splash
    KABOOM22,       // "KABOOM22" - Explosion 22
    AACANON3,       // "AACANON3" - AA cannon

    // Tanya sounds
    TANYA_DIE,      // "TANDETH1" - Tanya death
    GUN_5F,         // "MGUNINF1" - Machine gun infantry
    MISSILE_1,      // "MISSILE1" - Missile 1
    MISSILE_2,      // "MISSILE6" - Missile 2
    MISSILE_3,      // "MISSILE7" - Missile 3
    GUN_5R,         // (unused)

    // UI sounds
    BEEP,           // "PILLBOX1" - Beep
    CLICK,          // "RABEEP1"  - Click
    SILENCER,       // "SILENCER" - Silenced shot
    CANNON6,        // "TANK5"    - Tank cannon 1
    CANNON7,        // "TANK6"    - Tank cannon 2
    TORPEDO,        // "TORPEDO1" - Torpedo launch
    CANNON8,        // "TURRET1"  - Turret fire
    TESLA_POWER_UP, // "TSLACHG2" - Tesla charging
    TESLA_ZAP,      // "TESLA1"   - Tesla zap
    SQUISH,         // "SQUISHY2" - Squish
    SCOLD,          // "SCOLDY1"  - Scold
    RADAR_ON,       // "RADARON2" - Radar on
    RADAR_OFF,      // "RADARDN1" - Radar off
    PLACE_BUILDING_DOWN, // "PLACBLDG" - Building placement
    KABOOM30,       // "KABOOM30" - Explosion 30
    KABOOM25,       // "KABOOM25" - Explosion 25

    // Dog responses
    DOG_HURT,       // (unused)
    DOG_YES,        // "DOGW7"    - Dog acknowledgment
    CRUMBLE,        // "DOGW3PX"  - Crumble

    // Money sounds
    MONEY_UP,       // "CRMBLE2"  - Money up
    MONEY_DOWN,     // "CASHUP1"  - Money down (note: swapped in original)
    CONSTRUCTION,   // "CASHDN1"  - Construction

    // Network/system sounds
    GAME_CLOSED,    // "BUILD5"   - Game closed
    INCOMING_MESSAGE, // "BLEEP9" - Incoming message
    SYS_ERROR,      // "BLEEP6"   - System error
    OPTIONS_CHANGED, // "BLEEP5"  - Options changed
    GAME_FORMING,   // "BLEEP17"  - Game forming
    PLAYER_LEFT,    // "BLEEP13"  - Player left
    PLAYER_JOINED,  // "BLEEP12"  - Player joined
    DEPTH_CHARGE,   // "BLEEP11"  - Depth charge
    CASHTURN,       // "H2OBOMB2" - Cash register (note: name mismatch)

    // Tanya voice responses
    TANYA_CHEW,     // "CASHTURN" - "Chew on this!"
    TANYA_ROCK,     // "TUFFGUY1" - "Let's rock!"
    TANYA_LAUGH,    // "ROKROLL1" - Tanya laugh
    TANYA_SHAKE,    // "LAUGH1"   - "Shake it baby"
    TANYA_CHING,    // "CMON1"    - "Cha-ching!"
    TANYA_GOT,      // "BOMBIT1"  - "I got it"
    TANYA_KISS,     // "GOTIT1"   - "Kiss it bye-bye"
    TANYA_THERE,    // "KEEPEM1"  - "I'm there"
    TANYA_GIVE,     // "ONIT1"    - "Give it to me"
    TANYA_YEA,      // "LEFTY1"   - "Yea baby"
    TANYA_YES,      // "YEAH1"    - "Yes sir?"
    TANYA_WHATS,    // "YES1"     - "What's up?"

    // Misc
    WALLKILL2,      // "YO1"      - Wall kill
    TRIPLE_SHOT,    // "WALLKIL2" - Triple shot
    SUBSHOW,        // (unused)

    // Einstein
    E_AH,           // "GUN5"     - Einstein ah
    E_OK,           // "SUBSHOW1" - Einstein ok
    E_YES,          // "EINAH1"   - Einstein yes
    TRIP_MINE,      // "EINOK1"   - Trip mine

    // Spy responses
    SPY_COMMANDER,  // "EINYES1"  - "Commander?"
    SPY_YESSIR,     // "MINE1"    - "Yes sir?"
    SPY_INDEED,     // "SCOMND1"  - "Indeed"
    SPY_ONWAY,      // "SYESSIR1" - "On my way"
    SPY_KING,       // "SINDEED1" - "For king and country"

    // Medic responses
    MED_REPORTING,  // "SONWAY1"  - "Medic reporting"
    MED_YESSIR,     // "SKING1"   - "Yes sir?"
    MED_AFFIRM,     // "MRESPON1" - "Affirmative"
    MED_MOVEOUT,    // "MYESSIR1" - "Moving out"

    // Beep select
    BEEP_SELECT,    // "MAFFIRM1" - Selection beep

    // Thief responses
    THIEF_YEA,      // "MMOVOUT1" - "Yea?"
    THIEF_MOVEOUT,  // "BEEPSLCT" - "Moving out"
    THIEF_OKAY,     // "SYEAH1"   - "Okay"
    THIEF_WHAT,     // "ANTDIE"   - "What?"
    THIEF_AFFIRM,   // "ANTBITE"  - "Affirmative"

    // Stavros (Greek commando)
    STAVCMDR,       // "SMOUT1"   - "Commander?"
    STAVCRSE,       // "SOKAY1"   - "Of course"
    STAVYES,        // (unused)   - "Yes?"
    STAVMOV,        // "SWHAT1"   - "Moving"

    // Ant sounds
    BUZZY1,         // "SAFFIRM1" - Ant buzz

    // Rambo/Commando
    RAMBO1,         // "STAVCMDR" - Commando 1
    RAMBO2,         // "STAVCRSE" - Commando 2
    RAMBO3,         // "STAVYES"  - Commando 3

    // Mechanic responses
    MECHYES1,       // "STAVMOV"  - "Yes?"
    MECHHOWDY1,     // "BUZZY1"   - "Howdy"
    MECHRISE1,      // "RAMBO1"   - "Rise and shine"
    MECHHUH1,       // "RAMBO2"   - "Huh?"
    MECHHEAR1,      // "RAMBO3"   - "I hear ya"
    MECHLAFF1,      // "MYES1"    - Mechanic laugh
    MECHBOSS1,      // "MHOWDY1"  - "You're the boss"
    MECHYEEHAW1,    // "MRISE1"   - "Yeehaw!"
    MECHHOTDIG1,    // "MHUH1"    - "Hot diggity!"
    MECHWRENCH1,    // "MHEAR1"   - Wrench sound

    // Shock trooper responses
    STBURN1,        // "MLAFF1"   - "Burn!"
    STCHRGE1,       // "MBOSS1"   - "Charging"
    STCRISP1,       // "MYEEHAW1" - "Extra crispy"
    STDANCE1,       // "MHOTDIG1" - "Dance!"
    STJUICE1,       // "MWRENCH1" - "Let's juice it"
    STJUMP1,        // "JBURN1"   - "Jump start"
    STLIGHT1,       // "JCHRGE1"  - "Lights out"
    STPOWER1,       // "JCRISP1"  - "Fully charged"
    STSHOCK1,       // "JDANCE1"  - "Shocking"
    STYES1,         // "JJUICE1"  - "Yes?"

    // Chrono tank
    CHRONOTANK1,    // "JJUMP1"   - Chrono tank

    // Mechanic repair
    MECH_FIXIT1,    // "JLIGHT1"  - "I'll fix it"

    // MAD Tank
    MAD_CHARGE,     // "JPOWER1"  - MAD charging
    MAD_EXPLODE,    // "JSHOCK1"  - MAD explosion

    // Shock trooper
    SHOCK_TROOP1,   // "JYES1"    - Shock trooper

    // Beacon
    BEACON,         // "CHROTNK1" - Beacon sound

    COUNT
};

//===========================================================================
// Voice Response Types - Categories for unit responses
//===========================================================================
enum class ResponseType {
    SELECT,     // Unit selected
    MOVE,       // Unit ordered to move
    ATTACK,     // Unit ordered to attack
    COUNT
};

//===========================================================================
// Voice Variant - House-specific voice variants
//===========================================================================
enum class VoiceVariant : int8_t {
    ALLIED = 0,   // .V00 - .V03 files
    SOVIET = 1,   // .R00 - .R03 files
    COUNT
};

//===========================================================================
// Sound Effect Data
//===========================================================================
struct SoundEffectDef {
    const char* filename;   // Base filename (without extension)
    int8_t priority;        // Playback priority (higher = more important)
    bool hasVariants;       // True if has .V/.R house variants
};

//===========================================================================
// Response Array Definition
//===========================================================================
struct VoiceResponseSet {
    const VocType* selectResponses;
    int selectCount;
    const VocType* moveResponses;
    int moveCount;
    const VocType* attackResponses;
    int attackCount;
};

//===========================================================================
// Public API
//===========================================================================

/**
 * Get filename for a VOC type
 */
const char* Voice_GetFilename(VocType voc);

/**
 * Get sound effect definition
 */
const SoundEffectDef* Voice_GetSoundDef(VocType voc);

/**
 * Check if VOC type has house variants (.V/.R files)
 */
bool Voice_HasVariants(VocType voc);

/**
 * Get response set for infantry type
 */
const VoiceResponseSet* Voice_GetInfantryResponses(int infantryType);

/**
 * Get response set for vehicle type
 */
const VoiceResponseSet* Voice_GetVehicleResponses(int unitType);

/**
 * Get a random response VOC for the given type and action
 */
VocType Voice_GetResponse(int unitType, bool isInfantry,
                          ResponseType response);

/**
 * Build the full filename for a VOC with house variant
 * @param voc       VOC type
 * @param variant   House variant (Allied/Soviet)
 * @param outBuffer Output buffer for filename
 * @param bufSize   Size of output buffer
 */
void Voice_BuildFilename(VocType voc, VoiceVariant variant,
                         char* outBuffer, int bufSize);

#endif // GAME_VOICE_TYPES_H
