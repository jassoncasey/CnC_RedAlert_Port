/**
 * Red Alert macOS Port - Object Class Hierarchy
 *
 * Base classes for all game objects. The inheritance chain is:
 * AbstractClass -> ObjectClass -> MissionClass -> RadioClass -> TechnoClass -> FootClass
 *
 * Based on original ABSTRACT.H, OBJECT.H, MISSION.H, RADIO.H, TECHNO.H
 */

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "types.h"
#include <cstdint>
#include <cstring>

// Forward declarations
class ObjectClass;
class RadioClass;
class TechnoClass;
class FootClass;
class HouseClass;

//===========================================================================
// Helper functions
//===========================================================================
int Distance(int32_t coord1, int32_t coord2);
uint8_t Direction256(int32_t coord1, int32_t coord2);

//===========================================================================
// AbstractClass - Base of all game objects
//===========================================================================
class AbstractClass {
public:
    // Runtime type identification
    RTTIType rtti_;
    int16_t id_;

    // Position (COORDINATE = cell * 256 + lepton offset)
    int32_t coord_;

    // Height above ground (leptons)
    int16_t height_;

    // Is this object slot in use?
    bool isActive_;

    // Construction/destruction
    AbstractClass() : rtti_(RTTIType::NONE), id_(0), coord_(0), height_(0), isActive_(true) {}
    AbstractClass(RTTIType rtti, int id);
    virtual ~AbstractClass() = default;

    // Query functions
    virtual const char* Name() const { return ""; }
    virtual HousesType Owner() const { return HousesType::NONE; }
    RTTIType WhatAmI() const { return rtti_; }
    int ID() const { return id_; }

    // Coordinate queries
    virtual int32_t CenterCoord() const { return coord_; }
    virtual int32_t TargetCoord() const { return coord_; }

    // Direction and distance helpers
    uint8_t DirectionTo(const AbstractClass* object) const;
    uint8_t DirectionTo(int32_t coord) const;
    int DistanceTo(int32_t coord) const;
    int DistanceTo(const AbstractClass* object) const;

    // Movement queries
    virtual MoveType CanEnterCell(int16_t cell, FacingType facing = FacingType::NORTH) const;

    // AI processing
    virtual void AI() {}
};

//===========================================================================
// ObjectClass - Objects that exist on the map
//===========================================================================
class ObjectClass : public AbstractClass {
public:
    // Placement state
    bool isDown_;           // Object is placed on map
    bool isToDamage_;       // Marked for damage processing
    bool isToDisplay_;      // Needs redraw
    bool isInLimbo_;        // In limbo (transport, not yet placed)
    bool isSelected_;       // Currently selected by player
    bool isAnimAttached_;   // Has animation attached
    bool isFalling_;        // Falling (parachute, grenade)

    // Fall rate (leptons per tick)
    int16_t riser_;

    // Next object in cell list (linked list)
    ObjectClass* next_;

    // Current hit points
    int16_t strength_;

    // Construction
    ObjectClass() : AbstractClass(), isDown_(false), isToDamage_(false), isToDisplay_(false), isInLimbo_(true), isSelected_(false), strength_(0) {}
    ObjectClass(RTTIType rtti, int id);
    virtual ~ObjectClass() = default;

    // Type queries
    bool IsInfantry() const { return rtti_ == RTTIType::INFANTRY; }
    bool IsFoot() const;
    bool IsTechno() const;

    // Coordinate queries (override for different object types)
    virtual int32_t DockingCoord() const { return coord_; }
    virtual int32_t RenderCoord() const { return coord_; }
    virtual int32_t SortY() const { return coord_; }
    virtual int32_t FireCoord(int which) const { return coord_; }
    virtual int32_t ExitCoord() const { return coord_; }

    // Limbo control
    virtual bool Limbo();
    virtual bool Unlimbo(int32_t coord, DirType facing = DirType::N);
    virtual void Detach(uint32_t target, bool all = true);
    virtual void DetachAll(bool all = true);

    // Rendering
    virtual bool Render(bool forced) const { return false; }
    virtual void DrawIt(int x, int y, int window) const = 0;
    virtual const int16_t* OccupyList(bool placement = false) const;
    virtual const int16_t* OverlapList(bool redraw = false) const;
    virtual int HealthRatio() const;  // 0-256 scale (256 = 100%)
    virtual void Hidden() {}
    virtual void Look(bool incremental = false) {}
    virtual bool Mark(MarkType mark = MarkType::CHANGE);

