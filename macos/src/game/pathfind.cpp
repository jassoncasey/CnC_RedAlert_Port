/**
 * Red Alert macOS Port - Pathfinding Implementation
 *
 * A* pathfinding algorithm for unit movement.
 * Based on concepts from original FINDPATH.CPP
 */

#include "pathfind.h"
#include "mapclass.h"
#include "cell.h"
#include <queue>
#include <algorithm>
#include <cmath>

//===========================================================================
// Global PathFinder Instance
//===========================================================================
PathFinder Pathfinder;

//===========================================================================
// Direction Offsets
//
// These offsets correspond to FacingType enum:
// N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7
//===========================================================================

static const int DIR_OFFSET_X[8] = { 0,  1, 1, 1, 0, -1, -1, -1};
static const int DIR_OFFSET_Y[8] = {-1, -1, 0, 1, 1,  1,  0, -1};

// Cell offset for each direction (adds to CELL value)
static const int CELL_OFFSET[8] = {
    -MAP_CELL_W,        // N
    -MAP_CELL_W + 1,    // NE
    1,                  // E
    MAP_CELL_W + 1,     // SE
    MAP_CELL_W,         // S
    MAP_CELL_W - 1,     // SW
    -1,                 // W
    -MAP_CELL_W - 1     // NW
};

// Movement cost multipliers (diagonal costs more)
static const int MOVE_COST[8] = {
    10, 14, 10, 14, 10, 14, 10, 14  // 10 = straight, 14 ≈ 10 * sqrt(2)
};

//===========================================================================
// Convenience Functions
//===========================================================================

FacingType Cell_Direction(CELL from, CELL to) {
    int dx = Cell_X(to) - Cell_X(from);
    int dy = Cell_Y(to) - Cell_Y(from);

    // Clamp to -1, 0, 1
    if (dx > 0) dx = 1;
    else if (dx < 0) dx = -1;
    if (dy > 0) dy = 1;
    else if (dy < 0) dy = -1;

    // Map to facing
    if (dx == 0 && dy == -1) return FacingType::NORTH;
    if (dx == 1 && dy == -1) return FacingType::NORTH_EAST;
    if (dx == 1 && dy == 0) return FacingType::EAST;
    if (dx == 1 && dy == 1) return FacingType::SOUTH_EAST;
    if (dx == 0 && dy == 1) return FacingType::SOUTH;
    if (dx == -1 && dy == 1) return FacingType::SOUTH_WEST;
    if (dx == -1 && dy == 0) return FacingType::WEST;
    if (dx == -1 && dy == -1) return FacingType::NORTH_WEST;

    return FacingType::NORTH;  // Default
}

CELL Adjacent_Cell(CELL cell, FacingType dir) {
    int idx = static_cast<int>(dir);
    if (idx < 0 || idx >= 8) return cell;

    CELL newCell = cell + CELL_OFFSET[idx];

    // Check for wrap-around (invalid)
    int oldX = Cell_X(cell);
    int newX = Cell_X(newCell);
    int dx = newX - oldX;
    if (dx < -1 || dx > 1) return cell;

    // Check bounds
    if (newCell < 0 || newCell >= MAP_CELL_TOTAL) return cell;

    return newCell;
}

PathType Find_Path(CELL start, CELL target, SpeedType speed,
                   int maxCost, int threat) {
    return Pathfinder.FindPath(start, target, speed, maxCost, threat);
}

//===========================================================================
// PathFinder Implementation
//===========================================================================

PathFinder::PathFinder()
    : currentSpeed_(SpeedType::TRACK)
    , currentThreat_(-1)
{
    // Pre-allocate arrays for performance
    gScore_.resize(MAP_CELL_TOTAL, MAX_PATH_COST);
    cameFrom_.resize(MAP_CELL_TOTAL, -1);
    dirFrom_.resize(MAP_CELL_TOTAL, FacingType::NORTH);
    closed_.resize(MAP_CELL_TOTAL, false);
}

