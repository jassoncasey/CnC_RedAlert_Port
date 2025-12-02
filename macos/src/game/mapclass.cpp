/**
 * Red Alert macOS Port - MapClass Implementation
 *
 * Based on original MAP.CPP
 */

#include "mapclass.h"
#include "object.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>

//===========================================================================
// Global Map Instance
//===========================================================================
MapClass Map;

//===========================================================================
// Static Radius Data (pre-computed cell offsets for circular scans)
//===========================================================================

#define W MAP_CELL_W  // Short alias for cell width

const int MapClass::RADIUS_COUNT[11] = {
    1, 9, 21, 37, 61, 89, 121, 161, 205, 253, 309
};

// Pre-computed cell offsets for circular scans (ring by ring)
// clang-format off
const int MapClass::RADIUS_OFFSET[] = {
    0,  // r=0: center cell only
    // r=1: 8 cells
    (-W)-1,(-W),(-W)+1, -1,1, (W)-1,(W),(W)+1,
    // r=2: 12 cells
    (-W*2)-1,(-W*2),(-W*2)+1, (-W)-2,(-W)+2, -2,2,
    (W)-2,(W)+2, (W*2)-1,(W*2),(W*2)+1,
    // r=3: 16 cells
    (-W*3)-1,(-W*3),(-W*3)+1, (-W*2)-2,(-W*2)+2, (-W)-3,(-W)+3,
    -3,3, (W)-3,(W)+3, (W*2)-2,(W*2)+2, (W*3)-1,(W*3),(W*3)+1,
    // r=4: 24 cells
    (-W*4)-1,(-W*4),(-W*4)+1, (-W*3)-3,(-W*3)-2,(-W*3)+2,(-W*3)+3,
    (-W*2)-3,(-W*2)+3, (-W)-4,(-W)+4, -4,4, (W)-4,(W)+4,
    (W*2)-3,(W*2)+3, (W*3)-3,(W*3)-2,(W*3)+2,(W*3)+3,
    (W*4)-1,(W*4),(W*4)+1,
    // r=5: 28 cells
    (-W*5)-1,(-W*5),(-W*5)+1, (-W*4)-3,(-W*4)-2,(-W*4)+2,(-W*4)+3,
    (-W*3)-4,(-W*3)+4, (-W*2)-4,(-W*2)+4, (-W)-5,(-W)+5, -5,5,
    (W)-5,(W)+5, (W*2)-4,(W*2)+4, (W*3)-4,(W*3)+4,
    (W*4)-3,(W*4)-2,(W*4)+2,(W*4)+3, (W*5)-1,(W*5),(W*5)+1,
    // r=6: 32 cells
    (-W*6)-1,(-W*6),(-W*6)+1, (-W*5)-3,(-W*5)-2,(-W*5)+2,(-W*5)+3,
    (-W*4)-4,(-W*4)+4, (-W*3)-5,(-W*3)+5, (-W*2)-5,(-W*2)+5,
    (-W)-6,(-W)+6, -6,6, (W)-6,(W)+6, (W*2)-5,(W*2)+5,
    (W*3)-5,(W*3)+5, (W*4)-4,(W*4)+4,
    (W*5)-3,(W*5)-2,(W*5)+2,(W*5)+3, (W*6)-1,(W*6),(W*6)+1,
    // r=7: 40 cells
    (-W*7)-1,(-W*7),(-W*7)+1, (-W*6)-3,(-W*6)-2,(-W*6)+2,(-W*6)+3,
    (-W*5)-5,(-W*5)-4,(-W*5)+4,(-W*5)+5, (-W*4)-5,(-W*4)+5,
    (-W*3)-6,(-W*3)+6, (-W*2)-6,(-W*2)+6, (-W)-7,(-W)+7, -7,7,
    (W)-7,(W)+7, (W*2)-6,(W*2)+6, (W*3)-6,(W*3)+6, (W*4)-5,(W*4)+5,
    (W*5)-5,(W*5)-4,(W*5)+4,(W*5)+5,
    (W*6)-3,(W*6)-2,(W*6)+2,(W*6)+3, (W*7)-1,(W*7),(W*7)+1,
    // r=8: 44 cells
    (-W*8)-1,(-W*8),(-W*8)+1, (-W*7)-3,(-W*7)-2,(-W*7)+2,(-W*7)+3,
    (-W*6)-5,(-W*6)-4,(-W*6)+4,(-W*6)+5, (-W*5)-6,(-W*5)+6,
    (-W*4)-6,(-W*4)+6, (-W*3)-7,(-W*3)+7, (-W*2)-7,(-W*2)+7,
    (-W)-8,(-W)+8, -8,8, (W)-8,(W)+8, (W*2)-7,(W*2)+7,
    (W*3)-7,(W*3)+7, (W*4)-6,(W*4)+6, (W*5)-6,(W*5)+6,
    (W*6)-5,(W*6)-4,(W*6)+4,(W*6)+5,
    (W*7)-3,(W*7)-2,(W*7)+2,(W*7)+3, (W*8)-1,(W*8),(W*8)+1,
    // r=9: 48 cells
    (-W*9)-1,(-W*9),(-W*9)+1, (-W*8)-3,(-W*8)-2,(-W*8)+2,(-W*8)+3,
    (-W*7)-5,(-W*7)-4,(-W*7)+4,(-W*7)+5, (-W*6)-6,(-W*6)+6,
    (-W*5)-7,(-W*5)+7, (-W*4)-7,(-W*4)+7, (-W*3)-8,(-W*3)+8,
    (-W*2)-8,(-W*2)+8, (-W)-9,(-W)+9, -9,9, (W)-9,(W)+9,
    (W*2)-8,(W*2)+8, (W*3)-8,(W*3)+8, (W*4)-7,(W*4)+7,
    (W*5)-7,(W*5)+7, (W*6)-6,(W*6)+6,
    (W*7)-5,(W*7)-4,(W*7)+4,(W*7)+5,
    (W*8)-3,(W*8)-2,(W*8)+2,(W*8)+3, (W*9)-1,(W*9),(W*9)+1,
    // r=10: 56 cells
    (-W*10)-1,(-W*10),(-W*10)+1,
    (-W*9)-3,(-W*9)-2,(-W*9)+2,(-W*9)+3,
    (-W*8)-5,(-W*8)-4,(-W*8)+4,(-W*8)+5,
    (-W*7)-7,(-W*7)-6,(-W*7)+6,(-W*7)+7, (-W*6)-7,(-W*6)+7,
    (-W*5)-8,(-W*5)+8, (-W*4)-8,(-W*4)+8, (-W*3)-9,(-W*3)+9,
    (-W*2)-9,(-W*2)+9, (-W)-10,(-W)+10, -10,10, (W)-10,(W)+10,
    (W*2)-9,(W*2)+9, (W*3)-9,(W*3)+9, (W*4)-8,(W*4)+8,
    (W*5)-8,(W*5)+8, (W*6)-7,(W*6)+7,
    (W*7)-7,(W*7)-6,(W*7)+6,(W*7)+7,
    (W*8)-5,(W*8)-4,(W*8)+4,(W*8)+5,
    (W*9)-3,(W*9)-2,(W*9)+2,(W*9)+3, (W*10)-1,(W*10),(W*10)+1,
};
// clang-format on

