/**
 * Red Alert macOS Port - Aircraft Type Data Tables
 *
 * Static data tables for aircraft types.
 * Ported from original ADATA.CPP
 */

#include "aircraft_types.h"
#include <cstring>

//===========================================================================
// Aircraft Type Table - Static data for all aircraft types
//===========================================================================

const AircraftTypeData AircraftTypes[] = {
    // TRANSPORT - Chinook Helicopter
    {
        AircraftType::TRANSPORT, 0, "TRAN",
        AnimType::FBALL1, RemapType::ALTERNATE, 32,
        LandingType::HELIPAD, false, true, true, true, true,
        0, 0,
        90, 1500, 40, 4, ArmorType::LIGHT,
        WeaponType::NONE, WeaponType::NONE,
        5, 0
    },
    // BADGER - Soviet Bomber
    {
        AircraftType::BADGER, 0, "BADR",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::NONE, true, false, false, false, false,
        0, 0,
        70, 0, 80, 0, ArmorType::LIGHT,
        WeaponType::NONE, WeaponType::NONE,  // Drops bombs, not direct weapon
        0, 4
    },
    // U2 - Allied Spy Plane
    {
        AircraftType::U2, 0, "U2",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::NONE, true, false, false, false, false,
        0, 0,
        50, 0, 100, 8, ArmorType::LIGHT,
        WeaponType::CAMERA, WeaponType::NONE,
        0, 0
    },
    // MIG - Soviet Fighter
    {
        AircraftType::MIG, 0, "MIG",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::AIRSTRIP, true, true, false, false, true,
        0, 0,
        50, 1200, 100, 3, ArmorType::LIGHT,
        WeaponType::MAVERICK, WeaponType::NONE,
        0, 2
    },
    // YAK - Soviet Attack Plane
    {
        AircraftType::YAK, 0, "YAK",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::AIRSTRIP, true, true, false, false, true,
        0, 0,
        50, 800, 80, 3, ArmorType::LIGHT,
        WeaponType::VULCAN, WeaponType::NONE,
        0, 6
    },
    // HELI - Longbow Apache
    {
        AircraftType::HELI, 0, "HELI",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::HELIPAD, false, true, true, true, true,
        0, 0,
        100, 1200, 80, 4, ArmorType::LIGHT,
        WeaponType::HELLFIRE, WeaponType::VULCAN,
        0, 8
    },
    // HIND - Soviet Gunship
    {
        AircraftType::HIND, 0, "HIND",
        AnimType::FBALL1, RemapType::NORMAL, 32,
        LandingType::HELIPAD, false, true, true, true, true,
        0, 0,
        125, 1200, 60, 4, ArmorType::LIGHT,
        WeaponType::CHAINGUN, WeaponType::NONE,
        5, 0  // Can also carry passengers
    },
};

const int AircraftTypeCount = sizeof(AircraftTypes) / sizeof(AircraftTypes[0]);

//===========================================================================
// Helper Functions
//===========================================================================

const AircraftTypeData* GetAircraftType(AircraftType type) {
    int index = static_cast<int>(type);
    if (index >= 0 && index < AircraftTypeCount) {
        for (int i = 0; i < AircraftTypeCount; i++) {
            if (AircraftTypes[i].type == type) {
                return &AircraftTypes[i];
            }
        }
    }
    return nullptr;
}

AircraftType AircraftTypeFromName(const char* name) {
    if (name == nullptr) return AircraftType::NONE;

    for (int i = 0; i < AircraftTypeCount; i++) {
        if (strcasecmp(AircraftTypes[i].iniName, name) == 0) {
            return AircraftTypes[i].type;
        }
    }
    return AircraftType::NONE;
}
