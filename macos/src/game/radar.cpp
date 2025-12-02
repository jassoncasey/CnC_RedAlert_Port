/**
 * Red Alert macOS Port - Radar/Minimap Implementation
 *
 * Based on original RADAR.CPP (~4K lines)
 */

#include "radar.h"
#include "mapclass.h"
#include "cell.h"
#include "object.h"
#include <algorithm>
#include <cstring>

//===========================================================================
// Global Instance
//===========================================================================

RadarClass Radar;

//===========================================================================
// House Colors (ARGB format)
//===========================================================================

static const uint32_t HouseColors[] = {
    0xFF00AA00,  // SPAIN - Green (player default)
    0xFFAA0000,  // GREECE - Red
    0xFF0000AA,  // USSR - Blue
    0xFFAAAA00,  // ENGLAND - Yellow
    0xFF00AAAA,  // UKRAINE - Cyan
    0xFFAA00AA,  // GERMANY - Magenta
    0xFFFF6600,  // FRANCE - Orange
    0xFF888888,  // TURKEY - Gray
    0xFFFFFF00,  // GOOD - Bright yellow (allies)
    0xFFFF0000,  // BAD - Bright red (soviets)
    0xFF444444,  // NEUTRAL - Dark gray
    0xFF666666,  // SPECIAL - Medium gray
    0xFFFFFFFF,  // MULTI1-8 - Various
    0xFFFF00FF,
    0xFF00FFFF,
    0xFFFFFF00,
    0xFF8888FF,
    0xFFFF8888,
    0xFF88FF88,
    0xFFFF88FF,
};

//===========================================================================
// Terrain Colors (ARGB format)
//===========================================================================

static const uint32_t TerrainColors[] = {
    0xFF000000,  // Black (fog of war)
    0xFF2244AA,  // Water - Blue
    0xFF886644,  // Land/Clear - Brown
    0xFF666666,  // Road - Gray
    0xFFAAAA22,  // Tiberium/Ore - Yellow-green
    0xFF226622,  // Trees - Dark green
    0xFF444422,  // Rock - Dark brown
    0xFF884444,  // Beach - Tan
    0xFF222222,  // Cliff - Dark
    0xFFAA8866,  // Rough - Light brown
};

//===========================================================================
// RadarClass Implementation
//===========================================================================

RadarClass::RadarClass() {
    Init();
}

void RadarClass::Init() {
    map_ = nullptr;
    player_ = nullptr;

    isToRedraw_ = true;
    cursorRedraw_ = true;
    doesRadarExist_ = false;
    isRadarActive_ = false;
    isRadarActivating_ = false;
    isRadarDeactivating_ = false;
    isRadarJammed_ = false;
    isZoomed_ = false;

    radarScreenX_ = RADAR_X;
    radarScreenY_ = RADAR_Y;
    radarDisplayWidth_ = RADAR_WIDTH;
    radarDisplayHeight_ = RADAR_HEIGHT;

    radarCellX_ = 0;
    radarCellY_ = 0;
    radarCellWidth_ = 64;
    radarCellHeight_ = 64;
    zoomFactor_ = ZOOM_FACTOR_OUT;

    baseX_ = 0;
    baseY_ = 0;

    animFrame_ = 0;
    cursorPulseFrame_ = 0;

    pixelPtr_ = 0;
    std::memset(pixelStack_, 0, sizeof(pixelStack_));

    tacticalCell_ = 0;
    tacticalWidth_ = 20;
    tacticalHeight_ = 16;
}

void RadarClass::One_Time() {
    doesRadarExist_ = true;
    Calculate_Zoom_Parameters();
}

void RadarClass::AI() {
    // Handle activation animation
    if (isRadarActivating_) {
        animFrame_++;
        if (animFrame_ >= RADAR_ACTIVATED_FRAME) {
            isRadarActivating_ = false;
            isRadarActive_ = true;
            isToRedraw_ = true;
        }
    }

    // Handle deactivation animation
    if (isRadarDeactivating_) {
        animFrame_--;
        if (animFrame_ <= 0) {
            isRadarDeactivating_ = false;
            isRadarActive_ = false;
            isToRedraw_ = true;
        }
    }

    // Pulse animation for cursor
    cursorPulseFrame_ = (cursorPulseFrame_ + 1) % 24;
}

