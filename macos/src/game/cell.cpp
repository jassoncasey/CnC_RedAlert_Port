/**
 * Red Alert macOS Port - Cell Class Implementation
 *
 * Based on original CELL.CPP
 */

#include "cell.h"
#include <cstring>

//===========================================================================
// Static Adjacent Cell Offsets
//
// These offsets are added to a cell number to get the adjacent cell.
// Order: N, NE, E, SE, S, SW, W, NW (matches FacingType enum)
//===========================================================================
static const int AdjacentCellOffset[8] = {
    -MAP_CELL_W,        // N  (0)
    -MAP_CELL_W + 1,    // NE (1)
    1,                  // E  (2)
    MAP_CELL_W + 1,     // SE (3)
    MAP_CELL_W,         // S  (4)
    MAP_CELL_W - 1,     // SW (5)
    -1,                 // W  (6)
    -MAP_CELL_W - 1     // NW (7)
};

//===========================================================================
// Construction
//===========================================================================

CellClass::CellClass()
    : cellNumber_(0)
{
    Clear();
}

void CellClass::Clear() {
    // Reset flags
    isMapped_ = false;
    isVisible_ = false;
    isWaypoint_ = false;
    isFlagged_ = false;
    isToShroud_ = false;
    isPlot_ = false;
    isCursorHere_ = false;
    isRadarCursor_ = false;

    // Reset terrain
    templateType_ = TemplateType::CLEAR1;
    templateIcon_ = 0;
    overlay_ = OverlayType::NONE;
    overlayData_ = 0;
    smudge_ = SmudgeType::NONE;
    smudgeData_ = 0;

    // Reset ownership
    owner_ = HousesType::NONE;
    flag_.composite = 0;

    // Reset objects
    occupier_ = nullptr;
    for (int i = 0; i < MAX_OVERLAPPER; i++) {
        overlappers_[i] = nullptr;
    }

    // Reset zones
    for (int i = 0; i < static_cast<int>(MZoneType::COUNT); i++) {
        zones_[i] = 0;
    }

    // Reset effects
    jammed_ = 0;

    // Reset cached values
    land_ = LandType::CLEAR;
}

//===========================================================================
// Cell Identity
//===========================================================================

CELL CellClass::CellNumber() const {
    return cellNumber_;
}

void CellClass::SetCellNumber(CELL cell) {
    cellNumber_ = cell;
}

int32_t CellClass::CellCoord() const {
    return Cell_Coord(cellNumber_);
}

//===========================================================================
// Terrain Queries
//===========================================================================

void CellClass::RecalcLandType() {
    // Determine land type from template and overlay

    // Check overlay first (takes precedence)
    if (overlay_ != OverlayType::NONE) {
        // Walls
        if (overlay_ >= OverlayType::SANDBAG_WALL &&
            overlay_ <= OverlayType::WOOD_WALL) {
            land_ = LandType::WALL;
            return;
        }

        // Ore/gems
        bool isOre = overlay_ >= OverlayType::GOLD1 &&
                     overlay_ <= OverlayType::GOLD4;
        bool isGem = overlay_ >= OverlayType::GEMS1 &&
                     overlay_ <= OverlayType::GEMS4;
        if (isOre || isGem) {
            land_ = LandType::TIBERIUM;
            return;
        }
    }

    // Check template type
    switch (templateType_) {
        case TemplateType::WATER:
        case TemplateType::WATER2:
            land_ = LandType::WATER;
            break;

        case TemplateType::SHORE1:
        case TemplateType::SHORE2:
            land_ = LandType::BEACH;
            break;

        default:
            land_ = LandType::CLEAR;
            break;
    }
}

bool CellClass::IsWater() const {
    return land_ == LandType::WATER || land_ == LandType::RIVER;
}

