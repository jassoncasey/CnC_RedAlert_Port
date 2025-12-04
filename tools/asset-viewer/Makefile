# Asset Viewer Tool
#
# Displays and inspects Westwood game assets (MIX, SHP, PAL, AUD, VQA, etc.)
# Uses wwd-media for rendering and audio, libwestwood for asset decoding.
#
# Target: macOS 14+ (Sonoma), ARM64

CXX = clang++
OBJCXX = clang++

# Directories
BUILD_DIR = build
ROOT_DIR = ../..
WWD_MEDIA_DIR = $(ROOT_DIR)/libs/wwd-media
RA_DATA_DIR = $(ROOT_DIR)/libs/ra-data
LIBWESTWOOD_DIR = $(ROOT_DIR)/submodules/westwood-formats/impl

# Libraries
WWD_MEDIA_LIB = $(WWD_MEDIA_DIR)/build/libwwd-media.a
RA_DATA_LIB = $(RA_DATA_DIR)/build/libra-data.a
LIBWESTWOOD_LIB = $(LIBWESTWOOD_DIR)/build/libwestwood.a

# Compiler flags
ARCH = -arch arm64
MIN_VERSION = -mmacosx-version-min=14.0
# Compatibility layer uses C++23 to match libwestwood
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 $(ARCH) $(MIN_VERSION)
OBJCXXFLAGS = $(CXXFLAGS) -fobjc-arc

# Include paths
INCLUDES = -I. \
           -I$(WWD_MEDIA_DIR)/include \
           -I$(RA_DATA_DIR)/include \
           -I$(LIBWESTWOOD_DIR)/libwestwood/include

# Frameworks
FRAMEWORKS = -framework Cocoa -framework Metal -framework MetalKit \
             -framework QuartzCore -framework AudioToolbox \
             -framework AVFoundation -framework UniformTypeIdentifiers

# Linker flags
LDFLAGS = $(ARCH) $(MIN_VERSION) $(FRAMEWORKS)

# Target
TARGET = $(BUILD_DIR)/asset-viewer

# Compatibility layer object
COMPAT_OBJ = $(BUILD_DIR)/wwd_compat.o

# Default target
all: $(TARGET)

# Build wwd-media library
$(WWD_MEDIA_LIB):
	$(MAKE) -C $(WWD_MEDIA_DIR)

# Build ra-data library
$(RA_DATA_LIB):
	$(MAKE) -C $(RA_DATA_DIR)

# Build libwestwood (using cmake)
$(LIBWESTWOOD_LIB):
	cd $(LIBWESTWOOD_DIR) && mkdir -p build && cd build && \
		cmake .. -DCMAKE_BUILD_TYPE=Release -DWWD_BUILD_CLI=OFF && \
		$(MAKE)

# Compile compatibility layer
$(COMPAT_OBJ): wwd_compat.cpp wwd_compat.h
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ wwd_compat.cpp

# Link
$(TARGET): asset_viewer.mm $(COMPAT_OBJ) $(WWD_MEDIA_LIB) $(RA_DATA_LIB) $(LIBWESTWOOD_LIB)
	@mkdir -p $(BUILD_DIR)
	$(OBJCXX) $(OBJCXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ \
		asset_viewer.mm $(COMPAT_OBJ) $(WWD_MEDIA_LIB) $(RA_DATA_LIB) $(LIBWESTWOOD_LIB)

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
