/**
 * Red Alert macOS Port - Campaign Implementation
 *
 * Based on original SCENARIO.CPP (~5000 lines) and SCORE.CPP (~3000 lines)
 */

#include "campaign.h"
#include "scenario.h"
#include "../ui/game_ui.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

// Unit struct fields we need (matches units.h)
#define MAX_UNITS 256
#define MAX_BUILDINGS 128
#define MAX_PATH_WAYPOINTS 32
#define MAX_PASSENGERS 5
#define TEAM_PLAYER 1
#define STATE_DYING 5

// Local copies of struct definitions (to avoid header conflicts)
// These must match units.h exactly
struct CampaignUnit {
    uint8_t type;
    uint8_t team;
    uint8_t state;
    uint8_t facing;
    int16_t health;
    int16_t maxHealth;
    int32_t worldX;
    int32_t worldY;
    int32_t targetX;
    int32_t targetY;
    int16_t targetUnit;
    int16_t speed;
    int16_t attackRange;
    int16_t attackDamage;
    int16_t attackCooldown;
    int16_t attackRate;
    int16_t sightRange;
    uint8_t selected;
    uint8_t active;
    int16_t pathCells[MAX_PATH_WAYPOINTS];
    int8_t pathLength;
    int8_t pathIndex;
    int32_t nextWaypointX;
    int32_t nextWaypointY;
    int16_t cargo;
    int16_t homeRefinery;
    int16_t harvestTimer;
    int16_t lastAttacker;
    int16_t scatterTimer;
    int16_t passengers[MAX_PASSENGERS];
    int8_t passengerCount;
    int16_t transportId;
    int16_t loadTarget;
    char triggerName[24];
};

struct CampaignBuilding {
    uint8_t type;
    uint8_t team;
    int16_t health;
    int16_t maxHealth;
    int16_t cellX;
    int16_t cellY;
    uint8_t width;
    uint8_t height;
    uint8_t selected;
    uint8_t active;
    int16_t attackCooldown;
    int16_t sightRange;
    char triggerName[24];
};

// Forward declarations for unit system functions
extern "C" {
    CampaignUnit* Units_Get(int unitId);
    CampaignBuilding* Buildings_Get(int buildingId);
}

//===========================================================================
// Global Instance
//===========================================================================

CampaignClass Campaign;

//===========================================================================
// Campaign Name Table
//===========================================================================

static const char* CampaignNames[] = {
    "Allied",
    "Soviet",
    "Aftermath",
    "Counter-Strike"
};

const char* CampaignName(CampaignType campaign) {
    if (campaign < CampaignType::ALLIED || campaign >= CampaignType::COUNT) {
        return "Unknown";
    }
    return CampaignNames[static_cast<int>(campaign)];
}

//===========================================================================
// Allied Campaign Mission Data
//===========================================================================

