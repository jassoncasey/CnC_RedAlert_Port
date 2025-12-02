/**
 * Red Alert macOS Port - Sidebar Implementation
 *
 * Based on original SIDEBAR.CPP (~2000 lines)
 */

#include "sidebar.h"
#include "house.h"
#include "factory.h"
#include <cstring>
#include <algorithm>

//===========================================================================
// Global Instance
//===========================================================================

SidebarClass Sidebar;

//===========================================================================
// StripClass Implementation
//===========================================================================

StripClass::StripClass() {
    Init(0, 0, 0);
}

void StripClass::Init(int id, int x, int y) {
    id_ = id;
    x_ = x;
    y_ = y;

    buildableCount_ = 0;
    topIndex_ = 0;

    flasher_ = -1;
    flashCount_ = 0;
    isScrolling_ = false;
    isScrollingDown_ = false;
    slid_ = 0;

    isBuilding_ = false;
    needsRedraw_ = true;

    for (int i = 0; i < MAX_BUILDABLES; i++) {
        buildables_[i] = BuildType();
    }
}

bool StripClass::Add(RTTIType type, int id) {
    // Check if already in strip
    if (Find(type, id) >= 0) {
        return false;
    }

    // Check if room
    if (buildableCount_ >= MAX_BUILDABLES) {
        return false;
    }

    // Add to end
    buildables_[buildableCount_].buildableType = type;
    buildables_[buildableCount_].buildableId = id;
    buildables_[buildableCount_].factoryIndex = -1;
    buildableCount_++;

    needsRedraw_ = true;
    return true;
}

bool StripClass::Remove(RTTIType type, int id) {
    int index = Find(type, id);
    if (index < 0) {
        return false;
    }

    // Shift remaining items down
    for (int i = index; i < buildableCount_ - 1; i++) {
        buildables_[i] = buildables_[i + 1];
    }
    buildableCount_--;

    // Clear the last slot
    buildables_[buildableCount_] = BuildType();

    // Adjust flasher if needed
    if (flasher_ >= buildableCount_) {
        flasher_ = -1;
    }

    // Adjust top index if needed
    if (topIndex_ > 0 && topIndex_ + MAX_VISIBLE > buildableCount_) {
        topIndex_ = std::max(0, buildableCount_ - MAX_VISIBLE);
    }

    needsRedraw_ = true;
    return true;
}

void StripClass::Factory_Link(int factoryIndex, RTTIType type, int id) {
    int index = Find(type, id);
    if (index >= 0) {
        buildables_[index].factoryIndex = factoryIndex;
        needsRedraw_ = true;
    }
}

void StripClass::Factory_Unlink(int factoryIndex) {
    for (int i = 0; i < buildableCount_; i++) {
        if (buildables_[i].factoryIndex == factoryIndex) {
            buildables_[i].factoryIndex = -1;
            needsRedraw_ = true;
        }
    }
}

int StripClass::Find(RTTIType type, int id) const {
    for (int i = 0; i < buildableCount_; i++) {
        if (buildables_[i].buildableType == type &&
            buildables_[i].buildableId == id) {
            return i;
        }
    }
    return -1;
}

void StripClass::Recalc(HouseClass* house) {
    if (!house) return;

    // Check each buildable for validity
    // In the full implementation, this checks prerequisites
    // For now, we just validate that the type is reasonable

    bool changed = false;

    for (int i = buildableCount_ - 1; i >= 0; i--) {
        bool valid = true;

        // Basic validation - check type range
        int bid = buildables_[i].buildableId;
        int infMax = static_cast<int>(InfantryType::COUNT);
        int unitMax = static_cast<int>(UnitType::COUNT);
        int bldMax = static_cast<int>(BuildingType::COUNT);
        int airMax = static_cast<int>(AircraftType::COUNT);
        int spcMax = static_cast<int>(SpecialWeaponType::SPC_COUNT);
        switch (buildables_[i].buildableType) {
            case RTTIType::INFANTRY:
                valid = bid >= 0 && bid < infMax;
                break;
            case RTTIType::UNIT:
                valid = bid >= 0 && bid < unitMax;
                break;
            case RTTIType::BUILDING:
                valid = bid >= 0 && bid < bldMax;
                break;
            case RTTIType::AIRCRAFT:
                valid = bid >= 0 && bid < airMax;
                break;
            case RTTIType::SPECIAL:
                valid = bid >= 0 && bid < spcMax;
                break;
            default:
                valid = false;
                break;
        }

        // In full implementation, also check:
        // - house->Can_Build(type, id)
        // - prerequisite buildings exist
        // - tech level unlocked

        if (!valid) {
            // Abandon any production
            if (buildables_[i].factoryIndex >= 0) {
                FactoryClass* factory = &Factories[buildables_[i].factoryIndex];
                factory->Abandon();
            }

            // Remove from list
            Remove(buildables_[i].buildableType, buildables_[i].buildableId);
            changed = true;
        }
    }

    if (changed) {
        needsRedraw_ = true;
    }
}

