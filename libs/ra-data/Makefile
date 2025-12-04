# ra-data Library
#
# Red Alert game data: asset categorization, type definitions.
# Independent of rendering/audio - pure data logic.
#
# Target: macOS 14+ (Sonoma), ARM64

CXX = clang++

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

# Compiler flags
ARCH = -arch arm64
MIN_VERSION = -mmacosx-version-min=14.0
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(ARCH) $(MIN_VERSION)
INCLUDES = -I$(INCLUDE_DIR)

# Sources
SOURCES = $(SRC_DIR)/category.cpp
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Target
TARGET = $(BUILD_DIR)/libra-data.a

# Default target
all: $(TARGET)

# Static library
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $(OBJECTS)
	@echo "Built $(TARGET)"

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Test
test: $(TARGET)
	@echo "Building category test..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/test_category \
		$(SRC_DIR)/test_category.cpp $(TARGET)
	./$(BUILD_DIR)/test_category

.PHONY: all clean test
