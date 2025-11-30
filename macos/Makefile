# Red Alert macOS Port - Makefile
# Target: macOS 14+ (Sonoma), ARM64 only

CXX = clang++
CC = clang
OBJCXX = clang++

# Directories
SRC_DIR = src
BUILD_DIR = build
APP_NAME = RedAlert
APP_BUNDLE = $(APP_NAME).app

# Compiler flags
ARCH = -arch arm64
MIN_VERSION = -mmacosx-version-min=14.0
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(ARCH) $(MIN_VERSION)
OBJCXXFLAGS = $(CXXFLAGS) -fobjc-arc
INCLUDES = -I$(SRC_DIR) -Iinclude

# Frameworks
FRAMEWORKS = -framework Cocoa -framework Metal -framework MetalKit -framework QuartzCore -framework AudioToolbox -framework AVFoundation

# Linker flags
LDFLAGS = $(ARCH) $(MIN_VERSION) $(FRAMEWORKS)

# Sources (add as we go)
OBJCXX_SOURCES = $(SRC_DIR)/main.mm $(SRC_DIR)/graphics/metal/renderer.mm $(SRC_DIR)/input/input.mm $(SRC_DIR)/audio/audio.mm
CPP_SOURCES = $(SRC_DIR)/platform/file.cpp $(SRC_DIR)/platform/timing.cpp $(SRC_DIR)/platform/assets.cpp $(SRC_DIR)/platform/asset_paths.cpp \
              $(SRC_DIR)/game/gameloop.cpp $(SRC_DIR)/ui/menu.cpp \
              $(SRC_DIR)/assets/mixfile.cpp $(SRC_DIR)/assets/shpfile.cpp $(SRC_DIR)/assets/palfile.cpp $(SRC_DIR)/assets/audfile.cpp \
              $(SRC_DIR)/game/map.cpp $(SRC_DIR)/game/units.cpp \
              $(SRC_DIR)/game/infantry_types.cpp $(SRC_DIR)/game/unit_types.cpp $(SRC_DIR)/game/weapon_types.cpp \
              $(SRC_DIR)/game/building_types.cpp $(SRC_DIR)/game/aircraft_types.cpp \
              $(SRC_DIR)/game/ini.cpp $(SRC_DIR)/game/rules.cpp \
              $(SRC_DIR)/game/object.cpp $(SRC_DIR)/game/cell.cpp $(SRC_DIR)/game/mapclass.cpp $(SRC_DIR)/game/pathfind.cpp \
              $(SRC_DIR)/game/infantry.cpp $(SRC_DIR)/game/unit.cpp $(SRC_DIR)/game/building.cpp $(SRC_DIR)/game/aircraft.cpp \
              $(SRC_DIR)/game/bullet.cpp $(SRC_DIR)/game/combat.cpp \
              $(SRC_DIR)/game/house.cpp $(SRC_DIR)/game/team.cpp \
              $(SRC_DIR)/game/scenario.cpp $(SRC_DIR)/game/trigger.cpp

# Objects
OBJCXX_OBJECTS = $(patsubst $(SRC_DIR)/%.mm,$(BUILD_DIR)/%.o,$(OBJCXX_SOURCES))
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(OBJCXX_OBJECTS) $(CPP_OBJECTS)

# Default target
all: $(APP_BUNDLE)

# App bundle
$(APP_BUNDLE): $(BUILD_DIR)/$(APP_NAME) resources/Info.plist resources/AppIcon.icns
	@mkdir -p $(APP_BUNDLE)/Contents/MacOS
	@mkdir -p $(APP_BUNDLE)/Contents/Resources
	@cp $(BUILD_DIR)/$(APP_NAME) $(APP_BUNDLE)/Contents/MacOS/
	@cp resources/Info.plist $(APP_BUNDLE)/Contents/
	@cp resources/AppIcon.icns $(APP_BUNDLE)/Contents/Resources/
	@echo "Built $(APP_BUNDLE)"

# Link
$(BUILD_DIR)/$(APP_NAME): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^

# Compile Objective-C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.mm
	@mkdir -p $(dir $@)
	$(OBJCXX) $(OBJCXXFLAGS) $(INCLUDES) -c -o $@ $<

# Compile C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Clean
clean:
	rm -rf $(BUILD_DIR) $(APP_BUNDLE) $(APP_NAME).zip $(APP_NAME).dmg

# Run
run: $(APP_BUNDLE)
	./$(APP_BUNDLE)/Contents/MacOS/$(APP_NAME)

# Distribution package (ad-hoc signed zip)
dist: $(APP_BUNDLE)
	@echo "Signing app bundle (ad-hoc)..."
	codesign --force --deep --sign - $(APP_BUNDLE)
	@echo "Creating zip archive..."
	zip -r $(APP_NAME).zip $(APP_BUNDLE)
	@echo "Created $(APP_NAME).zip"
	@ls -lh $(APP_NAME).zip