void StripClass::Clear() {
    // Abandon all production
    for (int i = 0; i < buildableCount_; i++) {
        if (buildables_[i].factoryIndex >= 0) {
            FactoryClass* factory = &Factories[buildables_[i].factoryIndex];
            factory->Abandon();
        }
    }

    Init(id_, x_, y_);
}

bool StripClass::Scroll(bool up) {
    if (isScrolling_) {
        return false;  // Already scrolling
    }

    if (up && !Can_Scroll_Up()) {
        return false;
    }
    if (!up && !Can_Scroll_Down()) {
        return false;
    }

    isScrolling_ = true;
    isScrollingDown_ = !up;
    slid_ = 0;

    return true;
}

bool StripClass::Click(int x, int y, bool leftClick, HouseClass* house) {
    if (!house) return false;

    int index = Index_At(x, y);
    if (index < 0) {
        return false;  // Click outside items
    }

    // Get the actual buildable index (accounting for scroll)
    int buildableIndex = topIndex_ + index;
    if (buildableIndex >= buildableCount_) {
        return false;
    }

    BuildType& build = buildables_[buildableIndex];

    if (leftClick) {
        // Left click - start/resume production or place completed
        if (build.factoryIndex >= 0) {
            FactoryClass* factory = &Factories[build.factoryIndex];

            if (factory->Has_Completed()) {
                // Production complete - enter placement mode
                // In full implementation, triggers placement cursor
                return true;
            } else if (factory->isSuspended_) {
                // Resume suspended production
                factory->Start();
                return true;
            }
            // Already building - do nothing
            return false;
        } else {
            // Start new production
            FactoryClass* factory = Create_Factory();
            if (factory) {
                RTTIType btype = build.buildableType;
                int bid = build.buildableId;
                if (factory->Set(btype, bid, house)) {
                    factory->Start();
                    build.factoryIndex = factory->id_;
                    flasher_ = buildableIndex;
                    flashCount_ = 7;  // Flash for 7 frames
                    needsRedraw_ = true;
                    return true;
                } else {
                    Destroy_Factory(factory);
                }
            }
        }
    } else {
        // Right click - suspend or abandon
        if (build.factoryIndex >= 0) {
            FactoryClass* factory = &Factories[build.factoryIndex];

            if (factory->isSuspended_) {
                // Already suspended - abandon and refund
                factory->Abandon();
                build.factoryIndex = -1;
                needsRedraw_ = true;
                return true;
            } else {
                // Suspend production
                factory->Suspend();
                needsRedraw_ = true;
                return true;
            }
        }
    }

    return false;
}

int StripClass::Index_At(int x, int y) const {
    // Check if click is within strip bounds
    if (x < 0 || x >= OBJECT_WIDTH) {
        return -1;
    }

    // Calculate which slot was clicked (accounting for scroll offset)
    int adjustedY = y + slid_;
    int index = adjustedY / OBJECT_HEIGHT;

    if (index < 0 || index >= MAX_VISIBLE) {
        return -1;
    }

    return index;
}

