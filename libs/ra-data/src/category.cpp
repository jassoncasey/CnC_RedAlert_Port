/**
 * Red Alert Asset Categorization - Implementation
 *
 * Asset lists derived from:
 * - CnC_Remastered_Collection/CnCTDRAMapEditor/RedAlert/[Type]Types.cs
 * - OpenRA/mods/ra/rules/[type].yaml
 */

#include <ra/category.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <string>

namespace ra {

namespace {

// Convert string to uppercase
std::string to_upper(std::string_view sv) {
    std::string result(sv);
    for (char& c : result) c = std::toupper(static_cast<unsigned char>(c));
    return result;
}

// Get filename stem (without extension)
std::string stem(std::string_view filename) {
    auto dot = filename.rfind('.');
    if (dot == std::string_view::npos) return std::string(filename);
    return std::string(filename.substr(0, dot));
}

// Get extension (with dot, uppercase)
std::string extension(std::string_view filename) {
    auto dot = filename.rfind('.');
    if (dot == std::string_view::npos) return "";
    return to_upper(filename.substr(dot));
}

// Check if string starts with prefix
bool starts_with(const std::string& s, const char* prefix) {
    return s.rfind(prefix, 0) == 0;
}

// Check if string ends with suffix
bool ends_with(const std::string& s, const char* suffix) {
    size_t len = strlen(suffix);
    if (s.size() < len) return false;
    return s.compare(s.size() - len, len, suffix) == 0;
}

// Check if string matches any in list
template<size_t N>
bool is_one_of(const std::string& s, const std::array<const char*, N>& list) {
    for (const char* item : list) {
        if (s == item) return true;
    }
    return false;
}

// ============================================================================
// Asset Lists (from CnC_Remastered_Collection and OpenRA)
// ============================================================================

// Infantry: InfantryTypes.cs lines 24-49
constexpr std::array INFANTRY = {
    // Combat infantry
    "E1", "E2", "E3", "E4", "E5", "E6", "E7",
    // Special units
    "SPY", "THF", "MEDI", "MECH", "SHOK", "DOG",
    // Civilians
    "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "C10",
    // Named characters
    "EINSTEIN", "DELPHI", "CHAN", "GNRL",
    // Ants (campaign)
    "ANT", "FIREANT", "SCOUTANT", "WARRIORANT",
    // Zombie (unused?)
    "ZOMBIE"
};

// Vehicles: UnitTypes.cs lines 25-46
constexpr std::array VEHICLES = {
    // Tanks
    "1TNK", "2TNK", "3TNK", "4TNK",
    "FTNK", "STNK", "MTNK", "HTNK", "LTNK",
    "CTNK", "TTNK", "QTNK",
    // Support vehicles
    "APC", "MCV", "HARV", "ARTY", "V2RL", "MRLS",
    "JEEP", "TRUK", "DTRK", "MGG", "MRJ", "MNLY",
    // Ant vehicles (campaign)
    "ANT1", "ANT2", "ANT3"
};

// Aircraft: UnitTypes.cs lines 48-54
constexpr std::array AIRCRAFT = {
    "HELI", "HIND", "TRAN", "BADR", "MIG", "YAK", "U2"
};

// Vessels: UnitTypes.cs lines 56-62
constexpr std::array VESSELS = {
    "SS", "DD", "CA", "LST", "PT", "CARR", "SUB", "MSUB"
};

// Buildings: BuildingTypes.cs lines 25-87
constexpr std::array BUILDINGS = {
    // Power
    "POWR", "APWR", "NPWR", "NUK2",
    // Production
    "FACT", "WEAP", "WEAF", "BARR", "TENT",
    "AFLD", "SYRD", "SPEN", "HPAD",
    // Resource
    "PROC", "SILO",
    // Tech
    "ATEK", "STEK", "DOME", "FIX", "HOSP", "BIO",
    // Special
    "IRON", "PDOX", "GAP", "KENN", "FCOM", "MISS", "MSLO",
    // Civilian (V01-V19)
    "V01", "V02", "V03", "V04", "V05", "V06", "V07", "V08", "V09",
    "V10", "V11", "V12", "V13", "V14", "V15", "V16", "V17", "V18", "V19",
    // Special structures
    "QUEE", "LAR1", "LAR2",
    "BARREL", "BRL3"
};

// Defense: walls, turrets, mines
constexpr std::array DEFENSE = {
    // Turrets
    "GUN", "AGUN", "FTUR", "SAM", "TSLA", "PBOX", "HBOX",
    // Walls/fences
    "SBAG", "CYCL", "BRIK", "BARB", "WOOD", "FENC",
    // Mines
    "MINV", "MINP"
};

// Smudges: SmudgeTypes.cs lines 25-39
constexpr std::array SMUDGES = {
    // Craters
    "CR1", "CR2", "CR3", "CR4", "CR5", "CR6",
    // Scorch marks
    "SC1", "SC2", "SC3", "SC4", "SC5", "SC6",
    // Building bibs
    "BIB1", "BIB2", "BIB3"
};

// Overlays: OverlayTypes.cs lines 24-48
constexpr std::array OVERLAYS = {
    // Resources
    "GOLD01", "GOLD02", "GOLD03", "GOLD04",
    "GEM01", "GEM02", "GEM03", "GEM04",
    // Crates
    "WCRATE", "SCRATE", "WWCRATE",
    // Misc
    "FPLS"
};

// Animations/Effects
constexpr std::array ANIMATIONS = {
    // Fire
    "FIRE1", "FIRE2", "FIRE3", "FIRE4", "BURN",
    // Explosions
    "FBALL", "FB1", "FB2", "NAPALM", "BOMBLET",
    "ART-EXP", "VEH-HIT", "PIFF", "PIFFPIFF",
    // Smoke
    "SMOKE", "SMOK",
    // Special effects
    "ATOMSFX", "CHRONSFX", "IONSFX",
    // Misc animations
    "FLAG", "CRATE", "ELECT", "SPUTDOOR",
    "SELECT", "MOVEFLSH", "YOURWIN", "YOURLOSE"
};

// Projectiles
constexpr std::array PROJECTILES = {
    "BOMB", "MISSILE", "DRAGON", "BULLET", "LASER",
    "MLRS", "PSCRL", "120MM", "50CAL"
};

// Cursors
constexpr std::array CURSORS = {
    "MOUSE", "CURSOR"
};

// Music tracks (from SCORES.MIX)
constexpr std::array MUSIC = {
    // Full track names
    "BIGF226M", "CRUS226M", "FAC1226M", "FAC2226M",
    "HELL226M", "RUN1226M", "SMSH226M", "TREN226M", "WORK226M",
    // Short names
    "AWAIT", "DENSE", "MAP", "FOGGER", "MUD",
    "RADIO2", "ROLL", "SNAKE", "TERMINAT",
    "TWIN", "VECTOR", "VOLKOV", "2ND_HAND",
    // Additional
    "BIGF", "CRUS", "FAC1", "FAC2", "HELL", "RUN1",
    "SMSH", "TREN", "WORK", "SCORE"
};

// Voice/EVA (from SPEECH.MIX and units)
constexpr std::array VOICE = {
    // EVA announcements
    "SPEECH",
    "BLDGING", "CANCLD", "READY", "ONHOLD", "PRIMRY",
    "SOVBUILD", "ABLDGIN1", "REINFOR", "CONSCMP",
    "NUKE", "IRON1", "CHROCHR1", "CHROYES1",
    // Mission announcements
    "MISNWON1", "MISNLST1", "ACKNO", "AFFIRM",
    "AWAIT1", "READY1", "REPORT1", "YESSION"
};

// Terrain patterns
constexpr std::array TERRAIN_PREFIXES = {
    // Shores, cliffs, slopes
    "SH", "WC", "RC",
    // Roads, rivers, fords
    "RV", "RF", "F0",
    // Bridges
    "BRIDGE", "BR",
    // Clear/water
    "CLEAR", "WATER", "W1", "W2",
    // Interior elements
    "ARRO", "FLOR", "GFLR", "GSTR", "LWAL", "STRP", "WALL",
    // Misc terrain
    "D0", "P0", "FALLS", "FORD"
};

// ============================================================================
// Categorization Functions
// ============================================================================

AssetCategory categorize_audio(const std::string& base) {
    if (is_one_of(base, MUSIC)) return AssetCategory::Music;
    if (is_one_of(base, VOICE)) return AssetCategory::Voice;

    // Heuristics for common patterns
    if (starts_with(base, "SPEECH")) return AssetCategory::Voice;
    if (ends_with(base, "226M")) return AssetCategory::Music;

    return AssetCategory::SoundEffect;
}

bool is_terrain(const std::string& base) {
    for (const char* prefix : TERRAIN_PREFIXES) {
        if (starts_with(base, prefix)) return true;
    }
    // Trees (T01-T17, TC01-TC05)
    if ((starts_with(base, "T") || starts_with(base, "TC")) &&
        base.size() >= 2 && std::isdigit(base[1])) {
        return true;
    }
    return false;
}

} // anonymous namespace

// ============================================================================
// Public API
// ============================================================================

AssetCategory categorize(std::string_view filename, Theater theater) {
    std::string base = to_upper(stem(filename));
    std::string ext = extension(filename);

    // === By extension first ===
    if (ext == ".VQA" || ext == ".VQP") return AssetCategory::Cutscene;
    if (ext == ".AUD") return categorize_audio(base);
    if (ext == ".PAL") return AssetCategory::Interface;
    if (ext == ".INI") return AssetCategory::Rules;
    if (ext == ".FNT") return AssetCategory::Interface;
    if (ext == ".TMP") return AssetCategory::Terrain;
    if (ext == ".MIX") return AssetCategory::Unknown;  // Archives, not assets

    // === Cameo icons (suffix match) ===
    if (ends_with(base, "ICON")) return AssetCategory::Cameo;
    if (ends_with(base, "ICNH")) return AssetCategory::Cameo;

    // === Unit types ===
    if (is_one_of(base, INFANTRY)) return AssetCategory::Infantry;
    if (is_one_of(base, VEHICLES)) return AssetCategory::Vehicle;
    if (is_one_of(base, AIRCRAFT)) return AssetCategory::Aircraft;
    if (is_one_of(base, VESSELS)) return AssetCategory::Vessel;

    // === Structure types ===
    if (is_one_of(base, BUILDINGS)) return AssetCategory::Building;
    if (is_one_of(base, DEFENSE)) return AssetCategory::Defense;

    // === Terrain/decorations ===
    if (is_one_of(base, SMUDGES)) return AssetCategory::Smudge;
    if (is_one_of(base, OVERLAYS)) return AssetCategory::Overlay;
    if (is_terrain(base)) return AssetCategory::Terrain;

    // === Effects ===
    if (is_one_of(base, ANIMATIONS)) return AssetCategory::Animation;
    if (is_one_of(base, PROJECTILES)) return AssetCategory::Projectile;

    // === UI ===
    if (is_one_of(base, CURSORS)) return AssetCategory::Cursor;
    if (starts_with(base, "MOUSE")) return AssetCategory::Cursor;

    // === Interface elements ===
    if (starts_with(base, "TITLE") || starts_with(base, "DIALOG") ||
        starts_with(base, "CHOOSE") || starts_with(base, "HISCORE") ||
        starts_with(base, "MSLOGO") || starts_with(base, "TAB")) {
        return AssetCategory::Interface;
    }

    // === Rules/data files ===
    if (base == "RULES" || base == "AFTRMATH" || base == "ART" ||
        starts_with(base, "SCG") || starts_with(base, "SCU")) {
        return AssetCategory::Rules;
    }

    // Theater context for ambiguous terrain
    (void)theater;  // TODO: use for theater-specific terrain detection

    return AssetCategory::Unknown;
}

const char* category_name(AssetCategory cat) {
    switch (cat) {
        case AssetCategory::Infantry:    return "Infantry";
        case AssetCategory::Vehicle:     return "Vehicle";
        case AssetCategory::Aircraft:    return "Aircraft";
        case AssetCategory::Vessel:      return "Vessel";
        case AssetCategory::Building:    return "Building";
        case AssetCategory::Defense:     return "Defense";
        case AssetCategory::Terrain:     return "Terrain";
        case AssetCategory::Overlay:     return "Overlay";
        case AssetCategory::Smudge:      return "Smudge";
        case AssetCategory::Animation:   return "Animation";
        case AssetCategory::Projectile:  return "Projectile";
        case AssetCategory::Cameo:       return "Cameo";
        case AssetCategory::Cursor:      return "Cursor";
        case AssetCategory::Interface:   return "Interface";
        case AssetCategory::Music:       return "Music";
        case AssetCategory::SoundEffect: return "Sound Effect";
        case AssetCategory::Voice:       return "Voice";
        case AssetCategory::Cutscene:    return "Cutscene";
        case AssetCategory::Rules:       return "Rules";
        case AssetCategory::Unknown:     return "Unknown";
    }
    return "Unknown";
}

const char* theater_name(Theater theater) {
    switch (theater) {
        case Theater::Temperate: return "Temperate";
        case Theater::Snow:      return "Snow";
        case Theater::Interior:  return "Interior";
        case Theater::Unknown:   return "Unknown";
    }
    return "Unknown";
}

Theater detect_theater(std::string_view mix_name) {
    std::string upper = to_upper(mix_name);

    if (upper.find("TEMPERAT") != std::string::npos) return Theater::Temperate;
    if (upper.find("SNOW") != std::string::npos) return Theater::Snow;
    if (upper.find("INTERIOR") != std::string::npos) return Theater::Interior;

    return Theater::Unknown;
}

} // namespace ra
