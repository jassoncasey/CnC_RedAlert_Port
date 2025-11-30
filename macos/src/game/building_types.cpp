/**
 * Red Alert macOS Port - Building Type Data Tables
 *
 * Static data tables for building/structure types.
 * Ported from original BDATA.CPP
 */

#include "building_types.h"
#include <cstring>

//===========================================================================
// Building Type Table - Static data for all building types
// Note: Many values are defaults - actual values come from RULES.INI
//===========================================================================

const BuildingTypeData BuildingTypes[] = {
    // Allied Tech Center (ATEK)
    {
        BuildingType::ADVANCED_TECH, 0, "ATEK",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 1500, 5, -200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR | PrereqFlag::POWER, OwnerFlag::ALLIES
    },

    // Iron Curtain (IRON)
    {
        BuildingType::IRON_CURTAIN, 0, "IRON",
        FacingType::FACING_S, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 2800, 4, -200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::TECH | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Weapons Factory (WEAP)
    {
        BuildingType::WEAP, 0, "WEAP",
        FacingType::FACING_NONE, 384, 256, RemapType::ALTERNATE, BSizeType::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::UNIT, DirType::N,
        1000, 2000, 3, -100, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::PROC | PrereqFlag::POWER, OwnerFlag::ALL
    },

    // Chronosphere (PDOX)
    {
        BuildingType::CHRONOSPHERE, 0, "PDOX",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 2800, 4, -200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::TECH | PrereqFlag::POWER, OwnerFlag::ALLIES
    },

    // Pillbox (PBOX)
    {
        BuildingType::PILLBOX, 0, "PBOX",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_11,
        0x0000, 0x0020, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 400, 5, 0, ArmorType::CONCRETE,
        WeaponType::M60MG, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Camo Pillbox (HBOX)
    {
        BuildingType::CAMOPILLBOX, 0, "HBOX",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_11,
        0x0000, 0x0020, 0x0000,
        false, false, false, false, false, true, true, true, false, true, false, true,
        RTTIType::NONE, DirType::N,
        400, 600, 5, 0, ArmorType::CONCRETE,
        WeaponType::M60MG, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Radar Dome (DOME)
    {
        BuildingType::RADAR, 0, "DOME",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        1000, 1000, 10, -40, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::POWER, OwnerFlag::ALL
    },

    // Gap Generator (GAP)
    {
        BuildingType::GAP, 0, "GAP",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_12,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 500, 6, -60, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR | PrereqFlag::POWER, OwnerFlag::ALLIES
    },

    // Gun Turret (GUN)
    {
        BuildingType::TURRET, 0, "GUN",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_11,
        0x0000, 0x0030, 0x0000,
        false, false, false, false, false, false, true, true, false, false, true, true,
        RTTIType::NONE, DirType::N,
        400, 600, 5, 0, ArmorType::CONCRETE,
        WeaponType::TURRET_CANNON, WeaponType::NONE,
        PrereqFlag::RADAR, OwnerFlag::SOVIET
    },

    // AA Gun (AGUN)
    {
        BuildingType::AAGUN, 0, "AGUN",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_22,
        0x0000, 0x0030, 0x0000,
        false, false, false, false, false, false, true, true, false, false, true, true,
        RTTIType::NONE, DirType::N,
        600, 600, 5, 0, ArmorType::CONCRETE,
        WeaponType::AA_CANNON, WeaponType::NONE,
        PrereqFlag::RADAR, OwnerFlag::SOVIET
    },

    // Flame Tower (FTUR)
    {
        BuildingType::FLAME_TURRET, 0, "FTUR",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_11,
        0x0000, 0x0030, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 600, 3, 0, ArmorType::CONCRETE,
        WeaponType::FIRE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::SOVIET
    },

    // Construction Yard (FACT)
    {
        BuildingType::CONST, 0, "FACT",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::BUILDING, DirType::N,
        1500, 5000, 3, 0, ArmorType::CONCRETE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Ore Refinery (PROC)
    {
        BuildingType::REFINERY, 0, "PROC",
        FacingType::FACING_NONE, 0, 512, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        900, 2000, 4, -40, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::POWER, OwnerFlag::ALL
    },

    // Ore Silo (SILO)
    {
        BuildingType::STORAGE, 0, "SILO",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        150, 150, 2, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::PROC, OwnerFlag::ALL
    },

    // Helipad (HPAD)
    {
        BuildingType::HELIPAD, 0, "HPAD",
        FacingType::FACING_NONE, 128, 128, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, true, false, true, true, false, false, false, true,
        RTTIType::AIRCRAFT, DirType::N,
        400, 1500, 3, -10, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR, OwnerFlag::ALLIES
    },

    // SAM Site (SAM)
    {
        BuildingType::SAM, 0, "SAM",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_21,
        0x0000, 0x0060, 0x0000,
        false, false, false, false, false, true, true, true, false, false, true, true,
        RTTIType::NONE, DirType::N,
        400, 750, 3, 0, ArmorType::WOOD,
        WeaponType::NIKE, WeaponType::NONE,
        PrereqFlag::RADAR, OwnerFlag::SOVIET
    },

    // Airfield (AFLD)
    {
        BuildingType::AIRSTRIP, 0, "AFLD",
        FacingType::FACING_NONE, 512, 384, RemapType::ALTERNATE, BSizeType::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::AIRCRAFT, DirType::N,
        800, 2000, 5, -50, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR, OwnerFlag::SOVIET
    },

    // Power Plant (POWR)
    {
        BuildingType::POWER, 0, "POWR",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 300, 2, 100, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Advanced Power Plant (APWR)
    {
        BuildingType::ADVANCED_POWER, 0, "APWR",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        700, 500, 4, 200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::POWER, OwnerFlag::ALL
    },

    // Soviet Tech Center (STEK)
    {
        BuildingType::SOVIET_TECH, 0, "STEK",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 1500, 5, -200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Hospital (HOSP)
    {
        BuildingType::HOSPITAL, 0, "HOSP",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 500, 2, -20, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Allied Barracks (BARR)
    {
        BuildingType::BARRACKS, 0, "BARR",
        FacingType::FACING_NONE, 256, 384, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::INFANTRY, DirType::N,
        800, 300, 3, -20, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::POWER, OwnerFlag::ALLIES
    },

    // Soviet Barracks (TENT)
    {
        BuildingType::TENT, 0, "TENT",
        FacingType::FACING_NONE, 256, 384, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::INFANTRY, DirType::N,
        800, 300, 3, -20, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Kennel (KENN)
    {
        BuildingType::KENNEL, 0, "KENN",
        FacingType::FACING_NONE, 128, 256, RemapType::ALTERNATE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, true, false, true, true, false, false, false, true,
        RTTIType::INFANTRY, DirType::N,
        400, 200, 2, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::SOVIET
    },

    // Service Depot (FIX)
    {
        BuildingType::REPAIR, 0, "FIX",
        FacingType::FACING_NONE, 256, 384, RemapType::ALTERNATE, BSizeType::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        800, 1200, 3, -30, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::FACTORY, OwnerFlag::ALL
    },

    // Bio Research Lab (BIO)
    {
        BuildingType::BIO_LAB, 0, "BIO",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 1500, 5, -100, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::TECH | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Mission Control (unused)
    {
        BuildingType::MISSION, 0, "MISS",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, false, false, true, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 0, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },

    // Shipyard (SYRD)
    {
        BuildingType::SHIP_YARD, 0, "SYRD",
        FacingType::FACING_NONE, 332, 384, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::VESSEL, DirType::N,
        1500, 650, 3, -20, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::PROC | PrereqFlag::POWER, OwnerFlag::ALLIES
    },

    // Sub Pen (SPEN)
    {
        BuildingType::SUB_PEN, 0, "SPEN",
        FacingType::FACING_NONE, 256, 384, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::VESSEL, DirType::N,
        1500, 650, 3, -20, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::PROC | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Missile Silo (MSLO)
    {
        BuildingType::MSLO, 0, "MSLO",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_21,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        1000, 2500, 3, -100, ArmorType::CONCRETE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::TECH | PrereqFlag::POWER, OwnerFlag::ALL
    },

    // Forward Command (FCOM)
    {
        BuildingType::FORWARD_COM, 0, "FCOM",
        FacingType::FACING_S, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, true, false, false, true, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 1500, 10, -200, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::RADAR | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Tesla Coil (TSLA)
    {
        BuildingType::TESLA, 0, "TSLA",
        FacingType::FACING_NONE, 0, 0, RemapType::NORMAL, BSizeType::BSIZE_12,
        0x0000, 0x0060, 0x0000,
        false, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        400, 1500, 6, -150, ArmorType::CONCRETE,
        WeaponType::TESLA_COIL, WeaponType::NONE,
        PrereqFlag::TECH | PrereqFlag::POWER, OwnerFlag::SOVIET
    },

    // Fake Weapons Factory (WEAP fake)
    {
        BuildingType::FAKEWEAP, 0, "WEAP",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 25, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Fake Construction Yard (FACT fake)
    {
        BuildingType::FAKECONST, 0, "FACT",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 25, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Fake Shipyard (SYRD fake)
    {
        BuildingType::FAKE_YARD, 0, "SYRD",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 25, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALLIES
    },

    // Fake Sub Pen (SPEN fake)
    {
        BuildingType::FAKE_PEN, 0, "SPEN",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 25, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::SOVIET
    },

    // Fake Radar (DOME fake)
    {
        BuildingType::FAKE_RADAR, 0, "DOME",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false, true, true, false, false, false, true,
        RTTIType::NONE, DirType::N,
        500, 25, 3, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Sandbag Wall (SBAG)
    {
        BuildingType::SANDBAG_WALL, 0, "SBAG",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        25, 50, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Chain Link Fence (CYCL)
    {
        BuildingType::CYCLONE_WALL, 0, "CYCL",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        10, 75, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Concrete Wall (BRIK)
    {
        BuildingType::BRICK_WALL, 0, "BRIK",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        75, 100, 0, 0, ArmorType::CONCRETE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::SOVIET
    },

    // Barbed Wire (BARB)
    {
        BuildingType::BARBWIRE_WALL, 0, "BARB",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        10, 25, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Wood Fence (WOOD)
    {
        BuildingType::WOOD_WALL, 0, "WOOD",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, true, false, false,
        RTTIType::NONE, DirType::N,
        10, 25, 0, 0, ArmorType::WOOD,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::ALLIES
    },

    // Wire Fence (FENC)
    {
        BuildingType::FENCE, 0, "FENC",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, true, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        10, 75, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::BARRACKS, OwnerFlag::SOVIET
    },

    // Anti-Vehicle Mine (MINV)
    {
        BuildingType::AVMINE, 0, "MINV",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, true, true, false, false, true, false, false, true,
        RTTIType::NONE, DirType::N,
        5, 25, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Anti-Personnel Mine (MINP)
    {
        BuildingType::APMINE, 0, "MINP",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, false, false, true, true, false, false, true, false, false, true,
        RTTIType::NONE, DirType::N,
        5, 25, 0, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, OwnerFlag::ALL
    },

    // Civilian structures V01-V18
    {BuildingType::V01, 0, "V01", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V02, 0, "V02", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V03, 0, "V03", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V04, 0, "V04", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V05, 0, "V05", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_21, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V06, 0, "V06", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V07, 0, "V07", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V08, 0, "V08", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V09, 0, "V09", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V10, 0, "V10", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V11, 0, "V11", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V12, 0, "V12", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V13, 0, "V13", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V14, 0, "V14", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V15, 0, "V15", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V16, 0, "V16", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V17, 0, "V17", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V18, 0, "V18", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 100, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},

    // Water Pump (PUMP / V19)
    {BuildingType::PUMP, 0, "V19", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, true, true, false, true, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},

    // Civilian structures V20-V37
    {BuildingType::V20, 0, "V20", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V21, 0, "V21", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_21, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V22, 0, "V22", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_21, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V23, 0, "V23", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V24, 0, "V24", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V25, 0, "V25", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V26, 0, "V26", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_21, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V27, 0, "V27", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V28, 0, "V28", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V29, 0, "V29", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V30, 0, "V30", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V31, 0, "V31", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V32, 0, "V32", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V33, 0, "V33", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_22, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V34, 0, "V34", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V35, 0, "V35", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V36, 0, "V36", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},
    {BuildingType::V37, 0, "V37", FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_32, 0, 0, 0, false, false, true, false, false, false, true, true, true, true, false, false, RTTIType::NONE, DirType::N, 200, 0, 1, 0, ArmorType::WOOD, WeaponType::NONE, WeaponType::NONE, PrereqFlag::NONE, 0},

    // Explosive Barrel (BARL)
    {
        BuildingType::BARREL, 0, "BARL",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, true, false, true, true, true, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        20, 0, 1, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },

    // 3-Barrel Group (BRL3)
    {
        BuildingType::BARREL3, 0, "BRL3",
        FacingType::FACING_NONE, 0, 0, RemapType::ALTERNATE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, false, true, false, true, true, false, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        20, 0, 1, 0, ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },

    // Ant Queen (QUEE)
    {
        BuildingType::QUEEN, 0, "QUEE",
        FacingType::FACING_NONE, 256, 384, RemapType::NONE, BSizeType::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        false, false, true, false, false, false, true, true, false, false, false, false,
        RTTIType::NONE, DirType::N,
        800, 0, 4, 0, ArmorType::LIGHT,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },

    // Larva 1 (LAR1)
    {
        BuildingType::LARVA1, 0, "LAR1",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, true, true, false, false, false, true, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        150, 0, 1, 0, ArmorType::LIGHT,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },

    // Larva 2 (LAR2)
    {
        BuildingType::LARVA2, 0, "LAR2",
        FacingType::FACING_NONE, 0, 0, RemapType::NONE, BSizeType::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        false, true, true, false, false, false, true, true, true, false, false, false,
        RTTIType::NONE, DirType::N,
        150, 0, 1, 0, ArmorType::LIGHT,
        WeaponType::NONE, WeaponType::NONE,
        PrereqFlag::NONE, 0
    },
};

const int BuildingTypeCount = sizeof(BuildingTypes) / sizeof(BuildingTypes[0]);

//===========================================================================
// Building Size Dimensions
//===========================================================================
static const struct {
    int width;
    int height;
} BuildingSizes[] = {
    {1, 1},  // BSIZE_11
    {2, 1},  // BSIZE_21
    {1, 2},  // BSIZE_12
    {2, 2},  // BSIZE_22
    {2, 3},  // BSIZE_23
    {3, 2},  // BSIZE_32
    {3, 3},  // BSIZE_33
    {4, 2},  // BSIZE_42
    {5, 5},  // BSIZE_55
};

//===========================================================================
// Helper Functions
//===========================================================================

const BuildingTypeData* GetBuildingType(BuildingType type) {
    for (int i = 0; i < BuildingTypeCount; i++) {
        if (BuildingTypes[i].type == type) {
            return &BuildingTypes[i];
        }
    }
    return nullptr;
}

BuildingType BuildingTypeFromName(const char* name) {
    if (name == nullptr) return BuildingType::NONE;

    for (int i = 0; i < BuildingTypeCount; i++) {
        if (strcasecmp(BuildingTypes[i].iniName, name) == 0) {
            return BuildingTypes[i].type;
        }
    }
    return BuildingType::NONE;
}

void GetBuildingSize(BSizeType size, int& width, int& height) {
    int idx = static_cast<int>(size);
    if (idx >= 0 && idx < static_cast<int>(BSizeType::COUNT)) {
        width = BuildingSizes[idx].width;
        height = BuildingSizes[idx].height;
    } else {
        width = 1;
        height = 1;
    }
}

bool IsBuildingWall(BuildingType type) {
    switch (type) {
        case BuildingType::SANDBAG_WALL:
        case BuildingType::CYCLONE_WALL:
        case BuildingType::BRICK_WALL:
        case BuildingType::BARBWIRE_WALL:
        case BuildingType::WOOD_WALL:
        case BuildingType::FENCE:
            return true;
        default:
            return false;
    }
}

bool IsBuildingCivilian(BuildingType type) {
    int idx = static_cast<int>(type);
    int v01 = static_cast<int>(BuildingType::V01);
    int v37 = static_cast<int>(BuildingType::V37);

    return (idx >= v01 && idx <= v37) ||
           type == BuildingType::PUMP ||
           type == BuildingType::BARREL ||
           type == BuildingType::BARREL3;
}

bool IsBuildingFactory(BuildingType type) {
    const BuildingTypeData* data = GetBuildingType(type);
    return data != nullptr && data->factoryType != RTTIType::NONE;
}