const MissionData AlliedMissions[] = {
    // Mission 1: Rescue Einstein
    {
        1, "In the Thick of It",
        "Commander, Soviet forces have invaded Eastern Europe. Your mission "
        "is to establish a base and rescue Allied scientists from the Soviet "
        "advance. Build your base and eliminate all Soviet forces in the area.",
        VQType::ALLY01, VQType::ALLY01, VQType::ALLY02, VQType::NONE,
        ThemeType::HELL_MARCH, TheaterType::TEMPERATE,
        false, 2, -1
    },
    // Mission 2: Five to One
    {
        2, "Five to One",
        "We've located Einstein. He's being held in a Soviet compound to the "
        "north. Use Tanya to infiltrate the compound and rescue him. Avoid "
        "enemy detection until you reach the compound.",
        VQType::ALLY02, VQType::ALLY02, VQType::ALLY04, VQType::ALLY01,
        ThemeType::RUN_FOR_YOUR_LIFE, TheaterType::SNOW,
        false, 3, -1
    },
    // Mission 3: Dead End
    {
        3, "Dead End",
        "Soviet forces are building a large military base in this sector. You "
        "must establish your own base and destroy all Soviet structures. Watch "
        "for enemy reinforcements from the east.",
        VQType::ALLY04, VQType::ALLY04, VQType::ALLY05, VQType::ALLY02,
        ThemeType::FACE_THE_ENEMY_1, TheaterType::TEMPERATE,
        false, 4, -1
    },
    // Mission 4: Tanya's Tale
    {
        4, "Tanya's Tale",
        "Tanya has been captured by the Soviets. Rescue her before she is "
        "executed. Use stealth and precision to infiltrate the Soviet prison. "
        "Time is critical.",
        VQType::ALLY05, VQType::ALLY05, VQType::ALLY06, VQType::ALLY04,
        ThemeType::TRENCHES, TheaterType::SNOW,
        true, 5, 5
    },
    // Mission 5: Khalkis Island
    {
        5, "Khalkis Island",
        "The Soviets have established a submarine base on Khalkis Island. You "
        "must destroy all Soviet naval forces and the submarine pens. Naval "
        "support is available for this mission.",
        VQType::ALLY06, VQType::ALLY06, VQType::ALLY08, VQType::ALLY05,
        ThemeType::BIGFOOT, TheaterType::TEMPERATE,
        false, 6, -1
    },
    // Mission 6: Bridge Over River Grotz
    {
        6, "Bridge Over River Grotz",
        "A vital supply bridge must be defended from Soviet attack. Hold the "
        "bridge for 30 minutes while Allied forces evacuate civilians. Do not "
        "let the bridge be destroyed.",
        VQType::ALLY08, VQType::ALLY08, VQType::ALLY09, VQType::ALLY06,
        ThemeType::CRUSH, TheaterType::SNOW,
        false, 7, -1
    },
    // Mission 7: Core of the Matter
    {
        7, "Core of the Matter",
        "Soviet forces are transporting nuclear materials through this region. "
        "Intercept and destroy the convoy before it reaches its destination. "
        "Do not allow any convoy vehicles to escape.",
        VQType::ALLY09, VQType::ALLY09, VQType::ALLY10, VQType::ALLY08,
        ThemeType::FACE_THE_ENEMY_2, TheaterType::INTERIOR,
        true, 8, 8
    },
    // Mission 8: Sarin Gas Facility
    {
        8, "Sarin Gas Facility",
        "Intelligence reports a Soviet chemical weapons facility in this area. "
        "Destroy the facility and all chemical storage tanks. Be careful not "
        "to damage the tanks before evacuation is complete.",
        VQType::ALLY10, VQType::ALLY10, VQType::ALLY11, VQType::ALLY09,
        ThemeType::SMASH, TheaterType::SNOW,
        false, 9, -1
    },
    // Mission 9: Distant Thunder
    {
        9, "Distant Thunder",
        "Soviet Tesla coil research must be stopped. Infiltrate their research "
        "facility and capture the lead scientist. It is heavily defended.",
        VQType::ALLY11, VQType::ALLY11, VQType::ALLY12, VQType::ALLY10,
        ThemeType::WORKMEN, TheaterType::TEMPERATE,
        false, 10, -1
    },
    // Mission 10: Brothers in Arms
    {
        10, "Brothers in Arms",
        "Allied forces are pinned down and need immediate support. Break "
        "through Soviet lines and link up with friendly forces. Once linked, "
        "destroy all Soviet forces in the area.",
        VQType::ALLY12, VQType::ALLY12, VQType::ALLY14, VQType::ALLY11,
        ThemeType::AWAIT, TheaterType::SNOW,
        true, 11, 11
    },
    // Mission 11: Intervention
    {
        11, "Intervention",
        "We've located the Soviet command center for this region. Destroy it "
        "to disrupt their operations. Expect heavy resistance.",
        VQType::ALLY14, VQType::ALLY14, VQType::NONE, VQType::ALLY12,
        ThemeType::DENSE, TheaterType::TEMPERATE,
        false, 12, -1
    },
    // Mission 12: Soviet Demise
    {
        12, "Soviet Demise",
        "The Soviet headquarters must fall. Launch a full assault on their "
        "primary base. Destroy all structures and eliminate all Soviet forces.",
        VQType::NONE, VQType::NONE, VQType::NONE, VQType::ALLY14,
        ThemeType::VECTOR, TheaterType::SNOW,
        false, 13, -1
    },
    // Mission 13: Focused Blast
    {
        13, "Focused Blast",
        "A Soviet Iron Curtain device has been located. This technology must "
        "be destroyed before deployment. Watch for chronosphere effects.",
        VQType::NONE, VQType::NONE, VQType::NONE, VQType::NONE,
        ThemeType::TWIN_GUNS, TheaterType::INTERIOR,
        false, 14, -1
    },
    // Mission 14: Final Assault (End of Campaign)
    {
        14, "No Remorse",
        "This is it, Commander. The Kremlin itself. Destroy all Soviet forces "
        "and capture or destroy the Kremlin. Victory here means the end of the "
        "Soviet threat. Good luck.",
        VQType::NONE, VQType::NONE, VQType::MASTEFIN, VQType::NONE,
        ThemeType::HELL_MARCH, TheaterType::SNOW,
        false, -1, -1  // End of campaign
    }
};

//===========================================================================
// Soviet Campaign Mission Data
//===========================================================================