void RadarClass::Draw(uint32_t* framebuffer, int screenWidth,
                      int screenHeight) {
    if (!framebuffer) return;
    if (!doesRadarExist_) return;

    // Draw radar background (dark area)
    Draw_Rect(framebuffer, screenWidth,
              radarScreenX_, radarScreenY_,
              radarDisplayWidth_, radarDisplayHeight_,
              0xFF111111);

    // If not active, show disabled state
    if (!isRadarActive_) {
        // Draw "RADAR OFF" indicator or animation
        if (isRadarActivating_ || isRadarDeactivating_) {
            // Draw activation animation frame
            int progress = animFrame_ * radarDisplayHeight_;
            progress /= RADAR_ACTIVATED_FRAME;
            Draw_Rect(framebuffer, screenWidth,
                      radarScreenX_, radarScreenY_,
                      radarDisplayWidth_, progress,
                      0xFF222222);
        }
        return;
    }

    // Draw the radar map
    if (map_) {
        // Full redraw or incremental
        if (isToRedraw_) {
            // Draw all visible cells
            for (int cy = 0; cy < radarCellHeight_; cy++) {
                for (int cx = 0; cx < radarCellWidth_; cx++) {
                    int cellX = radarCellX_ + cx;
                    int cellY = radarCellY_ + cy;
                    if (cellX >= 0 && cellX < MAP_CELL_WIDTH &&
                        cellY >= 0 && cellY < MAP_CELL_HEIGHT) {
                        int16_t cell = cellY * MAP_CELL_WIDTH + cellX;
                        Render_Cell(cell, framebuffer, screenWidth);
                    }
                }
            }
            isToRedraw_ = false;
        } else {
            // Incremental update - process queued cells
            for (int i = 0; i < pixelPtr_; i++) {
                Render_Cell(pixelStack_[i], framebuffer, screenWidth);
            }
            pixelPtr_ = 0;
        }
    }

    // Draw tactical cursor (viewport bounds)
    Render_Cursor(framebuffer, screenWidth);

    // Draw radar border
    Draw_Rect_Outline(framebuffer, screenWidth,
                      radarScreenX_ - 1, radarScreenY_ - 1,
                      radarDisplayWidth_ + 2, radarDisplayHeight_ + 2,
                      0xFF444444);
}

void RadarClass::Activate(int control) {
    switch (control) {
        case -1:  // Toggle
            if (isRadarActive_) {
                isRadarDeactivating_ = true;
                animFrame_ = RADAR_ACTIVATED_FRAME;
            } else {
                isRadarActivating_ = true;
                animFrame_ = 0;
            }
            break;

        case 0:  // Off
            if (isRadarActive_) {
                isRadarDeactivating_ = true;
                animFrame_ = RADAR_ACTIVATED_FRAME;
            }
            break;

        case 1:  // On
            if (!isRadarActive_) {
                isRadarActivating_ = true;
                animFrame_ = 0;
            }
            break;

        case 4:  // Complete disable
            isRadarActive_ = false;
            isRadarActivating_ = false;
            isRadarDeactivating_ = false;
            doesRadarExist_ = false;
            break;
    }
}

void RadarClass::Zoom_Mode(int16_t centerCell) {
    isZoomed_ = !isZoomed_;

    if (centerCell >= 0 && isZoomed_) {
        Center_On_Cell(centerCell);
    }

    Calculate_Zoom_Parameters();
    isToRedraw_ = true;
}

bool RadarClass::Is_Zoomable() const {
    if (!map_) return false;

    // Can zoom if zoomed view would be different from unzoomed
    int unzoomedCells = std::min(radarDisplayWidth_, radarDisplayHeight_);
    int mapCells = std::min(map_->MapCellWidth(), map_->MapCellHeight());
    return mapCells > unzoomedCells / ZOOM_FACTOR_IN;
}

void RadarClass::Radar_Pixel(int16_t cell) {
    if (pixelPtr_ < PIXEL_STACK_SIZE) {
        // Check if cell already in queue
        for (int i = 0; i < pixelPtr_; i++) {
            if (pixelStack_[i] == cell) return;
        }
        pixelStack_[pixelPtr_++] = cell;
    }
}

void RadarClass::Plot_Radar_Pixel(int16_t cell, uint32_t* framebuffer,
                                  int screenWidth) {
    Render_Cell(cell, framebuffer, screenWidth);
}

