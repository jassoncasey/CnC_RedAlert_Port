/**
 * Red Alert macOS Port - Save/Load System
 *
 * Provides game state serialization for save/load functionality.
 * Based on original SAVELOAD.CPP (~1500 lines)
 *
 * Save file format:
 *   Header (160 bytes):
 *     - Magic (4 bytes): "RASG" (Red Alert Save Game)
 *     - Version (4 bytes): SAVEGAME_VERSION
 *     - Scenario (4 bytes): Current scenario number
 *     - House (4 bytes): Player's house type
 *     - Description (128 bytes): User description, null-terminated
 *     - Checksum (16 bytes): MD5 hash of data section
 *
 *   Data Section (variable):
 *     - Scenario state
 *     - House states
 *     - Map/cell data
 *     - All game objects (buildings, units, infantry, aircraft)
 *     - Triggers and teams
 *     - Factory/production state
 *     - Misc values (frame count, selection, etc.)
 */

#ifndef GAME_SAVELOAD_H
#define GAME_SAVELOAD_H

#include "types.h"
#include <cstdint>
#include <cstdio>
#include <vector>

//===========================================================================
// Constants
//===========================================================================

// Save file magic number
constexpr uint32_t SAVE_MAGIC = 0x47535241;  // "RASG" in little-endian

// Save game version - increment when format changes
// Includes sum of key structure sizes for compatibility checking
constexpr uint32_t SAVEGAME_VERSION = 0x00010001;

// Maximum description length
constexpr int SAVE_DESCRIP_MAX = 128;

// Maximum filename length
constexpr int SAVE_FILENAME_MAX = 256;

// Maximum save game slots
constexpr int SAVE_SLOT_MAX = 100;

// Header size
constexpr int SAVE_HEADER_SIZE = 160;

//===========================================================================
// Save File Header
//===========================================================================

#pragma pack(push, 1)
struct SaveGameHeader {
    uint32_t magic;                           // SAVE_MAGIC
    uint32_t version;                         // SAVEGAME_VERSION
    int32_t scenario;                         // Scenario number
    int32_t house;                            // Player's house (HousesType)
    char description[SAVE_DESCRIP_MAX];       // User description
    uint8_t checksum[16];                     // MD5 checksum of data
};
#pragma pack(pop)

static_assert(sizeof(SaveGameHeader) == SAVE_HEADER_SIZE, "SaveGameHeader size mismatch");

//===========================================================================
// SaveStream - Binary output stream for saving
//===========================================================================

class SaveStream {
public:
    SaveStream();
    ~SaveStream();

    // Open file for writing
    bool Open(const char* filename);
    void Close();
    bool IsOpen() const { return file_ != nullptr; }

    // Write raw data
    bool Write(const void* data, size_t size);

    // Write typed data
    bool WriteInt8(int8_t value) { return Write(&value, sizeof(value)); }
    bool WriteInt16(int16_t value) { return Write(&value, sizeof(value)); }
    bool WriteInt32(int32_t value) { return Write(&value, sizeof(value)); }
    bool WriteUInt8(uint8_t value) { return Write(&value, sizeof(value)); }
    bool WriteUInt16(uint16_t value) { return Write(&value, sizeof(value)); }
    bool WriteUInt32(uint32_t value) { return Write(&value, sizeof(value)); }
    bool WriteUInt64(uint64_t value) { return Write(&value, sizeof(value)); }
    bool WriteInt64(int64_t value) { return Write(&value, sizeof(value)); }
    bool WriteBool(bool value);
    bool WriteFloat(float value) { return Write(&value, sizeof(value)); }
    bool WriteString(const char* str, int maxLen);

    // Write pointer as ID (for object references)
    bool WriteObjectID(RTTIType type, int id);

    // Get bytes written
    size_t BytesWritten() const { return bytesWritten_; }

    // Calculate checksum of written data
    void CalculateChecksum(uint8_t* outChecksum);

private:
    FILE* file_;
    size_t bytesWritten_;
    std::vector<uint8_t> buffer_;  // For checksum calculation
};

//===========================================================================
// LoadStream - Binary input stream for loading
//===========================================================================

class LoadStream {
public:
    LoadStream();
    ~LoadStream();

    // Open file for reading
    bool Open(const char* filename);
    void Close();
    bool IsOpen() const { return file_ != nullptr; }

