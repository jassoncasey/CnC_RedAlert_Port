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
              $(SRC_DIR)/crypto/blowfish.cpp $(SRC_DIR)/crypto/mixkey.cpp \
              $(SRC_DIR)/assets/mixfile.cpp $(SRC_DIR)/assets/shpfile.cpp $(SRC_DIR)/assets/palfile.cpp $(SRC_DIR)/assets/audfile.cpp $(SRC_DIR)/assets/tmpfile.cpp $(SRC_DIR)/assets/assetloader.cpp \
              $(SRC_DIR)/game/map.cpp $(SRC_DIR)/game/units.cpp $(SRC_DIR)/game/sprites.cpp $(SRC_DIR)/game/sounds.cpp $(SRC_DIR)/game/terrain.cpp \
              $(SRC_DIR)/game/infantry_types.cpp $(SRC_DIR)/game/unit_types.cpp $(SRC_DIR)/game/weapon_types.cpp \
              $(SRC_DIR)/game/building_types.cpp $(SRC_DIR)/game/aircraft_types.cpp \
              $(SRC_DIR)/game/ini.cpp $(SRC_DIR)/game/rules.cpp \
              $(SRC_DIR)/game/object.cpp $(SRC_DIR)/game/cell.cpp $(SRC_DIR)/game/mapclass.cpp $(SRC_DIR)/game/pathfind.cpp \
              $(SRC_DIR)/game/infantry.cpp $(SRC_DIR)/game/unit.cpp $(SRC_DIR)/game/building.cpp $(SRC_DIR)/game/aircraft.cpp \
              $(SRC_DIR)/game/bullet.cpp $(SRC_DIR)/game/combat.cpp \
              $(SRC_DIR)/game/house.cpp $(SRC_DIR)/game/team.cpp \
              $(SRC_DIR)/game/scenario.cpp $(SRC_DIR)/game/trigger.cpp \
              $(SRC_DIR)/game/factory.cpp $(SRC_DIR)/game/sidebar.cpp \
              $(SRC_DIR)/game/radar.cpp $(SRC_DIR)/game/saveload.cpp \
              $(SRC_DIR)/game/anim.cpp $(SRC_DIR)/game/campaign.cpp \
              $(SRC_DIR)/video/vqa.cpp $(SRC_DIR)/video/music.cpp \
              $(SRC_DIR)/ui/game_ui.cpp

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

# Test sidebar/factory system
test_sidebar: $(BUILD_DIR)/test_sidebar
	@echo "Running sidebar/factory tests..."
	@./$(BUILD_DIR)/test_sidebar

SIDEBAR_TEST_OBJS = $(BUILD_DIR)/game/sidebar.o $(BUILD_DIR)/game/factory.o \
                    $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o \
                    $(BUILD_DIR)/game/object.o $(BUILD_DIR)/game/infantry_types.o \
                    $(BUILD_DIR)/game/unit_types.o $(BUILD_DIR)/game/building_types.o \
                    $(BUILD_DIR)/game/aircraft_types.o

$(BUILD_DIR)/test_sidebar: $(SRC_DIR)/tests/test_sidebar.cpp $(SIDEBAR_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test radar/minimap system
test_radar: $(BUILD_DIR)/test_radar
	@echo "Running radar/minimap tests..."
	@./$(BUILD_DIR)/test_radar

RADAR_TEST_OBJS = $(BUILD_DIR)/game/radar.o $(BUILD_DIR)/game/mapclass.o \
                  $(BUILD_DIR)/game/cell.o $(BUILD_DIR)/game/pathfind.o \
                  $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o \
                  $(BUILD_DIR)/game/object.o

$(BUILD_DIR)/test_radar: $(SRC_DIR)/tests/test_radar.cpp $(RADAR_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test save/load system
test_saveload: $(BUILD_DIR)/test_saveload
	@echo "Running save/load tests..."
	@./$(BUILD_DIR)/test_saveload

SAVELOAD_TEST_OBJS = $(BUILD_DIR)/game/saveload.o $(BUILD_DIR)/game/scenario.o \
                     $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o \
                     $(BUILD_DIR)/game/mapclass.o $(BUILD_DIR)/game/cell.o \
                     $(BUILD_DIR)/game/pathfind.o $(BUILD_DIR)/game/object.o \
                     $(BUILD_DIR)/game/ini.o $(BUILD_DIR)/game/trigger.o

$(BUILD_DIR)/test_saveload: $(SRC_DIR)/tests/test_saveload.cpp $(SAVELOAD_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test animation system
test_anim: $(BUILD_DIR)/test_anim
	@echo "Running animation system tests..."
	@./$(BUILD_DIR)/test_anim

ANIM_TEST_OBJS = $(BUILD_DIR)/game/anim.o $(BUILD_DIR)/game/object.o

$(BUILD_DIR)/test_anim: $(SRC_DIR)/tests/test_anim.cpp $(ANIM_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test campaign system
test_campaign: $(BUILD_DIR)/test_campaign
	@echo "Running campaign system tests..."
	@./$(BUILD_DIR)/test_campaign

CAMPAIGN_TEST_OBJS = $(BUILD_DIR)/game/campaign.o $(BUILD_DIR)/game/scenario.o \
                     $(BUILD_DIR)/game/house.o $(BUILD_DIR)/game/team.o \
                     $(BUILD_DIR)/game/trigger.o $(BUILD_DIR)/game/ini.o \
                     $(BUILD_DIR)/game/object.o

$(BUILD_DIR)/test_campaign: $(SRC_DIR)/tests/test_campaign.cpp $(CAMPAIGN_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test VQA video system
test_vqa: $(BUILD_DIR)/test_vqa
	@echo "Running VQA video tests..."
	@./$(BUILD_DIR)/test_vqa

$(BUILD_DIR)/test_vqa: $(SRC_DIR)/tests/test_vqa.cpp $(BUILD_DIR)/video/vqa.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test music system
test_music: $(BUILD_DIR)/test_music
	@echo "Running music system tests..."
	@./$(BUILD_DIR)/test_music

$(BUILD_DIR)/test_music: $(SRC_DIR)/tests/test_music.cpp $(BUILD_DIR)/video/music.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Test MIX decryption
test_mix_decrypt: $(BUILD_DIR)/test_mix_decrypt
	@echo "Running MIX decryption test..."
	@cd $(BUILD_DIR) && ./test_mix_decrypt

MIX_DECRYPT_TEST_OBJS = $(BUILD_DIR)/assets/mixfile.o $(BUILD_DIR)/crypto/blowfish.o $(BUILD_DIR)/crypto/mixkey.o

$(BUILD_DIR)/test_mix_decrypt: $(SRC_DIR)/tests/test_mix_decrypt.cpp $(MIX_DECRYPT_TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

.PHONY: all clean run dist dmg test_assets test_ini test_rules test_objects test_map test_entities test_combat test_ai test_scenario test_sidebar test_radar test_saveload test_anim test_campaign test_vqa test_music test_mix_decrypt