void RadarClass::Full_Redraw() {
    isToRedraw_ = true;
    cursorRedraw_ = true;
}

void RadarClass::Radar_Cursor(bool forced) {
    cursorRedraw_ = forced;
}

bool RadarClass::Click_In_Radar(int x, int y) const {
    return (x >= radarScreenX_ && x < radarScreenX_ + radarDisplayWidth_ &&
            y >= radarScreenY_ && y < radarScreenY_ + radarDisplayHeight_);
}

int16_t RadarClass::Click_Cell_Calc(int x, int y) const {
    if (!Click_In_Radar(x, y)) return -1;

    // Convert screen position to radar-relative
    int rx = x - radarScreenX_ - baseX_;
    int ry = y - radarScreenY_ - baseY_;

    // Convert to cell coordinates
    int cellX = radarCellX_ + (rx / zoomFactor_);
    int cellY = radarCellY_ + (ry / zoomFactor_);

    // Bounds check
    if (cellX < 0 || cellX >= MAP_CELL_WIDTH ||
        cellY < 0 || cellY >= MAP_CELL_HEIGHT) {
        return -1;
    }

    return static_cast<int16_t>(cellY * MAP_CELL_WIDTH + cellX);
}

bool RadarClass::Cell_On_Radar(int16_t cell) const {
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return false;

    int cellX = cell % MAP_CELL_WIDTH;
    int cellY = cell / MAP_CELL_WIDTH;

    return (cellX >= radarCellX_ && cellX < radarCellX_ + radarCellWidth_ &&
            cellY >= radarCellY_ && cellY < radarCellY_ + radarCellHeight_);
}

void RadarClass::Set_Radar_Position(int16_t cell) {
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return;

    int cellX = cell % MAP_CELL_WIDTH;
    int cellY = cell / MAP_CELL_WIDTH;

    // Center the view on this cell
    radarCellX_ = cellX - radarCellWidth_ / 2;
    radarCellY_ = cellY - radarCellHeight_ / 2;

    // Clamp to map bounds
    int maxX = MAP_CELL_WIDTH - radarCellWidth_;
    int maxY = MAP_CELL_HEIGHT - radarCellHeight_;
    radarCellX_ = std::max(0, std::min(radarCellX_, maxX));
    radarCellY_ = std::max(0, std::min(radarCellY_, maxY));

    isToRedraw_ = true;
}

void RadarClass::Center_On_Cell(int16_t cell) {
    Set_Radar_Position(cell);
}

void RadarClass::Map_Cell(int16_t cell, HouseClass* house) {
    (void)house;  // In full implementation, tracks per-house visibility
    Radar_Pixel(cell);
}

void RadarClass::Jam_Cell(int16_t cell, HouseClass* house) {
    (void)cell;
    (void)house;
    // In full implementation, sets jam bit for house
}

void RadarClass::UnJam_Cell(int16_t cell, HouseClass* house) {
    (void)cell;
    (void)house;
    // In full implementation, clears jam bit for house
}

void RadarClass::Set_Tactical_View(int16_t topLeftCell, int viewWidth,
                                   int viewHeight) {
    tacticalCell_ = topLeftCell;
    tacticalWidth_ = viewWidth;
    tacticalHeight_ = viewHeight;
    cursorRedraw_ = true;
}

//===========================================================================
// Private Methods
//===========================================================================

uint32_t RadarClass::Get_Cell_Color(int16_t cell) const {
    if (!map_) return TerrainColors[0];  // Black
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return TerrainColors[0];

    const CellClass& cellRef = (*map_)[cell];

    // Check visibility (fog of war)
    // In full implementation, check per-house mapping
    // For now, show all mapped cells
    if (!cellRef.IsMapped()) {
        return TerrainColors[0];  // Black for unexplored
    }

    // Priority: Unit > Building > Terrain
    uint32_t unitColor = Get_Unit_Color(cell);
    if (unitColor != 0) return unitColor;

    uint32_t buildingColor = Get_Building_Color(cell);
    if (buildingColor != 0) return buildingColor;

    return Get_Terrain_Color(cell);
}