bool CellClass::IsPassable(SpeedType speed) const {
    // Check based on speed type
    switch (speed) {
        case SpeedType::FOOT:
            // Infantry can't walk on water
            return land_ != LandType::WATER &&
                   land_ != LandType::RIVER &&
                   land_ != LandType::ROCK;

        case SpeedType::TRACK:
        case SpeedType::WHEEL:
            // Vehicles can't go on water or rock
            return land_ != LandType::WATER &&
                   land_ != LandType::RIVER &&
                   land_ != LandType::ROCK &&
                   land_ != LandType::WALL;

        case SpeedType::FLOAT:
            // Boats need water
            return land_ == LandType::WATER ||
                   land_ == LandType::RIVER;

        case SpeedType::WINGED:
            // Aircraft can go anywhere
            return true;

        default:
            return true;
    }
}

bool CellClass::IsClearToBuild() const {
    // Can't build on water
    if (IsWater()) return false;

    // Can't build on rock
    if (land_ == LandType::ROCK) return false;

    // Can't build on walls
    if (IsWall()) return false;

    // Can't build where there's already an occupant
    if (occupier_ != nullptr) return false;

    // Check for building flag
    if (flag_.occupy.building) return false;

    return true;
}

bool CellClass::IsClearToMove(SpeedType speed, bool ignoreInfantry,
                               bool ignoreVehicles) const {
    // First check basic passability
    if (!IsPassable(speed)) return false;

    // Check occupancy
    if (occupier_ != nullptr) {
        // For now, simplified - would check object type
        if (!ignoreInfantry && !ignoreVehicles) return false;
    }

    // Check flags
    if (flag_.occupy.monolith) return false;
    if (!ignoreVehicles && flag_.occupy.vehicle) return false;
    if (flag_.occupy.building) return false;

    return true;
}

bool CellClass::IsBridge() const {
    // Would check template for bridge types
    return false;
}

//===========================================================================
// Resource Queries (Ore/Gems)
//===========================================================================

bool CellClass::HasOre() const {
    return overlay_ >= OverlayType::GOLD1 && overlay_ <= OverlayType::GOLD4;
}

bool CellClass::HasGems() const {
    return overlay_ >= OverlayType::GEMS1 && overlay_ <= OverlayType::GEMS4;
}

int CellClass::OreValue() const {
    int ov = static_cast<int>(overlay_);
    if (HasOre()) {
        // Value based on overlay stage (1-4)
        int base = static_cast<int>(OverlayType::GOLD1);
        int stage = ov - base + 1;
        return stage * 25;  // 25 credits per stage
    }
    if (HasGems()) {
        int base = static_cast<int>(OverlayType::GEMS1);
        int stage = ov - base + 1;
        return stage * 50;  // 50 credits per stage
    }
    return 0;
}

int CellClass::ReduceOre(int amount) {
    if (!HasOre() && !HasGems()) return 0;

    int value = OreValue();
    int reduced = (amount < value) ? amount : value;

    // Reduce overlay stage
    int ov = static_cast<int>(overlay_);
    int stage;
    if (HasOre()) {
        int base = static_cast<int>(OverlayType::GOLD1);
        stage = ov - base;
        stage -= (reduced / 25);
        if (stage < 0) {
            ClearOverlay();
        } else {
            overlay_ = static_cast<OverlayType>(base + stage);
        }
    } else {
        int base = static_cast<int>(OverlayType::GEMS1);
        stage = ov - base;
        stage -= (reduced / 50);
        if (stage < 0) {
            ClearOverlay();
        } else {
            overlay_ = static_cast<OverlayType>(base + stage);
        }
    }

    RecalcLandType();
    return reduced;
}

bool CellClass::CanOreGrow() const {
    // Ore at stage 1-3 can grow
    return (overlay_ >= OverlayType::GOLD1 && overlay_ < OverlayType::GOLD4) ||
           (overlay_ >= OverlayType::GEMS1 && overlay_ < OverlayType::GEMS4);
}

bool CellClass::CanOreSpread() const {
    // Ore at stage 4 can spread
    return overlay_ == OverlayType::GOLD4 || overlay_ == OverlayType::GEMS4;
}

