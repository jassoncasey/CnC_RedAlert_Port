/**
 * Red Alert macOS Port - INI Parser Test
 *
 * Tests the INI file parser with sample data.
 */

#include "../game/ini.h"
#include <cstdio>
#include <cstring>
#include <cassert>

// Sample INI data similar to RULES.INI format
static const char* TestINIData = R"(
; Red Alert INI Test File
; This is a comment

[General]
; General game settings
GameSpeed=4
Difficulty=1
BuildSpeed=100%
Money=10000

[E1]
; Rifle Infantry
Name=Rifle Infantry
Cost=100
Speed=4
Ammo=-1
Owner=allies,soviet
Armor=none
Sight=2
TechLevel=0
Primary=M1Carbine
Cloakable=no

[LTANK]
; Light Tank
Name=Light Tank
Cost=700
Speed=9
Armor=heavy
Strength=400
TechLevel=2
Primary=75mm
Owner=allies

[WEAP]
; Weapons Factory
Name=Weapons Factory
Cost=2000
Power=-100
Armor=wood
Adjacent=2
Produces=LTANK,MTANK,APC
)";

static int testCount = 0;
static int passCount = 0;

#define TEST(name) do { \
    testCount++; \
    printf("  Test: %s... ", name); \
} while(0)

#define PASS() do { \
    passCount++; \
    printf("PASS\n"); \
} while(0)

#define FAIL(msg) do { \
    printf("FAIL (%s)\n", msg); \
} while(0)

void TestLoading() {
    printf("\n=== Loading Tests ===\n");

    INIClass ini;

    TEST("Load from buffer");
    bool loaded = ini.LoadFromBuffer(TestINIData, strlen(TestINIData));
    if (loaded && ini.IsLoaded()) {
        PASS();
    } else {
        FAIL("LoadFromBuffer returned false or IsLoaded is false");
        return;
    }

    TEST("Section count");
    int sectionCount = ini.SectionCount();
    if (sectionCount == 4) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 4 sections, got %d", sectionCount);
        FAIL(msg);
    }

    TEST("Section present (General)");
    if (ini.SectionPresent("General")) {
        PASS();
    } else {
        FAIL("General section not found");
    }

    TEST("Section present (case insensitive)");
    if (ini.SectionPresent("GENERAL") && ini.SectionPresent("general")) {
        PASS();
    } else {
        FAIL("Case insensitive section lookup failed");
    }

    TEST("Section not present");
    if (!ini.SectionPresent("NonExistent")) {
        PASS();
    } else {
        FAIL("Found nonexistent section");
    }
}

void TestEntries() {
    printf("\n=== Entry Tests ===\n");

    INIClass ini;
    ini.LoadFromBuffer(TestINIData, strlen(TestINIData));

    TEST("Entry count (E1)");
    int entryCount = ini.EntryCount("E1");
    if (entryCount == 10) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 10 entries, got %d", entryCount);
        FAIL(msg);
    }

    TEST("Entry present");
    if (ini.IsPresent("E1", "Cost")) {
        PASS();
    } else {
        FAIL("Cost entry not found in E1");
    }

    TEST("Entry present (case insensitive)");
    if (ini.IsPresent("e1", "COST") && ini.IsPresent("E1", "cost")) {
        PASS();
    } else {
        FAIL("Case insensitive entry lookup failed");
    }

    TEST("Get entry by index");
    const char* entry = ini.GetEntry("General", 0);
    if (entry != nullptr && strcmp(entry, "GameSpeed") == 0) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected GameSpeed, got %s", entry ? entry : "null");
        FAIL(msg);
    }
}