#undef W

//===========================================================================
// Construction
//===========================================================================

MapClass::MapClass()
    : xSize_(MAP_CELL_W)
    , ySize_(MAP_CELL_H)
    , size_(MAP_CELL_TOTAL)
    , mapCellX_(0)
    , mapCellY_(0)
    , mapCellWidth_(MAP_CELL_W)
    , mapCellHeight_(MAP_CELL_H)
    , totalValue_(0)
    , tiberiumGrowthCount_(0)
    , tiberiumSpreadCount_(0)
    , tiberiumScan_(0)
{
}

MapClass::~MapClass() {
    FreeCells();
}

//===========================================================================
// Initialization
//===========================================================================

void MapClass::OneTime() {
    xSize_ = MAP_CELL_W;
    ySize_ = MAP_CELL_H;
    size_ = xSize_ * ySize_;
    AllocCells();
}

void MapClass::InitClear() {
    InitCells();
    tiberiumScan_ = 0;
    tiberiumGrowthCount_ = 0;
    tiberiumSpreadCount_ = 0;
    tiberiumGrowth_.clear();
    tiberiumSpread_.clear();
}

void MapClass::AllocCells() {
    cells_.clear();
    cells_.resize(size_);
}

void MapClass::FreeCells() {
    cells_.clear();
}