bool CellClass::GrowOre() {
    if (!CanOreGrow()) return false;

    // Increment stage
    overlay_ = static_cast<OverlayType>(static_cast<int>(overlay_) + 1);
    return true;
}

bool CellClass::SpreadOre() {
    // Would spread to adjacent empty cell
    // For now, just return false
    return false;
}

//===========================================================================
// Object Queries
//===========================================================================

ObjectClass* CellClass::CellObject(int /*x*/, int /*y*/) const {
    // Return primary occupier or first overlapper
    if (occupier_) return occupier_;

    for (int i = 0; i < MAX_OVERLAPPER; i++) {
        if (overlappers_[i]) return overlappers_[i];
    }
    return nullptr;
}

TechnoClass* CellClass::CellTechno(int x, int y) const {
    ObjectClass* obj = CellObject(x, y);
    // Would dynamic_cast or check RTTI
    return reinterpret_cast<TechnoClass*>(obj);
}

BuildingClass* CellClass::CellBuilding() const {
    // Would check occupier RTTI
    return nullptr;
}

UnitClass* CellClass::CellUnit() const {
    // Would check occupier RTTI
    return nullptr;
}

InfantryClass* CellClass::CellInfantry() const {
    // Would check occupier RTTI
    return nullptr;
}

TerrainClass* CellClass::CellTerrain() const {
    // Would check occupier RTTI
    return nullptr;
}

//===========================================================================
// Object Management
//===========================================================================

void CellClass::OccupyDown(ObjectClass* object) {
    if (object == nullptr) return;

    // Set as primary occupier
    occupier_ = object;
}

void CellClass::OccupyUp(ObjectClass* object) {
    if (object == nullptr) return;

    if (occupier_ == object) {
        occupier_ = nullptr;
    }
}

void CellClass::OverlapDown(ObjectClass* object) {
    if (object == nullptr) return;

    // Find empty slot
    for (int i = 0; i < MAX_OVERLAPPER; i++) {
        if (overlappers_[i] == nullptr) {
            overlappers_[i] = object;
            return;
        }
    }
}

void CellClass::OverlapUp(ObjectClass* object) {
    if (object == nullptr) return;

    // Find and remove
    for (int i = 0; i < MAX_OVERLAPPER; i++) {
        if (overlappers_[i] == object) {
            overlappers_[i] = nullptr;
            return;
        }
    }
}

//===========================================================================
// Sub-cell Position Queries
//===========================================================================

bool CellClass::IsSpotFree(SpotType spot) const {
    switch (spot) {
        case SpotType::CENTER:     return !flag_.occupy.center;
        case SpotType::UPPER_LEFT: return !flag_.occupy.upperLeft;
        case SpotType::UPPER_RIGHT: return !flag_.occupy.upperRight;
        case SpotType::LOWER_LEFT: return !flag_.occupy.lowerLeft;
        case SpotType::LOWER_RIGHT: return !flag_.occupy.lowerRight;
        default: return false;
    }
}

int32_t CellClass::FreeSpot() const {
    // Return coordinate of first free sub-cell position
    int32_t baseCoord = CellCoord();
    int baseX = Coord_X(baseCoord);
    int baseY = Coord_Y(baseCoord);

    // Offsets for sub-positions (in leptons, cell is 256 leptons)
    // Center is at 128, quarters at 64 and 192
    if (IsSpotFree(SpotType::CENTER)) {
        return baseCoord;  // Center
    }
    if (IsSpotFree(SpotType::UPPER_LEFT)) {
        return XY_Coord(baseX - 64, baseY - 64);
    }
    if (IsSpotFree(SpotType::UPPER_RIGHT)) {
        return XY_Coord(baseX + 64, baseY - 64);
    }
    if (IsSpotFree(SpotType::LOWER_LEFT)) {
        return XY_Coord(baseX - 64, baseY + 64);
    }
    if (IsSpotFree(SpotType::LOWER_RIGHT)) {
        return XY_Coord(baseX + 64, baseY + 64);
    }

    return 0;  // No free spot
}

