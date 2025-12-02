/**
 * Red Alert macOS Port - Trigger System
 *
 * TriggerTypeClass - Trigger templates (event-action definitions)
 * TriggerClass - Active trigger instances
 *
 * Based on original TRIGGER.H, TRIGTYPE.H, TEVENT.H, TACTION.H
 */

#ifndef GAME_TRIGGER_H
#define GAME_TRIGGER_H

#include "types.h"
#include "house.h"
#include <cstdint>

// Forward declarations
class ObjectClass;
class TechnoClass;
class TeamTypeClass;
class TriggerClass;

//===========================================================================
// Constants
//===========================================================================

constexpr int TRIGGERTYPE_MAX = 80;
constexpr int TRIGGER_MAX = 100;

//===========================================================================
// Trigger Event Types
//===========================================================================

enum class TEventType : int8_t {
    NONE = -1,

    // Unit/building events
    PLAYER_ENTERED = 0,     // Player unit enters cell
    SPIED,                  // Spy infiltrates building
    THIEVED,                // Thief steals vehicle
    DISCOVERED,             // Unit discovers object
    HOUSE_DISCOVERED,       // Player discovers enemy house
    ATTACKED,               // Unit attacks object
    DESTROYED,              // Unit/building destroyed
    ANY,                    // Any object event

    // House events
    UNITS_DESTROYED,        // House loses all units
    BUILDINGS_DESTROYED,    // House loses all buildings
    ALL_DESTROYED,          // House loses everything
    CREDITS,                // House reaches credit amount
    NBUILDINGS_DESTROYED,   // N buildings destroyed
    NUNITS_DESTROYED,       // N units destroyed
    NOFACTORIES,            // No factories remaining
    LOW_POWER,              // Power below 100%

    // Time events
    TIME,                   // Elapsed time (frames)
    MISSION_TIMER_EXPIRED,  // Countdown timer reached zero

    // Construction events
    BUILD,                  // Specific building built
    BUILD_UNIT,             // Specific unit built
    BUILD_INFANTRY,         // Specific infantry built
    BUILD_AIRCRAFT,         // Specific aircraft built

    // Movement events
    LEAVES_MAP,             // Team leaves map
    ENTERS_ZONE,            // Enters waypoint zone
    CROSS_HORIZONTAL,       // Crosses horizontal line
    CROSS_VERTICAL,         // Crosses vertical line

    // Global flag events
    GLOBAL_SET,             // Global flag set to true
    GLOBAL_CLEAR,           // Global flag set to false

    // Misc events
    EVAC_CIVILIAN,          // Civilian evacuated
    FAKES_DESTROYED,        // Fake structures destroyed
    ALL_BRIDGES_DESTROYED,  // All bridges gone
    BUILDING_EXISTS,        // Specific building exists

    COUNT
};

//===========================================================================
// Trigger Action Types
//===========================================================================

enum class TActionType : int8_t {
    NONE = -1,

    // Win/Lose
    WIN = 0,                // Player wins mission
    LOSE,                   // Player loses mission
    WINLOSE,                // Win if captured, lose if destroyed
    ALLOWWIN,               // Allow mission to be won

    // Production
    BEGIN_PRODUCTION,       // Computer starts building
    AUTOCREATE,             // Computer auto-creates teams

    // Team management
    CREATE_TEAM,            // Create new unit team
    DESTROY_TEAM,           // Delete team
    ALL_HUNT,               // All enemy units hunt
    REINFORCEMENTS,         // Send reinforcements
    DZ,                     // Deploy drop zone marker
    FIRE_SALE,              // Sell buildings, go rampage

    // Media
    PLAY_MOVIE,             // Play video file
    PLAY_SOUND,             // Play sound effect
    PLAY_MUSIC,             // Play musical score
    PLAY_SPEECH,            // Play EVA speech
    TEXT_TRIGGER,           // Display text message

    // Trigger management
    DESTROY_TRIGGER,        // Delete trigger
    FORCE_TRIGGER,          // Force another trigger

