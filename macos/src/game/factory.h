/**
 * Red Alert macOS Port - Factory System
 *
 * FactoryClass - Manages production of a single object
 *
 * Based on original FACTORY.H, FACTORY.CPP
 */

#ifndef GAME_FACTORY_H
#define GAME_FACTORY_H

#include "types.h"
#include <cstdint>

// Forward declarations
class HouseClass;
class TechnoTypeClass;
class TechnoClass;

//===========================================================================
// Constants
//===========================================================================

constexpr int FACTORY_MAX = 32;        // Max factories per house
constexpr int STEP_COUNT = 54;         // Production animation steps
constexpr int QUEUE_MAX = 5;           // Max items in production queue

//===========================================================================
// QueueEntry - Item in production queue
//===========================================================================

struct QueueEntry {
    RTTIType type = RTTIType::NONE;
    int id = -1;
    bool isValid() const { return type != RTTIType::NONE && id >= 0; }
    void clear() { type = RTTIType::NONE; id = -1; }
};

//===========================================================================
// FactoryClass - Production Manager
//===========================================================================

class FactoryClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    int16_t id_;                        // Factory index
    bool isActive_;                     // Factory is active

    //-----------------------------------------------------------------------
    // Production State
    //-----------------------------------------------------------------------
    RTTIType productionType_;           // What category we're producing
    int productionId_;                  // Specific object ID
    TechnoClass* object_;               // Object being built (in limbo)
    SpecialWeaponType specialItem_;     // For special weapons

    //-----------------------------------------------------------------------
    // Cost Tracking
    //-----------------------------------------------------------------------
    int balance_;                       // Remaining cost to pay
    int originalBalance_;               // Initial cost (for refunds)

    //-----------------------------------------------------------------------
    // Timing
    //-----------------------------------------------------------------------
    int stage_;                         // Current production stage (0-54)
    int rate_;                          // Ticks per stage
    int ticksRemaining_;                // Ticks until next stage

    //-----------------------------------------------------------------------
    // Owner
    //-----------------------------------------------------------------------
    HouseClass* house_;                 // Owner house

    //-----------------------------------------------------------------------
    // Flags
    //-----------------------------------------------------------------------
    bool isSuspended_;                  // Production paused
    bool isDifferent_;                  // Stage changed (for animation)
    bool hasCompleted_;                 // Production finished

    //-----------------------------------------------------------------------
    // Queue
    //-----------------------------------------------------------------------
    QueueEntry queue_[QUEUE_MAX];       // Production queue (index 0 = current)
    int queueCount_;                    // Number of items in queue

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    FactoryClass();
    ~FactoryClass();

    void Init();

    //-----------------------------------------------------------------------
    // Production Control
    //-----------------------------------------------------------------------

    /**
     * Set up factory for producing an object
     * @param type RTTI type (INFANTRY, UNIT, BUILDING, etc.)
     * @param id Object subtype ID
     * @param house Owner house
     * @return true if setup succeeded
     */
    bool Set(RTTIType type, int id, HouseClass* house);

    /**
     * Set up factory for a special weapon
     */
    bool Set_Special(SpecialWeaponType special, HouseClass* house);

    /**
     * Start or resume production
     * @return true if production started
     */
    bool Start();

    /**
     * Suspend (pause) production
     * @return true if suspended
     */
    bool Suspend();

    /**
     * Abandon production and refund money
     * @return true if abandoned
     */
    bool Abandon();

    /**
     * Process one tick of production
     * Called every game tick while producing
     */
    void AI();

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------

    /**
     * Get production completion percentage (0-100)
     */
    int Completion() const;

    /**
     * Get cost to charge this tick
     */
    int Cost_Per_Tick() const;

    /**
     * Check if production has completed
     */
    bool Has_Completed() const { return hasCompleted_; }

    /**
     * Check if factory has changed since last check (for animation)
     */
    bool Has_Changed();

    /**
     * Check if production is in progress
     */
    bool Is_Building() const {
        return stage_ > 0 && !isSuspended_ && !hasCompleted_;
    }

    /**
     * Get the object being produced
     */
    TechnoClass* Get_Object() const { return object_; }

    /**
     * Get production type
     */
    RTTIType Get_Type() const { return productionType_; }

    /**
     * Get production ID
     */
    int Get_ID() const { return productionId_; }

    //-----------------------------------------------------------------------
    // Queue Management
    //-----------------------------------------------------------------------

    /**
     * Add item to end of queue
     * @return true if added, false if queue full
     */
    bool Queue_Add(RTTIType type, int id);

    /**
     * Remove item from queue by index
     * @return true if removed
     */
    bool Queue_Remove(int index);

    /**
     * Get queue count
     */
    int Queue_Count() const { return queueCount_; }

    /**
     * Get queue entry at index
     */
    const QueueEntry* Queue_Get(int index) const;

    /**
     * Start next queued item after current completes
     */
    void Queue_Advance();

    //-----------------------------------------------------------------------
    // Completion
    //-----------------------------------------------------------------------

    /**
     * Complete production and release object
     * @return The completed object (caller takes ownership)
     */
    TechnoClass* Complete();

private:
    /**
     * Calculate production rate based on power and difficulty
     */
    int Calculate_Rate(int baseTime);
};

//===========================================================================
// Global Factory Array
//===========================================================================

extern FactoryClass Factories[FACTORY_MAX];
extern int FactoryCount;

//===========================================================================
// Helper Functions
//===========================================================================

void Init_Factories();
FactoryClass* Create_Factory();
void Destroy_Factory(FactoryClass* factory);
FactoryClass* Find_Factory(RTTIType type, int id);

#endif // GAME_FACTORY_H
