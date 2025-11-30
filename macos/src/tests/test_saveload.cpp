/**
 * Red Alert macOS Port - Save/Load Tests
 *
 * Tests the save/load system including stream operations,
 * file format, and game state serialization.
 */

#include "../game/saveload.h"
#include "../game/scenario.h"
#include "../game/house.h"
#include "../game/mapclass.h"
#include "../game/cell.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

//===========================================================================
// External Global References (defined in linked object files)
//===========================================================================

extern ScenarioClass Scen;
extern HouseClass Houses[HOUSE_MAX];
extern HouseClass* PlayerPtr;
extern int HouseCount;
extern MapClass Map;

// Frame - defined locally for this test (saveload.cpp uses extern)
uint32_t Frame = 0;

//===========================================================================
// Test Framework
//===========================================================================

static int tests_passed = 0;
static int tests_failed = 0;

// Forward declare all tests
#define TEST(name) static void test_##name()

// Test registration - filled in at end of file
typedef void (*TestFunc)();
struct TestEntry {
    const char* name;
    TestFunc func;
};

static void run_test(const char* name, TestFunc func) {
    printf("  Testing %s...", name);
    func();
    printf(" OK\n");
    tests_passed++;
}

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf(" FAIL\n    Assertion failed: " #condition "\n    at %s:%d\n", \
                   __FILE__, __LINE__); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            printf(" FAIL\n    Assertion failed: " #a " == " #b "\n    at %s:%d\n", \
                   __FILE__, __LINE__); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define ASSERT_STREQ(a, b) \
    do { \
        if (strcmp((a), (b)) != 0) { \
            printf(" FAIL\n    Assertion failed: \"%s\" == \"%s\"\n    at %s:%d\n", \
                   (a), (b), __FILE__, __LINE__); \
            tests_failed++; \
            return; \
        } \
    } while(0)

//===========================================================================
// Stream Tests
//===========================================================================

TEST(save_stream_open_close) {
    SaveStream stream;
    ASSERT(!stream.IsOpen());

    // Create temp file
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    ASSERT(stream.Open(filename));
    ASSERT(stream.IsOpen());
    stream.Close();
    ASSERT(!stream.IsOpen());

    // Cleanup
    remove(filename);
}

TEST(save_stream_write_int) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    SaveStream stream;
    ASSERT(stream.Open(filename));

    stream.WriteInt8(42);
    stream.WriteInt16(1234);
    stream.WriteInt32(56789);
    stream.WriteUInt8(200);
    stream.WriteUInt16(60000);
    stream.WriteUInt32(0xDEADBEEF);

    stream.Close();

    // Verify by reading back
    FILE* f = fopen(filename, "rb");
    ASSERT(f != nullptr);

    int8_t i8;
    int16_t i16;
    int32_t i32;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

    fread(&i8, 1, 1, f);
    fread(&i16, 2, 1, f);
    fread(&i32, 4, 1, f);
    fread(&u8, 1, 1, f);
    fread(&u16, 2, 1, f);
    fread(&u32, 4, 1, f);

    fclose(f);
    remove(filename);

    ASSERT_EQ(i8, 42);
    ASSERT_EQ(i16, 1234);
    ASSERT_EQ(i32, 56789);
    ASSERT_EQ(u8, 200);
    ASSERT_EQ(u16, 60000);
    ASSERT_EQ(u32, 0xDEADBEEF);
}

TEST(load_stream_read_int) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    // Write test data
    FILE* f = fopen(filename, "wb");
    ASSERT(f != nullptr);

    int8_t i8 = -10;
    int16_t i16 = -5000;
    int32_t i32 = -123456;
    uint8_t u8 = 255;
    uint16_t u16 = 65535;
    uint32_t u32 = 0xCAFEBABE;

    fwrite(&i8, 1, 1, f);
    fwrite(&i16, 2, 1, f);
    fwrite(&i32, 4, 1, f);
    fwrite(&u8, 1, 1, f);
    fwrite(&u16, 2, 1, f);
    fwrite(&u32, 4, 1, f);
    fclose(f);

    // Read back using LoadStream
    LoadStream stream;
    ASSERT(stream.Open(filename));
    ASSERT(!stream.HasError());

    ASSERT_EQ(stream.ReadInt8(), -10);
    ASSERT_EQ(stream.ReadInt16(), -5000);
    ASSERT_EQ(stream.ReadInt32(), -123456);
    ASSERT_EQ(stream.ReadUInt8(), 255);
    ASSERT_EQ(stream.ReadUInt16(), 65535);
    ASSERT_EQ(stream.ReadUInt32(), 0xCAFEBABE);

    ASSERT(!stream.HasError());

    stream.Close();
    remove(filename);
}

