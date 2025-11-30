/**
 * Red Alert macOS Port - MapClass
 *
 * The main map container that holds all cells.
 * Based on original MAP.H
 */

#ifndef GAME_MAPCLASS_H
#define GAME_MAPCLASS_H

#include "types.h"
#include "cell.h"
#include <cstdint>
#include <vector>

// Forward declarations
class ObjectClass;
class HouseClass;

//===========================================================================
// MapClass - Main map container
//===========================================================================
class MapClass {
public:
    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------

    MapClass();
    ~MapClass();

    //-----------------------------------------------------------------------
    // Initialization
    //-----------------------------------------------------------------------

    void OneTime();          // One-time initialization
    void InitClear();        // Clear to known state
    void AllocCells();       // Allocate cell array
    void FreeCells();        // Free cell array
    void InitCells();        // Initialize cells to default state

    //-----------------------------------------------------------------------
    // Map Dimensions
    //-----------------------------------------------------------------------

    void SetMapDimensions(int x, int y, int w, int h);

    int MapCellX() const { return mapCellX_; }
    int MapCellY() const { return mapCellY_; }
    int MapCellWidth() const { return mapCellWidth_; }
    int MapCellHeight() const { return mapCellHeight_; }

    //-----------------------------------------------------------------------
    // Cell Access
    //-----------------------------------------------------------------------

    CellClass& operator[](CELL cell);
    const CellClass& operator[](CELL cell) const;
    CellClass& operator[](int32_t coord);
    const CellClass& operator[](int32_t coord) const;

    bool IsValidCell(CELL cell) const;
    bool InRadar(CELL cell) const;

    //-----------------------------------------------------------------------
    // Random Location
    //-----------------------------------------------------------------------

    CELL PickRandomLocation() const;
    CELL NearbyLocation(CELL cell, SpeedType speed, int zone = -1,
                        MZoneType check = MZoneType::NORMAL) const;

    //-----------------------------------------------------------------------
    // Object Placement
    //-----------------------------------------------------------------------

    void PlaceDown(CELL cell, ObjectClass* object);
    void PickUp(CELL cell, ObjectClass* object);
    void OverlapDown(CELL cell, ObjectClass* object);
    void OverlapUp(CELL cell, ObjectClass* object);

    //-----------------------------------------------------------------------
    // Visibility (Fog of War)
    //-----------------------------------------------------------------------

    void SightFrom(CELL cell, int sightRange, HouseClass* house,
                   bool incremental = false);
    void ShroudFrom(CELL cell, int sightRange);
    void ShroudTheMap();
    void RevealTheMap();

    //-----------------------------------------------------------------------
    // Zone Management
    //-----------------------------------------------------------------------

    bool ZoneReset(int method);
    bool ZoneCell(CELL cell, int zone);
    int ZoneSpan(CELL cell, int zone, MZoneType check);

    //-----------------------------------------------------------------------
    // Ore/Tiberium Management
    //-----------------------------------------------------------------------

    void Logic();            // Handle ore growth/spread
    int32_t TotalValue() const { return totalValue_; }
    void RecalculateTotalValue();

    //-----------------------------------------------------------------------
    // Utility
    //-----------------------------------------------------------------------

    ObjectClass* CloseObject(int32_t coord) const;
    int CellRegion(CELL cell) const;

    //-----------------------------------------------------------------------
    // Radius Scan Support
    //-----------------------------------------------------------------------

    // Get cells within a radius
    static int RadiusCount(int radius);
    static const int* RadiusOffsets(int radius);

private:
    //-----------------------------------------------------------------------
    // Cell Array
    //-----------------------------------------------------------------------

    std::vector<CellClass> cells_;
    int xSize_;
    int ySize_;
    int size_;

    //-----------------------------------------------------------------------
    // Map Bounds (playable area within the full array)
    //-----------------------------------------------------------------------

    int mapCellX_;
    int mapCellY_;
    int mapCellWidth_;
    int mapCellHeight_;

    //-----------------------------------------------------------------------
    // Resource Tracking
    //-----------------------------------------------------------------------

    int32_t totalValue_;

    // Ore growth tracking
    std::vector<CELL> tiberiumGrowth_;
    int tiberiumGrowthCount_;

    // Ore spread tracking
    std::vector<CELL> tiberiumSpread_;
    int tiberiumSpreadCount_;

    // Current scan position for incremental processing
    CELL tiberiumScan_;

    //-----------------------------------------------------------------------
    // Static Radius Data
    //-----------------------------------------------------------------------

    static const int RADIUS_COUNT[11];
    static const int RADIUS_OFFSET[];
};

//===========================================================================
// Global Map Instance
//===========================================================================
extern MapClass Map;

#endif // GAME_MAPCLASS_H
