/**
 * Red Alert macOS Port - Sidebar System
 *
 * SidebarClass - Manages the build menu UI
 * StripClass - Individual build column
 *
 * Based on original SIDEBAR.H, SIDEBAR.CPP
 */

#ifndef GAME_SIDEBAR_H
#define GAME_SIDEBAR_H

#include "types.h"
#include "factory.h"
#include <cstdint>

// Forward declarations
class HouseClass;

//===========================================================================
// Constants
//===========================================================================

// Sidebar dimensions (320x200 resolution)
constexpr int SIDE_X = 240;            // Sidebar X position
constexpr int SIDE_Y = 77;             // Sidebar Y position
constexpr int SIDE_WIDTH = 80;         // Sidebar width
constexpr int SIDE_HEIGHT = 123;       // Sidebar height

// Build strip dimensions
constexpr int COLUMNS = 2;             // Two build columns
constexpr int MAX_BUILDABLES = 75;     // Max items per strip
constexpr int MAX_VISIBLE = 4;         // Visible items without scrolling
constexpr int OBJECT_WIDTH = 32;       // Cameo width
constexpr int OBJECT_HEIGHT = 24;      // Cameo height
constexpr int STRIP_WIDTH = 35;        // Column width including spacing

// Top buttons
constexpr int TOP_HEIGHT = 13;         // Height of repair/upgrade/zoom row

// Button positions (in sidebar-local coordinates)
constexpr int COLUMN_ONE_X = 8;
constexpr int COLUMN_ONE_Y = 13;
constexpr int COLUMN_TWO_X = 43;
constexpr int COLUMN_TWO_Y = 13;

// Scroll rate
constexpr int SCROLL_RATE = 8;         // Pixels per frame when scrolling

//===========================================================================
// BuildType - Buildable Item Data
//===========================================================================

struct BuildType {
    int buildableId;                   // Object subtype ID
    RTTIType buildableType;            // RTTI_UNIT, RTTI_BUILDING, etc.
    int factoryIndex;                  // Factory index (-1 if not building)

    BuildType() : buildableId(-1), buildableType(RTTIType::NONE), factoryIndex(-1) {}
};

//===========================================================================
// StripClass - Build Column
//===========================================================================

class StripClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    int id_;                           // Column ID (0 or 1)
    int x_, y_;                        // Position

    //-----------------------------------------------------------------------
    // Buildables
    //-----------------------------------------------------------------------
    BuildType buildables_[MAX_BUILDABLES];
    int buildableCount_;               // Number of items in strip
    int topIndex_;                     // Top visible item (for scrolling)

    //-----------------------------------------------------------------------
    // Visual State
    //-----------------------------------------------------------------------
    int flasher_;                      // Index of flashing item (-1 if none)
    int flashCount_;                   // Flash animation counter
    bool isScrolling_;                 // Currently scrolling
    bool isScrollingDown_;             // Scroll direction
    int slid_;                         // Pixel offset during scroll

    //-----------------------------------------------------------------------
    // Production State
    //-----------------------------------------------------------------------
    bool isBuilding_;                  // Any item producing

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    StripClass();
    void Init(int id, int x, int y);

    //-----------------------------------------------------------------------
    // Buildable Management
    //-----------------------------------------------------------------------

    /**
     * Add a buildable item to the strip
     * @return true if added successfully
     */
    bool Add(RTTIType type, int id);

    /**
     * Remove a buildable item from the strip
     * @return true if removed
     */
    bool Remove(RTTIType type, int id);

    /**
     * Link a factory to a buildable button
     */
    void Factory_Link(int factoryIndex, RTTIType type, int id);

    /**
     * Unlink a factory from its button
     */
    void Factory_Unlink(int factoryIndex);

    /**
     * Find index of a buildable
     * @return index or -1 if not found
     */
    int Find(RTTIType type, int id) const;

    /**
     * Revalidate buildables against available factories
     */
    void Recalc(HouseClass* house);

    /**
     * Clear all buildables
     */
    void Clear();

    //-----------------------------------------------------------------------
    // Scrolling
    //-----------------------------------------------------------------------

    /**
     * Scroll the strip
     * @param up true to scroll up, false to scroll down
     * @return true if scroll was queued
     */
    bool Scroll(bool up);

    /**
     * Check if can scroll in direction
     */
    bool Can_Scroll_Up() const { return topIndex_ > 0; }
    bool Can_Scroll_Down() const { return topIndex_ + MAX_VISIBLE < buildableCount_; }

    //-----------------------------------------------------------------------
    // Input
    //-----------------------------------------------------------------------

    /**
     * Handle click on strip
     * @param x Click X relative to strip
     * @param y Click Y relative to strip
     * @param leftClick true for left click, false for right
     * @return true if click was handled
     */
    bool Click(int x, int y, bool leftClick, HouseClass* house);

    /**
     * Get index of item at position
     * @return item index or -1 if none
     */
    int Index_At(int x, int y) const;

    //-----------------------------------------------------------------------
    // Update
    //-----------------------------------------------------------------------

    /**
     * Process one tick of logic
     */
    void AI(HouseClass* house);

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------

    /**
     * Flag strip for redraw
     */
    void Flag_To_Redraw() { needsRedraw_ = true; }

    /**
     * Check if strip needs redraw
     */
    bool Needs_Redraw() const { return needsRedraw_; }

    /**
     * Clear redraw flag
     */
    void Clear_Redraw() { needsRedraw_ = false; }

