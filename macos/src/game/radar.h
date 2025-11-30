/**
 * Red Alert macOS Port - Radar/Minimap System
 *
 * Based on original RADAR.CPP (~4K lines)
 * Provides tactical minimap showing terrain, units, and buildings
 */

#pragma once

#include "types.h"
#include "house.h"
#include <cstdint>

//===========================================================================
// Constants
//===========================================================================

// Radar display dimensions (in pixels)
constexpr int RADAR_X = 256;        // Screen X position (right side)
constexpr int RADAR_Y = 8;          // Screen Y position (top area)
constexpr int RADAR_WIDTH = 72;     // Display width in pixels
constexpr int RADAR_HEIGHT = 69;    // Display height in pixels

// Radar animation
constexpr int RADAR_ACTIVATED_FRAME = 22;
constexpr int MAX_RADAR_FRAMES = 41;

// Pixel update queue
constexpr int PIXEL_STACK_SIZE = 400;

// Zoom factors
constexpr int ZOOM_FACTOR_OUT = 1;  // One pixel per cell (full map)
constexpr int ZOOM_FACTOR_IN = 3;   // 3x3 pixels per cell (zoomed)

// Radar colors (palette indices - simplified for port)
constexpr uint8_t RADAR_COLOR_BLACK = 0;
constexpr uint8_t RADAR_COLOR_WATER = 1;      // Blue
constexpr uint8_t RADAR_COLOR_LAND = 2;       // Brown/tan
constexpr uint8_t RADAR_COLOR_ROAD = 3;       // Gray
constexpr uint8_t RADAR_COLOR_TIBERIUM = 4;   // Yellow/green
constexpr uint8_t RADAR_COLOR_TREE = 5;       // Dark green

// House colors for units on radar (simplified)
constexpr uint8_t RADAR_COLOR_PLAYER = 10;    // Green
constexpr uint8_t RADAR_COLOR_ENEMY = 11;     // Red
constexpr uint8_t RADAR_COLOR_NEUTRAL = 12;   // Yellow
constexpr uint8_t RADAR_COLOR_BUILDING = 13;  // Brighter version

//===========================================================================
// RadarClass
//===========================================================================

class MapClass;  // Forward declaration

class RadarClass {
public:
    RadarClass();
    ~RadarClass() = default;

    // Initialization
    void Init();
    void One_Time();

    // AI and rendering
    void AI();
    void Draw(uint32_t* framebuffer, int screenWidth, int screenHeight);

    // Radar control
    void Activate(int control);  // -1=toggle, 0=off, 1=on
    void Zoom_Mode(int16_t centerCell = -1);
    bool Is_Zoomable() const;

    // Cell updates
    void Radar_Pixel(int16_t cell);  // Queue cell for redraw
    void Plot_Radar_Pixel(int16_t cell, uint32_t* framebuffer, int screenWidth);
    void Full_Redraw();

    // Cursor and click handling
    void Radar_Cursor(bool forced);
    bool Click_In_Radar(int x, int y) const;
    int16_t Click_Cell_Calc(int x, int y) const;
    bool Cell_On_Radar(int16_t cell) const;

    // Position control
    void Set_Radar_Position(int16_t cell);
    void Center_On_Cell(int16_t cell);

    // Map/Jam operations
    void Map_Cell(int16_t cell, HouseClass* house);
    void Jam_Cell(int16_t cell, HouseClass* house);
    void UnJam_Cell(int16_t cell, HouseClass* house);

    // State queries
    bool Is_Active() const { return isRadarActive_; }
    bool Is_Radar_Jammed() const { return isRadarJammed_; }
    bool Is_Zoomed() const { return isZoomed_; }

    // Link to map
    void Set_Map(MapClass* map) { map_ = map; }
    void Set_Player(HouseClass* player) { player_ = player; }

    // Tactical view (viewport bounds to show on radar)
    void Set_Tactical_View(int16_t topLeftCell, int viewWidth, int viewHeight);

    // Get radar screen position
    int GetX() const { return radarScreenX_; }
    int GetY() const { return radarScreenY_; }
    int GetWidth() const { return radarDisplayWidth_; }
    int GetHeight() const { return radarDisplayHeight_; }

private:
    // Color determination
    uint32_t Get_Cell_Color(int16_t cell) const;
    uint32_t Get_Terrain_Color(int16_t cell) const;
    uint32_t Get_Unit_Color(int16_t cell) const;
    uint32_t Get_Building_Color(int16_t cell) const;
    uint32_t House_Color(HousesType house) const;

    // Coordinate conversion
    void Cell_To_Radar_Pixel(int16_t cell, int& px, int& py) const;
    int16_t Radar_Pixel_To_Cell(int px, int py) const;

    // Rendering helpers
    void Render_Cell(int16_t cell, uint32_t* framebuffer, int screenWidth);
    void Render_Cursor(uint32_t* framebuffer, int screenWidth);
    void Draw_Pixel(uint32_t* framebuffer, int screenWidth,
                    int x, int y, uint32_t color);
    void Draw_Rect(uint32_t* framebuffer, int screenWidth,
                   int x, int y, int w, int h, uint32_t color);
    void Draw_Rect_Outline(uint32_t* framebuffer, int screenWidth,
                           int x, int y, int w, int h, uint32_t color);

    // Calculate zoom parameters
    void Calculate_Zoom_Parameters();

private:
    // Link to game systems
    MapClass* map_;
    HouseClass* player_;

    // Radar state flags
    bool isToRedraw_;           // Full redraw needed
    bool cursorRedraw_;         // Cursor needs redraw
    bool doesRadarExist_;       // Radar hardware exists
    bool isRadarActive_;        // Radar is on
    bool isRadarActivating_;    // Power-on animation
    bool isRadarDeactivating_;  // Power-off animation
    bool isRadarJammed_;        // Radar jammed by enemy
    bool isZoomed_;             // Zoomed in vs full map

    // Screen position
    int radarScreenX_;          // Screen X position
    int radarScreenY_;          // Screen Y position
    int radarDisplayWidth_;     // Actual display width
    int radarDisplayHeight_;    // Actual display height

    // Map view parameters
    int radarCellX_;            // Top-left cell X of radar view
    int radarCellY_;            // Top-left cell Y of radar view
    int radarCellWidth_;        // Cells visible horizontally
    int radarCellHeight_;       // Cells visible vertically
    int zoomFactor_;            // Pixels per cell

    // Centering offsets (for maps smaller than radar)
    int baseX_;
    int baseY_;

    // Animation state
    int animFrame_;             // Current animation frame
    int cursorPulseFrame_;      // Cursor blink/pulse frame

    // Pixel update queue (dirty cells)
    int16_t pixelStack_[PIXEL_STACK_SIZE];
    int pixelPtr_;

    // Tactical view (viewport cursor on radar)
    int16_t tacticalCell_;      // Top-left cell of main view
    int tacticalWidth_;         // Width in cells
    int tacticalHeight_;        // Height in cells

    // Visibility tracking per cell
    // Note: In full implementation, this would be per-cell flags
    // For now, we use a simple approach
};

//===========================================================================
// Global Instance
//===========================================================================

extern RadarClass Radar;