const MissionData SovietMissions[] = {
    // Mission 1: Lesson in Blood
    {
        1, "Lesson in Blood",
        "Comrade Commander, the capitalist West threatens our glorious Soviet "
        "Union. Crush the Allied forces in this region and secure our borders. "
        "Show them the might of the Red Army!",
        VQType::SOVIET01, VQType::SOVIET01, VQType::SOVIET02, VQType::NONE,
        ThemeType::HELL_MARCH, TheaterType::SNOW,
        false, 2, -1
    },
    // Mission 2: Testament of Power
    {
        2, "Testament of Power",
        "Allied spies have infiltrated our research facility. Hunt them down "
        "and eliminate them before they can escape with our secrets. Leave no "
        "witnesses.",
        VQType::SOVIET02, VQType::SOVIET02, VQType::SOVIET03, VQType::SOVIET01,
        ThemeType::RUN_FOR_YOUR_LIFE, TheaterType::TEMPERATE,
        false, 3, -1
    },
    // Mission 3: Red Dawn
    {
        3, "Red Dawn",
        "The time has come to strike deep into Allied territory. Establish a "
        "beachhead and destroy all Allied defenses. Reinforcements will arrive "
        "once you secure the landing zone.",
        VQType::SOVIET03, VQType::SOVIET03, VQType::SOVIET04, VQType::SOVIET02,
        ThemeType::FACE_THE_ENEMY_1, TheaterType::TEMPERATE,
        false, 4, -1
    },
    // Mission 4: Legacy of Tesla
    {
        4, "Legacy of Tesla",
        "Our scientists have developed a new weapon - the Tesla Coil. Defend "
        "the research facility while they complete their work. Do not let the "
        "Allies destroy our progress.",
        VQType::SOVIET04, VQType::SOVIET04, VQType::SOVIET05, VQType::SOVIET03,
        ThemeType::TRENCHES, TheaterType::SNOW,
        true, 5, 5
    },
    // Mission 5: Protect the Convoys
    {
        5, "Protect the Convoys",
        "Critical supplies must reach our front lines. Protect the convoy as "
        "it moves through enemy territory. If the convoy is destroyed, our "
        "offensive will fail.",
        VQType::SOVIET05, VQType::SOVIET05, VQType::SOVIET06, VQType::SOVIET04,
        ThemeType::BIGFOOT, TheaterType::SNOW,
        false, 6, -1
    },
    // Mission 6: Bridge to Victory
    {
        6, "Bridge to Victory",
        "Capture the Allied supply bridge and hold it against counterattack. "
        "Once secured, use it to launch attacks on Allied positions beyond "
        "the river.",
        VQType::SOVIET06, VQType::SOVIET06, VQType::SOVIET07, VQType::SOVIET05,
        ThemeType::CRUSH, TheaterType::TEMPERATE,
        false, 7, -1
    },
    // Mission 7: Operation Avalanche
    {
        7, "Operation Avalanche",
        "Allied forces are massing for a counterattack. Strike first and "
        "destroy their assembly areas. Speed is essential - they must not be "
        "allowed to organize their attack.",
        VQType::SOVIET07, VQType::SOVIET07, VQType::SOVIET08, VQType::SOVIET06,
        ThemeType::FACE_THE_ENEMY_2, TheaterType::SNOW,
        true, 8, 8
    },
    // Mission 8: Burning Bridges
    {
        8, "Burning Bridges",
        "Allied naval forces threaten our supply lines. Destroy their port "
        "facilities and sink their fleet. Control of the seas is vital to "
        "our victory.",
        VQType::SOVIET08, VQType::SOVIET08, VQType::SOVIET09, VQType::SOVIET07,
        ThemeType::SMASH, TheaterType::TEMPERATE,
        false, 9, -1
    },
    // Mission 9: Elba Island
    {
        9, "Elba Island",
        "Allied high command has established a base on Elba Island. Assault "
        "the island and destroy their headquarters. Naval transport will be "
        "provided for the invasion.",
        VQType::SOVIET09, VQType::SOVIET09, VQType::SOVIET10, VQType::SOVIET08,
        ThemeType::WORKMEN, TheaterType::TEMPERATE,
        false, 10, -1
    },
    // Mission 10: Capture the Tech
    {
        10, "Capture the Tech",
        "Allied Chronosphere technology must be captured for the Motherland. "
        "Secure the research facility intact. Scientists are to be captured, "
        "not killed.",
        VQType::SOVIET10, VQType::SOVIET10, VQType::SOVIET11, VQType::SOVIET09,
        ThemeType::AWAIT, TheaterType::INTERIOR,
        true, 11, 11
    },
    // Mission 11: Absolute Power
    {
        11, "Absolute Power",
        "Our Iron Curtain device is nearly complete. Defend the construction "
        "site until the device is operational. Once activated, victory will "
        "be assured.",
        VQType::SOVIET11, VQType::SOVIET11, VQType::SOVIET12, VQType::SOVIET10,
        ThemeType::DENSE, TheaterType::SNOW,
        false, 12, -1
    },
    // Mission 12: Test of Faith
    {
        12, "Test of Faith",
        "The Allied command structure must be dismantled. Destroy their "
        "command center and all supporting structures. Leave nothing standing.",
        VQType::SOVIET12, VQType::SOVIET12, VQType::SOVIET13, VQType::SOVIET11,
        ThemeType::VECTOR, TheaterType::TEMPERATE,
        false, 13, -1
    },
    // Mission 13: Trapped
    {
        13, "Trapped",
        "Allied forces have surrounded our forward base. Break out of the "
        "encirclement and destroy the enemy forces. Retreat is not an option.",
        VQType::SOVIET13, VQType::SOVIET13, VQType::NONE, VQType::SOVIET12,
        ThemeType::TWIN_GUNS, TheaterType::SNOW,
        false, 14, -1
    },
    // Mission 14: Final Chapter (End of Campaign)
    {
        14, "Shock Therapy",
        "The time has come to crush the Allied command once and for all. "
        "Destroy their headquarters and all remaining forces. The world "
        "will tremble before the might of the Soviet Union!",
        VQType::NONE, VQType::NONE, VQType::MASTEFIN, VQType::SOVIET13,
        ThemeType::HELL_MARCH, TheaterType::TEMPERATE,
        false, -1, -1  // End of campaign
    }
};