void TestGetValues() {
    printf("\n=== Get Value Tests ===\n");

    INIClass ini;
    ini.LoadFromBuffer(TestINIData, strlen(TestINIData));

    TEST("Get string");
    std::string name = ini.GetString("E1", "Name", "");
    if (name == "Rifle Infantry") {
        PASS();
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected 'Rifle Infantry', got '%s'", name.c_str());
        FAIL(msg);
    }

    TEST("Get string to buffer");
    char buffer[64];
    int len = ini.GetString("E1", "Name", "default", buffer, sizeof(buffer));
    if (len == 14 && strcmp(buffer, "Rifle Infantry") == 0) {
        PASS();
    } else {
        FAIL("Buffer string mismatch");
    }

    TEST("Get string default");
    std::string defValue = ini.GetString("E1", "NonExistent", "default");
    if (defValue == "default") {
        PASS();
    } else {
        FAIL("Default value not returned");
    }

    TEST("Get int");
    int cost = ini.GetInt("E1", "Cost", 0);
    if (cost == 100) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 100, got %d", cost);
        FAIL(msg);
    }

    TEST("Get int negative");
    int ammo = ini.GetInt("E1", "Ammo", 0);
    if (ammo == -1) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected -1, got %d", ammo);
        FAIL(msg);
    }

    TEST("Get int default");
    int defInt = ini.GetInt("E1", "NonExistent", 999);
    if (defInt == 999) {
        PASS();
    } else {
        FAIL("Default int not returned");
    }

    TEST("Get bool (no)");
    bool cloakable = ini.GetBool("E1", "Cloakable", true);
    if (!cloakable) {
        PASS();
    } else {
        FAIL("Expected false for 'no'");
    }

    TEST("Get bool default");
    bool defBool = ini.GetBool("E1", "NonExistent", true);
    if (defBool) {
        PASS();
    } else {
        FAIL("Default bool not returned");
    }

    TEST("Get fixed (percentage)");
    float buildSpeed = ini.GetFixed("General", "BuildSpeed", 0.0f);
    if (buildSpeed >= 0.99f && buildSpeed <= 1.01f) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected ~1.0, got %f", buildSpeed);
        FAIL(msg);
    }
}

void TestPutValues() {
    printf("\n=== Put Value Tests ===\n");

    INIClass ini;

    TEST("Put string (new section)");
    bool result = ini.PutString("NewSection", "NewEntry", "NewValue");
    std::string value = ini.GetString("NewSection", "NewEntry", "");
    if (result && value == "NewValue") {
        PASS();
    } else {
        FAIL("PutString failed");
    }

    TEST("Put int");
    ini.PutInt("NewSection", "IntValue", 12345);
    int intVal = ini.GetInt("NewSection", "IntValue", 0);
    if (intVal == 12345) {
        PASS();
    } else {
        FAIL("PutInt/GetInt mismatch");
    }

    TEST("Put hex");
    ini.PutHex("NewSection", "HexValue", 0xDEAD);
    int hexVal = ini.GetHex("NewSection", "HexValue", 0);
    if (hexVal == 0xDEAD) {
        PASS();
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Expected 0xDEAD, got 0x%X", hexVal);
        FAIL(msg);
    }

    TEST("Put bool");
    ini.PutBool("NewSection", "BoolValue", true);
    bool boolVal = ini.GetBool("NewSection", "BoolValue", false);
    if (boolVal) {
        PASS();
    } else {
        FAIL("PutBool/GetBool mismatch");
    }

    TEST("Overwrite existing");
    ini.PutString("NewSection", "NewEntry", "UpdatedValue");
    value = ini.GetString("NewSection", "NewEntry", "");
    if (value == "UpdatedValue") {
        PASS();
    } else {
        FAIL("Overwrite failed");
    }
}

void TestClear() {
    printf("\n=== Clear Tests ===\n");

    INIClass ini;
    ini.LoadFromBuffer(TestINIData, strlen(TestINIData));

    TEST("Clear entry");
    ini.Clear("E1", "Cost");
    if (!ini.IsPresent("E1", "Cost")) {
        PASS();
    } else {
        FAIL("Entry still present after clear");
    }

    TEST("Clear section");
    ini.Clear("LTANK");
    if (!ini.SectionPresent("LTANK")) {
        PASS();
    } else {
        FAIL("Section still present after clear");
    }

    TEST("Clear all");
    ini.Clear();
    if (!ini.IsLoaded()) {
        PASS();
    } else {
        FAIL("Data still present after clear all");
    }
}

int main() {
    printf("INI Parser Test\n");
    printf("================\n");

    TestLoading();
    TestEntries();
    TestGetValues();
    TestPutValues();
    TestClear();

    printf("\n================\n");
    printf("Results: %d/%d tests passed\n", passCount, testCount);

    return (passCount == testCount) ? 0 : 1;
}
