/**
 * Red Alert macOS Port - Save/Load System Implementation
 *
 * Implements game state serialization using binary format with
 * MD5 checksum validation.
 */

#include "saveload.h"
#include "scenario.h"
#include "house.h"
#include "mapclass.h"
#include "cell.h"
#include "infantry.h"
#include "unit.h"
#include "building.h"
#include "aircraft.h"
#include "bullet.h"
#include "trigger.h"
#include "team.h"
#include "factory.h"
#include <cstring>
#include <cstdlib>
#include <CommonCrypto/CommonDigest.h>

//===========================================================================
// External References
//===========================================================================

extern ScenarioClass Scen;
extern HouseClass Houses[HOUSE_MAX];
extern HouseClass* PlayerPtr;
extern int HouseCount;
extern MapClass Map;

// Frame counter
extern uint32_t Frame;

//===========================================================================
// Save File Path
//===========================================================================

static const char* GetSaveDirectory() {
    static char path[512] = {0};
    if (path[0] == 0) {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(path, sizeof(path), "%s/Library/Application Support/RedAlert/saves", home);
        } else {
            snprintf(path, sizeof(path), "saves");
        }
    }
    return path;
}

void Get_Save_Filename(int slot, char* buffer, int bufferSize) {
    if (slot < 0 || slot >= SAVE_SLOT_MAX) {
        slot = 0;
    }
    snprintf(buffer, bufferSize, "%s/SAVEGAME.%03d", GetSaveDirectory(), slot);
}

//===========================================================================
// SaveStream Implementation
//===========================================================================

SaveStream::SaveStream()
    : file_(nullptr), bytesWritten_(0) {
}

SaveStream::~SaveStream() {
    Close();
}

bool SaveStream::Open(const char* filename) {
    Close();

    // Ensure directory exists
    char dirPath[512];
    strncpy(dirPath, filename, sizeof(dirPath) - 1);
    dirPath[sizeof(dirPath) - 1] = '\0';

    // Find last separator and create directory
    char* lastSlash = strrchr(dirPath, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        // Create directory recursively (simple version)
        char cmd[600];
        snprintf(cmd, sizeof(cmd), "mkdir -p '%s'", dirPath);
        system(cmd);
    }

    file_ = fopen(filename, "wb");
    if (!file_) {
        return false;
    }

    bytesWritten_ = 0;
    buffer_.clear();
    return true;
}

void SaveStream::Close() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
    bytesWritten_ = 0;
}

bool SaveStream::Write(const void* data, size_t size) {
    if (!file_ || !data || size == 0) {
        return false;
    }

    size_t written = fwrite(data, 1, size, file_);
    if (written != size) {
        return false;
    }

    // Also store in buffer for checksum calculation
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    buffer_.insert(buffer_.end(), bytes, bytes + size);

    bytesWritten_ += size;
    return true;
}

bool SaveStream::WriteBool(bool value) {
    uint8_t v = value ? 1 : 0;
    return Write(&v, 1);
}

bool SaveStream::WriteString(const char* str, int maxLen) {
    char buffer[256];
    memset(buffer, 0, maxLen);
    if (str) {
        strncpy(buffer, str, maxLen - 1);
    }
    return Write(buffer, maxLen);
}

bool SaveStream::WriteObjectID(RTTIType type, int id) {
    WriteInt8(static_cast<int8_t>(type));
    return WriteInt16(static_cast<int16_t>(id));
}

void SaveStream::CalculateChecksum(uint8_t* outChecksum) {
    CC_MD5(buffer_.data(), static_cast<CC_LONG>(buffer_.size()), outChecksum);
}

//===========================================================================
// LoadStream Implementation
//===========================================================================

LoadStream::LoadStream()
    : file_(nullptr), bytesRead_(0), hasError_(false) {
}

LoadStream::~LoadStream() {
    Close();
}

bool LoadStream::Open(const char* filename) {
    Close();

    file_ = fopen(filename, "rb");
    if (!file_) {
        return false;
    }

    bytesRead_ = 0;
    hasError_ = false;
    return true;
}

void LoadStream::Close() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
    bytesRead_ = 0;
    hasError_ = false;
}