//===========================================================================
// ScoreClass Implementation
//===========================================================================

ScoreClass::ScoreClass() {
    Reset();
}

void ScoreClass::Reset() {
    unitsKilled_ = 0;
    enemyUnitsKilled_ = 0;
    buildingsDestroyed_ = 0;
    enemyBuildingsDestroyed_ = 0;
    civiliansKilled_ = 0;
    oreHarvested_ = 0;
    enemyOreLost_ = 0;
    elapsedTime_ = 0;
}

void ScoreClass::Add_Unit_Kill(HousesType killedHouse) {
    // Determine if this was a player or enemy unit
    // For now, assume player is always Greece/Good
    if (killedHouse == HousesType::GREECE ||
        killedHouse == HousesType::ENGLAND ||
        killedHouse == HousesType::FRANCE ||
        killedHouse == HousesType::GERMANY ||
        killedHouse == HousesType::SPAIN ||
        killedHouse == HousesType::TURKEY ||
        killedHouse == HousesType::GOOD) {
        unitsKilled_++;
    } else if (killedHouse == HousesType::USSR ||
               killedHouse == HousesType::UKRAINE ||
               killedHouse == HousesType::BAD) {
        enemyUnitsKilled_++;
    }
}

void ScoreClass::Add_Building_Kill(HousesType killedHouse) {
    if (killedHouse == HousesType::GREECE ||
        killedHouse == HousesType::ENGLAND ||
        killedHouse == HousesType::FRANCE ||
        killedHouse == HousesType::GERMANY ||
        killedHouse == HousesType::SPAIN ||
        killedHouse == HousesType::TURKEY ||
        killedHouse == HousesType::GOOD) {
        buildingsDestroyed_++;
    } else if (killedHouse == HousesType::USSR ||
               killedHouse == HousesType::UKRAINE ||
               killedHouse == HousesType::BAD) {
        enemyBuildingsDestroyed_++;
    }
}

void ScoreClass::Add_Civilian_Kill() {
    civiliansKilled_++;
}

void ScoreClass::Add_Ore_Harvested(int amount) {
    oreHarvested_ += amount;
}

void ScoreClass::Add_Enemy_Ore_Lost(int amount) {
    enemyOreLost_ += amount;
}

void ScoreClass::Set_Elapsed_Time(int frames) {
    elapsedTime_ = frames;
}

int ScoreClass::Calculate_Score() const {
    // Score calculation formula (simplified from original):
    // - Enemy units killed: 50 points each
    // - Enemy buildings destroyed: 100 points each
    // - Ore harvested: 1 point per 100 credits
    // - Time bonus: 1000 points if under 30 minutes
    // - Civilian penalty: -100 points each
    // - Player losses: -25 points per unit, -50 per building

    int score = 0;

    score += enemyUnitsKilled_ * 50;
    score += enemyBuildingsDestroyed_ * 100;
    score += oreHarvested_ / 100;

    // Time bonus (15 FPS, so 30 minutes = 27000 frames)
    if (elapsedTime_ < 27000) {
        score += 1000;
    }

    score -= civiliansKilled_ * 100;
    score -= unitsKilled_ * 25;
    score -= buildingsDestroyed_ * 50;

    if (score < 0) score = 0;

    return score;
}