int32_t CellClass::ClosestFreeSpot(int32_t coord) const {
    // Find free spot closest to given coordinate
    // For simplicity, just return any free spot
    return FreeSpot();
}

SpotType CellClass::SpotIndex(int32_t coord) {
    // Determine which sub-cell position a coordinate falls into
    int x = Coord_X(coord) % LEPTONS_PER_CELL;
    int y = Coord_Y(coord) % LEPTONS_PER_CELL;

    // Divide cell into quadrants with center
    int half = LEPTONS_PER_CELL / 2;
    int quarter = LEPTONS_PER_CELL / 4;

    // Check if in center region
    if (x >= quarter && x < half + quarter &&
        y >= quarter && y < half + quarter) {
        return SpotType::CENTER;
    }

    // Check quadrants
    if (x < half) {
        return (y < half) ? SpotType::UPPER_LEFT : SpotType::LOWER_LEFT;
    } else {
        return (y < half) ? SpotType::UPPER_RIGHT : SpotType::LOWER_RIGHT;
    }
}

//===========================================================================
// Flag Management
//===========================================================================

bool CellClass::PlaceFlag(HousesType house) {
    if (isFlagged_) return false;

    isFlagged_ = true;
    owner_ = house;
    return true;
}

bool CellClass::RemoveFlag() {
    if (!isFlagged_) return false;

    isFlagged_ = false;
    owner_ = HousesType::NONE;
    return true;
}

HousesType CellClass::FlagOwner() const {
    return isFlagged_ ? owner_ : HousesType::NONE;
}

//===========================================================================
// Overlay Management
//===========================================================================

void CellClass::SetOverlay(OverlayType type, uint8_t data) {
    overlay_ = type;
    overlayData_ = data;
    RecalcLandType();
}

void CellClass::ClearOverlay() {
    overlay_ = OverlayType::NONE;
    overlayData_ = 0;
    RecalcLandType();
}

bool CellClass::IsWall() const {
    return overlay_ >= OverlayType::SANDBAG_WALL &&
           overlay_ <= OverlayType::WOOD_WALL;
}

int CellClass::ReduceWall(int damage) {
    if (!IsWall()) return 0;

    // Walls have durability in overlayData_
    if (damage >= overlayData_) {
        int destroyed = overlayData_;
        ClearOverlay();
        return destroyed;
    }

    overlayData_ -= damage;
    return damage;
}

//===========================================================================
// Smudge Management
//===========================================================================

void CellClass::SetSmudge(SmudgeType type, uint8_t data) {
    smudge_ = type;
    smudgeData_ = data;
}

void CellClass::ClearSmudge() {
    smudge_ = SmudgeType::NONE;
    smudgeData_ = 0;
}

//===========================================================================
// Adjacent Cell Access
//===========================================================================

CELL CellClass::AdjacentCell(FacingType facing) const {
    int idx = static_cast<int>(facing);
    if (idx < 0 || idx >= 8) return cellNumber_;

    CELL adjacent = cellNumber_ + AdjacentCellOffset[idx];

    // Bounds check
    int x = Cell_X(adjacent);
    int y = Cell_Y(adjacent);
    if (x < 0 || x >= MAP_CELL_W || y < 0 || y >= MAP_CELL_H) {
        return cellNumber_;  // Return self if out of bounds
    }

    return adjacent;
}

//===========================================================================
// Rendering
//===========================================================================

void CellClass::Draw(int /*screenX*/, int /*screenY*/) const {
    // Would draw terrain, overlay, smudge
    // Actual rendering handled by graphics system
}

int CellClass::CellColor() const {
    // Return color for radar display
    if (IsWater()) return 0x000080;  // Blue
    if (HasOre()) return 0xFFFF00;   // Yellow
    if (HasGems()) return 0xFF00FF;  // Magenta
    if (IsWall()) return 0x808080;   // Gray
    if (flag_.occupy.building) return 0xFFFFFF;  // White

    return 0x008000;  // Green (clear terrain)
}
