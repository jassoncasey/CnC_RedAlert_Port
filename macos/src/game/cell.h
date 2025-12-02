/**
 * Red Alert macOS Port - Cell Class
 *
 * Represents a single map cell in the 128x128 grid.
 * Based on original CELL.H
 */

#ifndef GAME_CELL_H
#define GAME_CELL_H

#include "types.h"
#include <cstdint>

// Forward declarations
class ObjectClass;
class TechnoClass;
class BuildingClass;
class UnitClass;
class InfantryClass;
class AircraftClass;
class TerrainClass;

// Maximum number of overlapping objects per cell
constexpr int MAX_OVERLAPPER = 6;

// Sub-cell positions for infantry (5 spots per cell)
enum class SpotType : int8_t {
    CENTER = 0,
    UPPER_LEFT,
    UPPER_RIGHT,
    LOWER_LEFT,
    LOWER_RIGHT,

    COUNT
};

//===========================================================================
// CellClass - Represents a single map cell
//===========================================================================
class CellClass {
public:
    //-----------------------------------------------------------------------
    // State Flags
    //-----------------------------------------------------------------------

    // Visibility and radar flags
    bool isMapped_ : 1;          // Cell has been explored (partially visible)
    bool isVisible_ : 1;         // Cell is currently visible (no shroud)
    bool isWaypoint_ : 1;        // Has a waypoint assigned
    bool isFlagged_ : 1;         // Has a house flag placed
    bool isToShroud_ : 1;        // Scheduled for shroud regrowth
    bool isPlot_ : 1;            // Needs radar plot update
    bool isCursorHere_ : 1;      // Building placement cursor over cell
    bool isRadarCursor_ : 1;     // Radar cursor over cell

    //-----------------------------------------------------------------------
    // Terrain Layers
    //-----------------------------------------------------------------------

    // Primary layer - base terrain template
    TemplateType templateType_;  // Template/terrain type
    uint8_t templateIcon_;       // Icon index within template

    // Secondary layer - overlays (walls, resources)
    OverlayType overlay_;        // Overlay type (wall, ore, etc.)
    uint8_t overlayData_;        // Overlay variant/strength (1-4 for ore)

    // Tertiary layer - smudges (craters, scorches)
    SmudgeType smudge_;          // Smudge type
    uint8_t smudgeData_;         // Smudge variant

    //-----------------------------------------------------------------------
    // Ownership and Occupancy
    //-----------------------------------------------------------------------

    HousesType owner_;           // Owner for flags/walls

    // Occupancy flags for sub-cell positions
    union {
        struct {
            uint8_t center : 1;      // Center position occupied
            uint8_t upperLeft : 1;   // NW position
            uint8_t upperRight : 1;  // NE position
            uint8_t lowerLeft : 1;   // SW position
            uint8_t lowerRight : 1;  // SE position
            uint8_t vehicle : 1;     // Vehicle passing through
            uint8_t monolith : 1;    // Immovable blockage
            uint8_t building : 1;    // Building present
        } occupy;
        uint8_t composite;
    } flag_;

    //-----------------------------------------------------------------------
    // Object References
    //-----------------------------------------------------------------------

    ObjectClass* occupier_;                 // Primary occupant
    ObjectClass* overlappers_[MAX_OVERLAPPER]; // Objects extending into cell

    //-----------------------------------------------------------------------
    // Movement Zones
    //-----------------------------------------------------------------------

    // Zone indices for pathfinding
    uint8_t zones_[static_cast<int>(MZoneType::COUNT)];

    //-----------------------------------------------------------------------
    // Special Effects
    //-----------------------------------------------------------------------

    uint16_t jammed_;            // Gap generator jam counter

    //-----------------------------------------------------------------------
    // Cached Values
    //-----------------------------------------------------------------------

    LandType land_;              // Calculated land type (for movement)

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------

    CellClass();
    void Clear();

    //-----------------------------------------------------------------------
    // Cell Identity
    //-----------------------------------------------------------------------

    CELL CellNumber() const;
    void SetCellNumber(CELL cell);
    int32_t CellCoord() const;

    //-----------------------------------------------------------------------
    // Terrain Queries
    //-----------------------------------------------------------------------

    LandType GetLandType() const { return land_; }
    void RecalcLandType();

    bool IsWater() const;
    bool IsPassable(SpeedType speed) const;
    bool IsClearToBuild() const;
    bool IsClearToMove(SpeedType speed, bool ignoreInfantry = false,
                       bool ignoreVehicles = false) const;
    bool IsBridge() const;

    //-----------------------------------------------------------------------
    // Resource Queries (Ore/Gems)
    //-----------------------------------------------------------------------

    bool HasOre() const;
    bool HasGems() const;
    int OreValue() const;
    int ReduceOre(int amount);
    bool CanOreGrow() const;
    bool CanOreSpread() const;
    bool GrowOre();
    bool SpreadOre();

    //-----------------------------------------------------------------------
    // Object Queries
    //-----------------------------------------------------------------------

    ObjectClass* CellOccupier() const { return occupier_; }
    ObjectClass* CellObject(int x = 0, int y = 0) const;
    TechnoClass* CellTechno(int x = 0, int y = 0) const;
    BuildingClass* CellBuilding() const;
    UnitClass* CellUnit() const;
    InfantryClass* CellInfantry() const;
    TerrainClass* CellTerrain() const;

    //-----------------------------------------------------------------------
    // Object Management
    //-----------------------------------------------------------------------

    void OccupyDown(ObjectClass* object);
    void OccupyUp(ObjectClass* object);
    void OverlapDown(ObjectClass* object);
    void OverlapUp(ObjectClass* object);

    //-----------------------------------------------------------------------
    // Sub-cell Position Queries
    //-----------------------------------------------------------------------

    bool IsSpotFree(SpotType spot) const;
    int32_t FreeSpot() const;
    int32_t ClosestFreeSpot(int32_t coord) const;
    static SpotType SpotIndex(int32_t coord);

    //-----------------------------------------------------------------------
    // Flag Management
    //-----------------------------------------------------------------------

    bool PlaceFlag(HousesType house);
    bool RemoveFlag();
    bool HasFlag() const { return isFlagged_; }
    HousesType FlagOwner() const;

    //-----------------------------------------------------------------------
    // Visibility
    //-----------------------------------------------------------------------

    bool IsMapped() const { return isMapped_; }
    bool IsVisible() const { return isVisible_; }
    void SetMapped(bool mapped) { isMapped_ = mapped; }
    void SetVisible(bool visible) { isVisible_ = visible; }

    //-----------------------------------------------------------------------
    // Overlay Management
    //-----------------------------------------------------------------------

    void SetOverlay(OverlayType type, uint8_t data = 0);
    void ClearOverlay();
    bool IsWall() const;
    int ReduceWall(int damage);

    //-----------------------------------------------------------------------
    // Smudge Management
    //-----------------------------------------------------------------------

    void SetSmudge(SmudgeType type, uint8_t data = 0);
    void ClearSmudge();

    //-----------------------------------------------------------------------
    // Adjacent Cell Access
    //-----------------------------------------------------------------------

    CELL AdjacentCell(FacingType facing) const;

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------

    void Draw(int screenX, int screenY) const;
    int CellColor() const;  // For radar display

private:
    CELL cellNumber_;   // This cell's index (0-16383)
};

#endif // GAME_CELL_H
