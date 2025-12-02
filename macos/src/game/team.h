/**
 * Red Alert macOS Port - AI Team Management
 *
 * TeamTypeClass - Team templates (blueprints for unit groups)
 * TeamClass - Active team instances (groups of units executing orders)
 *
 * Based on original TEAM.H and TEAMTYPE.H
 */

#ifndef GAME_TEAM_H
#define GAME_TEAM_H

#include "types.h"
#include "house.h"
#include <cstdint>

// Forward declarations
class TechnoClass;
class FootClass;

//===========================================================================
// Constants
//===========================================================================

// Maximum teams of each type
constexpr int TEAMTYPE_MAX = 60;
constexpr int TEAM_MAX = 60;

// Maximum units in a team
constexpr int TEAM_MEMBER_MAX = 25;

// Maximum mission steps in a team script
constexpr int TEAM_MISSION_MAX = 20;

//===========================================================================
// Team Mission Types
//===========================================================================

enum class TeamMissionType : int8_t {
    NONE = -1,
    ATTACK = 0,         // Attack specified quarry type
    ATTACK_WAYPOINT,    // Attack at waypoint
    CHANGE_FORMATION,   // Change to specified formation
    MOVE,               // Move to waypoint
    MOVE_TO_CELL,       // Move to specific cell
    GUARD,              // Guard current location
    JUMP,               // Jump to mission step
    ATTACK_TARCOM,      // Attack target comm
    UNLOAD,             // Unload transported units
    DEPLOY,             // Deploy (MCV)
    FOLLOW,             // Follow leader
    ENTER,              // Enter building/transport
    SPY,                // Spy infiltration
    PATROL,             // Patrol to waypoint
    SET_GLOBAL,         // Set global variable
    INVULNERABLE,       // Make team invulnerable
    LOAD,               // Load into transport

    COUNT
};

// Team formation types
enum class FormationType : int8_t {
    NONE = 0,
    LINE,               // Line formation
    WEDGE,              // Wedge formation
    COLUMN,             // Column formation
    DOUBLE_COLUMN,      // Double column

    COUNT
};

//===========================================================================
// Team Mission Step
//===========================================================================

struct TeamMissionData {
    TeamMissionType mission;    // Mission type
    int8_t argument;            // Mission argument (waypoint, etc.)
};

//===========================================================================
// Team Member Specification
//===========================================================================

struct TeamMemberSpec {
    RTTIType type;              // Type of unit (INFANTRY, UNIT, etc.)
    int8_t typeIndex;           // Specific unit type index
    int8_t count;               // Number required
};

//===========================================================================
// TeamTypeClass - Team blueprint/template
//===========================================================================

class TeamTypeClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    char name_[24];                     // Unique name
    int16_t id_;                        // Index in array
    bool isActive_;                     // Template is valid

    //-----------------------------------------------------------------------
    // Ownership
    //-----------------------------------------------------------------------
    HousesType house_;                  // Owning house

    //-----------------------------------------------------------------------
    // Flags
    //-----------------------------------------------------------------------
    bool isRoundabout_ : 1;             // Avoid enemy when moving
    bool isSuicide_ : 1;                // Fight to death
    bool isAutocreate_ : 1;             // Auto-create when conditions met
    bool isPrebuilt_ : 1;               // Units start already built
    bool isReinforcable_ : 1;           // Can receive reinforcements
    bool isTransient_ : 1;              // Temporary team
    bool isAlert_ : 1;                  // For alert-triggered teams only
    bool isWhiner_ : 1;                 // Complains when damaged
    bool isLooseRecruit_ : 1;           // Recruit any available unit
    bool isAggressive_ : 1;             // Attack enemies on sight
    bool isAnnoyance_ : 1;              // Harass rather than assault

    //-----------------------------------------------------------------------
    // Composition
    //-----------------------------------------------------------------------
    int8_t priority_;                   // Team priority (higher=likely)
    int8_t maxAllowed_;                 // Maximum instances of this team
    int8_t initNum_;                    // Initial number to create
    int8_t fear_;                       // Fear level (0=brave, 255=coward)

    int8_t memberCount_;                // Number of member specs
    TeamMemberSpec members_[8];         // Member specifications

    //-----------------------------------------------------------------------
    // Mission Script
    //-----------------------------------------------------------------------
    int8_t missionCount_;               // Number of mission steps
    TeamMissionData missions_[TEAM_MISSION_MAX];  // Mission script

    //-----------------------------------------------------------------------
    // Waypoint
    //-----------------------------------------------------------------------
    int8_t waypoint_;                   // Starting waypoint

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    TeamTypeClass();
    void Init();

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------
    const char* Name() const { return name_; }
    int TotalCount() const;             // Total units needed
    bool IsAvailable() const;           // Can create instance

    //-----------------------------------------------------------------------
    // Instance Management
    //-----------------------------------------------------------------------
    TeamClass* Create_Instance();

    //-----------------------------------------------------------------------
    // Static
    //-----------------------------------------------------------------------
    static TeamTypeClass* From_Name(const char* name);
    static TeamTypeClass* Suggested_New_Team(HouseClass* house, bool alert);
};