void ScoreClass::Presentation() {
    // Score screen would be displayed here
    // For now, just a placeholder
}

//===========================================================================
// CampaignClass Implementation
//===========================================================================

CampaignClass::CampaignClass() {
    Init();
}

void CampaignClass::Init() {
    currentCampaign_ = CampaignType::NONE;
    currentMission_ = 0;
    difficulty_ = DifficultyType::NORMAL;
    totalScore_ = 0;
    carryoverMoney_ = 0;
    chosenVariant_ = ScenarioVarType::A;

    for (int i = 0; i < 20; i++) {
        missionStates_[i] = MissionState::NOT_PLAYED;
    }

    for (int i = 0; i < 32; i++) {
        carryoverUnits_[i] = 0;
        carryoverBuildings_[i] = 0;
    }

    score_.Reset();
}

bool CampaignClass::Start_Campaign(CampaignType campaign,
                                   DifficultyType difficulty) {
    if (campaign == CampaignType::NONE || campaign >= CampaignType::COUNT) {
        return false;
    }

    Init();
    currentCampaign_ = campaign;
    currentMission_ = 1;
    difficulty_ = difficulty;

    // Mark first mission as available
    missionStates_[0] = MissionState::NOT_PLAYED;

    return Start_Mission(1);
}

int CampaignClass::Get_Mission_Count(CampaignType campaign) {
    switch (campaign) {
        case CampaignType::ALLIED:
            return ALLIED_MISSION_COUNT;
        case CampaignType::SOVIET:
            return SOVIET_MISSION_COUNT;
        case CampaignType::AFTERMATH:
            return AFTERMATH_MISSION_COUNT;
        case CampaignType::COUNTERSTRIKE:
            return COUNTERSTRIKE_MISSION_COUNT;
        default:
            return 0;
    }
}

bool CampaignClass::Start_Mission(int missionNum) {
    if (currentCampaign_ == CampaignType::NONE) {
        return false;
    }

    int maxMissions = Get_Mission_Count(currentCampaign_);
    if (missionNum < 1 || missionNum > maxMissions) {
        return false;
    }

    currentMission_ = missionNum;

    // Update mission state
    if (missionNum > 0 && missionNum <= 20) {
        missionStates_[missionNum - 1] = MissionState::IN_PROGRESS;
    }

    // Reset score for this mission
    score_.Reset();

    // Generate scenario filename
    char filename[64];
    ScenarioPlayerType player = (currentCampaign_ == CampaignType::SOVIET)
                                 ? ScenarioPlayerType::USSR
                                 : ScenarioPlayerType::GREECE;
    Make_Scenario_Name(filename, sizeof(filename), missionNum, player,
                       ScenarioDirType::EAST, chosenVariant_);

    // Set scenario name and load
    strncpy(Scen.name_, filename, sizeof(Scen.name_) - 1);
    Scen.scenario_ = missionNum;
    Scen.difficulty_ = difficulty_;

    // Load carry-over is called after mission starts (when GameUI is ready)
    // See main.mm which should call Campaign.Load_Carryover() after init

    return true;
}

void CampaignClass::Mission_Won() {
    if (currentCampaign_ == CampaignType::NONE) return;

    // Update mission state
    if (currentMission_ > 0 && currentMission_ <= 20) {
        missionStates_[currentMission_ - 1] = MissionState::COMPLETED;
    }

    // Calculate and add score
    score_.Set_Elapsed_Time(Scen.elapsedTime_);
    int missionScore = score_.Calculate_Score();
    totalScore_ += missionScore;

    // Save carry-over state
    Save_Carryover();

    // Check if campaign complete
    if (Is_Campaign_Complete()) {
        // Campaign ending would be handled here
        return;
    }

    // Advance to next mission
    int nextMission = Get_Next_Mission();
    if (nextMission > 0) {
        currentMission_ = nextMission;
        chosenVariant_ = ScenarioVarType::A;  // Reset variant choice
    }
}

void CampaignClass::Mission_Lost() {
    if (currentCampaign_ == CampaignType::NONE) return;

    // Update mission state
    if (currentMission_ > 0 && currentMission_ <= 20) {
        missionStates_[currentMission_ - 1] = MissionState::FAILED;
    }

    // Player can retry the mission
}

MissionState CampaignClass::Get_Mission_State(int missionNum) const {
    if (missionNum < 1 || missionNum > 20) {
        return MissionState::NOT_PLAYED;
    }
    return missionStates_[missionNum - 1];
}

bool CampaignClass::Is_Mission_Available(int missionNum) const {
    if (missionNum < 1 || missionNum > Get_Mission_Count(currentCampaign_)) {
        return false;
    }

    // First mission is always available
    if (missionNum == 1) return true;

    // Mission is available if previous mission was completed
    MissionState prevState = Get_Mission_State(missionNum - 1);
    return prevState == MissionState::COMPLETED;
}