void MapClass::InitCells() {
    totalValue_ = 0;
    for (int i = 0; i < MAP_CELL_TOTAL; i++) {
        cells_[i].Clear();
        cells_[i].SetCellNumber(static_cast<CELL>(i));
    }
}

//===========================================================================
// Map Dimensions
//===========================================================================

void MapClass::SetMapDimensions(int x, int y, int w, int h) {
    mapCellX_ = x;
    mapCellY_ = y;
    mapCellWidth_ = w;
    mapCellHeight_ = h;
}

//===========================================================================
// Cell Access
//===========================================================================

CellClass& MapClass::operator[](CELL cell) {
    if (cell < 0 || cell >= size_) {
        // Return cell 0 for out-of-bounds (matches original behavior)
        return cells_[0];
    }
    return cells_[cell];
}

const CellClass& MapClass::operator[](CELL cell) const {
    if (cell < 0 || cell >= size_) {
        return cells_[0];
    }
    return cells_[cell];
}

CellClass& MapClass::operator[](int32_t coord) {
    return (*this)[Coord_Cell(coord)];
}

const CellClass& MapClass::operator[](int32_t coord) const {
    return (*this)[Coord_Cell(coord)];
}

bool MapClass::IsValidCell(CELL cell) const {
    return cell >= 0 && cell < size_;
}

bool MapClass::InRadar(CELL cell) const {
    if (!IsValidCell(cell)) return false;

    int x = Cell_X(cell);
    int y = Cell_Y(cell);

    return x >= mapCellX_ && x < mapCellX_ + mapCellWidth_ &&
           y >= mapCellY_ && y < mapCellY_ + mapCellHeight_;
}

//===========================================================================
// Random Location
//===========================================================================

CELL MapClass::PickRandomLocation() const {
    int x = mapCellX_ + (rand() % mapCellWidth_);
    int y = mapCellY_ + (rand() % mapCellHeight_);
    return XY_Cell(x, y);
}

CELL MapClass::NearbyLocation(CELL cell, SpeedType speed, int /*zone*/,
                              MZoneType /*check*/) const {
    // Find a passable cell near the given cell
    if (!IsValidCell(cell)) return cell;

    // Check the cell itself first
    if (cells_[cell].IsPassable(speed)) {
        return cell;
    }

    // Search in expanding rings
    for (int radius = 1; radius <= 10; radius++) {
        int count = RadiusCount(radius);
        const int* offsets = RadiusOffsets(radius);

        // Start from where the previous radius ended
        int start = (radius > 0) ? RADIUS_COUNT[radius - 1] : 0;

        for (int i = start; i < count; i++) {
            CELL newCell = cell + offsets[i];
            if (IsValidCell(newCell) && cells_[newCell].IsPassable(speed)) {
                return newCell;
            }
        }
    }

    return cell;  // No passable cell found
}

//===========================================================================
// Object Placement
//===========================================================================

void MapClass::PlaceDown(CELL cell, ObjectClass* object) {
    if (!IsValidCell(cell) || object == nullptr) return;
    cells_[cell].OccupyDown(object);
}