uint32_t RadarClass::Get_Terrain_Color(int16_t cell) const {
    if (!map_) return TerrainColors[2];  // Default land
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return TerrainColors[2];

    const CellClass& cellRef = (*map_)[cell];
    LandType land = cellRef.GetLandType();

    switch (land) {
        case LandType::WATER:
            return TerrainColors[1];  // Blue

        case LandType::ROAD:
            return TerrainColors[3];  // Gray

        case LandType::ROCK:
        case LandType::WALL:
            return TerrainColors[6];  // Dark brown

        case LandType::BEACH:
            return TerrainColors[7];  // Tan

        case LandType::ROUGH:
            return TerrainColors[9];  // Light brown

        case LandType::CLEAR:
        default:
            return TerrainColors[2];  // Brown
    }
}

uint32_t RadarClass::Get_Unit_Color(int16_t cell) const {
    if (!map_) return 0;
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return 0;

    const CellClass& cellRef = (*map_)[cell];

    // Check for unit occupier
    ObjectClass* obj = cellRef.CellOccupier();
    if (!obj) return 0;

    RTTIType type = obj->WhatAmI();
    if (type != RTTIType::INFANTRY && type != RTTIType::UNIT &&
        type != RTTIType::AIRCRAFT) {
        return 0;
    }

    // Get house color
    HousesType house = obj->Owner();
    return House_Color(house);
}

uint32_t RadarClass::Get_Building_Color(int16_t cell) const {
    if (!map_) return 0;
    if (cell < 0 || cell >= MAP_CELL_TOTAL) return 0;

    const CellClass& cellRef = (*map_)[cell];

    // Check for building occupier
    ObjectClass* obj = cellRef.CellOccupier();
    if (!obj) return 0;

    if (obj->WhatAmI() != RTTIType::BUILDING) return 0;

    // Get house color (slightly brighter for buildings)
    HousesType house = obj->Owner();
    uint32_t color = House_Color(house);

    // Brighten the color for buildings
    uint32_t r = std::min(255u, ((color >> 16) & 0xFF) + 40);
    uint32_t g = std::min(255u, ((color >> 8) & 0xFF) + 40);
    uint32_t b = std::min(255u, (color & 0xFF) + 40);
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

uint32_t RadarClass::House_Color(HousesType house) const {
    int idx = static_cast<int>(house);
    constexpr int maxIdx = sizeof(HouseColors) / sizeof(HouseColors[0]);
    if (idx >= 0 && idx < maxIdx) {
        return HouseColors[idx];
    }
    return HouseColors[10];  // Neutral gray
}

void RadarClass::Cell_To_Radar_Pixel(int16_t cell, int& px, int& py) const {
    int cellX = cell % MAP_CELL_WIDTH;
    int cellY = cell / MAP_CELL_WIDTH;

    px = radarScreenX_ + baseX_ + (cellX - radarCellX_) * zoomFactor_;
    py = radarScreenY_ + baseY_ + (cellY - radarCellY_) * zoomFactor_;
}

int16_t RadarClass::Radar_Pixel_To_Cell(int px, int py) const {
    int rx = px - radarScreenX_ - baseX_;
    int ry = py - radarScreenY_ - baseY_;

    int cellX = radarCellX_ + rx / zoomFactor_;
    int cellY = radarCellY_ + ry / zoomFactor_;

    if (cellX < 0 || cellX >= MAP_CELL_WIDTH ||
        cellY < 0 || cellY >= MAP_CELL_HEIGHT) {
        return -1;
    }

    return static_cast<int16_t>(cellY * MAP_CELL_WIDTH + cellX);
}

void RadarClass::Render_Cell(int16_t cell, uint32_t* framebuffer,
                             int screenWidth) {
    if (!Cell_On_Radar(cell)) return;

    int px, py;
    Cell_To_Radar_Pixel(cell, px, py);

    uint32_t color = Get_Cell_Color(cell);

    // Draw cell (size depends on zoom factor)
    if (zoomFactor_ == 1) {
        Draw_Pixel(framebuffer, screenWidth, px, py, color);
    } else {
        int zf = zoomFactor_;
        Draw_Rect(framebuffer, screenWidth, px, py, zf, zf, color);
    }
}

void RadarClass::Render_Cursor(uint32_t* framebuffer, int screenWidth) {
    if (!isRadarActive_) return;

    // Calculate tactical view bounds on radar
    int tacX = tacticalCell_ % MAP_CELL_WIDTH;
    int tacY = tacticalCell_ / MAP_CELL_WIDTH;

    int cursorX = radarScreenX_ + baseX_ + (tacX - radarCellX_) * zoomFactor_;
    int cursorY = radarScreenY_ + baseY_ + (tacY - radarCellY_) * zoomFactor_;
    int cursorW = tacticalWidth_ * zoomFactor_;
    int cursorH = tacticalHeight_ * zoomFactor_;

    // Clamp to radar bounds
    if (cursorX < radarScreenX_) {
        cursorW -= (radarScreenX_ - cursorX);
        cursorX = radarScreenX_;
    }
    if (cursorY < radarScreenY_) {
        cursorH -= (radarScreenY_ - cursorY);
        cursorY = radarScreenY_;
    }
    if (cursorX + cursorW > radarScreenX_ + radarDisplayWidth_) {
        cursorW = radarScreenX_ + radarDisplayWidth_ - cursorX;
    }
    if (cursorY + cursorH > radarScreenY_ + radarDisplayHeight_) {
        cursorH = radarScreenY_ + radarDisplayHeight_ - cursorY;
    }

    if (cursorW <= 0 || cursorH <= 0) return;

    // Pulse color based on animation frame
    uint32_t cursorColor = (cursorPulseFrame_ < 12) ? 0xFF00FF00 : 0xFF00AA00;

    // Draw cursor outline
    Draw_Rect_Outline(framebuffer, screenWidth,
                      cursorX, cursorY, cursorW, cursorH, cursorColor);
}

void RadarClass::Draw_Pixel(uint32_t* framebuffer, int screenWidth,
                            int x, int y, uint32_t color) {
    if (x >= 0 && x < screenWidth && y >= 0) {
        framebuffer[y * screenWidth + x] = color;
    }
}

void RadarClass::Draw_Rect(uint32_t* framebuffer, int screenWidth,
                           int x, int y, int w, int h, uint32_t color) {
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            Draw_Pixel(framebuffer, screenWidth, px, py, color);
        }
    }
}

