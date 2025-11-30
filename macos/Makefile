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
              $(SRC_DIR)/game/building_types.cpp $(SRC_DIR)/game/ini.cpp $(SRC_DIR)/game/rules.cpp

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

.PHONY: all clean run dist dmg test_assets test_ini test_rules