const char* CampaignClass::Get_Briefing() const {
    return Get_Briefing(currentMission_);
}

const char* CampaignClass::Get_Briefing(int missionNum) const {
    const MissionData* data = Get_Mission_Data(missionNum);
    if (data) {
        return data->briefing;
    }
    return "Mission briefing unavailable.";
}

const MissionData* CampaignClass::Get_Mission_Data(int missionNum) const {
    const MissionData* table = Get_Mission_Table(currentCampaign_);
    if (!table) return nullptr;

    int maxMissions = Get_Mission_Count(currentCampaign_);
    if (missionNum < 1 || missionNum > maxMissions) {
        return nullptr;
    }

    return &table[missionNum - 1];
}

void CampaignClass::Play_Briefing() {
    const MissionData* data = Get_Mission_Data(currentMission_);
    if (!data) return;

    // Would play intro movie if set
    // Would display briefing text
    // Would play briefing movie if set
}

int CampaignClass::Get_Next_Mission() const {
    const MissionData* data = Get_Mission_Data(currentMission_);
    if (!data) return -1;

    // If mission has map choice and player chose variant B
    if (data->hasMapChoice && chosenVariant_ == ScenarioVarType::B) {
        return data->nextMissionB;
    }

    return data->nextMissionA;
}

void CampaignClass::Choose_Variant(ScenarioVarType variant) {
    chosenVariant_ = variant;
}

bool CampaignClass::Has_Map_Choice() const {
    const MissionData* data = Get_Mission_Data(currentMission_);
    if (!data) return false;
    return data->hasMapChoice;
}

void CampaignClass::Advance_Mission() {
    int nextMission = Get_Next_Mission();
    if (nextMission > 0) {
        Start_Mission(nextMission);
    }
}

bool CampaignClass::Is_Campaign_Complete() const {
    const MissionData* data = Get_Mission_Data(currentMission_);
    if (!data) return true;

    // Campaign is complete if current mission has no next mission and was won
    if (data->nextMissionA == -1) {
        MissionState state = Get_Mission_State(currentMission_);
        return state == MissionState::COMPLETED;
    }

    return false;
}

void CampaignClass::Show_Score_Screen() {
    score_.Presentation();
}

// Helper to count non-zero entries in an array
static int CountNonZero(const int* arr, int count) {
    int result = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i] > 0) result++;
    }
    return result;
}

void CampaignClass::Save_Carryover() {
    if (!Scen.isToCarryOver_) return;

    // Save current credits
    int currentCredits = GameUI_GetCredits();

    // Apply carryover percentage (default 100%)
    int percent = Scen.carryOverPercent_;
    if (percent <= 0) percent = 100;

    carryoverMoney_ = (currentCredits * percent) / 100;

    // Apply carryover cap if set
    if (Scen.carryOverCap_ > 0) {
        int maxCarryover = Scen.carryOverCap_ * 100;  // Cap is in hundreds
        if (carryoverMoney_ > maxCarryover) {
            carryoverMoney_ = maxCarryover;
        }
    }

    // Reset unit/building counts
    for (int i = 0; i < 32; i++) {
        carryoverUnits_[i] = 0;
        carryoverBuildings_[i] = 0;
    }

    // Count surviving player units by type
    for (int i = 0; i < MAX_UNITS; i++) {
        CampaignUnit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_PLAYER) continue;
        if (unit->state == STATE_DYING) continue;

        int type = unit->type;
        if (type >= 0 && type < 32) {
            carryoverUnits_[type]++;
        }
    }

    // Count surviving player buildings by type
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        CampaignBuilding* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_PLAYER) continue;

        int type = bld->type;
        if (type >= 0 && type < 32) {
            carryoverBuildings_[type]++;
        }
    }

    fprintf(stderr, "Save_Carryover: money=%d, %d unit types, %d building types\n",
            carryoverMoney_,
            CountNonZero(carryoverUnits_, 32),
            CountNonZero(carryoverBuildings_, 32));
}

void CampaignClass::Load_Carryover() {
    if (!Scen.isToInherit_) return;

    // Add carryover money to starting credits
    if (carryoverMoney_ > 0) {
        GameUI_AddCredits(carryoverMoney_);
        fprintf(stderr, "Load_Carryover: Added %d credits (now %d)\n",
                carryoverMoney_, GameUI_GetCredits());
    }

    // Note: Unit/building spawning from carryover would require
    // integration with mission loading. The original game spawns
    // carryover units at specific waypoints or near the construction
    // yard. For now we just carry over money, which is the most
    // impactful part of the carryover system.
}

