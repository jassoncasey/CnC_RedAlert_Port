# wwd-media - Shared Media Library
# Metal renderer, CoreAudio playback, VQA video decoder
#
# Target: macOS 14+ (Sonoma), ARM64

CXX = clang++
OBJCXX = clang++

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Output
LIB_NAME = libwwd-media.a

# Compiler flags
ARCH = -arch arm64
MIN_VERSION = -mmacosx-version-min=14.0
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(ARCH) $(MIN_VERSION)
OBJCXXFLAGS = $(CXXFLAGS) -fobjc-arc
INCLUDES = -I$(INCLUDE_DIR)

# Sources
OBJCXX_SOURCES = $(SRC_DIR)/renderer.mm $(SRC_DIR)/audio.mm
CPP_SOURCES = $(SRC_DIR)/vqa.cpp

# Objects
OBJCXX_OBJECTS = $(patsubst $(SRC_DIR)/%.mm,$(BUILD_DIR)/%.o,$(OBJCXX_SOURCES))
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(OBJCXX_OBJECTS) $(CPP_OBJECTS)

# Default target
all: $(BUILD_DIR)/$(LIB_NAME)

# Static library
$(BUILD_DIR)/$(LIB_NAME): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^
	@echo "Built $(LIB_NAME)"

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
	rm -rf $(BUILD_DIR)

# Install headers (for dependent projects)
install-headers:
	@echo "Headers are in $(INCLUDE_DIR)/wwd/"

.PHONY: all clean install-headers