    // Map reveal
    REVEAL_ALL,             // Show entire map
    REVEAL_SOME,            // Reveal area around cell
    REVEAL_ZONE,            // Reveal waypoint zone

    // Timer control
    START_TIMER,            // Start mission timer
    STOP_TIMER,             // Stop mission timer
    ADD_TIMER,              // Add time to timer
    SUB_TIMER,              // Subtract time from timer
    SET_TIMER,              // Set and start timer

    // Global flags
    SET_GLOBAL,             // Set global flag to true
    CLEAR_GLOBAL,           // Set global flag to false

    // Building control
    BASE_BUILDING,          // Automated base construction
    DESTROY_OBJECT,         // Destroy attached building

    // Special weapons
    ONE_SPECIAL,            // Grant one-time special weapon
    FULL_SPECIAL,           // Grant repeating special weapon

    // Targeting
    PREFERRED_TARGET,       // Set preferred attack target

    // Misc
    CREEP_SHADOW,           // Shroud regrows one step
    LAUNCH_NUKES,           // Launch fake nuclear missiles

    COUNT
};

//===========================================================================
// Event Data Requirements
//===========================================================================

enum class EventNeedType : uint8_t {
    NEED_NONE = 0x00,
    NEED_HOUSE = 0x01,
    NEED_NUMBER = 0x02,
    NEED_STRUCTURE = 0x04,
    NEED_UNIT = 0x08,
    NEED_INFANTRY = 0x10,
    NEED_AIRCRAFT = 0x20,
    NEED_TEAM = 0x40,
    NEED_WAYPOINT = 0x80
};

//===========================================================================
// Trigger Attachment Types
//===========================================================================

enum class AttachType : uint8_t {
    NONE = 0x00,
    CELL = 0x01,            // Attaches to map cell
    OBJECT = 0x02,          // Attaches to unit/building
    MAP = 0x04,             // Map-wide trigger
    HOUSE = 0x08,           // House-specific trigger
    GENERAL = 0x10,         // General game state trigger
    TEAM = 0x20             // Attached to team
};

//===========================================================================
// Multi-Event/Action Control
//===========================================================================

enum class MultiStyleType : int8_t {
    ONLY = 0,               // Only main event/action
    AND,                    // Both must occur/execute
    OR,                     // Either can trigger/execute
    LINKED                  // Event-action pairs linked
};

//===========================================================================
// Trigger Persistence Types
//===========================================================================

enum class PersistantType : int8_t {
    VOLATILE = 0,           // Destroyed after first execution
    SEMIPERSISTANT = 1,     // Executes when all attachments trigger
    PERSISTANT = 2          // Never deleted, can trigger repeatedly
};

//===========================================================================
// TEventClass - Event Condition
//===========================================================================

struct TEventClass {
    TEventType event;           // Event type
    int16_t teamIndex;          // Team type index (-1 if none)

    union {
        int8_t structure;       // Building type
        int8_t unit;            // Unit type
        int8_t infantry;        // Infantry type
        int8_t aircraft;        // Aircraft type
        HousesType house;       // Faction
        int32_t value;          // Generic number (time, count, etc.)
    } data;

    TEventClass();
    void Reset();

    // Check if event matches
    bool Matches(TEventType checkEvent) const { return event == checkEvent; }

    // Get what type of data this event needs
    static EventNeedType Event_Needs(TEventType type);
    static AttachType Attaches_To(TEventType type);
};

//===========================================================================
// TActionClass - Action to Perform
//===========================================================================

struct TActionClass {
    TActionType action;         // Action type
    int16_t teamIndex;          // Team type index (-1 if none)
    int16_t triggerIndex;       // Trigger type index (-1 if none)

    union {
        int8_t theme;           // Music track
        int8_t sound;           // Sound effect
        int8_t speech;          // EVA voice
        HousesType house;       // Faction
        int8_t special;         // Special weapon
        QuarryType quarry;      // Preferred target
        int8_t movie;           // Video to play
        bool boolean;           // Boolean value
        int32_t value;          // Generic number
    } data;

    TActionClass();
    void Reset();