//===========================================================================
// TeamClass - Active team instance
//===========================================================================

class TeamClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    TeamTypeClass* typeClass_;          // Template this was created from
    int16_t id_;                        // Instance ID
    bool isActive_;                     // Team is active

    //-----------------------------------------------------------------------
    // Ownership
    //-----------------------------------------------------------------------
    HousesType house_;                  // Owning house

    //-----------------------------------------------------------------------
    // State
    //-----------------------------------------------------------------------
    bool isForcedActive_ : 1;           // Forced to stay active
    bool isHasBeen_ : 1;                // Has reached full strength
    bool isUnderStrength_ : 1;          // Below acceptable strength
    bool isReforming_ : 1;              // Returning to base to regroup
    bool isLagging_ : 1;                // Members falling behind
    bool isMoving_ : 1;                 // Currently moving
    bool isFull_ : 1;                   // At full strength

    //-----------------------------------------------------------------------
    // Members
    //-----------------------------------------------------------------------
    int8_t memberCount_;                // Current member count
    FootClass* members_[TEAM_MEMBER_MAX];  // Member units

    //-----------------------------------------------------------------------
    // Mission Execution
    //-----------------------------------------------------------------------
    int8_t currentMission_;             // Current mission step index
    int8_t suspendedMission_;           // Suspended mission step
    int16_t missionTimer_;              // Ticks until mission step complete

    //-----------------------------------------------------------------------
    // Target
    //-----------------------------------------------------------------------
    uint32_t target_;                   // Current target
    int32_t destination_;               // Current destination coordinate

    //-----------------------------------------------------------------------
    // Formation
    //-----------------------------------------------------------------------
    FormationType formation_;           // Current formation
    int32_t formationCenter_;           // Formation center coordinate

    //-----------------------------------------------------------------------
    // Zone
    //-----------------------------------------------------------------------
    int8_t zone_;                       // Current zone

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    TeamClass();
    TeamClass(TeamTypeClass* type);
    ~TeamClass();

    void Init(TeamTypeClass* type);

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------
    const char* Name() const;
    bool IsEmpty() const { return memberCount_ == 0; }
    bool IsFull() const;
    bool IsUnderStrength() const;
    int Strength() const;               // Returns 0-256 (current/required)

    //-----------------------------------------------------------------------
    // Member Management
    //-----------------------------------------------------------------------
    bool Add(FootClass* unit);
    bool Remove(FootClass* unit);
    void Recruit();                     // Try to recruit available units
    FootClass* Leader() const;          // Get team leader

    //-----------------------------------------------------------------------
    // Mission Execution
    //-----------------------------------------------------------------------
    void AI();                          // Per-frame AI processing
    bool Next_Mission();                // Advance to next mission step
    bool Execute_Mission();             // Execute current mission
    void Assign_Mission_Target(uint32_t target);

    //-----------------------------------------------------------------------
    // Movement
    //-----------------------------------------------------------------------
    bool Move_To(int32_t coord);
    bool Is_At_Destination() const;
    void Calc_Center();                 // Calculate formation center

    //-----------------------------------------------------------------------
    // Combat
    //-----------------------------------------------------------------------
    void Attack(uint32_t target);
    uint32_t Find_Target(QuarryType quarry);
    void Take_Damage(TechnoClass* source);

    //-----------------------------------------------------------------------
    // Dissolution
    //-----------------------------------------------------------------------
    void Disband();                     // Disband and free members
    void Suspend();                     // Suspend operations
    void Resume();                      // Resume operations
};

//===========================================================================
// Global Team Arrays
//===========================================================================

extern TeamTypeClass TeamTypes[TEAMTYPE_MAX];
extern TeamClass Teams[TEAM_MAX];
extern int TeamTypeCount;
extern int TeamCount;

//===========================================================================
// Helper Functions
//===========================================================================

void Init_TeamTypes();
void Init_Teams();
TeamClass* Create_Team(TeamTypeClass* type);
void Destroy_Team(TeamClass* team);

#endif // GAME_TEAM_H