const MissionData* CampaignClass::Get_Mission_Table(
    CampaignType campaign) const {
    switch (campaign) {
        case CampaignType::ALLIED:
            return AlliedMissions;
        case CampaignType::SOVIET:
            return SovietMissions;
        default:
            return nullptr;
    }
}

//===========================================================================
// Scenario Filename Generation
//===========================================================================

void CampaignClass::Make_Scenario_Name(
    char* buffer, int bufSize,
    int scenario,
    ScenarioPlayerType player,
    ScenarioDirType dir,
    ScenarioVarType var)
{
    if (!buffer || bufSize < 16) return;

    // Player character
    char playerChar;
    switch (player) {
        case ScenarioPlayerType::USSR:    playerChar = 'U'; break;
        case ScenarioPlayerType::SPAIN:   playerChar = 'S'; break;
        case ScenarioPlayerType::JAPAN:   playerChar = 'J'; break;
        case ScenarioPlayerType::MULTI:   playerChar = 'M'; break;
        default:                          playerChar = 'G'; break;  // Greece
    }

    // Direction character
    char dirChar = (dir == ScenarioDirType::WEST) ? 'W' : 'E';

    // Variation character
    char varChar;
    switch (var) {
        case ScenarioVarType::B: varChar = 'B'; break;
        case ScenarioVarType::C: varChar = 'C'; break;
        case ScenarioVarType::D: varChar = 'D'; break;
        default:                 varChar = 'A'; break;
    }

    // Format: SC<player><nn><dir><var>.INI
    if (scenario < 100) {
        snprintf(buffer, bufSize, "SC%c%02d%c%c.INI",
                 playerChar, scenario, dirChar, varChar);
    } else {
        // Extended format for expansion missions
        int first = scenario / 36;
        int second = scenario % 36;
        char firstChar = (first < 10) ? ('0' + first) : ('A' + first - 10);
        char secondChar = (second < 10) ? ('0' + second) : ('A' + second - 10);
        snprintf(buffer, bufSize, "SC%c%c%c%c%c.INI",
                 playerChar, firstChar, secondChar, dirChar, varChar);
    }
}

bool CampaignClass::Parse_Scenario_Name(
    const char* name,
    int& scenario,
    ScenarioPlayerType& player,
    ScenarioDirType& dir,
    ScenarioVarType& var)
{
    if (!name || strlen(name) < 8) return false;

    // Verify prefix
    if (name[0] != 'S' || name[1] != 'C') return false;

    // Parse player
    switch (name[2]) {
        case 'G': case 'g': player = ScenarioPlayerType::GREECE; break;
        case 'U': case 'u': player = ScenarioPlayerType::USSR; break;
        case 'S': case 's': player = ScenarioPlayerType::SPAIN; break;
        case 'J': case 'j': player = ScenarioPlayerType::JAPAN; break;
        case 'M': case 'm': player = ScenarioPlayerType::MULTI; break;
        default: return false;
    }

    // Parse scenario number (2 digits)
    if (name[3] >= '0' && name[3] <= '9' &&
        name[4] >= '0' && name[4] <= '9') {
        scenario = (name[3] - '0') * 10 + (name[4] - '0');
    } else {
        // Extended format
        int first = 0, second = 0;
        if (name[3] >= '0' && name[3] <= '9') first = name[3] - '0';
        else if (name[3] >= 'A' && name[3] <= 'Z') first = name[3] - 'A' + 10;
        else return false;

        if (name[4] >= '0' && name[4] <= '9') second = name[4] - '0';
        else if (name[4] >= 'A' && name[4] <= 'Z') second = name[4] - 'A' + 10;
        else return false;

        scenario = first * 36 + second;
    }

    // Parse direction
    switch (name[5]) {
        case 'E': case 'e': dir = ScenarioDirType::EAST; break;
        case 'W': case 'w': dir = ScenarioDirType::WEST; break;
        default: dir = ScenarioDirType::EAST; break;
    }

    // Parse variation
    switch (name[6]) {
        case 'A': case 'a': var = ScenarioVarType::A; break;
        case 'B': case 'b': var = ScenarioVarType::B; break;
        case 'C': case 'c': var = ScenarioVarType::C; break;
        case 'D': case 'd': var = ScenarioVarType::D; break;
        default: var = ScenarioVarType::A; break;
    }

    return true;
}

//===========================================================================
// Campaign Progress Persistence
//===========================================================================

