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
FRAMEWORKS = -framework Cocoa -framework Metal -framework MetalKit -framework QuartzCore

# Linker flags
LDFLAGS = $(ARCH) $(MIN_VERSION) $(FRAMEWORKS)

# Sources (add as we go)
OBJCXX_SOURCES = $(SRC_DIR)/main.mm $(SRC_DIR)/graphics/metal/renderer.mm
CPP_SOURCES = $(SRC_DIR)/platform/file.cpp $(SRC_DIR)/platform/timing.cpp $(SRC_DIR)/platform/assets.cpp

# Objects
OBJCXX_OBJECTS = $(patsubst $(SRC_DIR)/%.mm,$(BUILD_DIR)/%.o,$(OBJCXX_SOURCES))
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(OBJCXX_OBJECTS) $(CPP_OBJECTS)

# Default target
all: $(APP_BUNDLE)

# App bundle
$(APP_BUNDLE): $(BUILD_DIR)/$(APP_NAME) resources/Info.plist
	@mkdir -p $(APP_BUNDLE)/Contents/MacOS
	@mkdir -p $(APP_BUNDLE)/Contents/Resources
	@cp $(BUILD_DIR)/$(APP_NAME) $(APP_BUNDLE)/Contents/MacOS/
	@cp resources/Info.plist $(APP_BUNDLE)/Contents/
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
	rm -rf $(BUILD_DIR) $(APP_BUNDLE)

# Run
run: $(APP_BUNDLE)
	./$(APP_BUNDLE)/Contents/MacOS/$(APP_NAME)

.PHONY: all clean run