    // Selection
    virtual bool Select();
    virtual void Unselect();
    virtual void ClickedAsTarget(int priority = 7) {}

    // Combat
    virtual bool InRange(int32_t coord, int weapon = 0) const;
    virtual int WeaponRange(int weapon = 0) const;
    virtual ResultType TakeDamage(int& damage, int distance, WarheadType warhead,
                                   TechnoClass* source = nullptr, bool forced = false);
    virtual void Scatter(int32_t coord, bool forced = false, bool nokidding = false) {}
    virtual bool CatchFire() { return false; }
    virtual void FireOut() {}
    virtual int Value() const { return 0; }

    // AI
    virtual MissionType GetMission() const { return MissionType::NONE; }
    virtual void AI() override;

    // Static constants
    static constexpr int FLIGHT_LEVEL = 256;  // Leptons above ground for aircraft
};

//===========================================================================
// MissionClass - AI order processing
//===========================================================================
class MissionClass : public ObjectClass {
public:
    // Current and queued missions
    MissionType mission_;
    MissionType suspendedMission_;
    MissionType missionQueue_;

    // Mission state machine status
    int status_;

    // Mission processing timer (frames until next AI tick)
    int16_t timer_;

    // Construction
    MissionClass() : ObjectClass(), mission_(MissionType::SLEEP), suspendedMission_(MissionType::NONE), missionQueue_(MissionType::NONE), status_(0), timer_(0) {}
    MissionClass(RTTIType rtti, int id);
    virtual ~MissionClass() = default;

    // Mission control
    virtual MissionType GetMission() const override { return mission_; }
    virtual void AssignMission(MissionType mission);
    virtual bool Commence();
    virtual void SetMission(MissionType mission);
    void ShortenMissionTimer() { timer_ = 0; }

    // Mission handlers (return delay in frames until next call)
    virtual int MissionSleep();
    virtual int MissionAmbush();
    virtual int MissionAttack();
    virtual int MissionCapture();
    virtual int MissionGuard();
    virtual int MissionGuardArea();
    virtual int MissionHarvest();
    virtual int MissionHunt();
    virtual int MissionMove();
    virtual int MissionRetreat();
    virtual int MissionReturn();
    virtual int MissionStop();
    virtual int MissionUnload();
    virtual int MissionEnter();
    virtual int MissionConstruction();
    virtual int MissionDeconstruction();
    virtual int MissionRepair();
    virtual int MissionMissile();

    // Mission override/restore
    virtual void OverrideMission(MissionType mission, uint32_t target1, uint32_t target2);
    virtual bool RestoreMission();

    // Static helpers
    static const char* MissionName(MissionType mission);
    static MissionType MissionFromName(const char* name);
    static bool IsRecruitableMission(MissionType mission);

    // AI
    virtual void AI() override;
};

//===========================================================================
// RadioClass - Inter-object communication
//===========================================================================
class RadioClass : public MissionClass {
public:
    // Last received messages (history buffer)
    RadioMessageType oldMessages_[3];

    // Current radio contact (two-way link)
    RadioClass* radio_;

    // Construction
    RadioClass() : MissionClass(), radio_(nullptr) { oldMessages_[0] = oldMessages_[1] = oldMessages_[2] = RadioMessageType::STATIC; }
    RadioClass(RTTIType rtti, int id);
    virtual ~RadioClass() = default;

    // Radio status
    bool InRadioContact() const { return radio_ != nullptr; }
    void RadioOff() { radio_ = nullptr; }
    TechnoClass* ContactWithWhom() const { return reinterpret_cast<TechnoClass*>(radio_); }

    // Message handling
    virtual RadioMessageType ReceiveMessage(RadioClass* from, RadioMessageType message, int32_t& param);
    virtual RadioMessageType TransmitMessage(RadioMessageType message, int32_t& param, RadioClass* to = nullptr);
    virtual RadioMessageType TransmitMessage(RadioMessageType message, RadioClass* to);

    // Override limbo to handle radio contacts
    virtual bool Limbo() override;