bool CampaignClass::Save_Progress(const char* filename) const {
    if (!filename) return false;

    FILE* fp = fopen(filename, "wb");
    if (!fp) return false;

    // Write header
    uint32_t magic = 0x43414D50;  // "CAMP"
    uint32_t version = 1;
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);

    // Write campaign state
    int8_t campaign = static_cast<int8_t>(currentCampaign_);
    int8_t diff = static_cast<int8_t>(difficulty_);
    fwrite(&campaign, sizeof(campaign), 1, fp);
    fwrite(&currentMission_, sizeof(currentMission_), 1, fp);
    fwrite(&diff, sizeof(diff), 1, fp);
    fwrite(&totalScore_, sizeof(totalScore_), 1, fp);

    // Write mission states
    fwrite(missionStates_, sizeof(missionStates_), 1, fp);

    // Write carryover
    fwrite(&carryoverMoney_, sizeof(carryoverMoney_), 1, fp);

    fclose(fp);
    return true;
}

bool CampaignClass::Load_Progress(const char* filename) {
    if (!filename) return false;

    FILE* fp = fopen(filename, "rb");
    if (!fp) return false;

    // Read and verify header
    uint32_t magic, version;
    fread(&magic, sizeof(magic), 1, fp);
    fread(&version, sizeof(version), 1, fp);

    if (magic != 0x43414D50 || version != 1) {
        fclose(fp);
        return false;
    }

    // Read campaign state
    int8_t campaign, diff;
    fread(&campaign, sizeof(campaign), 1, fp);
    fread(&currentMission_, sizeof(currentMission_), 1, fp);
    fread(&diff, sizeof(diff), 1, fp);
    fread(&totalScore_, sizeof(totalScore_), 1, fp);

    currentCampaign_ = static_cast<CampaignType>(campaign);
    difficulty_ = static_cast<DifficultyType>(diff);

    // Read mission states
    fread(missionStates_, sizeof(missionStates_), 1, fp);

    // Read carryover
    fread(&carryoverMoney_, sizeof(carryoverMoney_), 1, fp);

    fclose(fp);
    return true;
}

//===========================================================================
// C-style Wrapper for main.mm (avoids header conflicts)
//===========================================================================

extern "C" void Campaign_Load_Carryover(void) {
    Campaign.Load_Carryover();
}

extern "C" bool Campaign_Is_Active(void) {
    return Campaign.Is_Campaign_Active();
}

extern "C" bool Campaign_Has_Map_Choice(void) {
    return Campaign.Has_Map_Choice();
}

extern "C" void Campaign_Choose_Variant(int variant) {
    ScenarioVarType var = (variant == 0) ? ScenarioVarType::A : ScenarioVarType::B;
    Campaign.Choose_Variant(var);
}

extern "C" void Campaign_Mission_Won(void) {
    Campaign.Mission_Won();
}

extern "C" int Campaign_Get_Current_Mission(void) {
    return Campaign.Get_Current_Mission();
}

extern "C" bool Campaign_Is_Complete(void) {
    return Campaign.Is_Campaign_Complete();
}

extern "C" int Campaign_Get_Type(void) {
    CampaignType ct = Campaign.Get_Campaign();
    return (ct == CampaignType::SOVIET) ? 1 : 0;
}

extern "C" const char* Campaign_Get_Next_Mission_Name(void) {
    int nextMission = Campaign.Get_Current_Mission();
    static char buf[64];
    CampaignType ct = Campaign.Get_Campaign();
    bool isSoviet = (ct == CampaignType::SOVIET);
    snprintf(buf, sizeof(buf), "%s Mission %d",
             isSoviet ? "Soviet" : "Allied", nextMission);
    return buf;
}

extern "C" int Campaign_Get_Score_UnitsLost(void) {
    return Campaign.Score().Units_Killed();
}

extern "C" int Campaign_Get_Score_EnemyUnitsKilled(void) {
    return Campaign.Score().Enemy_Units_Killed();
}

extern "C" int Campaign_Get_Score_BuildingsLost(void) {
    return Campaign.Score().Buildings_Destroyed();
}

extern "C" int Campaign_Get_Score_EnemyBuildingsKilled(void) {
    return Campaign.Score().Enemy_Buildings_Destroyed();
}

extern "C" int Campaign_Get_Score_CiviliansKilled(void) {
    return Campaign.Score().Civilians_Killed();
}

extern "C" int Campaign_Get_Score_OreHarvested(void) {
    return Campaign.Score().Ore_Harvested();
}

extern "C" int Campaign_Get_Score_ElapsedTime(void) {
    return Campaign.Score().Elapsed_Time();
}

extern "C" int Campaign_Get_Score_MissionScore(void) {
    return Campaign.Score().Calculate_Score();
}

extern "C" int Campaign_Get_Total_Score(void) {
    return Campaign.Get_Total_Score();
}

extern "C" void Campaign_Reset_Score(void) {
    Campaign.Score().Reset();
}
