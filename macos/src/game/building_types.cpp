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

// Shorthand for compact data table rows
#define T true
#define F false
#define BT BuildingType
#define FT FacingType
#define RM RemapType
#define BS BSizeType
#define RT RTTIType
#define DT DirType
#define AT ArmorType
#define WT WeaponType
#define PF PrereqFlag
#define OF OwnerFlag

const BuildingTypeData BuildingTypeDefaults[] = {
    // Allied Tech Center (ATEK)
    {
        BT::ADVANCED_TECH, 0, "ATEK",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 1500, 5, -200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR | PF::POWER, OF::ALLIES,
        7, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Iron Curtain (IRON)
    {
        BT::IRON_CURTAIN, 0, "IRON",
        FT::SOUTH, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 2800, 4, -200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::TECH | PF::POWER, OF::SOVIET,
        10, 80, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Weapons Factory (WEAP)
    {
        BT::WEAP, 0, "WEAP",
        FT::NONE, 384, 256, RM::ALTERNATE, BS::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::UNIT, DT::N,
        1000, 2000, 3, -100, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::PROC | PF::POWER, OF::ALL,
        3, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Chronosphere (PDOX)
    {
        BT::CHRONOSPHERE, 0, "PDOX",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 2800, 4, -200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::TECH | PF::POWER, OF::ALLIES,
        10, 80, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Pillbox (PBOX)
    {
        BT::PILLBOX, 0, "PBOX",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_11,
        0x0000, 0x0020, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 400, 5, 0, AT::CONCRETE,
        WT::M60MG, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        2, 15, F, T, F  // tech, pts, capturable, crewed, bib
    },

    // Camo Pillbox (HBOX)
    {
        BT::CAMOPILLBOX, 0, "HBOX",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_11,
        0x0000, 0x0020, 0x0000,
        F, F, F, F, F, T, T, T, F, T, F, T,
        RT::NONE, DT::N,
        400, 600, 5, 0, AT::CONCRETE,
        WT::M60MG, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        5, 20, F, T, F  // tech, pts, capturable, crewed, bib
    },

    // Radar Dome (DOME)
    {
        BT::RADAR, 0, "DOME",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        1000, 1000, 10, -40, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::POWER, OF::ALL,
        2, 30, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Gap Generator (GAP)
    {
        BT::GAP, 0, "GAP",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_12,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 500, 6, -60, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR | PF::POWER, OF::ALLIES,
        5, 30, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Gun Turret (GUN)
    {
        BT::TURRET, 0, "GUN",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_11,
        0x0000, 0x0030, 0x0000,
        F, F, F, F, F, F, T, T, F, F, T, T,
        RT::NONE, DT::N,
        400, 600, 5, 0, AT::CONCRETE,
        WT::TURRET_CANNON, WT::NONE,
        PF::RADAR, OF::SOVIET,
        3, 20, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // AA Gun (AGUN)
    {
        BT::AAGUN, 0, "AGUN",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_22,
        0x0000, 0x0030, 0x0000,
        F, F, F, F, F, F, T, T, F, F, T, T,
        RT::NONE, DT::N,
        600, 600, 5, 0, AT::CONCRETE,
        WT::AA_CANNON, WT::NONE,
        PF::RADAR, OF::SOVIET,
        5, 20, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Flame Tower (FTUR)
    {
        BT::FLAME_TURRET, 0, "FTUR",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_11,
        0x0000, 0x0030, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 600, 3, 0, AT::CONCRETE,
        WT::FIRE, WT::NONE,
        PF::BARRACKS, OF::SOVIET,
        2, 20, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Construction Yard (FACT)
    {
        BT::CONST, 0, "FACT",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::BUILDING, DT::N,
        1500, 5000, 3, 0, AT::CONCRETE,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        -1, 100, T, F, T  // tech=-1 (can't build), pts, cap, crew, bib
    },

    // Ore Refinery (PROC)
    {
        BT::REFINERY, 0, "PROC",
        FT::NONE, 0, 512, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        900, 2000, 4, -40, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::POWER, OF::ALL,
        1, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Ore Silo (SILO)
    {
        BT::STORAGE, 0, "SILO",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        150, 150, 2, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::PROC, OF::ALL,
        2, 5, T, F, F  // tech, pts, capturable, crewed, bib
    },

    // Helipad (HPAD)
    {
        BT::HELIPAD, 0, "HPAD",
        FT::NONE, 128, 128, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, T, F, T, T, F, F, F, T,
        RT::AIRCRAFT, DT::N,
        400, 1500, 3, -10, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR, OF::ALLIES,
        5, 30, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // SAM Site (SAM)
    {
        BT::SAM, 0, "SAM",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_21,
        0x0000, 0x0060, 0x0000,
        F, F, F, F, F, T, T, T, F, F, T, T,
        RT::NONE, DT::N,
        400, 750, 3, 0, AT::WOOD,
        WT::NIKE, WT::NONE,
        PF::RADAR, OF::SOVIET,
        5, 25, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Airfield (AFLD)
    {
        BT::AIRSTRIP, 0, "AFLD",
        FT::NONE, 512, 384, RM::ALTERNATE, BS::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::AIRCRAFT, DT::N,
        800, 2000, 5, -50, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR, OF::SOVIET,
        5, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Power Plant (POWR)
    {
        BT::POWER, 0, "POWR",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 300, 2, 100, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        1, 20, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Advanced Power Plant (APWR)
    {
        BT::ADVANCED_POWER, 0, "APWR",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        700, 500, 4, 200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::POWER, OF::ALL,
        3, 30, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Soviet Tech Center (STEK)
    {
        BT::SOVIET_TECH, 0, "STEK",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 1500, 5, -200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR | PF::POWER, OF::SOVIET,
        7, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Hospital (HOSP)
    {
        BT::HOSPITAL, 0, "HOSP",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 500, 2, -20, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        5, 20, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Allied Barracks (BARR)
    {
        BT::BARRACKS, 0, "BARR",
        FT::NONE, 256, 384, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::INFANTRY, DT::N,
        800, 300, 3, -20, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::POWER, OF::ALLIES,
        1, 20, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Soviet Barracks (TENT)
    {
        BT::TENT, 0, "TENT",
        FT::NONE, 256, 384, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::INFANTRY, DT::N,
        800, 300, 3, -20, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::POWER, OF::SOVIET,
        1, 20, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Kennel (KENN)
    {
        BT::KENNEL, 0, "KENN",
        FT::NONE, 128, 256, RM::ALTERNATE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, T, F, T, T, F, F, F, T,
        RT::INFANTRY, DT::N,
        400, 200, 2, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::SOVIET,
        2, 10, T, F, F  // tech, pts, capturable, crewed, bib
    },

    // Service Depot (FIX)
    {
        BT::REPAIR, 0, "FIX",
        FT::NONE, 256, 384, RM::ALTERNATE, BS::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        800, 1200, 3, -30, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::FACTORY, OF::ALL,
        4, 25, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Bio Research Lab (BIO)
    {
        BT::BIO_LAB, 0, "BIO",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 1500, 5, -100, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::TECH | PF::POWER, OF::SOVIET,
        -1, 40, T, F, T  // tech=-1 (scenario only), pts, cap, crew, bib
    },

    // Mission Control (unused)
    {
        BT::MISSION, 0, "MISS",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, F, F, T, F, F, T,
        RT::NONE, DT::N,
        400, 0, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 0, F, F, F  // tech=-1, pts, capturable, crewed, bib
    },

    // Shipyard (SYRD)
    {
        BT::SHIP_YARD, 0, "SYRD",
        FT::NONE, 332, 384, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::VESSEL, DT::N,
        1500, 650, 3, -20, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::PROC | PF::POWER, OF::ALLIES,
        3, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Sub Pen (SPEN)
    {
        BT::SUB_PEN, 0, "SPEN",
        FT::NONE, 256, 384, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::VESSEL, DT::N,
        1500, 650, 3, -20, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::PROC | PF::POWER, OF::SOVIET,
        3, 40, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Missile Silo (MSLO)
    {
        BT::MSLO, 0, "MSLO",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_21,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        1000, 2500, 3, -100, AT::CONCRETE,
        WT::NONE, WT::NONE,
        PF::TECH | PF::POWER, OF::ALL,
        10, 80, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Forward Command (FCOM)
    {
        BT::FORWARD_COM, 0, "FCOM",
        FT::SOUTH, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, T, F, F, T, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 1500, 10, -200, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::RADAR | PF::POWER, OF::SOVIET,
        5, 30, T, F, T  // tech, pts, capturable, crewed, bib
    },

    // Tesla Coil (TSLA)
    {
        BT::TESLA, 0, "TSLA",
        FT::NONE, 0, 0, RM::NORMAL, BS::BSIZE_12,
        0x0000, 0x0060, 0x0000,
        F, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        400, 1500, 6, -150, AT::CONCRETE,
        WT::TESLA_COIL, WT::NONE,
        PF::TECH | PF::POWER, OF::SOVIET,
        7, 40, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Fake Weapons Factory (WEAP fake)
    {
        BT::FAKEWEAP, 0, "WEAP",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_32,
        0x0000, 0x0000, 0x0000,
        T, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 25, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        5, 5, F, F, T  // tech, pts, capturable, crewed, bib
    },

    // Fake Construction Yard (FACT fake)
    {
        BT::FAKECONST, 0, "FACT",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        T, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 25, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        5, 5, F, F, T  // tech, pts, capturable, crewed, bib
    },

    // Fake Shipyard (SYRD fake)
    {
        BT::FAKE_YARD, 0, "SYRD",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        T, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 25, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALLIES,
        5, 5, F, F, T  // tech, pts, capturable, crewed, bib
    },

    // Fake Sub Pen (SPEN fake)
    {
        BT::FAKE_PEN, 0, "SPEN",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_33,
        0x0000, 0x0000, 0x0000,
        T, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 25, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::SOVIET,
        5, 5, F, F, T  // tech, pts, capturable, crewed, bib
    },

    // Fake Radar (DOME fake)
    {
        BT::FAKE_RADAR, 0, "DOME",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        T, F, F, F, F, F, T, T, F, F, F, T,
        RT::NONE, DT::N,
        500, 25, 3, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        5, 5, F, F, T  // tech, pts, capturable, crewed, bib
    },

    // Sandbag Wall (SBAG)
    {
        BT::SANDBAG_WALL, 0, "SBAG",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        25, 50, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        1, 1, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Chain Link Fence (CYCL)
    {
        BT::CYCLONE_WALL, 0, "CYCL",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        10, 75, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        2, 1, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Concrete Wall (BRIK)
    {
        BT::BRICK_WALL, 0, "BRIK",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        75, 100, 0, 0, AT::CONCRETE,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::SOVIET,
        2, 1, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Barbed Wire (BARB)
    {
        BT::BARBWIRE_WALL, 0, "BARB",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        10, 25, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        1, 1, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Wood Fence (WOOD)
    {
        BT::WOOD_WALL, 0, "WOOD",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, T, F, F,
        RT::NONE, DT::N,
        10, 25, 0, 0, AT::WOOD,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::ALLIES,
        -1, 1, F, F, F  // tech=-1 (scenario only), pts, cap, crew, bib
    },

    // Wire Fence (FENC)
    {
        BT::FENCE, 0, "FENC",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, T, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        10, 75, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::BARRACKS, OF::SOVIET,
        2, 1, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Anti-Vehicle Mine (MINV)
    {
        BT::AVMINE, 0, "MINV",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, T, T, F, F, T, F, F, T,
        RT::NONE, DT::N,
        5, 25, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        -1, 0, F, F, F  // tech=-1 (not buildable), pts, cap, crew, bib
    },

    // Anti-Personnel Mine (MINP)
    {
        BT::APMINE, 0, "MINP",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, F, F, T, T, F, F, T, F, F, T,
        RT::NONE, DT::N,
        5, 25, 0, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::NONE, OF::ALL,
        -1, 0, F, F, F  // tech=-1 (not buildable), pts, cap, crew, bib
    },

    // Civilian structures V01-V18 (compact format)
    // type, idx, name, facing, exit_x/y, remap, size, occ, flags..., prod, dir,
    // hp, cost, sight, power, armor, wpn1, wpn2, prereq, owner, tech, pts, cap,
    // crewed, bib
    #define CIV22 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22
    #define CIV21 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_21
    #define CIV11 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11
    #define CIVFL F,F,T,F,F,F,T,T,T,T,F,F,RT::NONE,DT::N
    #define CIVFX F,F,T,F,T,F,T,T,T,T,F,F,RT::NONE,DT::N
    #define CIVEND AT::WOOD, WT::NONE, WT::NONE, PF::NONE, 0, -1, 5, F, F, F
    {BT::V01, 0, "V01", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V02, 0, "V02", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V03, 0, "V03", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V04, 0, "V04", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V05, 0, "V05", CIV21, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V06, 0, "V06", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V07, 0, "V07", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V08, 0, "V08", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V09, 0, "V09", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V10, 0, "V10", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V11, 0, "V11", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V12, 0, "V12", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V13, 0, "V13", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V14, 0, "V14", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V15, 0, "V15", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V16, 0, "V16", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V17, 0, "V17", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    {BT::V18, 0, "V18", CIV11, 0, 0, 0, CIVFX, 100, 0, 1, 0, CIVEND},
    #undef CIV22
    #undef CIV21
    #undef CIV11
    #undef CIVFL
    #undef CIVFX
    #undef CIVEND

    // Water Pump (PUMP / V19)
    {
        BT::PUMP, 0, "V19",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11,
        0, 0, 0, F, T, T, F, T, F, T, T, T, T, F, F,
        RT::NONE, DT::N, 200, 0, 1, 0, AT::WOOD,
        WT::NONE, WT::NONE, PF::NONE, 0,
        -1, 5, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Civilian structures V20-V37 (compact format)
    #define CIV22 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_22
    #define CIV21 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_21
    #define CIV11 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11
    #define CIV32 FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_32
    #define CIVFL F,F,T,F,F,F,T,T,T,T,F,F,RT::NONE,DT::N
    #define CIVEND AT::WOOD, WT::NONE, WT::NONE, PF::NONE, 0, -1, 5, F, F, F
    {BT::V20, 0, "V20", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V21, 0, "V21", CIV21, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V22, 0, "V22", CIV21, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V23, 0, "V23", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V24, 0, "V24", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V25, 0, "V25", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V26, 0, "V26", CIV21, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V27, 0, "V27", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V28, 0, "V28", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V29, 0, "V29", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V30, 0, "V30", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V31, 0, "V31", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V32, 0, "V32", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V33, 0, "V33", CIV22, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V34, 0, "V34", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V35, 0, "V35", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V36, 0, "V36", CIV11, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    {BT::V37, 0, "V37", CIV32, 0, 0, 0, CIVFL, 200, 0, 1, 0, CIVEND},
    #undef CIV22
    #undef CIV21
    #undef CIV11
    #undef CIV32
    #undef CIVFL
    #undef CIVEND

    // Explosive Barrel (BARL)
    {
        BT::BARREL, 0, "BARL",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, T, F, T, T, T, T, T, F, F, F,
        RT::NONE, DT::N,
        20, 0, 1, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 0, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // 3-Barrel Group (BRL3)
    {
        BT::BARREL3, 0, "BRL3",
        FT::NONE, 0, 0, RM::ALTERNATE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, F, T, F, T, T, F, T, T, F, F, F,
        RT::NONE, DT::N,
        20, 0, 1, 0, AT::NONE,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 0, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Ant Queen (QUEE)
    {
        BT::QUEEN, 0, "QUEE",
        FT::NONE, 256, 384, RM::NONE, BS::BSIZE_22,
        0x0000, 0x0000, 0x0000,
        F, F, T, F, F, F, T, T, F, F, F, F,
        RT::NONE, DT::N,
        800, 0, 4, 0, AT::LIGHT,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 50, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Larva 1 (LAR1)
    {
        BT::LARVA1, 0, "LAR1",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, T, T, F, F, F, T, T, T, F, F, F,
        RT::NONE, DT::N,
        150, 0, 1, 0, AT::LIGHT,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 10, F, F, F  // tech, pts, capturable, crewed, bib
    },

    // Larva 2 (LAR2)
    {
        BT::LARVA2, 0, "LAR2",
        FT::NONE, 0, 0, RM::NONE, BS::BSIZE_11,
        0x0000, 0x0000, 0x0000,
        F, T, T, F, F, F, T, T, T, F, F, F,
        RT::NONE, DT::N,
        150, 0, 1, 0, AT::LIGHT,
        WT::NONE, WT::NONE,
        PF::NONE, 0,
        -1, 10, F, F, F  // tech, pts, capturable, crewed, bib
    },
};

const int BuildingTypeCount =
    sizeof(BuildingTypeDefaults) / sizeof(BuildingTypeDefaults[0]);

//===========================================================================
// Mutable Building Type Data (runtime copy)
//===========================================================================
static BuildingTypeData g_buildingTypes[128];  // Max building types
static bool g_buildingTypesInitialized = false;

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

void InitBuildingTypes() {
    if (g_buildingTypesInitialized) return;

    // Copy const defaults to mutable storage
    for (int i = 0; i < BuildingTypeCount && i < 128; i++) {
        g_buildingTypes[i] = BuildingTypeDefaults[i];
    }
    g_buildingTypesInitialized = true;
}

BuildingTypeData* GetBuildingType(BuildingType type) {
    // Auto-init if not done
    if (!g_buildingTypesInitialized) {
        InitBuildingTypes();
    }

    for (int i = 0; i < BuildingTypeCount; i++) {
        if (g_buildingTypes[i].type == type) {
            return &g_buildingTypes[i];
        }
    }
    return nullptr;
}

const BuildingTypeData* GetBuildingTypeConst(BuildingType type) {
    return GetBuildingType(type);
}

BuildingType BuildingTypeFromName(const char* name) {
    if (name == nullptr) return BT::NONE;

    // Auto-init if not done
    if (!g_buildingTypesInitialized) {
        InitBuildingTypes();
    }

    for (int i = 0; i < BuildingTypeCount; i++) {
        if (strcasecmp(g_buildingTypes[i].iniName, name) == 0) {
            return g_buildingTypes[i].type;
        }
    }
    return BT::NONE;
}

void GetBuildingSize(BSizeType size, int& width, int& height) {
    int idx = static_cast<int>(size);
    if (idx >= 0 && idx < static_cast<int>(BS::COUNT)) {
        width = BuildingSizes[idx].width;
        height = BuildingSizes[idx].height;
    } else {
        width = 1;
        height = 1;
    }
}

bool IsBuildingWall(BuildingType type) {
    switch (type) {
        case BT::SANDBAG_WALL:
        case BT::CYCLONE_WALL:
        case BT::BRICK_WALL:
        case BT::BARBWIRE_WALL:
        case BT::WOOD_WALL:
        case BT::FENCE:
            return true;
        default:
            return false;
    }
}

bool IsBuildingCivilian(BuildingType type) {
    int idx = static_cast<int>(type);
    int v01 = static_cast<int>(BT::V01);
    int v37 = static_cast<int>(BT::V37);

    return (idx >= v01 && idx <= v37) ||
           type == BT::PUMP ||
           type == BT::BARREL ||
           type == BT::BARREL3;
}

bool IsBuildingFactory(BuildingType type) {
    const BuildingTypeData* data = GetBuildingType(type);
    return data != nullptr && data->factoryType != RT::NONE;
}