TEST(stream_bool) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));

    saver.WriteBool(true);
    saver.WriteBool(false);
    saver.WriteBool(true);
    saver.Close();

    LoadStream loader;
    ASSERT(loader.Open(filename));

    ASSERT_EQ(loader.ReadBool(), true);
    ASSERT_EQ(loader.ReadBool(), false);
    ASSERT_EQ(loader.ReadBool(), true);

    loader.Close();
    remove(filename);
}

TEST(stream_string) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));

    saver.WriteString("Hello, World!", 32);
    saver.WriteString("Test", 16);
    saver.Close();

    LoadStream loader;
    ASSERT(loader.Open(filename));

    char buf1[32], buf2[16];
    loader.ReadString(buf1, 32);
    loader.ReadString(buf2, 16);

    ASSERT_STREQ(buf1, "Hello, World!");
    ASSERT_STREQ(buf2, "Test");

    loader.Close();
    remove(filename);
}

TEST(stream_object_id) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));

    saver.WriteObjectID(RTTIType::INFANTRY, 42);
    saver.WriteObjectID(RTTIType::BUILDING, 100);
    saver.WriteObjectID(RTTIType::NONE, -1);
    saver.Close();

    LoadStream loader;
    ASSERT(loader.Open(filename));

    RTTIType type;
    int id;

    loader.ReadObjectID(type, id);
    ASSERT_EQ(type, RTTIType::INFANTRY);
    ASSERT_EQ(id, 42);

    loader.ReadObjectID(type, id);
    ASSERT_EQ(type, RTTIType::BUILDING);
    ASSERT_EQ(id, 100);

    loader.ReadObjectID(type, id);
    ASSERT_EQ(type, RTTIType::NONE);
    ASSERT_EQ(id, -1);

    loader.Close();
    remove(filename);
}

//===========================================================================
// Header Tests
//===========================================================================

TEST(header_size) {
    ASSERT_EQ(sizeof(SaveGameHeader), SAVE_HEADER_SIZE);
    ASSERT_EQ(sizeof(SaveGameHeader), 160);
}

TEST(header_magic) {
    SaveGameHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = SAVE_MAGIC;

    // Check magic is "RASG" in little-endian
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&header.magic);
    ASSERT_EQ(bytes[0], 'A');
    ASSERT_EQ(bytes[1], 'R');
    ASSERT_EQ(bytes[2], 'S');
    ASSERT_EQ(bytes[3], 'G');
}

//===========================================================================
// File Path Tests
//===========================================================================

TEST(save_filename_generation) {
    char filename[256];

    Get_Save_Filename(0, filename, sizeof(filename));
    ASSERT(strstr(filename, "SAVEGAME.000") != nullptr);

    Get_Save_Filename(1, filename, sizeof(filename));
    ASSERT(strstr(filename, "SAVEGAME.001") != nullptr);

    Get_Save_Filename(99, filename, sizeof(filename));
    ASSERT(strstr(filename, "SAVEGAME.099") != nullptr);
}

TEST(save_filename_bounds) {
    char filename1[256], filename2[256];

    // Negative slot should clamp to 0
    Get_Save_Filename(-1, filename1, sizeof(filename1));
    Get_Save_Filename(0, filename2, sizeof(filename2));
    ASSERT_STREQ(filename1, filename2);

    // Over max should clamp to 0
    Get_Save_Filename(1000, filename1, sizeof(filename1));
    ASSERT_STREQ(filename1, filename2);
}

//===========================================================================
// Scenario Save/Load Tests
//===========================================================================

TEST(scenario_save_load) {
    // Setup scenario
    Scen.Init();
    Scen.scenario_ = 5;
    Scen.theater_ = TheaterType::SNOW;
    strncpy(Scen.name_, "SCU05EA", sizeof(Scen.name_));
    strncpy(Scen.description_, "Test Mission", sizeof(Scen.description_));
    Scen.playerHouse_ = HousesType::USSR;
    Scen.difficulty_ = DifficultyType::HARD;
    Scen.elapsedTime_ = 12345;
    Scen.missionTimer_ = 6789;
    Scen.waypoints_[0] = 100;
    Scen.waypoints_[5] = 200;
    Scen.globalFlags_[3] = true;
    Scen.isEndOfGame_ = true;
    Scen.isTanyaEvac_ = true;

    // Save
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_scen_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));
    ASSERT(Save_Scenario(saver));
    saver.Close();

    // Clear and reload
    Scen.Init();
    // Note: Don't check Init() clears scenario_ - just verify load restores

    LoadStream loader;
    ASSERT(loader.Open(filename));
    ASSERT(Load_Scenario(loader));
    loader.Close();

    // Verify
    ASSERT_EQ(Scen.scenario_, 5);
    ASSERT_EQ(Scen.theater_, TheaterType::SNOW);
    ASSERT_STREQ(Scen.name_, "SCU05EA");
    ASSERT_STREQ(Scen.description_, "Test Mission");
    ASSERT_EQ(Scen.playerHouse_, HousesType::USSR);
    ASSERT_EQ(Scen.difficulty_, DifficultyType::HARD);
    ASSERT_EQ(Scen.elapsedTime_, 12345);
    ASSERT_EQ(Scen.missionTimer_, 6789);
    ASSERT_EQ(Scen.waypoints_[0], 100);
    ASSERT_EQ(Scen.waypoints_[5], 200);
    ASSERT_EQ(Scen.globalFlags_[3], true);
    ASSERT_EQ(Scen.isEndOfGame_, true);
    ASSERT_EQ(Scen.isTanyaEvac_, true);

    remove(filename);
}