void StripClass::AI(HouseClass* house) {
    (void)house;

    // Process scrolling animation
    if (isScrolling_) {
        slid_ += SCROLL_RATE;

        if (slid_ >= OBJECT_HEIGHT) {
            // Scroll complete
            slid_ = 0;
            isScrolling_ = false;

            if (isScrollingDown_) {
                topIndex_++;
            } else {
                topIndex_--;
            }
            int maxIdx = buildableCount_ - MAX_VISIBLE;
            topIndex_ = std::max(0, std::min(maxIdx, topIndex_));
            needsRedraw_ = true;
        }
    }

    // Process flash animation
    if (flashCount_ > 0) {
        flashCount_--;
        if (flashCount_ == 0) {
            flasher_ = -1;
        }
        needsRedraw_ = true;
    }

    // Check factories for changes
    isBuilding_ = false;
    for (int i = 0; i < buildableCount_; i++) {
        if (buildables_[i].factoryIndex >= 0) {
            FactoryClass* factory = &Factories[buildables_[i].factoryIndex];

            if (factory->Has_Changed()) {
                needsRedraw_ = true;
            }

            if (factory->Is_Building()) {
                isBuilding_ = true;
            }

            // Check for completion
            if (factory->Has_Completed()) {
                // Play "unit ready" sound in full implementation
                needsRedraw_ = true;
            }
        }
    }
}

BuildType* StripClass::Get_Buildable(int index) {
    int actualIndex = topIndex_ + index;
    if (actualIndex < 0 || actualIndex >= buildableCount_) {
        return nullptr;
    }
    return &buildables_[actualIndex];
}

const BuildType* StripClass::Get_Buildable(int index) const {
    int actualIndex = topIndex_ + index;
    if (actualIndex < 0 || actualIndex >= buildableCount_) {
        return nullptr;
    }
    return &buildables_[actualIndex];
}

//===========================================================================
// SidebarClass Implementation
//===========================================================================

SidebarClass::SidebarClass() {
    Init();
}

void SidebarClass::Init() {
    isActive_ = false;
    isToRedraw_ = true;

    isRepairActive_ = false;
    isUpgradeActive_ = false;
    isDemolishActive_ = false;

    playerHouse_ = nullptr;

    // Initialize columns
    columns_[0].Init(0, SIDE_X + COLUMN_ONE_X, SIDE_Y + COLUMN_ONE_Y);
    columns_[1].Init(1, SIDE_X + COLUMN_TWO_X, SIDE_Y + COLUMN_TWO_Y);
}

void SidebarClass::Activate(int control) {
    if (control < 0) {
        // Toggle
        isActive_ = !isActive_;
    } else {
        isActive_ = (control != 0);
    }
    isToRedraw_ = true;
}

bool SidebarClass::Add(RTTIType type, int id) {
    int col = Which_Column(type);
    if (col < 0 || col >= COLUMNS) {
        return false;
    }

    bool result = columns_[col].Add(type, id);
    if (result) {
        isToRedraw_ = true;
    }
    return result;
}

bool SidebarClass::Remove(RTTIType type, int id) {
    int col = Which_Column(type);
    if (col < 0 || col >= COLUMNS) {
        return false;
    }

    bool result = columns_[col].Remove(type, id);
    if (result) {
        isToRedraw_ = true;
    }
    return result;
}

void SidebarClass::Factory_Link(int factoryIndex, RTTIType type, int id) {
    int col = Which_Column(type);
    if (col >= 0 && col < COLUMNS) {
        columns_[col].Factory_Link(factoryIndex, type, id);
    }
}

void SidebarClass::Factory_Unlink(int factoryIndex) {
    for (int i = 0; i < COLUMNS; i++) {
        columns_[i].Factory_Unlink(factoryIndex);
    }
}

void SidebarClass::Recalc() {
    for (int i = 0; i < COLUMNS; i++) {
        columns_[i].Recalc(playerHouse_);
    }
}

void SidebarClass::Clear() {
    for (int i = 0; i < COLUMNS; i++) {
        columns_[i].Clear();
    }
    isToRedraw_ = true;
}

void SidebarClass::Toggle_Repair() {
    isRepairActive_ = !isRepairActive_;
    if (isRepairActive_) {
        isUpgradeActive_ = false;
        isDemolishActive_ = false;
    }
    isToRedraw_ = true;
}