void MapClass::PickUp(CELL cell, ObjectClass* object) {
    if (!IsValidCell(cell) || object == nullptr) return;
    cells_[cell].OccupyUp(object);
}

void MapClass::OverlapDown(CELL cell, ObjectClass* object) {
    if (!IsValidCell(cell) || object == nullptr) return;
    cells_[cell].OverlapDown(object);
}

void MapClass::OverlapUp(CELL cell, ObjectClass* object) {
    if (!IsValidCell(cell) || object == nullptr) return;
    cells_[cell].OverlapUp(object);
}

//===========================================================================
// Visibility (Fog of War)
//===========================================================================

void MapClass::SightFrom(CELL cell, int sightRange, HouseClass* /*house*/,
                         bool incremental) {
    if (!InRadar(cell)) return;
    if (sightRange <= 0 || sightRange > 10) return;

    int xx = Cell_X(cell);
    int count = RADIUS_COUNT[sightRange];
    const int* ptr = &RADIUS_OFFSET[0];

    // Incremental scans only scan the outer rings
    if (incremental && sightRange > 2) {
        ptr += RADIUS_COUNT[sightRange - 3];
        count -= RADIUS_COUNT[sightRange - 3];
    }

    while (count--) {
        CELL newCell = cell + *ptr++;

        if (!IsValidCell(newCell)) continue;

        int xdiff = Cell_X(newCell) - xx;
        if (xdiff < 0) xdiff = -xdiff;
        if (xdiff > sightRange) continue;

        // Mark the cell as mapped and visible
        cells_[newCell].SetMapped(true);
        cells_[newCell].SetVisible(true);
    }
}

void MapClass::ShroudFrom(CELL cell, int sightRange) {
    if (!InRadar(cell)) return;
    if (sightRange <= 0 || sightRange > 10) return;

    int xx = Cell_X(cell);
    int count = RADIUS_COUNT[sightRange];
    const int* ptr = &RADIUS_OFFSET[0];

    while (count--) {
        CELL newCell = cell + *ptr++;

        if (!IsValidCell(newCell)) continue;

        int xdiff = Cell_X(newCell) - xx;
        if (xdiff < 0) xdiff = -xdiff;
        if (xdiff > sightRange) continue;

        // Mark the cell as not visible (but still mapped)
        cells_[newCell].SetVisible(false);
    }
}

void MapClass::ShroudTheMap() {
    for (int i = 0; i < size_; i++) {
        cells_[i].SetMapped(false);
        cells_[i].SetVisible(false);
    }
}

void MapClass::RevealTheMap() {
    for (int i = 0; i < size_; i++) {
        cells_[i].SetMapped(true);
        cells_[i].SetVisible(true);
    }
}

//===========================================================================
// Zone Management
//===========================================================================

bool MapClass::ZoneReset(int /*method*/) {
    // Reset all zone values
    for (int i = 0; i < size_; i++) {
        for (int z = 0; z < static_cast<int>(MZoneType::COUNT); z++) {
            cells_[i].zones_[z] = 0;
        }
    }
    return true;
}

bool MapClass::ZoneCell(CELL cell, int zone) {
    if (!IsValidCell(cell)) return false;

    // Set zone for all movement zone types
    for (int z = 0; z < static_cast<int>(MZoneType::COUNT); z++) {
        cells_[cell].zones_[z] = static_cast<uint8_t>(zone);
    }
    return true;
}

