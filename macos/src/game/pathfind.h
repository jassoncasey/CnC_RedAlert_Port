/**
 * Red Alert macOS Port - Pathfinding
 *
 * A* pathfinding for unit movement.
 * Based on concepts from original FINDPATH.CPP
 */

#ifndef GAME_PATHFIND_H
#define GAME_PATHFIND_H

#include "types.h"
#include <cstdint>
#include <vector>

// Forward declarations
class CellClass;
class MapClass;

//===========================================================================
// Path Constants
//===========================================================================

constexpr int MAX_PATH_LENGTH = 300;    // Maximum path waypoints
constexpr int MAX_PATH_COST = 0x7FFF;   // Maximum cost before giving up

//===========================================================================
// PathType - Result of pathfinding
//===========================================================================
struct PathType {
    CELL start;                          // Starting cell
    CELL target;                         // Target cell
    int length;                          // Number of moves in path
    int cost;                            // Total movement cost
    std::vector<FacingType> commands;    // Movement directions

    PathType() : start(0), target(0), length(0), cost(0) {}

    void Clear() {
        start = 0;
        target = 0;
        length = 0;
        cost = 0;
        commands.clear();
    }

    bool IsValid() const { return length > 0; }
};

//===========================================================================
// PathFinder - A* pathfinding implementation
//===========================================================================
class PathFinder {
public:
    PathFinder();
    ~PathFinder();

    //-----------------------------------------------------------------------
    // Main Pathfinding
    //-----------------------------------------------------------------------

    /**
     * Find a path from start to target
     *
     * @param start      Starting cell
     * @param target     Target cell
     * @param speed      Movement type (affects passability)
     * @param maxCost    Maximum search cost before giving up
     * @param threat     Threat threshold (avoid enemy areas if > 0)
     * @return           Path result (check IsValid())
     */
    PathType FindPath(CELL start, CELL target, SpeedType speed,
                      int maxCost = MAX_PATH_COST, int threat = -1);

    /**
     * Find path from coordinate to coordinate
     */
    PathType FindPath(int32_t startCoord, int32_t targetCoord, SpeedType speed,
                      int maxCost = MAX_PATH_COST, int threat = -1);

    //-----------------------------------------------------------------------
    // Path Utilities
    //-----------------------------------------------------------------------

    /**
     * Get cell at position in path
     */
    static CELL PathCell(const PathType& path, int index);

    /**
     * Optimize path by removing unnecessary waypoints
     */
    static void OptimizePath(PathType& path);

    /**
     * Check if there's a clear line of sight between two cells
     */
    static bool LineOfSight(CELL from, CELL to, SpeedType speed);

private:
    //-----------------------------------------------------------------------
    // A* Node
    //-----------------------------------------------------------------------

    struct Node {
        CELL cell;
        int g;          // Cost from start
        int h;          // Heuristic (estimated cost to target)
        int f;          // Total = g + h
        CELL parent;
        FacingType dir; // Direction taken to reach this node

        bool operator>(const Node& other) const { return f > other.f; }
    };

    //-----------------------------------------------------------------------
    // Internal Methods
    //-----------------------------------------------------------------------

    int Heuristic(CELL from, CELL to) const;
    int MoveCost(CELL from, CELL to, FacingType dir, SpeedType speed) const;
    void ReconstructPath(PathType& path, CELL start, CELL target);

    //-----------------------------------------------------------------------
    // State
    //-----------------------------------------------------------------------

    std::vector<int> gScore_;      // g-score for each cell
    std::vector<CELL> cameFrom_;   // Parent cell for reconstruction
    std::vector<FacingType> dirFrom_; // Direction taken to reach cell
    std::vector<bool> closed_;     // Closed set

    SpeedType currentSpeed_;
    int currentThreat_;
};

//===========================================================================
// Global PathFinder Instance
//===========================================================================
extern PathFinder Pathfinder;

//===========================================================================
// Convenience Functions
//===========================================================================

/**
 * Find path between two cells
 */
PathType Find_Path(CELL start, CELL target, SpeedType speed,
                   int maxCost = MAX_PATH_COST, int threat = -1);

/**
 * Get direction from one cell to an adjacent cell
 */
FacingType Cell_Direction(CELL from, CELL to);

/**
 * Get adjacent cell in a direction
 */
CELL Adjacent_Cell(CELL cell, FacingType dir);

#endif // GAME_PATHFIND_H