void SidebarClass::Toggle_Upgrade() {
    isUpgradeActive_ = !isUpgradeActive_;
    if (isUpgradeActive_) {
        isRepairActive_ = false;
        isDemolishActive_ = false;
    }
    isToRedraw_ = true;
}

void SidebarClass::Zoom() {
    // Would trigger zoom/center on selected unit
    // For now, just a placeholder
}

bool SidebarClass::Input(int key, int x, int y,
                         bool leftClick, bool rightClick) {
    if (!isActive_) {
        return false;
    }

    // TAB key toggles sidebar
    if (key == '\t') {
        Activate(-1);
        return true;
    }

    // Check if point is in sidebar
    if (!Point_In_Sidebar(x, y)) {
        return false;
    }

    // Convert to sidebar-local coordinates
    int localX = x - SIDE_X;
    int localY = y - SIDE_Y;

    // Check top buttons (repair, upgrade, zoom)
    if (localY < TOP_HEIGHT) {
        if (leftClick) {
            if (localX < 32) {
                Toggle_Repair();
                return true;
            } else if (localX < 56) {
                Toggle_Upgrade();
                return true;
            } else {
                Zoom();
                return true;
            }
        }
        return false;
    }

    // Check strip clicks
    for (int i = 0; i < COLUMNS; i++) {
        int stripX = (i == 0) ? COLUMN_ONE_X : COLUMN_TWO_X;
        int stripY = COLUMN_ONE_Y;

        int relX = localX - stripX;
        int relY = localY - stripY;

        if (relX >= 0 && relX < OBJECT_WIDTH &&
            relY >= 0 && relY < OBJECT_HEIGHT * MAX_VISIBLE) {

            if (leftClick || rightClick) {
                return columns_[i].Click(relX, relY, leftClick, playerHouse_);
            }
        }
    }

    // Check scroll buttons (at top/bottom of each strip)
    // Would be implemented for up/down arrows

    return false;
}

bool SidebarClass::Point_In_Sidebar(int x, int y) const {
    return x >= SIDE_X && x < SIDE_X + SIDE_WIDTH &&
           y >= SIDE_Y && y < SIDE_Y + SIDE_HEIGHT;
}

void SidebarClass::AI() {
    if (!isActive_) {
        return;
    }

    // Update strips
    for (int i = 0; i < COLUMNS; i++) {
        columns_[i].AI(playerHouse_);
        if (columns_[i].Needs_Redraw()) {
            isToRedraw_ = true;
        }
    }

    // Process factories
    for (int i = 0; i < FACTORY_MAX; i++) {
        if (Factories[i].isActive_) {
            Factories[i].AI();
        }
    }
}

int SidebarClass::Which_Column(RTTIType type) {
    switch (type) {
        case RTTIType::BUILDING:
            return 0;  // Structures in left column

        case RTTIType::INFANTRY:
        case RTTIType::UNIT:
        case RTTIType::AIRCRAFT:
        case RTTIType::VESSEL:
            return 1;  // Units in right column

        case RTTIType::SPECIAL:
            return 0;  // Special weapons with structures

        default:
            return -1;
    }
}

bool SidebarClass::Start_Production(RTTIType type, int id) {
    if (!playerHouse_) return false;

    // Find or create factory
    FactoryClass* factory = Find_Factory(type, id);
    if (!factory) {
        factory = Create_Factory();
        if (!factory) return false;

        if (!factory->Set(type, id, playerHouse_)) {
            Destroy_Factory(factory);
            return false;
        }
    }

    if (factory->Start()) {
        Factory_Link(factory->id_, type, id);
        return true;
    }

    return false;
}

bool SidebarClass::Suspend_Production(RTTIType type, int id) {
    FactoryClass* factory = Find_Factory(type, id);
    if (factory) {
        return factory->Suspend();
    }
    return false;
}

bool SidebarClass::Abandon_Production(RTTIType type, int id) {
    FactoryClass* factory = Find_Factory(type, id);
    if (factory) {
        Factory_Unlink(factory->id_);
        return factory->Abandon();
    }
    return false;
}