    // Static message names (for debugging)
    static const char* MessageName(RadioMessageType message);
};

//===========================================================================
// TechnoClass - Combat-capable objects (buildings, units, aircraft)
//===========================================================================
class TechnoClass : public RadioClass {
public:
    // State flags
    bool isUseless_;           // Should be sold/sacrificed
    bool isTickedOff_;         // Damaged with malice (not friendly fire)
    bool isCloakable_;         // Can cloak
    bool isLeader_;            // Primary factory or team leader
    bool isALoaner_;           // Transport for reinforcements
    bool isLocked_;            // Has entered the map
    bool isInRecoilState_;     // Firing recoil animation
    bool isTethered_;          // Attached to transport during unload
    bool isOwnedByPlayer_;     // Human player owns this
    bool isDiscoveredByPlayer_;
    bool isDiscoveredByComputer_;
    bool isALemon_;            // Takes random damage
    bool isSecondShot_;        // Ready for second quick shot

    // Combat modifiers (fixed point, 256 = 1.0)
    int16_t armorBias_;        // Armor multiplier
    int16_t firepowerBias_;    // Damage multiplier

    // Timers
    int16_t idleTimer_;        // Idle animation timer
    int16_t ironCurtainTimer_; // Iron curtain effect

    // Spy tracking (bitfield of houses spying)
    uint16_t spiedBy_;

    // Archive target (home position, pending transport, etc.)
    uint32_t archiveTarget_;

    // Owning house
    HousesType house_;

    // Cloak state
    CloakType cloakState_;
    int16_t cloakTimer_;
    int16_t cloakStage_;       // Visual cloaking stage

    // Targets
    uint32_t tarCom_;          // Primary target
    uint32_t suspendedTarCom_; // Suspended target
    uint32_t navCom_;          // Navigation target (destination)
    uint32_t suspendedNavCom_; // Suspended nav target

    // Weapon timers
    int16_t arm_[2];           // Rearm countdown for each weapon
    int16_t ammo_;             // Ammunition count (-1 = infinite)

    // Price paid (for refund calculation)
    int16_t pricePaid_;

    // Turret facing (if applicable)
    DirType turretFacing_;
    DirType turretFacingTarget_;

    // Construction
    TechnoClass() : RadioClass(), isUseless_(false), isTickedOff_(false), isCloakable_(false), isLeader_(false),
                    isALoaner_(false), isLocked_(false), isInRecoilState_(false), isTethered_(false),
                    isOwnedByPlayer_(false), isDiscoveredByPlayer_(false), isDiscoveredByComputer_(false),
                    isALemon_(false), isSecondShot_(false), armorBias_(256), firepowerBias_(256),
                    idleTimer_(0), ironCurtainTimer_(0), spiedBy_(0), archiveTarget_(0),
                    house_(HousesType::NONE), cloakState_(CloakType::UNCLOAKED), cloakTimer_(0), cloakStage_(0),
                    tarCom_(0), suspendedTarCom_(0), navCom_(0), suspendedNavCom_(0),
                    ammo_(-1), pricePaid_(0), turretFacing_(DirType::N), turretFacingTarget_(DirType::N)
                    { arm_[0] = arm_[1] = 0; }
    TechnoClass(RTTIType rtti, int id);
    virtual ~TechnoClass() = default;

    // Owner queries
    virtual HousesType Owner() const override { return house_; }
    bool IsOwnedByPlayer() const { return isOwnedByPlayer_; }
    void SetHouse(HousesType house);

    // Combat
    virtual bool IsAllowedToRetaliate() const;
    virtual bool IsAllowedToScatter() const;
    virtual bool CanFire() const;
    virtual int RearmTime(int weapon = 0) const;
    virtual void Assign_Target(uint32_t target);
    virtual uint32_t GetTarget() const { return tarCom_; }
    virtual bool Fire_At(int32_t targetCoord, int weapon = 0);
    virtual int GetWeapon(int which = 0) const { return -1; }  // Override in derived classes

    // Cloaking
    virtual void Cloak();
    virtual void Uncloak();
    bool IsCloaked() const { return cloakState_ == CloakType::CLOAKED; }

    // Rendering helpers
    virtual int ShapeNumber() const { return 0; }
    virtual void* GetImageData() const { return nullptr; }