    // Execute the action
    bool Execute(HousesType house, ObjectClass* object,
                 int triggerId, int16_t cell);
};

//===========================================================================
// TDEventClass - Event Instance State
//===========================================================================

struct TDEventClass {
    bool isTripped;             // Event has occurred
    int32_t timer;              // Timer for time-based events

    TDEventClass() : isTripped(false), timer(0) {}
    void Reset() { isTripped = false; timer = 0; }
};

//===========================================================================
// TriggerTypeClass - Trigger Template
//===========================================================================

class TriggerTypeClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    char name_[24];                     // Unique name
    int16_t id_;                        // Index in array
    bool isActive_;                     // Template is valid

    //-----------------------------------------------------------------------
    // Persistence
    //-----------------------------------------------------------------------
    PersistantType persistence_;        // How trigger survives execution

    //-----------------------------------------------------------------------
    // Ownership
    //-----------------------------------------------------------------------
    HousesType house_;                  // House that owns this trigger

    //-----------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------
    TEventClass event1_;                // First event condition
    TEventClass event2_;                // Second event condition
    MultiStyleType eventControl_;       // How events combine

    //-----------------------------------------------------------------------
    // Actions
    //-----------------------------------------------------------------------
    TActionClass action1_;              // First action
    TActionClass action2_;              // Second action
    MultiStyleType actionControl_;      // How actions combine

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    TriggerTypeClass();
    void Init();

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------
    const char* Name() const { return name_; }
    AttachType Attaches_To() const;

    //-----------------------------------------------------------------------
    // Instance Creation
    //-----------------------------------------------------------------------
    TriggerClass* Create_Instance();

    //-----------------------------------------------------------------------
    // Static
    //-----------------------------------------------------------------------
    static TriggerTypeClass* From_Name(const char* name);
    static TriggerTypeClass* From_ID(int id);
};

//===========================================================================
// TriggerClass - Active Trigger Instance
//===========================================================================

class TriggerClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    TriggerTypeClass* typeClass_;       // Template
    int16_t id_;                        // Instance ID
    bool isActive_;                     // Active in game

    //-----------------------------------------------------------------------
    // Event State
    //-----------------------------------------------------------------------
    TDEventClass event1State_;          // First event state
    TDEventClass event2State_;          // Second event state

    //-----------------------------------------------------------------------
    // Attachment
    //-----------------------------------------------------------------------
    int attachCount_;                   // Number of attachments
    int16_t cell_;                      // Cell if cell-based

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    TriggerClass();
    TriggerClass(TriggerTypeClass* type);
    ~TriggerClass();

    void Init(TriggerTypeClass* type);

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------
    const char* Name() const;
    HousesType House() const;

    //-----------------------------------------------------------------------
    // Execution
    //-----------------------------------------------------------------------
    bool Spring(TEventType event, ObjectClass* object,
                int16_t cell, bool forced = false);

    //-----------------------------------------------------------------------
    // Attachment
    //-----------------------------------------------------------------------
    void Attach(ObjectClass* object);
    void Detach(ObjectClass* object);
    void Attach_To_Cell(int16_t cell);

    //-----------------------------------------------------------------------
    // Static
    //-----------------------------------------------------------------------
    static void Spring_All(TEventType event, HousesType house);
    static void Spring_All(TEventType event, ObjectClass* object);
};

//===========================================================================
// Global Trigger Arrays
//===========================================================================

extern TriggerTypeClass TriggerTypes[TRIGGERTYPE_MAX];
extern TriggerClass Triggers[TRIGGER_MAX];
extern int TriggerTypeCount;
extern int TriggerCount;

//===========================================================================
// Helper Functions
//===========================================================================

void Init_TriggerTypes();
void Init_Triggers();
TriggerClass* Create_Trigger(TriggerTypeClass* type);
void Destroy_Trigger(TriggerClass* trigger);

// Evaluate all triggers for an event
void Process_Triggers(TEventType event, HousesType house = HousesType::NONE,
                      ObjectClass* object = nullptr, int16_t cell = -1);

#endif // GAME_TRIGGER_H