    // Read raw data
    bool Read(void* data, size_t size);

    // Read typed data
    int8_t ReadInt8();
    int16_t ReadInt16();
    int32_t ReadInt32();
    uint8_t ReadUInt8();
    uint16_t ReadUInt16();
    uint32_t ReadUInt32();
    uint64_t ReadUInt64();
    int64_t ReadInt64();
    bool ReadBool();
    float ReadFloat();
    bool ReadString(char* str, int maxLen);

    // Read object reference
    bool ReadObjectID(RTTIType& type, int& id);

    // Get bytes read
    size_t BytesRead() const { return bytesRead_; }

    // Check for read errors
    bool HasError() const { return hasError_; }

    // Calculate checksum of data from current position
    void CalculateChecksum(size_t dataSize, uint8_t* outChecksum);

private:
    FILE* file_;
    size_t bytesRead_;
    bool hasError_;
};

//===========================================================================
// Save/Load Functions
//===========================================================================

/**
 * Save game to slot (0-99)
 * @param slot Save slot number
 * @param description User description (max 127 chars)
 * @return true if save succeeded
 */
bool Save_Game(int slot, const char* description);

/**
 * Load game from slot (0-99)
 * @param slot Save slot number
 * @return true if load succeeded
 */
bool Load_Game(int slot);

/**
 * Get save game info without loading
 * @param slot Save slot number
 * @param header Output header info
 * @return true if slot has valid save
 */
bool Get_Save_Info(int slot, SaveGameHeader* header);

/**
 * Check if save slot exists and is valid
 * @param slot Save slot number
 * @return true if slot has valid save
 */
bool Save_Exists(int slot);

/**
 * Delete save game
 * @param slot Save slot number
 * @return true if deleted successfully
 */
bool Delete_Save(int slot);

/**
 * Get filename for save slot
 * @param slot Save slot number
 * @param buffer Output buffer
 * @param bufferSize Buffer size
 */
void Get_Save_Filename(int slot, char* buffer, int bufferSize);

//===========================================================================
// Object Serialization Interface
//===========================================================================

// Forward declarations
class ScenarioClass;
class HouseClass;
class MapClass;
class CellClass;
class InfantryClass;
class UnitClass;
class BuildingClass;
class AircraftClass;
class BulletClass;
class TriggerClass;
class TriggerTypeClass;
class TeamClass;
class TeamTypeClass;
class FactoryClass;

/**
 * Save all game state to stream
 */
bool Put_All(SaveStream& stream);

/**
 * Load all game state from stream
 */
bool Get_All(LoadStream& stream);

/**
 * Convert all object pointers to IDs before save
 */
void Code_All_Pointers();

/**
 * Convert all object IDs back to pointers after load
 */
void Decode_All_Pointers();

//===========================================================================
// Per-class Save/Load (called by Put_All/Get_All)
//===========================================================================

// Scenario
bool Save_Scenario(SaveStream& stream);
bool Load_Scenario(LoadStream& stream);

// Houses
bool Save_Houses(SaveStream& stream);
bool Load_Houses(LoadStream& stream);

// Map/Cells
bool Save_Map(SaveStream& stream);
bool Load_Map(LoadStream& stream);

// Game objects
bool Save_Infantry(SaveStream& stream);
bool Load_Infantry(LoadStream& stream);

bool Save_Units(SaveStream& stream);
bool Load_Units(LoadStream& stream);

bool Save_Buildings(SaveStream& stream);
bool Load_Buildings(LoadStream& stream);

bool Save_Aircraft(SaveStream& stream);
bool Load_Aircraft(LoadStream& stream);

bool Save_Bullets(SaveStream& stream);
bool Load_Bullets(LoadStream& stream);

// AI
bool Save_Triggers(SaveStream& stream);
bool Load_Triggers(LoadStream& stream);

bool Save_Teams(SaveStream& stream);
bool Load_Teams(LoadStream& stream);

// Production
bool Save_Factories(SaveStream& stream);
bool Load_Factories(LoadStream& stream);

// Misc (frame counter, selection, etc.)
bool Save_Misc_Values(SaveStream& stream);
bool Load_Misc_Values(LoadStream& stream);

#endif // GAME_SAVELOAD_H