private:
    bool needsRedraw_;

    /**
     * Get buildable info at visible index
     */
    BuildType* Get_Buildable(int index);
    const BuildType* Get_Buildable(int index) const;
};

//===========================================================================
// SidebarClass - Main Sidebar Manager
//===========================================================================

class SidebarClass {
public:
    //-----------------------------------------------------------------------
    // State
    //-----------------------------------------------------------------------
    bool isActive_;                    // Sidebar visible
    bool isToRedraw_;                  // Needs full redraw

    //-----------------------------------------------------------------------
    // Control Buttons
    //-----------------------------------------------------------------------
    bool isRepairActive_;              // Repair mode on
    bool isUpgradeActive_;             // Upgrade/sell mode on
    bool isDemolishActive_;            // Demolish mode on

    //-----------------------------------------------------------------------
    // Build Strips
    //-----------------------------------------------------------------------
    StripClass columns_[COLUMNS];

    //-----------------------------------------------------------------------
    // Owner
    //-----------------------------------------------------------------------
    HouseClass* playerHouse_;

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    SidebarClass();
    void Init();

    //-----------------------------------------------------------------------
    // Sidebar Control
    //-----------------------------------------------------------------------

    /**
     * Activate or deactivate sidebar
     * @param control 1 = on, 0 = off, -1 = toggle
     */
    void Activate(int control);

    /**
     * Set the player house
     */
    void Set_House(HouseClass* house) { playerHouse_ = house; }

    //-----------------------------------------------------------------------
    // Buildable Management
    //-----------------------------------------------------------------------

    /**
     * Add a buildable to appropriate strip
     * @param type Object type (determines which strip)
     * @param id Object subtype ID
     * @return true if added
     */
    bool Add(RTTIType type, int id);

    /**
     * Remove a buildable from sidebar
     * @return true if removed
     */
    bool Remove(RTTIType type, int id);

    /**
     * Link a factory to its button
     */
    void Factory_Link(int factoryIndex, RTTIType type, int id);

    /**
     * Unlink a factory
     */
    void Factory_Unlink(int factoryIndex);

    /**
     * Revalidate all buildables
     */
    void Recalc();

    /**
     * Clear all buildables
     */
    void Clear();

    //-----------------------------------------------------------------------
    // Control Button Actions
    //-----------------------------------------------------------------------

    /**
     * Toggle repair mode
     */
    void Toggle_Repair();

    /**
     * Toggle upgrade/sell mode
     */
    void Toggle_Upgrade();

    /**
     * Activate zoom
     */
    void Zoom();

    //-----------------------------------------------------------------------
    // Input Handling
    //-----------------------------------------------------------------------

    /**
     * Process input
     * @param key Key code (if any)
     * @param x Mouse X
     * @param y Mouse Y
     * @param leftClick Left button down
     * @param rightClick Right button down
     * @return true if input was consumed
     */
    bool Input(int key, int x, int y, bool leftClick, bool rightClick);

    /**
     * Check if point is in sidebar
     */
    bool Point_In_Sidebar(int x, int y) const;

    //-----------------------------------------------------------------------
    // Update
    //-----------------------------------------------------------------------

    /**
     * Process one tick of logic
     */
    void AI();

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------

    /**
     * Flag for full redraw
     */
    void Flag_To_Redraw() { isToRedraw_ = true; }

    /**
     * Get strip for rendering
     */
    const StripClass& Get_Column(int index) const { return columns_[index]; }

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------

    /**
     * Get which strip a type goes into
     * 0 = structures, 1 = units
     */
    static int Which_Column(RTTIType type);

private:
    /**
     * Start production of item
     */
    bool Start_Production(RTTIType type, int id);

    /**
     * Suspend production of item
     */
    bool Suspend_Production(RTTIType type, int id);

    /**
     * Abandon production of item
     */
    bool Abandon_Production(RTTIType type, int id);
};

//===========================================================================
// Global Instance
//===========================================================================

extern SidebarClass Sidebar;

#endif // GAME_SIDEBAR_H