//===========================================================================
// House Save/Load Tests
//===========================================================================

TEST(house_save_load) {
    // Setup houses
    Init_Houses();
    HouseCount = 2;

    Houses[0].Init(HousesType::GREECE);
    Houses[0].isActive_ = true;
    Houses[0].isHuman_ = true;
    Houses[0].credits_ = 10000;
    Houses[0].power_ = 500;
    Houses[0].drain_ = 300;
    Houses[0].bKilled_ = 5;
    Houses[0].uKilled_ = 10;

    Houses[1].Init(HousesType::USSR);
    Houses[1].isActive_ = true;
    Houses[1].isHuman_ = false;
    Houses[1].credits_ = 5000;
    Houses[1].enemy_ = HousesType::GREECE;

    PlayerPtr = &Houses[0];

    // Save
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_house_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));
    ASSERT(Save_Houses(saver));
    saver.Close();

    // Clear and reload
    Init_Houses();
    HouseCount = 0;
    PlayerPtr = nullptr;

    LoadStream loader;
    ASSERT(loader.Open(filename));
    ASSERT(Load_Houses(loader));
    loader.Close();

    // Verify
    ASSERT_EQ(HouseCount, 2);
    ASSERT(PlayerPtr == &Houses[0]);

    ASSERT_EQ(Houses[0].type_, HousesType::GREECE);
    ASSERT_EQ(Houses[0].isActive_, true);
    ASSERT_EQ(Houses[0].isHuman_, true);
    ASSERT_EQ(Houses[0].credits_, 10000);
    ASSERT_EQ(Houses[0].power_, 500);
    ASSERT_EQ(Houses[0].drain_, 300);
    ASSERT_EQ(Houses[0].bKilled_, 5);
    ASSERT_EQ(Houses[0].uKilled_, 10);

    ASSERT_EQ(Houses[1].type_, HousesType::USSR);
    ASSERT_EQ(Houses[1].isHuman_, false);
    ASSERT_EQ(Houses[1].credits_, 5000);
    ASSERT_EQ(Houses[1].enemy_, HousesType::GREECE);

    remove(filename);
}

//===========================================================================
// Map Save/Load Tests
//===========================================================================

TEST(map_save_load) {
    // Initialize the global Map properly
    Map.OneTime();
    Map.SetMapDimensions(10, 10, 40, 40);

    // Save map dimensions and verify they can be written/read
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_map_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));
    ASSERT(Save_Map(saver));
    saver.Close();

    // Change dimensions before load
    Map.SetMapDimensions(20, 20, 60, 60);
    ASSERT_EQ(Map.MapCellX(), 20);

    LoadStream loader;
    ASSERT(loader.Open(filename));
    ASSERT(Load_Map(loader));
    loader.Close();

    // Verify dimensions were restored
    ASSERT_EQ(Map.MapCellX(), 10);
    ASSERT_EQ(Map.MapCellY(), 10);
    ASSERT_EQ(Map.MapCellWidth(), 40);
    ASSERT_EQ(Map.MapCellHeight(), 40);

    Map.FreeCells();
    remove(filename);
}

//===========================================================================
// Full Save/Load Cycle Tests
//===========================================================================