# DMG distribution (prettier, with background)
dmg: $(APP_BUNDLE)
	@echo "Signing app bundle (ad-hoc)..."
	codesign --force --deep --sign - $(APP_BUNDLE)
	@echo "Creating DMG..."
	hdiutil create -volname "Red Alert" -srcfolder $(APP_BUNDLE) -ov -format UDZO $(APP_NAME).dmg
	@echo "Created $(APP_NAME).dmg"
	@ls -lh $(APP_NAME).dmg

# Test asset loading
test_assets: $(BUILD_DIR)/test_assets
	@echo "Running asset loading test..."
	@cd $(BUILD_DIR) && ./test_assets

$(BUILD_DIR)/test_assets: $(SRC_DIR)/tests/test_assets.cpp $(BUILD_DIR)/assets/mixfile.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test INI parser
test_ini: $(BUILD_DIR)/test_ini
	@echo "Running INI parser test..."
	@./$(BUILD_DIR)/test_ini

$(BUILD_DIR)/test_ini: $(SRC_DIR)/tests/test_ini.cpp $(BUILD_DIR)/game/ini.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test RULES.INI loading
test_rules: $(BUILD_DIR)/test_rules
	@echo "Running RULES.INI test..."
	@cd $(BUILD_DIR) && ./test_rules

$(BUILD_DIR)/test_rules: $(SRC_DIR)/tests/test_rules.cpp $(BUILD_DIR)/game/ini.o $(BUILD_DIR)/game/rules.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test object hierarchy
test_objects: $(BUILD_DIR)/test_objects
	@echo "Running object hierarchy test..."
	@./$(BUILD_DIR)/test_objects

$(BUILD_DIR)/test_objects: $(SRC_DIR)/tests/test_objects.cpp $(BUILD_DIR)/game/object.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test map/cell/pathfinding
test_map: $(BUILD_DIR)/test_map
	@echo "Running map/cell/pathfinding test..."
	@./$(BUILD_DIR)/test_map

$(BUILD_DIR)/test_map: $(SRC_DIR)/tests/test_map.cpp $(BUILD_DIR)/game/cell.o $(BUILD_DIR)/game/mapclass.o $(BUILD_DIR)/game/pathfind.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test entities (infantry, units, buildings, aircraft)
test_entities: $(BUILD_DIR)/test_entities
	@echo "Running entity tests..."
	@./$(BUILD_DIR)/test_entities

ENTITY_TEST_OBJS = $(BUILD_DIR)/game/object.o $(BUILD_DIR)/game/cell.o $(BUILD_DIR)/game/mapclass.o \
                   $(BUILD_DIR)/game/pathfind.o $(BUILD_DIR)/game/infantry_types.o $(BUILD_DIR)/game/unit_types.o \
                   $(BUILD_DIR)/game/building_types.o $(BUILD_DIR)/game/aircraft_types.o \
                   $(BUILD_DIR)/game/infantry.o $(BUILD_DIR)/game/unit.o $(BUILD_DIR)/game/building.o $(BUILD_DIR)/game/aircraft.o

$(BUILD_DIR)/test_entities: $(SRC_DIR)/tests/test_entities.cpp $(ENTITY_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test combat system (bullets, damage, weapons)
test_combat: $(BUILD_DIR)/test_combat
	@echo "Running combat system tests..."
	@./$(BUILD_DIR)/test_combat

COMBAT_TEST_OBJS = $(BUILD_DIR)/game/object.o $(BUILD_DIR)/game/cell.o $(BUILD_DIR)/game/mapclass.o \
                   $(BUILD_DIR)/game/pathfind.o $(BUILD_DIR)/game/weapon_types.o \
                   $(BUILD_DIR)/game/bullet.o $(BUILD_DIR)/game/combat.o

$(BUILD_DIR)/test_combat: $(SRC_DIR)/tests/test_combat.cpp $(COMBAT_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test AI system (house, teams)
test_ai: $(BUILD_DIR)/test_ai
	@echo "Running AI system tests..."
	@./$(BUILD_DIR)/test_ai

AI_TEST_OBJS = $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o $(BUILD_DIR)/game/object.o

$(BUILD_DIR)/test_ai: $(SRC_DIR)/tests/test_ai.cpp $(AI_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test scenario/trigger system
test_scenario: $(BUILD_DIR)/test_scenario
	@echo "Running scenario/trigger tests..."
	@./$(BUILD_DIR)/test_scenario

SCENARIO_TEST_OBJS = $(BUILD_DIR)/game/scenario.o $(BUILD_DIR)/game/trigger.o \
                     $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o \
                     $(BUILD_DIR)/game/ini.o $(BUILD_DIR)/game/object.o

$(BUILD_DIR)/test_scenario: $(SRC_DIR)/tests/test_scenario.cpp $(SCENARIO_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

.PHONY: all clean run dist dmg test_assets test_ini test_rules test_objects test_map test_entities test_combat test_ai test_scenario