bool LoadStream::Read(void* data, size_t size) {
    if (!file_ || !data || size == 0) {
        hasError_ = true;
        return false;
    }

    size_t read = fread(data, 1, size, file_);
    if (read != size) {
        hasError_ = true;
        return false;
    }

    bytesRead_ += size;
    return true;
}

int8_t LoadStream::ReadInt8() {
    int8_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

int16_t LoadStream::ReadInt16() {
    int16_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

int32_t LoadStream::ReadInt32() {
    int32_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

uint8_t LoadStream::ReadUInt8() {
    uint8_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

uint16_t LoadStream::ReadUInt16() {
    uint16_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

uint32_t LoadStream::ReadUInt32() {
    uint32_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

uint64_t LoadStream::ReadUInt64() {
    uint64_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

int64_t LoadStream::ReadInt64() {
    int64_t value = 0;
    Read(&value, sizeof(value));
    return value;
}

bool LoadStream::ReadBool() {
    uint8_t value = 0;
    Read(&value, 1);
    return value != 0;
}

float LoadStream::ReadFloat() {
    float value = 0;
    Read(&value, sizeof(value));
    return value;
}

bool LoadStream::ReadString(char* str, int maxLen) {
    return Read(str, maxLen);
}

bool LoadStream::ReadObjectID(RTTIType& type, int& id) {
    type = static_cast<RTTIType>(ReadInt8());
    id = ReadInt16();
    return !hasError_;
}

void LoadStream::CalculateChecksum(size_t dataSize, uint8_t* outChecksum) {
    if (!file_) {
        memset(outChecksum, 0, 16);
        return;
    }

    // Save position
    long pos = ftell(file_);

    // Read data and calculate checksum
    std::vector<uint8_t> buffer(dataSize);
    fread(buffer.data(), 1, dataSize, file_);
    CC_MD5(buffer.data(), static_cast<CC_LONG>(dataSize), outChecksum);

    // Restore position
    fseek(file_, pos, SEEK_SET);
}

//===========================================================================
// Save/Load Entry Points
//===========================================================================

bool Save_Game(int slot, const char* description) {
    if (slot < 0 || slot >= SAVE_SLOT_MAX) {
        return false;
    }

    char filename[SAVE_FILENAME_MAX];
    Get_Save_Filename(slot, filename, sizeof(filename));

    SaveStream stream;
    if (!stream.Open(filename)) {
        return false;
    }

    // Prepare header
    SaveGameHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = SAVE_MAGIC;
    header.version = SAVEGAME_VERSION;
    header.scenario = Scen.scenario_;
    header.house = static_cast<int32_t>(Scen.playerHouse_);

    if (description) {
        strncpy(header.description, description, SAVE_DESCRIP_MAX - 1);
    }

    // Write placeholder header (checksum will be updated later)
    if (!stream.Write(&header, sizeof(header))) {
        stream.Close();
        return false;
    }

    // Encode all object pointers to IDs
    Code_All_Pointers();

    // Write all game data
    if (!Put_All(stream)) {
        Decode_All_Pointers();  // Restore pointers
        stream.Close();
        return false;
    }

    // Decode pointers back to usable state
    Decode_All_Pointers();

    // Calculate checksum of data (excluding header)
    stream.CalculateChecksum(header.checksum);

    // Rewrite header with correct checksum
    stream.Close();

    // Reopen to update header
    FILE* f = fopen(filename, "r+b");
    if (f) {
        fseek(f, 0, SEEK_SET);
        fwrite(&header, sizeof(header), 1, f);
        fclose(f);
    }

    return true;
}

bool Load_Game(int slot) {
    if (slot < 0 || slot >= SAVE_SLOT_MAX) {
        return false;
    }

    char filename[SAVE_FILENAME_MAX];
    Get_Save_Filename(slot, filename, sizeof(filename));

    LoadStream stream;
    if (!stream.Open(filename)) {
        return false;
    }

    // Read header
    SaveGameHeader header;
    if (!stream.Read(&header, sizeof(header))) {
        stream.Close();
        return false;
    }

    // Validate magic
    if (header.magic != SAVE_MAGIC) {
        stream.Close();
        return false;
    }

    // Validate version
    if (header.version != SAVEGAME_VERSION) {
        stream.Close();
        return false;
    }

    // TODO: Verify checksum (would need to read all data twice)

    // Clear current game state
    // Note: In full implementation, this would call Clear_Scenario()

    // Load all game data
    if (!Get_All(stream)) {
        stream.Close();
        return false;
    }

    // Decode all object IDs back to pointers
    Decode_All_Pointers();

    stream.Close();
    return true;
}

bool Get_Save_Info(int slot, SaveGameHeader* header) {
    if (!header || slot < 0 || slot >= SAVE_SLOT_MAX) {
        return false;
    }

    char filename[SAVE_FILENAME_MAX];
    Get_Save_Filename(slot, filename, sizeof(filename));

    FILE* f = fopen(filename, "rb");
    if (!f) {
        return false;
    }

    size_t read = fread(header, 1, sizeof(SaveGameHeader), f);
    fclose(f);

    if (read != sizeof(SaveGameHeader)) {
        return false;
    }

    // Validate magic
    if (header->magic != SAVE_MAGIC) {
        return false;
    }

    return true;
}

bool Save_Exists(int slot) {
    SaveGameHeader header;
    return Get_Save_Info(slot, &header);
}

bool Delete_Save(int slot) {
    if (slot < 0 || slot >= SAVE_SLOT_MAX) {
        return false;
    }

    char filename[SAVE_FILENAME_MAX];
    Get_Save_Filename(slot, filename, sizeof(filename));

    return remove(filename) == 0;
}

//===========================================================================
// Put_All - Save all game state
//===========================================================================

bool Put_All(SaveStream& stream) {
    // Save in specific order for proper loading

    // 1. Scenario
    if (!Save_Scenario(stream)) return false;

    // 2. Houses (needed by objects)
    if (!Save_Houses(stream)) return false;

    // 3. Map/Cells
    if (!Save_Map(stream)) return false;

    // 4. Triggers (needed by objects)
    if (!Save_Triggers(stream)) return false;

    // 5. Teams
    if (!Save_Teams(stream)) return false;

    // 6. Game objects
    if (!Save_Infantry(stream)) return false;
    if (!Save_Units(stream)) return false;
    if (!Save_Buildings(stream)) return false;
    if (!Save_Aircraft(stream)) return false;
    if (!Save_Bullets(stream)) return false;

    // 7. Factories/Production
    if (!Save_Factories(stream)) return false;

    // 8. Misc values
    if (!Save_Misc_Values(stream)) return false;

    return true;
}

//===========================================================================
// Get_All - Load all game state
//===========================================================================

bool Get_All(LoadStream& stream) {
    // Load in same order as save

    // 1. Scenario
    if (!Load_Scenario(stream)) return false;

    // 2. Houses
    if (!Load_Houses(stream)) return false;

    // 3. Map/Cells
    if (!Load_Map(stream)) return false;

    // 4. Triggers
    if (!Load_Triggers(stream)) return false;

    // 5. Teams
    if (!Load_Teams(stream)) return false;

    // 6. Game objects
    if (!Load_Infantry(stream)) return false;
    if (!Load_Units(stream)) return false;
    if (!Load_Buildings(stream)) return false;
    if (!Load_Aircraft(stream)) return false;
    if (!Load_Bullets(stream)) return false;

    // 7. Factories
    if (!Load_Factories(stream)) return false;

    // 8. Misc values
    if (!Load_Misc_Values(stream)) return false;

    return true;
}

//===========================================================================
// Pointer Encoding/Decoding
//===========================================================================

void Code_All_Pointers() {
    // In full implementation, this would:
    // 1. Convert object pointers to TARGET IDs
    // 2. Convert house pointers to house type indices
    // 3. Convert trigger/team pointers to indices

    // For now, this is a placeholder
    // Objects store their references as IDs already in most cases
}

void Decode_All_Pointers() {
    // In full implementation, this would:
    // 1. Convert TARGET IDs back to object pointers
    // 2. Convert house type indices back to house pointers
    // 3. Convert trigger/team indices back to pointers

    // For now, this is a placeholder
}

//===========================================================================
// Scenario Save/Load
//===========================================================================

bool Save_Scenario(SaveStream& stream) {
    stream.WriteInt32(Scen.scenario_);
    stream.WriteInt8(static_cast<int8_t>(Scen.theater_));
    stream.WriteString(Scen.name_, SCENARIO_NAME_MAX);
    stream.WriteString(Scen.description_, DESCRIPTION_MAX);

    // Movies
    stream.WriteInt8(static_cast<int8_t>(Scen.introMovie_));
    stream.WriteInt8(static_cast<int8_t>(Scen.briefMovie_));
    stream.WriteInt8(static_cast<int8_t>(Scen.winMovie_));
    stream.WriteInt8(static_cast<int8_t>(Scen.loseMovie_));
    stream.WriteInt8(static_cast<int8_t>(Scen.actionMovie_));

    // Music
    stream.WriteInt8(static_cast<int8_t>(Scen.theme_));

    // Timers
    stream.WriteInt32(Scen.elapsedTime_);
    stream.WriteInt32(Scen.missionTimer_);
    stream.WriteInt32(Scen.shroudTimer_);

    // Player
    stream.WriteInt8(static_cast<int8_t>(Scen.playerHouse_));
    stream.WriteInt8(static_cast<int8_t>(Scen.difficulty_));
    stream.WriteInt8(static_cast<int8_t>(Scen.computerDifficulty_));

    // Financial
    stream.WriteInt32(Scen.carryOverMoney_);
    stream.WriteInt32(Scen.carryOverCap_);
    stream.WriteInt32(Scen.carryOverPercent_);
    stream.WriteInt32(Scen.buildPercent_);

    // Waypoints
    for (int i = 0; i < WAYPT_COUNT; i++) {
        stream.WriteInt16(Scen.waypoints_[i]);
    }

    // Global flags
    for (int i = 0; i < GLOBAL_FLAG_COUNT; i++) {
        stream.WriteBool(Scen.globalFlags_[i]);
    }

    // Flags (packed as uint16)
    uint16_t flags = 0;
    if (Scen.isToCarryOver_) flags |= 0x0001;
    if (Scen.isToInherit_) flags |= 0x0002;
    if (Scen.isInheritTimer_) flags |= 0x0004;
    if (Scen.isEndOfGame_) flags |= 0x0008;
    if (Scen.isOneTimeOnly_) flags |= 0x0010;
    if (Scen.isNoMapSel_) flags |= 0x0020;
    if (Scen.isTanyaEvac_) flags |= 0x0040;
    if (Scen.isSkipScore_) flags |= 0x0080;
    if (Scen.isNoSpyPlane_) flags |= 0x0100;
    if (Scen.isTruckCrate_) flags |= 0x0200;
    if (Scen.isMoneyTiberium_) flags |= 0x0400;
    if (Scen.isBridgeDestroyed_) flags |= 0x0800;
    if (Scen.isVariant_) flags |= 0x1000;
    stream.WriteUInt16(flags);

    return !stream.IsOpen() || true;  // No error check on stream
}

bool Load_Scenario(LoadStream& stream) {
    Scen.scenario_ = stream.ReadInt32();
    Scen.theater_ = static_cast<TheaterType>(stream.ReadInt8());
    stream.ReadString(Scen.name_, SCENARIO_NAME_MAX);
    stream.ReadString(Scen.description_, DESCRIPTION_MAX);

    // Movies
    Scen.introMovie_ = static_cast<VQType>(stream.ReadInt8());
    Scen.briefMovie_ = static_cast<VQType>(stream.ReadInt8());
    Scen.winMovie_ = static_cast<VQType>(stream.ReadInt8());
    Scen.loseMovie_ = static_cast<VQType>(stream.ReadInt8());
    Scen.actionMovie_ = static_cast<VQType>(stream.ReadInt8());

    // Music
    Scen.theme_ = static_cast<ThemeType>(stream.ReadInt8());

    // Timers
    Scen.elapsedTime_ = stream.ReadInt32();
    Scen.missionTimer_ = stream.ReadInt32();
    Scen.shroudTimer_ = stream.ReadInt32();

    // Player
    Scen.playerHouse_ = static_cast<HousesType>(stream.ReadInt8());
    Scen.difficulty_ = static_cast<DifficultyType>(stream.ReadInt8());
    Scen.computerDifficulty_ = static_cast<DifficultyType>(stream.ReadInt8());

    // Financial
    Scen.carryOverMoney_ = stream.ReadInt32();
    Scen.carryOverCap_ = stream.ReadInt32();
    Scen.carryOverPercent_ = stream.ReadInt32();
    Scen.buildPercent_ = stream.ReadInt32();

    // Waypoints
    for (int i = 0; i < WAYPT_COUNT; i++) {
        Scen.waypoints_[i] = stream.ReadInt16();
    }

    // Global flags
    for (int i = 0; i < GLOBAL_FLAG_COUNT; i++) {
        Scen.globalFlags_[i] = stream.ReadBool();
    }

    // Flags
    uint16_t flags = stream.ReadUInt16();
    Scen.isToCarryOver_ = (flags & 0x0001) != 0;
    Scen.isToInherit_ = (flags & 0x0002) != 0;
    Scen.isInheritTimer_ = (flags & 0x0004) != 0;
    Scen.isEndOfGame_ = (flags & 0x0008) != 0;
    Scen.isOneTimeOnly_ = (flags & 0x0010) != 0;
    Scen.isNoMapSel_ = (flags & 0x0020) != 0;
    Scen.isTanyaEvac_ = (flags & 0x0040) != 0;
    Scen.isSkipScore_ = (flags & 0x0080) != 0;
    Scen.isNoSpyPlane_ = (flags & 0x0100) != 0;
    Scen.isTruckCrate_ = (flags & 0x0200) != 0;
    Scen.isMoneyTiberium_ = (flags & 0x0400) != 0;
    Scen.isBridgeDestroyed_ = (flags & 0x0800) != 0;
    Scen.isVariant_ = (flags & 0x1000) != 0;

    return !stream.HasError();
}

//===========================================================================
// Houses Save/Load
//===========================================================================

bool Save_Houses(SaveStream& stream) {
    // Write house count
    stream.WriteInt32(HouseCount);

    // Write player house index
    int playerIndex = -1;
    if (PlayerPtr) {
        for (int i = 0; i < HOUSE_MAX; i++) {
            if (&Houses[i] == PlayerPtr) {
                playerIndex = i;
                break;
            }
        }
    }
    stream.WriteInt32(playerIndex);

    // Write each house
    for (int i = 0; i < HOUSE_MAX; i++) {
        HouseClass& h = Houses[i];

        // Identity
        stream.WriteInt8(static_cast<int8_t>(h.type_));
        stream.WriteInt16(h.id_);
        stream.WriteBool(h.isActive_);
        stream.WriteBool(h.isHuman_);
        stream.WriteBool(h.isPlayerControl_);
        stream.WriteBool(h.isDefeated_);
        stream.WriteBool(h.isToWin_);
        stream.WriteBool(h.isToLose_);
        stream.WriteBool(h.isAlerted_);
        stream.WriteBool(h.isDiscovered_);
        stream.WriteBool(h.isMaxedOut_);

        // Alliances
        stream.WriteUInt32(h.allies_);

        // Resources
        stream.WriteInt32(h.credits_);
        stream.WriteInt32(h.tiberium_);
        stream.WriteInt32(h.capacity_);
        stream.WriteInt32(h.drain_);
        stream.WriteInt32(h.power_);

        // Stats
        stream.WriteInt32(h.bKilled_);
        stream.WriteInt32(h.uKilled_);
        stream.WriteInt32(h.iKilled_);
        stream.WriteInt32(h.aKilled_);
        stream.WriteInt32(h.bLost_);
        stream.WriteInt32(h.uLost_);
        stream.WriteInt32(h.iLost_);
        stream.WriteInt32(h.aLost_);
        stream.WriteInt32(h.bBuilt_);
        stream.WriteInt32(h.uBuilt_);
        stream.WriteInt32(h.iBuilt_);
        stream.WriteInt32(h.aBuilt_);
        stream.WriteInt32(h.harvested_);

        // Scans
        stream.WriteUInt64(h.bScan_);
        stream.WriteUInt64(h.uScan_);
        stream.WriteUInt64(h.iScan_);
        stream.WriteUInt64(h.aScan_);
        stream.WriteUInt64(h.vScan_);

        // AI
        stream.WriteInt8(static_cast<int8_t>(h.difficulty_));
        stream.WriteInt8(static_cast<int8_t>(h.state_));
        stream.WriteInt16(h.alertTimer_);
        stream.WriteInt16(h.aiTimer_);

        // Urgency levels
        for (int j = 0; j < static_cast<int>(StrategyType::COUNT); j++) {
            stream.WriteInt8(static_cast<int8_t>(h.urgency_[j]));
        }

        // Build suggestions
        stream.WriteInt8(h.buildBuilding_);
        stream.WriteInt8(h.buildUnit_);
        stream.WriteInt8(h.buildInfantry_);
        stream.WriteInt8(h.buildAircraft_);

        // Attack
        stream.WriteInt8(static_cast<int8_t>(h.enemy_));
        stream.WriteInt8(static_cast<int8_t>(h.lastAttacker_));
        stream.WriteInt32(h.lastAttackFrame_);

        // Base
        stream.WriteInt32(h.baseCenter_);
        stream.WriteInt16(h.baseRadius_);
    }

    return true;
}

bool Load_Houses(LoadStream& stream) {
    // Read house count
    HouseCount = stream.ReadInt32();

    // Read player house index
    int playerIndex = stream.ReadInt32();

    // Read each house
    for (int i = 0; i < HOUSE_MAX; i++) {
        HouseClass& h = Houses[i];

        // Identity
        h.type_ = static_cast<HousesType>(stream.ReadInt8());
        h.id_ = stream.ReadInt16();
        h.isActive_ = stream.ReadBool();
        h.isHuman_ = stream.ReadBool();
        h.isPlayerControl_ = stream.ReadBool();
        h.isDefeated_ = stream.ReadBool();
        h.isToWin_ = stream.ReadBool();
        h.isToLose_ = stream.ReadBool();
        h.isAlerted_ = stream.ReadBool();
        h.isDiscovered_ = stream.ReadBool();
        h.isMaxedOut_ = stream.ReadBool();

        // Alliances
        h.allies_ = stream.ReadUInt32();

        // Resources
        h.credits_ = stream.ReadInt32();
        h.tiberium_ = stream.ReadInt32();
        h.capacity_ = stream.ReadInt32();
        h.drain_ = stream.ReadInt32();
        h.power_ = stream.ReadInt32();

        // Stats
        h.bKilled_ = stream.ReadInt32();
        h.uKilled_ = stream.ReadInt32();
        h.iKilled_ = stream.ReadInt32();
        h.aKilled_ = stream.ReadInt32();
        h.bLost_ = stream.ReadInt32();
        h.uLost_ = stream.ReadInt32();
        h.iLost_ = stream.ReadInt32();
        h.aLost_ = stream.ReadInt32();
        h.bBuilt_ = stream.ReadInt32();
        h.uBuilt_ = stream.ReadInt32();
        h.iBuilt_ = stream.ReadInt32();
        h.aBuilt_ = stream.ReadInt32();
        h.harvested_ = stream.ReadInt32();

        // Scans
        h.bScan_ = stream.ReadUInt64();
        h.uScan_ = stream.ReadUInt64();
        h.iScan_ = stream.ReadUInt64();
        h.aScan_ = stream.ReadUInt64();
        h.vScan_ = stream.ReadUInt64();

        // AI
        h.difficulty_ = static_cast<DifficultyType>(stream.ReadInt8());
        h.state_ = static_cast<HouseStateType>(stream.ReadInt8());
        h.alertTimer_ = stream.ReadInt16();
        h.aiTimer_ = stream.ReadInt16();

        // Urgency levels
        for (int j = 0; j < static_cast<int>(StrategyType::COUNT); j++) {
            h.urgency_[j] = static_cast<UrgencyType>(stream.ReadInt8());
        }

        // Build suggestions
        h.buildBuilding_ = stream.ReadInt8();
        h.buildUnit_ = stream.ReadInt8();
        h.buildInfantry_ = stream.ReadInt8();
        h.buildAircraft_ = stream.ReadInt8();

        // Attack
        h.enemy_ = static_cast<HousesType>(stream.ReadInt8());
        h.lastAttacker_ = static_cast<HousesType>(stream.ReadInt8());
        h.lastAttackFrame_ = stream.ReadInt32();

        // Base
        h.baseCenter_ = stream.ReadInt32();
        h.baseRadius_ = stream.ReadInt16();
    }

    // Restore player pointer
    if (playerIndex >= 0 && playerIndex < HOUSE_MAX) {
        PlayerPtr = &Houses[playerIndex];
    } else {
        PlayerPtr = nullptr;
    }

    return !stream.HasError();
}

//===========================================================================
// Map Save/Load
//===========================================================================

bool Save_Map(SaveStream& stream) {
    // Map dimensions
    stream.WriteInt32(Map.MapCellX());
    stream.WriteInt32(Map.MapCellY());
    stream.WriteInt32(Map.MapCellWidth());
    stream.WriteInt32(Map.MapCellHeight());

    // Save each cell
    for (int i = 0; i < MAP_CELL_TOTAL; i++) {
        const CellClass& cell = Map[i];

        // Basic cell data
        stream.WriteInt8(static_cast<int8_t>(cell.GetLandType()));
        stream.WriteInt8(static_cast<int8_t>(cell.overlay_));
        stream.WriteUInt8(cell.overlayData_);

        // Flags
        uint8_t flags = 0;
        if (cell.IsVisible()) flags |= 0x01;
        if (cell.IsMapped()) flags |= 0x02;
        // Waypoint flag not tracked per-cell, skip
        stream.WriteUInt8(flags);
    }

    return true;
}

bool Load_Map(LoadStream& stream) {
    // Map dimensions
    int mapX = stream.ReadInt32();
    int mapY = stream.ReadInt32();
    int mapW = stream.ReadInt32();
    int mapH = stream.ReadInt32();

    Map.SetMapDimensions(mapX, mapY, mapW, mapH);

    // Load each cell
    for (int i = 0; i < MAP_CELL_TOTAL; i++) {
        CellClass& cell = Map[i];

        // Basic cell data
        LandType land = static_cast<LandType>(stream.ReadInt8());
        OverlayType overlay = static_cast<OverlayType>(stream.ReadInt8());
        uint8_t overlayData = stream.ReadUInt8();

        cell.land_ = land;
        cell.SetOverlay(overlay, overlayData);

        // Flags
        uint8_t flags = stream.ReadUInt8();
        if (flags & 0x01) cell.SetVisible(true);
        if (flags & 0x02) cell.SetMapped(true);
        // Waypoint flag not tracked per-cell
    }

    return !stream.HasError();
}

//===========================================================================
// Game Object Save/Load - Stubs for now
//===========================================================================

// Infantry
bool Save_Infantry(SaveStream& stream) {
    // Placeholder: write count of 0
    stream.WriteInt32(0);
    return true;
}

bool Load_Infantry(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;  // TODO: Load infantry
    return !stream.HasError();
}

// Units
bool Save_Units(SaveStream& stream) {
    stream.WriteInt32(0);
    return true;
}

bool Load_Units(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;
    return !stream.HasError();
}

// Buildings
bool Save_Buildings(SaveStream& stream) {
    stream.WriteInt32(0);
    return true;
}

bool Load_Buildings(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;
    return !stream.HasError();
}

// Aircraft
bool Save_Aircraft(SaveStream& stream) {
    stream.WriteInt32(0);
    return true;
}

bool Load_Aircraft(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;
    return !stream.HasError();
}

// Bullets
bool Save_Bullets(SaveStream& stream) {
    stream.WriteInt32(0);
    return true;
}

bool Load_Bullets(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;
    return !stream.HasError();
}

// Triggers
bool Save_Triggers(SaveStream& stream) {
    stream.WriteInt32(0);  // TriggerType count
    stream.WriteInt32(0);  // Trigger instance count
    return true;
}

bool Load_Triggers(LoadStream& stream) {
    int typeCount = stream.ReadInt32();
    (void)typeCount;
    int instCount = stream.ReadInt32();
    (void)instCount;
    return !stream.HasError();
}

// Teams
bool Save_Teams(SaveStream& stream) {
    stream.WriteInt32(0);  // TeamType count
    stream.WriteInt32(0);  // Team instance count
    return true;
}

bool Load_Teams(LoadStream& stream) {
    int typeCount = stream.ReadInt32();
    (void)typeCount;
    int instCount = stream.ReadInt32();
    (void)instCount;
    return !stream.HasError();
}

// Factories
bool Save_Factories(SaveStream& stream) {
    stream.WriteInt32(0);
    return true;
}

bool Load_Factories(LoadStream& stream) {
    int count = stream.ReadInt32();
    (void)count;
    return !stream.HasError();
}

// Misc values
bool Save_Misc_Values(SaveStream& stream) {
    stream.WriteUInt32(Frame);
    return true;
}

bool Load_Misc_Values(LoadStream& stream) {
    Frame = stream.ReadUInt32();
    return !stream.HasError();
}