TEST(full_save_load_cycle) {
    // Initialize game state
    Scen.Init();
    Scen.scenario_ = 3;
    Scen.theater_ = TheaterType::TEMPERATE;
    Scen.playerHouse_ = HousesType::GREECE;
    strncpy(Scen.description_, "Test Save", sizeof(Scen.description_));

    Init_Houses();
    HouseCount = 1;
    Houses[0].Init(HousesType::GREECE);
    Houses[0].isActive_ = true;
    Houses[0].isHuman_ = true;
    Houses[0].credits_ = 7500;
    PlayerPtr = &Houses[0];

    Map.OneTime();
    Map.AllocCells();
    Map.InitCells();
    Map.SetMapDimensions(0, 0, 64, 64);

    Frame = 1000;

    // Save game
    ASSERT(Save_Game(99, "Full Test Save"));

    // Verify save exists
    ASSERT(Save_Exists(99));

    // Get save info
    SaveGameHeader info;
    ASSERT(Get_Save_Info(99, &info));
    ASSERT_EQ(info.magic, SAVE_MAGIC);
    ASSERT_EQ(info.scenario, 3);
    ASSERT_EQ(info.house, static_cast<int32_t>(HousesType::GREECE));
    ASSERT_STREQ(info.description, "Full Test Save");

    // Clear state
    Scen.Init();
    Init_Houses();
    HouseCount = 0;
    PlayerPtr = nullptr;
    Frame = 0;

    // Load game
    ASSERT(Load_Game(99));

    // Verify state restored
    ASSERT_EQ(Scen.scenario_, 3);
    ASSERT_EQ(Scen.theater_, TheaterType::TEMPERATE);
    ASSERT_EQ(Scen.playerHouse_, HousesType::GREECE);
    ASSERT_EQ(HouseCount, 1);
    ASSERT(PlayerPtr == &Houses[0]);
    ASSERT_EQ(Houses[0].credits_, 7500);
    ASSERT_EQ(Frame, 1000);

    // Delete save
    ASSERT(Delete_Save(99));
    ASSERT(!Save_Exists(99));

    Map.FreeCells();
}

TEST(save_not_exists) {
    // Non-existent slot
    ASSERT(!Save_Exists(98));

    // Invalid header shouldn't exist
    SaveGameHeader info;
    ASSERT(!Get_Save_Info(98, &info));
}

TEST(load_nonexistent_fails) {
    // Loading non-existent save should fail gracefully
    ASSERT(!Load_Game(97));
}

//===========================================================================
// Checksum Tests
//===========================================================================

TEST(checksum_calculation) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_cksum_%d.sav", rand());

    SaveStream saver;
    ASSERT(saver.Open(filename));

    // Write some data
    saver.WriteInt32(12345);
    saver.WriteString("Test data", 32);
    saver.WriteInt64(0xDEADBEEFCAFEBABE);

    // Get checksum
    uint8_t checksum[16];
    saver.CalculateChecksum(checksum);

    saver.Close();

    // Checksum should be non-zero
    bool allZero = true;
    for (int i = 0; i < 16; i++) {
        if (checksum[i] != 0) {
            allZero = false;
            break;
        }
    }
    ASSERT(!allZero);

    remove(filename);
}

//===========================================================================
// Misc Values Tests
//===========================================================================

TEST(misc_values_save_load) {
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ra_test_misc_%d.sav", rand());

    Frame = 54321;

    SaveStream saver;
    ASSERT(saver.Open(filename));
    ASSERT(Save_Misc_Values(saver));
    saver.Close();

    Frame = 0;

    LoadStream loader;
    ASSERT(loader.Open(filename));
    ASSERT(Load_Misc_Values(loader));
    loader.Close();

    ASSERT_EQ(Frame, 54321);

    remove(filename);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert Save/Load Tests\n");
    printf("=========================\n\n");

    printf("Stream Tests:\n");
    run_test("save_stream_open_close", test_save_stream_open_close);
    run_test("save_stream_write_int", test_save_stream_write_int);
    run_test("load_stream_read_int", test_load_stream_read_int);
    run_test("stream_bool", test_stream_bool);
    run_test("stream_string", test_stream_string);
    run_test("stream_object_id", test_stream_object_id);

    printf("\nHeader Tests:\n");
    run_test("header_size", test_header_size);
    run_test("header_magic", test_header_magic);

    printf("\nFile Path Tests:\n");
    run_test("save_filename_generation", test_save_filename_generation);
    run_test("save_filename_bounds", test_save_filename_bounds);

    printf("\nScenario Tests:\n");
    run_test("scenario_save_load", test_scenario_save_load);

    printf("\nHouse Tests:\n");
    run_test("house_save_load", test_house_save_load);

    printf("\nMap Tests:\n");
    run_test("map_save_load", test_map_save_load);

    printf("\nFull Cycle Tests:\n");
    run_test("full_save_load_cycle", test_full_save_load_cycle);
    run_test("save_not_exists", test_save_not_exists);
    run_test("load_nonexistent_fails", test_load_nonexistent_fails);

    printf("\nChecksum Tests:\n");
    run_test("checksum_calculation", test_checksum_calculation);

    printf("\nMisc Values Tests:\n");
    run_test("misc_values_save_load", test_misc_values_save_load);

    printf("\n=========================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("=========================\n");

    return tests_failed > 0 ? 1 : 0;
}