int MapClass::ZoneSpan(CELL cell, int zone, MZoneType check) {
    if (!IsValidCell(cell)) return 0;

    // Flood fill zone assignment
    // This is a simplified version - the original uses a more complex algorithm
    int count = 0;
    int zoneIdx = static_cast<int>(check);

    // Simple BFS flood fill
    std::vector<CELL> queue;
    queue.push_back(cell);

    while (!queue.empty()) {
        CELL current = queue.back();
        queue.pop_back();

        if (!IsValidCell(current)) continue;
        if (cells_[current].zones_[zoneIdx] == zone) continue;

        // Check passability based on zone type
        SpeedType speed = SpeedType::TRACK;
        if (check == MZoneType::WATER) speed = SpeedType::FLOAT;

        if (!cells_[current].IsPassable(speed)) continue;

        cells_[current].zones_[zoneIdx] = static_cast<uint8_t>(zone);
        count++;

        // Add adjacent cells
        for (int dir = 0; dir < 8; dir++) {
            FacingType face = static_cast<FacingType>(dir);
            CELL adj = cells_[current].AdjacentCell(face);
            if (adj != current) {
                queue.push_back(adj);
            }
        }
    }

    return count;
}

//===========================================================================
// Ore/Tiberium Management
//===========================================================================

void MapClass::Logic() {
    // Process a portion of the map each frame for ore growth/spread
    int remaining = MAP_CELL_TOTAL - static_cast<int>(tiberiumScan_);
    int scanCount = std::min(64, remaining);

    for (int i = 0; i < scanCount; i++) {
        CELL cell = tiberiumScan_ + i;
        if (IsValidCell(cell)) {
            CellClass& c = cells_[cell];

            // Check for ore growth
            if (c.CanOreGrow()) {
                if ((rand() % 256) < 4) {  // ~1.5% chance per scan
                    c.GrowOre();
                }
            }

            // Check for ore spread
            if (c.CanOreSpread()) {
                if ((rand() % 256) < 2) {  // ~0.8% chance per scan
                    // Find adjacent empty cell to spread to
                    for (int dir = 0; dir < 8; dir++) {
                        CELL adj = c.AdjacentCell(static_cast<FacingType>(dir));
                        if (IsValidCell(adj) && !cells_[adj].HasOre() &&
                            !cells_[adj].HasGems() && !cells_[adj].IsWater()) {
                            cells_[adj].SetOverlay(OverlayType::GOLD1);
                            break;
                        }
                    }
                }
            }
        }
    }

    tiberiumScan_ += scanCount;
    if (tiberiumScan_ >= MAP_CELL_TOTAL) {
        tiberiumScan_ = 0;
    }
}

void MapClass::RecalculateTotalValue() {
    totalValue_ = 0;
    for (int i = 0; i < size_; i++) {
        totalValue_ += cells_[i].OreValue();
    }
}

//===========================================================================
// Utility
//===========================================================================

ObjectClass* MapClass::CloseObject(int32_t coord) const {
    CELL cell = Coord_Cell(coord);
    if (!IsValidCell(cell)) return nullptr;

    // Check the cell at the coordinate
    ObjectClass* obj = cells_[cell].CellObject();
    if (obj) return obj;

    // Check adjacent cells
    for (int dir = 0; dir < 8; dir++) {
        CELL adj = cells_[cell].AdjacentCell(static_cast<FacingType>(dir));
        if (IsValidCell(adj)) {
            obj = cells_[adj].CellObject();
            if (obj) return obj;
        }
    }

    return nullptr;
}

int MapClass::CellRegion(CELL cell) const {
    if (!IsValidCell(cell)) return 0;

    // Divide map into regions (used for AI)
    // 4x4 cells per region
    int x = Cell_X(cell) / 4;
    int y = Cell_Y(cell) / 4;
    return y * (MAP_CELL_W / 4) + x;
}

//===========================================================================
// Radius Scan Support
//===========================================================================

int MapClass::RadiusCount(int radius) {
    if (radius < 0) radius = 0;
    if (radius > 10) radius = 10;
    return RADIUS_COUNT[radius];
}

const int* MapClass::RadiusOffsets(int radius) {
    (void)radius;  // All offsets are in one array, indexed by count
    return RADIUS_OFFSET;
}