    // AI
    virtual void AI() override;

    // Per-cell processing callback
    virtual void PerCellProcess(PCPType pcp) {}
};

//===========================================================================
// FootClass - Mobile units (infantry, vehicles, aircraft, vessels)
//===========================================================================
class FootClass : public TechnoClass {
public:
    // Movement state
    bool isInitiated_;         // Movement sequence initiated
    bool isMovingOntoBridge_;  // Crossing a bridge
    bool isUnloading_;         // In unload sequence
    bool isScattering_;        // Scattering from threat
    bool isPrimaryFacing_;     // Body using primary facing
    bool isRotating_;          // Body is rotating
    bool isFiring_;            // Currently firing
    bool isDriving_;           // Currently driving (has momentum)
    bool isToLook_;            // Should recalculate targets
    bool isDeploying_;         // Deploying (MCV, etc.)
    bool isNewNavCom_;         // NavCom just changed
    bool isPlanning_;          // AI is planning path

    // Path data
    FacingType path_[24];      // Current path (sequence of directions)
    int pathLength_;           // Length of path
    int pathIndex_;            // Current position in path

    // Movement targets
    int32_t headTo_;           // Immediate destination cell center
    uint32_t member_;          // Team membership

    // Speed control
    int16_t speed_;            // Current speed (0-255)
    int16_t speedAccum_;       // Speed accumulator for sub-cell movement

    // Group membership
    int8_t group_;             // Ctrl+# group assignment (-1 = none)

    // Body facing
    DirType bodyFacing_;
    DirType bodyFacingTarget_;

    // Construction
    FootClass() : TechnoClass(), isInitiated_(false), isMovingOntoBridge_(false), isUnloading_(false),
                  isScattering_(false), isPrimaryFacing_(false), isRotating_(false), isFiring_(false),
                  isDriving_(false), isToLook_(false), isDeploying_(false), isNewNavCom_(false), isPlanning_(false),
                  pathLength_(0), pathIndex_(0), headTo_(0), member_(0), speed_(0), speedAccum_(0), group_(-1),
                  bodyFacing_(DirType::N), bodyFacingTarget_(DirType::N) { memset(path_, 0, sizeof(path_)); }
    FootClass(RTTIType rtti, int id);
    virtual ~FootClass() = default;

    // Movement
    virtual bool StartDrive(int32_t destination);
    virtual bool StopDrive();
    virtual void DoTurn(DirType facing);
    virtual bool IsDriving() const { return isDriving_; }

    // Pathfinding
    virtual bool BasicPath(int32_t destination);
    virtual MoveType CanEnterCell(int16_t cell, FacingType facing = FacingType::NORTH) const override;

    // Speed
    virtual int TopSpeed() const;
    int CurrentSpeed() const { return speed_; }
    void SetSpeed(int speed) { speed_ = static_cast<int16_t>(speed); }

    // Group control
    int Group() const { return group_; }
    void SetGroup(int group) { group_ = static_cast<int8_t>(group); }

    // AI
    virtual void AI() override;
};

//===========================================================================
// Object Pool Management
//===========================================================================

/**
 * Simple object pool for game objects.
 * Objects are allocated from fixed-size arrays.
 */
template<typename T, int MaxCount>
class ObjectPool {
public:
    ObjectPool() : count_(0) {
        for (int i = 0; i < MaxCount; i++) {
            objects_[i] = nullptr;
        }
    }

    T* Allocate() {
        for (int i = 0; i < MaxCount; i++) {
            if (objects_[i] == nullptr) {
                objects_[i] = new T();
                count_++;
                return objects_[i];
            }
        }
        return nullptr;  // Pool exhausted
    }

    void Free(T* obj) {
        if (obj == nullptr) return;
        int id = obj->ID();
        if (id >= 0 && id < MaxCount && objects_[id] == obj) {
            delete objects_[id];
            objects_[id] = nullptr;
            count_--;
        }
    }

    T* Get(int id) {
        if (id >= 0 && id < MaxCount) {
            return objects_[id];
        }
        return nullptr;
    }

    int Count() const { return count_; }
    int Capacity() const { return MaxCount; }

private:
    T* objects_[MaxCount];
    int count_;
};

#endif // GAME_OBJECT_H