PathFinder::~PathFinder() {
}

PathType PathFinder::FindPath(CELL start, CELL target, SpeedType speed,
                              int maxCost, int threat) {
    PathType result;
    result.start = start;
    result.target = target;

    // Validate inputs
    if (!Map.IsValidCell(start) || !Map.IsValidCell(target)) {
        return result;
    }

    // Already at target?
    if (start == target) {
        result.length = 0;
        result.cost = 0;
        return result;
    }

    // Check if target is reachable at all
    if (!Map[target].IsPassable(speed)) {
        // Try to find nearest passable cell to target
        CELL nearTarget = Map.NearbyLocation(target, speed);
        if (nearTarget == target || !Map[nearTarget].IsPassable(speed)) {
            return result;  // No valid target
        }
        target = nearTarget;
        result.target = target;
    }

    currentSpeed_ = speed;
    currentThreat_ = threat;

    // Reset state
    std::fill(gScore_.begin(), gScore_.end(), MAX_PATH_COST);
    std::fill(cameFrom_.begin(), cameFrom_.end(), static_cast<CELL>(-1));
    std::fill(closed_.begin(), closed_.end(), false);

    // Priority queue (min-heap by f-score)
    auto cmp = [](const Node& a, const Node& b) { return a.f > b.f; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> openSet(cmp);

    // Initialize start node
    Node startNode;
    startNode.cell = start;
    startNode.g = 0;
    startNode.h = Heuristic(start, target);
    startNode.f = startNode.g + startNode.h;
    startNode.parent = -1;
    startNode.dir = FacingType::NORTH;

    gScore_[start] = 0;
    openSet.push(startNode);

    int iterations = 0;
    const int maxIterations = MAP_CELL_TOTAL;

    while (!openSet.empty() && iterations < maxIterations) {
        iterations++;

        Node current = openSet.top();
        openSet.pop();

        // Skip if already processed
        if (closed_[current.cell]) continue;
        closed_[current.cell] = true;

        // Check if we reached the target
        if (current.cell == target) {
            result.cost = current.g;
            ReconstructPath(result, start, target);
            return result;
        }

        // Exceeded cost limit?
        if (current.g > maxCost) continue;

        // Explore neighbors
        for (int d = 0; d < 8; d++) {
            FacingType dir = static_cast<FacingType>(d);
            CELL neighbor = Adjacent_Cell(current.cell, dir);

            // Skip if same cell (edge wrap) or already closed
            if (neighbor == current.cell) continue;
            if (closed_[neighbor]) continue;

            // Calculate move cost
            int moveCost = MoveCost(current.cell, neighbor, dir, speed);
            if (moveCost >= MAX_PATH_COST) continue;

            int tentativeG = current.g + moveCost;

            // Skip if we found a worse path
            if (tentativeG >= gScore_[neighbor]) continue;

            // Record this path
            gScore_[neighbor] = tentativeG;
            cameFrom_[neighbor] = current.cell;
            dirFrom_[neighbor] = dir;

            // Add to open set
            Node neighborNode;
            neighborNode.cell = neighbor;
            neighborNode.g = tentativeG;
            neighborNode.h = Heuristic(neighbor, target);
            neighborNode.f = neighborNode.g + neighborNode.h;
            neighborNode.parent = current.cell;
            neighborNode.dir = dir;

            openSet.push(neighborNode);
        }
    }

    // No path found
    return result;
}

PathType PathFinder::FindPath(int32_t startCoord, int32_t targetCoord,
                              SpeedType speed, int maxCost, int threat) {
    return FindPath(Coord_Cell(startCoord), Coord_Cell(targetCoord),
                    speed, maxCost, threat);
}

int PathFinder::Heuristic(CELL from, CELL to) const {
    // Chebyshev distance (allows diagonal movement)
    int dx = std::abs(Cell_X(to) - Cell_X(from));
    int dy = std::abs(Cell_Y(to) - Cell_Y(from));

    // Diagonal distance: max(dx, dy) * 10 + (min) * 4
    // This favors paths that use diagonals appropriately
    int minD = std::min(dx, dy);
    int maxD = std::max(dx, dy);
    return maxD * 10 + minD * 4;
}

int PathFinder::MoveCost(CELL from, CELL to, FacingType dir, SpeedType speed) const {
    (void)from;  // Unused in basic implementation

    if (!Map.IsValidCell(to)) return MAX_PATH_COST;

    const CellClass& cell = Map[to];

    // Check basic passability
    if (!cell.IsPassable(speed)) return MAX_PATH_COST;

    // Base movement cost
    int cost = MOVE_COST[static_cast<int>(dir)];

    // Terrain modifiers
    LandType land = cell.GetLandType();
    switch (land) {
        case LandType::ROAD:
            cost = cost * 8 / 10;  // 20% faster on roads
            break;
        case LandType::ROUGH:
            cost = cost * 12 / 10;  // 20% slower on rough terrain
            break;
        case LandType::BEACH:
            if (speed != SpeedType::FLOAT) {
                cost = cost * 15 / 10;  // 50% slower on beach for land units
            }
            break;
        default:
            break;
    }

    // Threat avoidance (would check enemy presence)
    // For now, just avoid occupied cells
    if (cell.CellOccupier() != nullptr) {
        cost += 50;  // Penalty for occupied cells
    }

    return cost;
}

void PathFinder::ReconstructPath(PathType& path, CELL start, CELL target) {
    // Build path by following parent pointers backward
    std::vector<FacingType> reversePath;
    CELL current = target;

    while (current != start && cameFrom_[current] >= 0) {
        reversePath.push_back(dirFrom_[current]);
        current = cameFrom_[current];
    }

    // Reverse to get start-to-end order
    path.commands.clear();
    path.commands.reserve(reversePath.size());
    for (auto it = reversePath.rbegin(); it != reversePath.rend(); ++it) {
        path.commands.push_back(*it);
    }
    path.length = static_cast<int>(path.commands.size());
}

CELL PathFinder::PathCell(const PathType& path, int index) {
    if (index < 0 || index > path.length) return path.start;

    CELL cell = path.start;
    for (int i = 0; i < index && i < static_cast<int>(path.commands.size()); i++) {
        cell = Adjacent_Cell(cell, path.commands[i]);
    }
    return cell;
}

void PathFinder::OptimizePath(PathType& path) {
    if (path.length <= 2) return;

    // Remove redundant waypoints where we can go straight
    std::vector<FacingType> optimized;
    optimized.reserve(path.commands.size());

    CELL current = path.start;
    size_t i = 0;

    while (i < path.commands.size()) {
        // Try to find longest straight line from current position
        size_t j = i;
        CELL target = current;

        while (j < path.commands.size()) {
            CELL next = Adjacent_Cell(target, path.commands[j]);
            if (LineOfSight(current, next, SpeedType::TRACK)) {
                target = next;
                j++;
            } else {
                break;
            }
        }

        // Add the direction to the optimized path
        if (j > i) {
            // We can skip some waypoints - calculate direct direction
            FacingType dir = Cell_Direction(current, target);
            optimized.push_back(dir);
            current = target;
            i = j;
        } else {
            // Can't optimize, keep original
            optimized.push_back(path.commands[i]);
            current = Adjacent_Cell(current, path.commands[i]);
            i++;
        }
    }

    path.commands = std::move(optimized);
    path.length = static_cast<int>(path.commands.size());
}

bool PathFinder::LineOfSight(CELL from, CELL to, SpeedType speed) {
    // Bresenham line algorithm to check if all cells are passable
    int x0 = Cell_X(from);
    int y0 = Cell_Y(from);
    int x1 = Cell_X(to);
    int y1 = Cell_Y(to);

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        CELL cell = XY_Cell(x0, y0);
        if (!Map[cell].IsPassable(speed)) {
            return false;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    return true;
}