void RadarClass::Draw_Rect_Outline(uint32_t* framebuffer, int screenWidth,
                                   int x, int y, int w, int h,
                                   uint32_t color) {
    // Top and bottom
    for (int px = x; px < x + w; px++) {
        Draw_Pixel(framebuffer, screenWidth, px, y, color);
        Draw_Pixel(framebuffer, screenWidth, px, y + h - 1, color);
    }
    // Left and right
    for (int py = y; py < y + h; py++) {
        Draw_Pixel(framebuffer, screenWidth, x, py, color);
        Draw_Pixel(framebuffer, screenWidth, x + w - 1, py, color);
    }
}

void RadarClass::Calculate_Zoom_Parameters() {
    if (!map_) {
        // Default parameters for 64x64 map
        radarCellWidth_ = 64;
        radarCellHeight_ = 64;
        zoomFactor_ = 1;
        baseX_ = (radarDisplayWidth_ - radarCellWidth_ * zoomFactor_) / 2;
        baseY_ = (radarDisplayHeight_ - radarCellHeight_ * zoomFactor_) / 2;
        return;
    }

    int mapWidth = map_->MapCellWidth();
    int mapHeight = map_->MapCellHeight();

    if (isZoomed_) {
        // Zoomed in: 3 pixels per cell
        zoomFactor_ = ZOOM_FACTOR_IN;
        radarCellWidth_ = radarDisplayWidth_ / zoomFactor_;
        radarCellHeight_ = radarDisplayHeight_ / zoomFactor_;

        // Clamp to map size
        radarCellWidth_ = std::min(radarCellWidth_, mapWidth);
        radarCellHeight_ = std::min(radarCellHeight_, mapHeight);
    } else {
        // Zoomed out: fit entire map
        int zoomX = radarDisplayWidth_ / mapWidth;
        int zoomY = radarDisplayHeight_ / mapHeight;
        zoomFactor_ = std::max(1, std::min(zoomX, zoomY));

        radarCellWidth_ = mapWidth;
        radarCellHeight_ = mapHeight;
        radarCellX_ = 0;
        radarCellY_ = 0;
    }

    // Calculate centering offsets
    int usedWidth = radarCellWidth_ * zoomFactor_;
    int usedHeight = radarCellHeight_ * zoomFactor_;
    baseX_ = (radarDisplayWidth_ - usedWidth) / 2;
    baseY_ = (radarDisplayHeight_ - usedHeight) / 2;
    baseX_ = std::max(0, baseX_);
    baseY_ = std::max(0, baseY_);

    isToRedraw_ = true;
}
