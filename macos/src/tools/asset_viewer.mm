/**
 * Red Alert Asset Viewer
 *
 * Two-tab interface:
 * Tab 1: File browser - select root directory, view discovered assets
 * Tab 2: Asset review - browse and approve/reject individual assets
 *
 * Features:
 * - Native macOS folder picker
 * - Recursive directory scanning
 * - Recursive MIX file scanning (MIX within MIX)
 * - Filename database for CRC->name mapping
 * - Tree view of discovered assets
 * - Sprite/animation preview with Metal renderer
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "assets/mixfile.h"
#include "assets/shpfile.h"
#include "assets/audfile.h"
#include "audio/audio.h"
#include "graphics/metal/renderer.h"
#include "video/vqa.h"

// Forward declarations
@class AssetViewerDelegate;
@class FileTreeNode;

// Asset type enumeration for viewer (prefixed to avoid conflicts)
enum class ViewerAssetType {
    Unknown,
    MixArchive,
    ShpSprite,
    PalPalette,
    AudAudio,
    VqaVideo,
    TmpTemplate,
    IniConfig,
    Directory,
    IsoImage
};

// Get asset type from filename extension
static ViewerAssetType getViewerAssetType(const std::string& name) {
    if (name.length() < 4) return ViewerAssetType::Unknown;

    std::string ext = name.substr(name.length() - 4);
    for (char& c : ext) c = toupper(c);

    if (ext == ".MIX") return ViewerAssetType::MixArchive;
    if (ext == ".SHP") return ViewerAssetType::ShpSprite;
    if (ext == ".PAL") return ViewerAssetType::PalPalette;
    if (ext == ".AUD") return ViewerAssetType::AudAudio;
    if (ext == ".VQA") return ViewerAssetType::VqaVideo;
    if (ext == ".TMP") return ViewerAssetType::TmpTemplate;
    if (ext == ".INI") return ViewerAssetType::IniConfig;
    if (ext == ".ISO") return ViewerAssetType::IsoImage;
    return ViewerAssetType::Unknown;
}

// Get type name string
static const char* getTypeName(ViewerAssetType type) {
    switch (type) {
        case ViewerAssetType::MixArchive: return "MIX Archive";
        case ViewerAssetType::ShpSprite: return "SHP Sprite";
        case ViewerAssetType::PalPalette: return "PAL Palette";
        case ViewerAssetType::AudAudio: return "AUD Audio";
        case ViewerAssetType::VqaVideo: return "VQA Video";
        case ViewerAssetType::TmpTemplate: return "TMP Template";
        case ViewerAssetType::IniConfig: return "INI Config";
        case ViewerAssetType::Directory: return "Directory";
        case ViewerAssetType::IsoImage: return "ISO Image";
        default: return "Unknown";
    }
}

// File tree node for outline view
@interface FileTreeNode : NSObject
@property (strong) NSString* name;
@property (strong) NSString* fullPath;
@property (assign) ViewerAssetType assetType;
@property (assign) uint32_t size;
@property (assign) uint32_t crc;
@property (assign) BOOL isContainer;
@property (strong) NSMutableArray<FileTreeNode*>* children;
@property (weak) FileTreeNode* parent;
@property (assign) int depth;
// For assets inside MIX files - store the MIX chain to extract
@property (strong) NSMutableArray<NSString*>* mixChain;
@property (strong) NSMutableArray<NSNumber*>* crcChain;
@end

@implementation FileTreeNode
- (instancetype)init {
    self = [super init];
    if (self) {
        _children = [NSMutableArray array];
        _mixChain = [NSMutableArray array];
        _crcChain = [NSMutableArray array];
        _isContainer = NO;
        _depth = 0;
    }
    return self;
}
@end

// Filename database for CRC lookup
class FilenameDB {
public:
    void loadFromFile(const char* path) {
        FILE* f = fopen(path, "r");
        if (!f) {
            NSLog(@"Could not open filename database: %s", path);
            return;
        }

        char line[256];
        while (fgets(line, sizeof(line), f)) {
            // Skip comments and empty lines
            if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
                continue;

            // Trim trailing whitespace
            int len = (int)strlen(line);
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'
                              || line[len-1] == ' '))
                line[--len] = 0;

            if (len == 0) continue;

            // Calculate CRC and store mapping
            uint32_t crc = Mix_CalculateCRC(line);
            crcToName[crc] = std::string(line);
        }

        fclose(f);
        NSLog(@"Loaded %zu filenames into database", crcToName.size());
    }

    const char* lookup(uint32_t crc) const {
        auto it = crcToName.find(crc);
        if (it != crcToName.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }

    bool hasName(uint32_t crc) const {
        return crcToName.find(crc) != crcToName.end();
    }

private:
    std::map<uint32_t, std::string> crcToName;
};

// Global state
static FilenameDB g_filenameDB;
static FileTreeNode* g_rootNode = nil;
static std::vector<FileTreeNode*> g_allAssets;  // Flat list for review tab
static int g_currentAssetIndex = 0;
static ShpFileHandle g_currentSHP = nullptr;
static Palette g_palette;
static bool g_paletteLoaded = false;
static int g_currentFrame = 0;
static bool g_animating = true;
static NSTimer* g_animTimer = nil;

// Review status
enum ReviewStatus { PENDING = 0, GOOD = 1, BAD = 2 };
static std::map<std::string, ReviewStatus> g_reviewStatus;

// Preview background mode
enum PreviewBackground { BG_BLACK = 0, BG_GRAY = 1, BG_CHECKER = 2 };
static PreviewBackground g_previewBg = BG_CHECKER;

// Audio playback state
static AudData* g_currentAUD = nullptr;
static SoundHandle g_playingSound = 0;
static bool g_audioInitialized = false;
static NSButton* g_playButton = nil;
static NSButton* g_stopButton = nil;
static NSTextField* g_audioInfoLabel = nil;
static NSButton* g_bgToggleButton = nil;

// Zoom level for sprite preview (1x, 2x, 4x)
static int g_zoomLevel = 1;
static NSTextField* g_zoomLabel = nil;

// VQA video playback state
static VQAPlayer* g_currentVQA = nullptr;
static bool g_vqaPlaying = false;
static NSTextField* g_videoInfoLabel = nil;
static NSTimer* g_vqaTimer = nil;

// VQA audio buffer (must persist during playback)
static int16_t* g_vqaAudioBuffer = nullptr;
static int g_vqaAudioBufferSize = 0;
static AudioSample g_vqaAudioSample;

// AUD audio sample (must persist during playback - Audio_Play stores pointer)
static AudioSample g_audAudioSample;

// ISO mounting - track mounted volumes to unmount on exit
static std::vector<std::string> g_mountedVolumes;

// Mount an ISO file and return the mount path, or empty string on failure
static NSString* mountISO(NSString* isoPath) {
    NSTask* task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/hdiutil";
    task.arguments = @[@"attach", @"-nobrowse", @"-noverify", @"-noautoopen",
                       isoPath];

    NSPipe* pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    task.standardError = [NSPipe pipe];

    NSError* error = nil;
    [task launchAndReturnError:&error];
    if (error) {
        NSLog(@"Failed to launch hdiutil: %@", error);
        return nil;
    }

    [task waitUntilExit];

    if (task.terminationStatus != 0) {
        NSLog(@"hdiutil attach failed for: %@", isoPath);
        return nil;
    }

    // Parse output to find mount point (last column of last line with /Volumes/)
    NSData* data = [pipe.fileHandleForReading readDataToEndOfFile];
    NSString* output = [[NSString alloc] initWithData:data
                                             encoding:NSUTF8StringEncoding];

    // Find line containing /Volumes/
    NSArray* lines = [output componentsSeparatedByString:@"\n"];
    for (NSString* line in lines) {
        NSRange range = [line rangeOfString:@"/Volumes/"];
        if (range.location != NSNotFound) {
            NSString* mountPath = [line substringFromIndex:range.location];
            // Trim whitespace
            mountPath = [mountPath stringByTrimmingCharactersInSet:
                        [NSCharacterSet whitespaceAndNewlineCharacterSet]];

            g_mountedVolumes.push_back([mountPath UTF8String]);
            NSLog(@"Mounted ISO: %@ -> %@", isoPath, mountPath);
            return mountPath;
        }
    }

    NSLog(@"Could not find mount point in hdiutil output");
    return nil;
}

// Unmount all mounted ISOs
static void unmountAllISOs() {
    for (const auto& volumePath : g_mountedVolumes) {
        NSTask* task = [[NSTask alloc] init];
        task.launchPath = @"/usr/bin/hdiutil";
        task.arguments = @[@"detach",
                          [NSString stringWithUTF8String:volumePath.c_str()]];
        task.standardOutput = [NSPipe pipe];
        task.standardError = [NSPipe pipe];

        NSError* error = nil;
        [task launchAndReturnError:&error];
        if (!error) {
            [task waitUntilExit];
            NSLog(@"Unmounted: %s", volumePath.c_str());
        }
    }
    g_mountedVolumes.clear();
}

// =============================================================================
// CAPABILITY CHECKLIST - Exhaustive list of what our code must handle
// =============================================================================

enum CapabilityStatus { CAP_UNTESTED = 0, CAP_WORKING = 1, CAP_BROKEN = 2 };

struct Capability {
    const char* id;           // Unique ID for persistence
    const char* category;     // Group (MIX, SHP, PAL, etc.)
    const char* name;         // Display name
    const char* description;  // What this capability does
    const char* testHint;     // How to test / what files to use
    CapabilityStatus status;  // Current status
    const char* notes;        // User notes
};

// The exhaustive capability checklist
static std::vector<Capability> g_capabilities = {
    // =========================================================================
    // MIX ARCHIVE LOADING
    // =========================================================================
    {"mix.open_unencrypted", "MIX Archives", "Open unencrypted MIX",
     "Load MIX files without encryption (old format)",
     "Test with: LOCAL.MIX, SPEECH.MIX", CAP_UNTESTED, ""},
    {"mix.open_encrypted", "MIX Archives", "Open encrypted MIX (Blowfish)",
     "Load MIX files with Blowfish encryption (RA format)",
     "Test with: REDALERT.MIX, MAIN.MIX", CAP_UNTESTED, ""},
    {"mix.nested", "MIX Archives", "Nested MIX files",
     "Load MIX files contained within other MIX files",
     "Test with: REDALERT.MIX -> CONQUER.MIX", CAP_UNTESTED, ""},
    {"mix.crc_lookup", "MIX Archives", "CRC-based file lookup",
     "Find files by CRC hash (Westwood's filename hashing)",
     "Look up known filenames by CRC", CAP_UNTESTED, ""},
    {"mix.enumerate", "MIX Archives", "Enumerate all entries",
     "List all files in a MIX by index",
     "Count and list entries in MAIN.MIX", CAP_UNTESTED, ""},

    // =========================================================================
    // SHP SPRITE LOADING & RENDERING
    // =========================================================================
    {"shp.load_format80", "SHP Sprites", "Load Format80 (LCW) compressed",
     "Decompress LCW/Format80 encoded SHP frames",
     "Most unit sprites use this", CAP_UNTESTED, ""},
    {"shp.load_format40", "SHP Sprites", "Load Format40 (XOR delta)",
     "Decompress XOR-delta encoded SHP frames",
     "Animation frames often use delta encoding", CAP_UNTESTED, ""},
    {"shp.load_format20", "SHP Sprites", "Load Format20 (XOR + LCW)",
     "Decompress combined XOR+LCW frames",
     "Some complex animations", CAP_UNTESTED, ""},
    {"shp.load_uncompressed", "SHP Sprites", "Load uncompressed frames",
     "Handle raw uncompressed SHP frames",
     "Some simple sprites", CAP_UNTESTED, ""},
    {"shp.render_static", "SHP Sprites", "Render static sprite",
     "Display a single SHP frame at position",
     "Any unit standing still", CAP_UNTESTED, ""},
    {"shp.render_animated", "SHP Sprites", "Render animated sprite",
     "Cycle through SHP frames for animation",
     "Walking infantry, rotating turret", CAP_UNTESTED, ""},
    {"shp.transparency", "SHP Sprites", "Transparency (index 0)",
     "Treat palette index 0 as transparent",
     "All sprites should have transparent background", CAP_UNTESTED, ""},
    {"shp.shadow_frames", "SHP Sprites", "Shadow frame rendering",
     "Render shadow frames with proper blending",
     "Aircraft shadows, building shadows", CAP_UNTESTED, ""},
    {"shp.team_color_remap", "SHP Sprites", "Team color remapping",
     "Apply house color remap tables to sprites",
     "Units should show correct team colors", CAP_UNTESTED, ""},
    {"shp.facing_directions", "SHP Sprites", "8/16/32 facing directions",
     "Select correct frame for unit facing",
     "Vehicle rotation, infantry directions", CAP_UNTESTED, ""},
    {"shp.turret_offset", "SHP Sprites", "Turret frame offsets",
     "Render turret as separate layer with offset",
     "Tanks with rotating turrets", CAP_UNTESTED, ""},
    {"shp.damage_states", "SHP Sprites", "Building damage states",
     "Show correct frames for damaged/destroyed buildings",
     "Building health affects appearance", CAP_UNTESTED, ""},

    // =========================================================================
    // TMP TERRAIN TILES
    // =========================================================================
    {"tmp.load_temperate", "TMP Terrain", "Load temperate tiles",
     "Load .TMP files from TEMPERAT theater",
     "TEMPERAT.MIX templates", CAP_UNTESTED, ""},
    {"tmp.load_snow", "TMP Terrain", "Load snow tiles",
     "Load .TMP files from SNOW theater",
     "SNOW.MIX templates", CAP_UNTESTED, ""},
    {"tmp.load_interior", "TMP Terrain", "Load interior tiles",
     "Load .TMP files from INTERIOR theater",
     "INTERIOR.MIX templates", CAP_UNTESTED, ""},
    {"tmp.render_tile", "TMP Terrain", "Render single tile",
     "Display one tile from a template",
     "Basic terrain rendering", CAP_UNTESTED, ""},
    {"tmp.multi_tile_templates", "TMP Terrain", "Multi-tile templates",
     "Handle templates with multiple tiles (e.g., 3x3 cliff)",
     "Cliffs, large terrain features", CAP_UNTESTED, ""},
    {"tmp.shore_transitions", "TMP Terrain", "Shore/water transitions",
     "Proper water-to-land edge tiles",
     "Coastlines, rivers", CAP_UNTESTED, ""},
    {"tmp.cliff_tiles", "TMP Terrain", "Cliff and elevation tiles",
     "Render cliff faces and height transitions",
     "Map elevation changes", CAP_UNTESTED, ""},
    {"tmp.road_tiles", "TMP Terrain", "Road and bridge tiles",
     "Render roads, bridges, rail tracks",
     "Infrastructure terrain", CAP_UNTESTED, ""},

    // =========================================================================
    // PAL PALETTE LOADING
    // =========================================================================
    {"pal.load_6bit", "PAL Palettes", "Load 6-bit palettes",
     "Scale 6-bit (0-63) values to 8-bit (0-255)",
     "All Red Alert palettes are 6-bit", CAP_UNTESTED, ""},
    {"pal.temperate", "PAL Palettes", "TEMPERAT.PAL",
     "Load temperate theater palette",
     "Main game palette", CAP_UNTESTED, ""},
    {"pal.snow", "PAL Palettes", "SNOW.PAL",
     "Load snow theater palette",
     "Winter missions", CAP_UNTESTED, ""},
    {"pal.interior", "PAL Palettes", "INTERIOR.PAL",
     "Load interior theater palette",
     "Indoor missions", CAP_UNTESTED, ""},
    {"pal.unittem", "PAL Palettes", "Unit palettes (UNITTEM, etc.)",
     "Load unit-specific palettes",
     "Some units have special palettes", CAP_UNTESTED, ""},
    {"pal.animation", "PAL Palettes", "Palette animation/cycling",
     "Animate palette entries (water shimmer, etc.)",
     "Rotating palette colors", CAP_UNTESTED, ""},
    {"pal.remap_tables", "PAL Palettes", "House color remap tables",
     "Generate/load color remap tables for teams",
     "Red, blue, green, etc. team colors", CAP_UNTESTED, ""},

    // =========================================================================
    // AUD AUDIO PLAYBACK
    // =========================================================================
    {"aud.load_ima_adpcm", "AUD Audio", "IMA ADPCM decoding",
     "Decompress IMA ADPCM compressed audio",
     "Most sound effects and music", CAP_UNTESTED, ""},
    {"aud.load_ws_adpcm", "AUD Audio", "Westwood ADPCM decoding",
     "Decompress Westwood's custom ADPCM format",
     "Some older audio files", CAP_UNTESTED, ""},
    {"aud.load_pcm", "AUD Audio", "Uncompressed PCM",
     "Handle raw PCM audio data",
     "Some simple sounds", CAP_UNTESTED, ""},
    {"aud.sample_rates", "AUD Audio", "Sample rate conversion",
     "Convert various sample rates to output rate",
     "22050Hz, 11025Hz, etc. to 44100Hz", CAP_UNTESTED, ""},
    {"aud.8bit_samples", "AUD Audio", "8-bit audio samples",
     "Handle 8-bit unsigned audio",
     "Older sound effects", CAP_UNTESTED, ""},
    {"aud.16bit_samples", "AUD Audio", "16-bit audio samples",
     "Handle 16-bit signed audio",
     "Higher quality sounds", CAP_UNTESTED, ""},
    {"aud.stereo", "AUD Audio", "Stereo audio",
     "Handle stereo (2-channel) audio",
     "Music tracks", CAP_UNTESTED, ""},
    {"aud.mono", "AUD Audio", "Mono audio",
     "Handle mono (1-channel) audio",
     "Most sound effects", CAP_UNTESTED, ""},
    {"aud.play_sound", "AUD Audio", "Play sound effect",
     "Trigger and mix sound effects",
     "Weapon sounds, explosions", CAP_UNTESTED, ""},
    {"aud.play_music", "AUD Audio", "Play background music",
     "Stream and loop music tracks",
     "HELL MARCH, etc.", CAP_UNTESTED, ""},
    {"aud.play_speech", "AUD Audio", "Play speech/voice",
     "Play EVA and unit voice responses",
     "\"Unit ready\", \"Building\"", CAP_UNTESTED, ""},
    {"aud.volume_control", "AUD Audio", "Volume control",
     "Adjust sound/music volume",
     "Per-channel and master volume", CAP_UNTESTED, ""},
    {"aud.pan_control", "AUD Audio", "Stereo panning",
     "Pan sounds left/right based on position",
     "3D sound positioning", CAP_UNTESTED, ""},
    {"aud.simultaneous", "AUD Audio", "Multiple simultaneous sounds",
     "Mix multiple sounds playing at once",
     "Battle sounds overlapping", CAP_UNTESTED, ""},

    // =========================================================================
    // VQA VIDEO PLAYBACK
    // =========================================================================
    {"vqa.load_header", "VQA Video", "Parse VQA header",
     "Read VQA file structure and metadata",
     "All cutscene files", CAP_UNTESTED, ""},
    {"vqa.decode_video", "VQA Video", "Decode video frames",
     "Decompress VQA video using codebook",
     "Cutscene visuals", CAP_UNTESTED, ""},
    {"vqa.decode_audio", "VQA Video", "Decode VQA audio",
     "Extract and decompress audio track",
     "Cutscene sound", CAP_UNTESTED, ""},
    {"vqa.audio_sync", "VQA Video", "Audio/video synchronization",
     "Keep audio and video in sync",
     "No drift during playback", CAP_UNTESTED, ""},
    {"vqa.palette_changes", "VQA Video", "Handle palette changes",
     "Apply palette updates during playback",
     "Color changes in cutscenes", CAP_UNTESTED, ""},
    {"vqa.looping", "VQA Video", "Looping video",
     "Loop VQA for menu backgrounds",
     "Main menu backgrounds", CAP_UNTESTED, ""},
    {"vqa.scaling", "VQA Video", "Video scaling",
     "Scale video to fit window",
     "320x200 to larger displays", CAP_UNTESTED, ""},

    // =========================================================================
    // INI CONFIGURATION PARSING
    // =========================================================================
    {"ini.parse_sections", "INI Config", "Parse INI sections",
     "Read [Section] headers correctly",
     "All .INI files", CAP_UNTESTED, ""},
    {"ini.parse_keyvalues", "INI Config", "Parse key=value pairs",
     "Read Key=Value entries",
     "All configuration", CAP_UNTESTED, ""},
    {"ini.rules_ini", "INI Config", "RULES.INI parsing",
     "Load game rules and unit stats",
     "Unit costs, speeds, weapons", CAP_UNTESTED, ""},
    {"ini.mission_ini", "INI Config", "Mission .INI parsing",
     "Load mission map and triggers",
     "SCG01EA.INI etc.", CAP_UNTESTED, ""},
    {"ini.art_ini", "INI Config", "ART.INI parsing",
     "Load sprite definitions and animations",
     "Frame counts, offsets", CAP_UNTESTED, ""},

    // =========================================================================
    // MAP/MISSION LOADING
    // =========================================================================
    {"map.mappack", "Map/Mission", "[MapPack] terrain loading",
     "Decompress and load terrain data",
     "LCW compressed terrain", CAP_UNTESTED, ""},
    {"map.overlaypack", "Map/Mission", "[OverlayPack] overlay loading",
     "Load ore, walls, and overlays",
     "Resources and obstacles", CAP_UNTESTED, ""},
    {"map.waypoints", "Map/Mission", "Waypoint parsing",
     "Load spawn points and AI waypoints",
     "[Waypoints] section", CAP_UNTESTED, ""},
    {"map.triggers", "Map/Mission", "Trigger parsing",
     "Load mission triggers and events",
     "[Trigs] section", CAP_UNTESTED, ""},
    {"map.teamtypes", "Map/Mission", "TeamTypes parsing",
     "Load AI team compositions",
     "[TeamTypes] section", CAP_UNTESTED, ""},
    {"map.celltriggers", "Map/Mission", "Cell trigger parsing",
     "Load per-cell trigger assignments",
     "[CellTriggers] section", CAP_UNTESTED, ""},
    {"map.structures", "Map/Mission", "Structure placement",
     "Load and place buildings",
     "[STRUCTURES] section", CAP_UNTESTED, ""},
    {"map.units", "Map/Mission", "Unit placement",
     "Load and place vehicles",
     "[UNITS] section", CAP_UNTESTED, ""},
    {"map.infantry", "Map/Mission", "Infantry placement",
     "Load and place infantry with sub-cells",
     "[INFANTRY] section", CAP_UNTESTED, ""},
    {"map.terrain_objects", "Map/Mission", "Terrain object placement",
     "Load trees and terrain decorations",
     "[TERRAIN] section", CAP_UNTESTED, ""},
    {"map.smudges", "Map/Mission", "Smudge placement",
     "Load craters and scorch marks",
     "[SMUDGE] section", CAP_UNTESTED, ""},
    {"map.base_rebuild", "Map/Mission", "AI base rebuild list",
     "Load AI building priorities",
     "[Base] section", CAP_UNTESTED, ""},
    {"map.reinforcements", "Map/Mission", "Reinforcement parsing",
     "Load scripted reinforcements",
     "[Reinforcements] section", CAP_UNTESTED, ""},

    // =========================================================================
    // TRIGGER SYSTEM
    // =========================================================================
    {"trigger.time_elapsed", "Triggers", "Time-based triggers",
     "Fire triggers after time passes",
     "Event type 1: Time", CAP_UNTESTED, ""},
    {"trigger.unit_discovered", "Triggers", "Unit discovered triggers",
     "Fire when unit enters shroud",
     "Event type 2: Discovered", CAP_UNTESTED, ""},
    {"trigger.entered_by", "Triggers", "Zone entry triggers",
     "Fire when unit enters cell/zone",
     "Event type 3: Entered by", CAP_UNTESTED, ""},
    {"trigger.destroyed", "Triggers", "Destruction triggers",
     "Fire when object destroyed",
     "Event type 6: Destroyed", CAP_UNTESTED, ""},
    {"trigger.attacked", "Triggers", "Attack triggers",
     "Fire when object attacked",
     "Event type 7: Attacked", CAP_UNTESTED, ""},
    {"trigger.all_destroyed", "Triggers", "All destroyed triggers",
     "Fire when all of type destroyed",
     "Event type 24: All destroyed", CAP_UNTESTED, ""},
    {"trigger.action_win", "Triggers", "Win action",
     "Trigger victory condition",
     "Action type 2: Win", CAP_UNTESTED, ""},
    {"trigger.action_lose", "Triggers", "Lose action",
     "Trigger defeat condition",
     "Action type 3: Lose", CAP_UNTESTED, ""},
    {"trigger.action_reveal", "Triggers", "Reveal map action",
     "Reveal area of map",
     "Action type 16: Reveal", CAP_UNTESTED, ""},
    {"trigger.action_reinforcements", "Triggers", "Create reinforcements",
     "Spawn reinforcement team",
     "Action type 7: Reinforce", CAP_UNTESTED, ""},
    {"trigger.action_text", "Triggers", "Display text action",
     "Show mission text on screen",
     "Action type 10: Text", CAP_UNTESTED, ""},

    // =========================================================================
    // RENDERING PIPELINE
    // =========================================================================
    {"render.8bit_framebuffer", "Rendering", "8-bit paletted framebuffer",
     "Render to indexed color buffer",
     "Core rendering approach", CAP_UNTESTED, ""},
    {"render.palette_lookup", "Rendering", "Palette to RGBA conversion",
     "Convert indexed to true color for display",
     "GPU texture upload", CAP_UNTESTED, ""},
    {"render.metal_present", "Rendering", "Metal presentation",
     "Display framebuffer via Metal",
     "macOS GPU rendering", CAP_UNTESTED, ""},
    {"render.scaling", "Rendering", "Display scaling",
     "Scale 640x400 to window size",
     "Window resize handling", CAP_UNTESTED, ""},
    {"render.clipping", "Rendering", "Sprite clipping",
     "Clip sprites to viewport bounds",
     "Edge-of-screen sprites", CAP_UNTESTED, ""},
    {"render.layering", "Rendering", "Z-order/layering",
     "Render objects in correct depth order",
     "Units behind buildings", CAP_UNTESTED, ""},
    {"render.fog_of_war", "Rendering", "Fog of war",
     "Darken unexplored/non-visible areas",
     "Shroud rendering", CAP_UNTESTED, ""},
    {"render.minimap", "Rendering", "Radar/minimap rendering",
     "Render overview map in sidebar",
     "Tactical display", CAP_UNTESTED, ""},
    {"render.health_bars", "Rendering", "Unit health bars",
     "Draw health indicators over selected units",
     "Selection feedback", CAP_UNTESTED, ""},
    {"render.selection_box", "Rendering", "Selection rectangle",
     "Draw box selection outline",
     "Multi-unit selection", CAP_UNTESTED, ""},

    // =========================================================================
    // INPUT HANDLING
    // =========================================================================
    {"input.mouse_click", "Input", "Mouse click handling",
     "Detect and process mouse clicks",
     "Unit selection, commands", CAP_UNTESTED, ""},
    {"input.mouse_drag", "Input", "Mouse drag/box select",
     "Handle click-and-drag selection",
     "Multi-unit selection", CAP_UNTESTED, ""},
    {"input.keyboard", "Input", "Keyboard shortcuts",
     "Handle hotkeys and shortcuts",
     "Attack move, stop, etc.", CAP_UNTESTED, ""},
    {"input.scroll_edge", "Input", "Edge scrolling",
     "Scroll map when mouse at edge",
     "Map navigation", CAP_UNTESTED, ""},
    {"input.scroll_keys", "Input", "Keyboard scrolling",
     "Scroll map with arrow keys/WASD",
     "Map navigation", CAP_UNTESTED, ""},
    {"input.right_click", "Input", "Right-click commands",
     "Issue move/attack orders",
     "Unit commands", CAP_UNTESTED, ""},

    // =========================================================================
    // GAME LOGIC
    // =========================================================================
    {"logic.pathfinding", "Game Logic", "A* pathfinding",
     "Calculate paths avoiding obstacles",
     "Unit movement", CAP_UNTESTED, ""},
    {"logic.collision", "Game Logic", "Unit collision",
     "Prevent unit overlap",
     "Movement blocking", CAP_UNTESTED, ""},
    {"logic.combat", "Game Logic", "Damage calculation",
     "Apply weapon damage with armor types",
     "Combat resolution", CAP_UNTESTED, ""},
    {"logic.projectiles", "Game Logic", "Projectile physics",
     "Move bullets/missiles to targets",
     "Weapon effects", CAP_UNTESTED, ""},
    {"logic.harvesting", "Game Logic", "Ore harvesting",
     "Harvesters collect and return ore",
     "Economy", CAP_UNTESTED, ""},
    {"logic.building", "Game Logic", "Building placement/construction",
     "Build new structures",
     "Base building", CAP_UNTESTED, ""},
    {"logic.production", "Game Logic", "Unit production queues",
     "Queue and build units/buildings",
     "Factory production", CAP_UNTESTED, ""},
    {"logic.ai_basic", "Game Logic", "Basic AI behavior",
     "AI unit targeting and engagement",
     "Enemy response", CAP_UNTESTED, ""},
    {"logic.ai_teams", "Game Logic", "AI team scripting",
     "Execute TeamType missions",
     "Scripted attacks", CAP_UNTESTED, ""},

    // =========================================================================
    // UI/MENUS
    // =========================================================================
    {"ui.main_menu", "UI/Menus", "Main menu",
     "Display and navigate main menu",
     "New Game, Load, Options", CAP_UNTESTED, ""},
    {"ui.sidebar", "UI/Menus", "Game sidebar",
     "Render build icons and radar",
     "In-game interface", CAP_UNTESTED, ""},
    {"ui.button_click", "UI/Menus", "Button interaction",
     "Clickable UI buttons with feedback",
     "Menu navigation", CAP_UNTESTED, ""},
    {"ui.slider", "UI/Menus", "Slider controls",
     "Adjustable sliders for options",
     "Volume controls", CAP_UNTESTED, ""},
    {"ui.mission_briefing", "UI/Menus", "Mission briefing screen",
     "Display mission text and map preview",
     "Pre-mission screen", CAP_UNTESTED, ""},
    {"ui.score_screen", "UI/Menus", "Score/stats screen",
     "Display end-of-mission statistics",
     "Victory/defeat screen", CAP_UNTESTED, ""},
};

// Track which capability is currently being tested
static int g_currentCapability = 0;

// Map capability IDs to status (for persistence)
static std::map<std::string, CapabilityStatus> g_capabilityStatus;
static std::map<std::string, std::string> g_capabilityNotes;

// NSOutlineView for capability checklist
static NSOutlineView* g_capabilityOutline = nil;

// Category outline delegate (for asset discovery - keep for file browser)
@class CategoryOutlineDelegate;
static CategoryOutlineDelegate* g_categoryDelegate = nil;

// Capability outline delegate
@class CapabilityOutlineDelegate;
static CapabilityOutlineDelegate* g_capabilityDelegate = nil;

// Asset categories for file organization (for file browser and asset test)
struct AssetCategory {
    const char* name;
    const char* description;
    std::vector<std::string> patterns;  // Patterns to match asset names
    std::vector<FileTreeNode*> assets;
};

// Current category/asset indices for review
static int g_currentCategory = 0;
static int g_currentCategoryAsset = 0;
static NSOutlineView* g_categoryOutline = nil;

// Predefined asset categories with matching patterns
static std::vector<AssetCategory> g_categories = {
    // === UNITS ===
    {"Infantry", "Infantry unit sprites (E1-E7, civilians, dogs, etc.)",
     {"E1", "E2", "E3", "E4", "E5", "E6", "E7", "SPY", "THF", "MEDI", "MECH",
      "SHOK", "DOG", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9",
      "C10", "EINSTEIN", "DELPHI", "CHAN"}, {}},
    {"Vehicles", "Vehicle unit sprites (tanks, APCs, MCVs)",
     {"1TNK", "2TNK", "3TNK", "4TNK", "FTNK", "STNK", "MTNK", "HTNK", "LTNK",
      "APC", "MCV", "HARV", "ARTY", "V2RL", "MRLS", "JEEP", "TRUK", "MGG",
      "MRJ", "MNLY", "CTNK", "TTNK", "QTNK", "DTRK"}, {}},
    {"Aircraft", "Aircraft sprites (helicopters, planes)",
     {"HELI", "HIND", "TRAN", "BADR", "MIG", "YAK", "SPY", "U2"}, {}},
    {"Ships", "Naval vessel sprites",
     {"SS", "DD", "CA", "LST", "PT", "CARR", "SUB", "MSUB"}, {}},

    // === BUILDINGS ===
    {"Power", "Power generation buildings",
     {"POWR", "APWR", "NPWR", "NUK2"}, {}},
    {"Production", "Unit production buildings (factories, barracks)",
     {"PBOX", "HBOX", "TENT", "BARR", "WEAP", "WEAF", "AFLD", "FACT",
      "SYRD", "SPEN", "STEK"}, {}},
    {"Defense", "Defensive structures (turrets, walls, fences)",
     {"GUN", "AGUN", "FTUR", "TSLA", "SAM", "DOME", "GAP", "PBOX", "HBOX",
      "SBAG", "CYCL", "BRIK", "BARB", "WOOD", "FENC"}, {}},
    {"Support", "Support buildings (radar, tech, silos)",
     {"DOME", "HPAD", "FIX", "ATEK", "STEK", "SILO", "PROC", "MISS",
      "BIO", "EYE", "HOSP", "IRON"}, {}},
    {"Base", "Base structures (construction yards, special)",
     {"FACT", "SYRD", "MSLO", "IRON", "PDOX", "TMPL", "WEAP"}, {}},

    // === TERRAIN ===
    {"Terrain - Temperate", "Temperate theater terrain tiles",
     {"TEMP", "BRIDGE", "SHORE", "WATER", "CLEAR"}, {}},
    {"Terrain - Snow", "Snow theater terrain tiles",
     {"SNOW", "ICE"}, {}},
    {"Terrain - Interior", "Interior/dungeon terrain tiles",
     {"INTERIOR", "FLOOR", "WALL"}, {}},
    {"Trees", "Tree and vegetation sprites",
     {"T01", "T02", "T03", "T04", "T05", "T06", "T07", "T08", "T09", "T10",
      "T11", "T12", "T13", "T14", "T15", "T16", "T17", "TC01", "TC02", "TC03",
      "TC04", "TC05"}, {}},
    {"Smudges", "Craters, scorch marks, bibs",
     {"CR1", "CR2", "CR3", "CR4", "CR5", "CR6", "SC1", "SC2", "SC3", "SC4",
      "SC5", "SC6", "BIB1", "BIB2", "BIB3"}, {}},
    {"Overlays", "Ore, gems, walls, fences",
     {"GOLD", "GEM", "ORE", "BARB", "SBAG", "CYCL", "BRIK", "WOOD", "FENC",
      "V12", "V13", "V14", "V15", "V16", "V17", "V18"}, {}},

    // === EFFECTS ===
    {"Explosions", "Explosion and impact effects",
     {"VEH-HIT", "ART-EXP", "NAPALM", "FIRE", "BURN", "PIFF", "PIFFPIFF",
      "FBALL", "FB1", "FB2", "BOMBLET", "BOMB", "SMOKE", "SMOK"}, {}},
    {"Projectiles", "Weapon projectiles",
     {"BOMB", "LASER", "DRAGON", "MISSILE", "MLRS", "BULLET", "PSCRL",
      "SONIC", "ATOMSFX"}, {}},
    {"Animations", "Miscellaneous animations",
     {"FLAG", "CRATE", "WOOD", "ELECT", "SPUTDOOR", "SELECT", "MOVEFLSH",
      "YOURWIN", "YOURLOSE", "CHRONSFX", "IONSFX", "ATOMSFX"}, {}},

    // === UI ===
    {"Sidebar Icons", "Build menu icons",
     {"ICON", "CAMEO"}, {}},
    {"Cursors", "Mouse cursor sprites",
     {"MOUSE", "CURS"}, {}},
    {"Interface", "Interface elements",
     {"HISCORE", "TITLE", "MSLOGO", "CHOOSE", "DIALOG", "TAB"}, {}},

    // === AUDIO ===
    {"Music", "Background music tracks",
     {"BIGF", "CRUS", "FAC1", "FAC2", "HELL", "RUN1", "SMSH", "TREN",
      "WORK", "AWAIT", "DENSE", "MAP", "FOGGER", "MUD", "RADIO2", "ROLL",
      "SNAKE", "TERMINAT", "TWIN", "VECTOR", "VOLKOV", "2ND_HAND"}, {}},
    {"Sound Effects", "Game sound effects",
     {"GIRLOKAY", "GIRLYEAH", "GIRLBOMR", "MANOKAY", "MANYEAH", "MANYELL",
      "APTS", "BLDG", "CONSTRU", "CRUMBLE", "EXPLOD", "MGUN", "RIFLE"}, {}},
    {"Voice", "EVA and unit voices",
     {"SPEECH", "CASHTURN", "CHROCHR1", "FREEFALL", "IRONCHG1", "BLDGING",
      "SOVBUILD", "CANCLD", "ABLDGIN1", "ONHOLD", "PRIMRY", "READY"}, {}},

    // === VIDEO ===
    {"Cutscenes", "FMV cutscene videos",
     {"ALLY1", "ALLY2", "ALLY3", "ALLY4", "ALLY5", "ALLY6", "ALLY7", "ALLY8",
      "ALLY9", "ALLY10", "ALLEND", "SOV1", "SOV2", "SOV3", "SOV4", "SOV5",
      "SOV6", "SOV7", "SOV8", "SOV9", "SOV10", "SOVEND", "INTRO", "PROLOG",
      "ANTEND", "ANTS", "BMAP"}, {}},
    {"Menus", "Menu background videos",
     {"TITLE", "MAIN", "CHOOSE", "WINSOV", "LOSESOV", "WINALL", "LOSEALL"}, {}},

    // === CONFIG ===
    {"Rules", "Game rules configuration",
     {"RULES", "AFTRMATH"}, {}},
    {"Missions", "Mission INI files",
     {"SCG", "SCU", "MCV", "MISSION"}, {}},
    {"Art", "Art/sprite definitions",
     {"ART", "CONQUER"}, {}},

    // === CATCH-ALL ===
    {"Other - Uncategorized", "Assets not matching any category", {}, {}},
};

// UI references
static NSOutlineView* g_outlineView = nil;
static NSTextField* g_assetInfoLabel = nil;
static NSTextField* g_pathLabel = nil;
static MTKView* g_metalView = nil;
static NSTabView* g_tabView = nil;
static NSTextField* g_reviewNameLabel = nil;
static NSTextField* g_reviewInfoLabel = nil;
static NSTextField* g_reviewStatusLabel = nil;
static NSTextField* g_progressLabel = nil;
static NSButton* g_goodButton = nil;
static NSButton* g_badButton = nil;

// Calculate total size of a node (recursive for containers)
static uint64_t calculateNodeSize(FileTreeNode* node) {
    if (!node.isContainer) {
        return node.size;
    }

    uint64_t total = 0;
    for (FileTreeNode* child in node.children) {
        total += calculateNodeSize(child);
    }
    node.size = (uint32_t)MIN(total, UINT32_MAX);
    return total;
}

// Scan a MIX file and add its contents to the tree
static void scanMixFile(FileTreeNode* parentNode, MixFileHandle mix,
                        const std::string& mixPath, int depth,
                        NSMutableArray* mixChain, NSMutableArray* crcChain);

// Recursively scan a directory
static void scanDirectory(FileTreeNode* parentNode, NSString* path,
                         int depth) {
    if (depth > 10) return;  // Prevent infinite recursion

    NSFileManager* fm = [NSFileManager defaultManager];
    NSError* error = nil;
    NSArray* contents = [fm contentsOfDirectoryAtPath:path error:&error];

    if (error) {
        NSLog(@"Error scanning directory %@: %@", path, error);
        return;
    }

    for (NSString* item in contents) {
        NSString* fullPath = [path stringByAppendingPathComponent:item];
        BOOL isDir = NO;
        [fm fileExistsAtPath:fullPath isDirectory:&isDir];

        FileTreeNode* node = [[FileTreeNode alloc] init];
        node.name = item;
        node.fullPath = fullPath;
        node.parent = parentNode;
        node.depth = depth;

        if (isDir) {
            node.assetType = ViewerAssetType::Directory;
            node.isContainer = YES;
            [parentNode.children addObject:node];
            scanDirectory(node, fullPath, depth + 1);
        } else {
            std::string nameStr = [item UTF8String];
            node.assetType = getViewerAssetType(nameStr);

            // Get file size
            NSDictionary* attrs = [fm attributesOfItemAtPath:fullPath
                                                      error:nil];
            node.size = (uint32_t)[attrs fileSize];

            // If it's a MIX file, scan its contents
            if (node.assetType == ViewerAssetType::MixArchive) {
                node.isContainer = YES;
                [parentNode.children addObject:node];

                MixFileHandle mix = Mix_Open([fullPath UTF8String]);
                if (mix) {
                    NSMutableArray* chain = [NSMutableArray array];
                    [chain addObject:fullPath];
                    NSMutableArray* crcChain = [NSMutableArray array];
                    scanMixFile(node, mix, [fullPath UTF8String], depth + 1,
                               chain, crcChain);
                    Mix_Close(mix);
                }
            } else if (node.assetType == ViewerAssetType::IsoImage) {
                // ISO files - mount and scan contents
                node.isContainer = YES;
                [parentNode.children addObject:node];

                NSString* mountPath = mountISO(fullPath);
                if (mountPath) {
                    scanDirectory(node, mountPath, depth + 1);
                }
            } else {
                node.isContainer = NO;
                [parentNode.children addObject:node];

                // Add viewable assets to flat list
                if (node.assetType == ViewerAssetType::ShpSprite ||
                    node.assetType == ViewerAssetType::PalPalette ||
                    node.assetType == ViewerAssetType::AudAudio ||
                    node.assetType == ViewerAssetType::VqaVideo) {
                    g_allAssets.push_back(node);
                }
            }
        }
    }
}

// Scan a MIX file and add its contents
static void scanMixFile(FileTreeNode* parentNode, MixFileHandle mix,
                        const std::string& mixPath, int depth,
                        NSMutableArray* mixChain, NSMutableArray* crcChain) {
    if (depth > 10) return;

    int count = Mix_GetFileCount(mix);

    // Track nested MIX files to scan after
    std::vector<std::tuple<FileTreeNode*, uint32_t, uint32_t>> nestedMixes;

    for (int i = 0; i < count; i++) {
        uint32_t crc, size;
        if (!Mix_GetEntryByIndex(mix, i, &crc, &size)) continue;

        FileTreeNode* node = [[FileTreeNode alloc] init];
        node.crc = crc;
        node.size = size;
        node.parent = parentNode;
        node.depth = depth;

        // Copy the MIX chain for extraction later
        [node.mixChain addObjectsFromArray:mixChain];
        [node.crcChain addObjectsFromArray:crcChain];
        [node.crcChain addObject:@(crc)];

        // Try to look up the name
        const char* name = g_filenameDB.lookup(crc);
        if (name) {
            node.name = [NSString stringWithUTF8String:name];
            std::string nameStr(name);
            node.assetType = getViewerAssetType(nameStr);
        } else {
            // Show as hex CRC
            node.name = [NSString stringWithFormat:@"%08X", crc];
            node.assetType = ViewerAssetType::Unknown;
        }

        node.fullPath = [NSString stringWithFormat:@"%s::%@",
                        mixPath.c_str(), node.name];

        // Check if this is a nested MIX
        if (node.assetType == ViewerAssetType::MixArchive) {
            node.isContainer = YES;
            nestedMixes.push_back({node, crc, size});
        } else {
            node.isContainer = NO;

            // Add viewable assets to flat list
            if (node.assetType == ViewerAssetType::ShpSprite ||
                node.assetType == ViewerAssetType::PalPalette ||
                node.assetType == ViewerAssetType::AudAudio ||
                node.assetType == ViewerAssetType::VqaVideo) {
                g_allAssets.push_back(node);
            }
        }

        [parentNode.children addObject:node];
    }

    // Now scan nested MIX files
    for (auto& tuple : nestedMixes) {
        FileTreeNode* node = std::get<0>(tuple);
        uint32_t crc = std::get<1>(tuple);

        uint32_t dataSize;
        void* data = Mix_AllocReadFileByCRC(mix, crc, &dataSize);
        if (data) {
            MixFileHandle nested = Mix_OpenMemory(data, dataSize, TRUE);
            if (nested) {
                NSMutableArray* newChain = [NSMutableArray arrayWithArray:
                                           mixChain];
                NSMutableArray* newCrcChain = [NSMutableArray arrayWithArray:
                                              crcChain];
                [newCrcChain addObject:@(crc)];

                scanMixFile(node, nested, [node.fullPath UTF8String],
                           depth + 1, newChain, newCrcChain);
                Mix_Close(nested);
            }
        }
    }
}

// Extract asset from MIX chain
static void* extractFromMixChain(FileTreeNode* node, uint32_t* outSize) {
    if (node.mixChain.count == 0 || node.crcChain.count == 0) {
        return nullptr;
    }

    // Open the base MIX file
    NSString* basePath = node.mixChain[0];
    MixFileHandle mix = Mix_Open([basePath UTF8String]);
    if (!mix) return nullptr;

    void* data = nullptr;
    uint32_t size = 0;

    // Navigate through nested MIX files
    for (NSUInteger i = 0; i < node.crcChain.count; i++) {
        uint32_t crc = [node.crcChain[i] unsignedIntValue];

        // If this is the last CRC, read and return the data
        if (i == node.crcChain.count - 1) {
            data = Mix_AllocReadFileByCRC(mix, crc, &size);
            break;
        }

        // Otherwise, open the nested MIX
        uint32_t nestedSize;
        void* nestedData = Mix_AllocReadFileByCRC(mix, crc, &nestedSize);
        Mix_Close(mix);

        if (!nestedData) return nullptr;

        mix = Mix_OpenMemory(nestedData, nestedSize, TRUE);
        if (!mix) return nullptr;
    }

    Mix_Close(mix);

    if (outSize) *outSize = size;
    return data;
}

// Load default palette
static void loadDefaultPalette() {
    // Use a simple grayscale palette as fallback
    for (int i = 0; i < 256; i++) {
        g_palette.colors[i][0] = i;
        g_palette.colors[i][1] = i;
        g_palette.colors[i][2] = i;
    }
    g_paletteLoaded = true;
    Renderer_SetPalette(&g_palette);

    // Try to find TEMPERAT.PAL in the assets
    for (FileTreeNode* node : g_allAssets) {
        if (node.assetType == ViewerAssetType::PalPalette) {
            NSString* name = [node.name uppercaseString];
            if ([name isEqualToString:@"TEMPERAT.PAL"] ||
                [name isEqualToString:@"UNITTEM.PAL"]) {

                // Try to load this palette
                if (node.mixChain.count > 0) {
                    uint32_t size;
                    void* data = extractFromMixChain(node, &size);
                    if (data && size >= 768) {
                        // PAL files are 6-bit per channel, scale to 8-bit
                        uint8_t* palData = (uint8_t*)data;
                        for (int i = 0; i < 256; i++) {
                            g_palette.colors[i][0] = palData[i * 3 + 0] << 2;
                            g_palette.colors[i][1] = palData[i * 3 + 1] << 2;
                            g_palette.colors[i][2] = palData[i * 3 + 2] << 2;
                        }
                        free(data);
                        Renderer_SetPalette(&g_palette);
                        NSLog(@"Loaded palette: %@", node.name);
                        return;
                    }
                    if (data) free(data);
                } else {
                    // Direct file
                    FILE* f = fopen([node.fullPath UTF8String], "rb");
                    if (f) {
                        uint8_t palData[768];
                        if (fread(palData, 768, 1, f) == 1) {
                            for (int i = 0; i < 256; i++) {
                                g_palette.colors[i][0] = palData[i*3+0] << 2;
                                g_palette.colors[i][1] = palData[i*3+1] << 2;
                                g_palette.colors[i][2] = palData[i*3+2] << 2;
                            }
                            Renderer_SetPalette(&g_palette);
                        }
                        fclose(f);
                        NSLog(@"Loaded palette: %@", node.name);
                        return;
                    }
                }
            }
        }
    }
}

// Categorize all discovered assets into predefined categories
static void categorizeAssets() {
    // Clear existing categorizations
    for (auto& cat : g_categories) {
        cat.assets.clear();
    }

    // Track which asset names have been added to each category (deduplicate)
    std::vector<std::set<std::string>> seenNames(g_categories.size());

    for (FileTreeNode* node : g_allAssets) {
        std::string name = [node.name UTF8String];
        // Convert to uppercase for matching
        std::string upperName = name;
        for (char& c : upperName) c = toupper(c);

        // Remove extension for pattern matching
        size_t dotPos = upperName.rfind('.');
        std::string baseName = (dotPos != std::string::npos)
            ? upperName.substr(0, dotPos) : upperName;

        bool matched = false;

        // Try to match against each category's patterns
        for (size_t catIdx = 0; catIdx < g_categories.size() - 1; catIdx++) {
            auto& cat = g_categories[catIdx];

            for (const auto& pattern : cat.patterns) {
                // Check if name starts with pattern or contains it
                if (baseName.find(pattern) != std::string::npos ||
                    baseName.rfind(pattern, 0) == 0) {
                    // Only add if we haven't seen this asset name in this category
                    if (seenNames[catIdx].find(upperName) == seenNames[catIdx].end()) {
                        cat.assets.push_back(node);
                        seenNames[catIdx].insert(upperName);
                    }
                    matched = true;
                    break;
                }
            }
            if (matched) break;
        }

        // If not matched, add to "Other - Uncategorized" (also deduplicate)
        if (!matched) {
            size_t lastIdx = g_categories.size() - 1;
            if (seenNames[lastIdx].find(upperName) == seenNames[lastIdx].end()) {
                g_categories.back().assets.push_back(node);
                seenNames[lastIdx].insert(upperName);
            }
        }
    }

    NSLog(@"Categorized %zu assets into %zu categories",
          g_allAssets.size(), g_categories.size());

    // Log category counts
    for (const auto& cat : g_categories) {
        if (!cat.assets.empty()) {
            NSLog(@"  %s: %zu assets", cat.name, cat.assets.size());
        }
    }
}

// Load and display current asset from category view
// Stop any playing audio
static void stopAudio() {
    if (g_playingSound) {
        Audio_Stop(g_playingSound);
        g_playingSound = 0;
    }
}

// Forward declarations
static void playCurrentAUD();
static void stopVQA();
static void freeVqaAudioBuffer();

// Load AUD file from node
static void loadAUD(FileTreeNode* node) {
    // Free previous AUD
    if (g_currentAUD) {
        Aud_Free(g_currentAUD);
        g_currentAUD = nullptr;
    }
    stopVQA();  // Stop any playing video
    if (g_currentVQA) {
        delete g_currentVQA;
        g_currentVQA = nullptr;
    }
    stopAudio();

    // Initialize audio system if needed
    if (!g_audioInitialized) {
        if (Audio_Init()) {
            g_audioInitialized = true;
            NSLog(@"Audio system initialized");
        } else {
            NSLog(@"Failed to initialize audio system");
            return;
        }
    }

    // Load the AUD
    if (node.mixChain.count > 0) {
        uint32_t size;
        void* data = extractFromMixChain(node, &size);
        if (data) {
            g_currentAUD = Aud_Load(data, size);
            free(data);
        }
    } else {
        g_currentAUD = Aud_LoadFile([node.fullPath UTF8String]);
    }

    if (g_currentAUD) {
        float duration = (float)g_currentAUD->sampleCount /
                        (float)g_currentAUD->sampleRate;
        NSLog(@"Loaded AUD: %@ (%u samples, %u Hz, %d ch, %.2f sec)",
              node.name, g_currentAUD->sampleCount, g_currentAUD->sampleRate,
              g_currentAUD->channels, duration);

        // Update audio info label
        if (g_audioInfoLabel) {
            float duration = (float)g_currentAUD->sampleCount /
                           (float)g_currentAUD->sampleRate;
            [g_audioInfoLabel setStringValue:
                [NSString stringWithFormat:
                    @"Audio: %u Hz, %d ch, %.1f sec",
                    g_currentAUD->sampleRate, g_currentAUD->channels, duration]];
        }

        // Enable play/stop buttons
        if (g_playButton) [g_playButton setEnabled:YES];
        if (g_stopButton) [g_stopButton setEnabled:YES];

        // Auto-play the audio
        playCurrentAUD();
    } else {
        if (g_audioInfoLabel) {
            [g_audioInfoLabel setStringValue:@"Failed to load AUD"];
        }
        if (g_playButton) [g_playButton setEnabled:NO];
        if (g_stopButton) [g_stopButton setEnabled:NO];
    }
}

// Play currently loaded AUD
static void playCurrentAUD() {
    if (!g_currentAUD) {
        NSLog(@"playCurrentAUD: No AUD loaded");
        return;
    }
    if (!g_audioInitialized) {
        NSLog(@"playCurrentAUD: Audio not initialized");
        return;
    }

    stopAudio();

    // Fill global AudioSample (must persist - Audio_Play stores pointer)
    g_audAudioSample.data = (uint8_t*)g_currentAUD->samples;
    g_audAudioSample.dataSize = g_currentAUD->sampleCount * g_currentAUD->channels * sizeof(int16_t);
    g_audAudioSample.sampleRate = g_currentAUD->sampleRate;
    g_audAudioSample.channels = g_currentAUD->channels;
    g_audAudioSample.bitsPerSample = 16;

    float duration = (float)g_currentAUD->sampleCount / (float)g_currentAUD->sampleRate;
    NSLog(@"Playing AUD: %u samples, %u Hz, %d ch, %u bytes, %.2f sec",
          g_currentAUD->sampleCount, g_currentAUD->sampleRate,
          g_currentAUD->channels, g_audAudioSample.dataSize, duration);

    g_playingSound = Audio_Play(&g_audAudioSample, 255, 0, FALSE);
    if (g_playingSound) {
        NSLog(@"Audio_Play returned handle: %u", g_playingSound);
    } else {
        NSLog(@"Audio_Play FAILED - returned 0");
    }
}

// Forward declarations
static void freeVqaAudioBuffer();
static void updatePreview();

// Stop VQA playback
static void stopVQA() {
    if (g_vqaTimer) {
        [g_vqaTimer invalidate];
        g_vqaTimer = nil;
    }
    g_vqaPlaying = false;
    if (g_currentVQA) {
        g_currentVQA->Stop();
    }
    // Stop VQA audio and free buffer
    stopAudio();
    freeVqaAudioBuffer();
}

// Load VQA file from node
static void loadVQA(FileTreeNode* node) {
    // Stop any current playback
    stopVQA();
    stopAudio();

    // Free previous VQA
    if (g_currentVQA) {
        delete g_currentVQA;
        g_currentVQA = nullptr;
    }

    // Clear AUD so play button plays VQA not old audio
    if (g_currentAUD) {
        Aud_Free(g_currentAUD);
        g_currentAUD = nullptr;
    }

    // Create new player
    g_currentVQA = new VQAPlayer();

    bool loaded = false;
    if (node.mixChain.count > 0) {
        uint32_t size;
        void* data = extractFromMixChain(node, &size);
        if (data) {
            loaded = g_currentVQA->Load(data, size);
            free(data);
        }
    } else {
        loaded = g_currentVQA->Load([node.fullPath UTF8String]);
    }

    if (loaded) {
        NSLog(@"Loaded VQA: %@ (%dx%d, %d frames, %d fps)",
              node.name,
              g_currentVQA->GetWidth(),
              g_currentVQA->GetHeight(),
              g_currentVQA->GetFrameCount(),
              g_currentVQA->GetFPS());

        // Update video info label
        if (g_videoInfoLabel) {
            float duration = (float)g_currentVQA->GetFrameCount() /
                           (float)g_currentVQA->GetFPS();
            [g_videoInfoLabel setStringValue:
                [NSString stringWithFormat:
                    @"Video: %dx%d, %d fps, %.1f sec, %d frames",
                    g_currentVQA->GetWidth(),
                    g_currentVQA->GetHeight(),
                    g_currentVQA->GetFPS(),
                    duration,
                    g_currentVQA->GetFrameCount()]];
        }

        // Enable play/stop buttons
        if (g_playButton) [g_playButton setEnabled:YES];
        if (g_stopButton) [g_stopButton setEnabled:YES];

        // Decode first frame so it displays immediately when loaded
        g_currentVQA->Play();
        g_currentVQA->NextFrame();
        g_currentVQA->Stop();
        updatePreview();
    } else {
        NSLog(@"Failed to load VQA: %@", node.name);
        if (g_videoInfoLabel) {
            [g_videoInfoLabel setStringValue:@"Failed to load VQA"];
        }
        if (g_playButton) [g_playButton setEnabled:NO];
        if (g_stopButton) [g_stopButton setEnabled:NO];
        delete g_currentVQA;
        g_currentVQA = nullptr;
    }
}

// Free VQA audio buffer
static void freeVqaAudioBuffer() {
    if (g_vqaAudioBuffer) {
        free(g_vqaAudioBuffer);
        g_vqaAudioBuffer = nullptr;
        g_vqaAudioBufferSize = 0;
    }
}

// Play currently loaded VQA
static void playCurrentVQA() {
    if (!g_currentVQA || !g_currentVQA->IsLoaded()) return;

    // Initialize audio system if needed
    if (!g_audioInitialized) {
        if (Audio_Init()) {
            g_audioInitialized = true;
            NSLog(@"Audio system initialized for VQA playback");
        }
    }

    // Stop any existing audio and free old VQA audio buffer
    stopAudio();
    freeVqaAudioBuffer();

    // Start VQA video playback
    g_currentVQA->Play();
    g_vqaPlaying = true;

    // Decode first frame immediately so it displays right away
    g_currentVQA->NextFrame();
    updatePreview();

    // Start VQA audio if available
    if (g_audioInitialized && g_currentVQA->HasAudio() &&
        g_currentVQA->HasPreDecodedAudio()) {
        // Get total audio samples from VQA
        int sampleRate = g_currentVQA->GetAudioSampleRate();
        int channels = g_currentVQA->GetAudioChannels();
        int frameCount = g_currentVQA->GetFrameCount();
        int fps = g_currentVQA->GetFPS();

        // Estimate total samples: (frames / fps) * sampleRate * channels
        int estimatedSamples = ((frameCount * sampleRate) / fps) * channels + 10000;

        // Allocate persistent buffer for audio samples
        g_vqaAudioBuffer = (int16_t*)malloc(estimatedSamples * sizeof(int16_t));
        if (g_vqaAudioBuffer) {
            int totalSamples = g_currentVQA->GetAudioSamples(g_vqaAudioBuffer,
                                                              estimatedSamples);
            if (totalSamples > 0) {
                g_vqaAudioBufferSize = totalSamples;

                // Set up audio sample (must persist for playback)
                g_vqaAudioSample.data = (uint8_t*)g_vqaAudioBuffer;
                g_vqaAudioSample.dataSize = totalSamples * sizeof(int16_t);
                g_vqaAudioSample.sampleRate = sampleRate;
                g_vqaAudioSample.channels = channels;
                g_vqaAudioSample.bitsPerSample = 16;

                g_playingSound = Audio_Play(&g_vqaAudioSample, 255, 0, FALSE);
                if (g_playingSound) {
                    NSLog(@"Playing VQA audio: %d samples at %d Hz",
                          totalSamples, sampleRate);
                }
            } else {
                freeVqaAudioBuffer();
            }
        }
    }
}

static void loadCategoryAsset() {
    // Clean up previous assets
    if (g_currentSHP) {
        Shp_Free(g_currentSHP);
        g_currentSHP = nullptr;
    }
    if (g_currentAUD) {
        Aud_Free(g_currentAUD);
        g_currentAUD = nullptr;
    }
    stopVQA();    // Stop any playing video
    if (g_currentVQA) {
        delete g_currentVQA;
        g_currentVQA = nullptr;
    }
    stopAudio();  // Stop any playing audio
    freeVqaAudioBuffer();  // Free VQA audio buffer
    g_currentFrame = 0;

    // Restore default palette after VQA playback (VQA modifies palette)
    if (g_paletteLoaded) {
        Renderer_SetPalette(&g_palette);
    }

    if (g_currentCategory >= (int)g_categories.size()) return;
    auto& cat = g_categories[g_currentCategory];
    if (cat.assets.empty() || g_currentCategoryAsset >= (int)cat.assets.size())
        return;

    FileTreeNode* node = cat.assets[g_currentCategoryAsset];

    // Update review tab labels
    if (g_reviewNameLabel) {
        [g_reviewNameLabel setStringValue:node.name];
    }

    if (g_reviewInfoLabel) {
        NSString* info = [NSString stringWithFormat:
            @"Category: %s\nType: %s\nSize: %u bytes\nPath: %@",
            cat.name, getTypeName(node.assetType), node.size, node.fullPath];
        [g_reviewInfoLabel setStringValue:info];
    }

    // Update status
    std::string key = [node.fullPath UTF8String];
    ReviewStatus status = PENDING;
    auto it = g_reviewStatus.find(key);
    if (it != g_reviewStatus.end()) status = it->second;

    if (g_reviewStatusLabel) {
        const char* statusStr = "PENDING";
        if (status == GOOD) statusStr = "GOOD";
        else if (status == BAD) statusStr = "BAD";
        [g_reviewStatusLabel setStringValue:
            [NSString stringWithUTF8String:statusStr]];
    }

    // Update progress
    if (g_progressLabel) {
        int reviewed = 0;
        for (auto& p : g_reviewStatus) {
            if (p.second != PENDING) reviewed++;
        }
        [g_progressLabel setStringValue:
            [NSString stringWithFormat:
                @"Category: %d/%zu | Asset: %d/%zu | Reviewed: %d/%zu",
                g_currentCategory + 1, g_categories.size(),
                g_currentCategoryAsset + 1, cat.assets.size(),
                reviewed, g_allAssets.size()]];
    }

    // Clear audio info if not an audio file
    if (g_audioInfoLabel && node.assetType != ViewerAssetType::AudAudio) {
        [g_audioInfoLabel setStringValue:@""];
    }
    // Clear video info if not a video file
    if (g_videoInfoLabel && node.assetType != ViewerAssetType::VqaVideo) {
        [g_videoInfoLabel setStringValue:@""];
    }
    // Enable play/stop buttons only for audio/video
    bool playable = (node.assetType == ViewerAssetType::AudAudio ||
                     node.assetType == ViewerAssetType::VqaVideo);
    if (!playable) {
        if (g_playButton) [g_playButton setEnabled:NO];
        if (g_stopButton) [g_stopButton setEnabled:NO];
    }

    // Load based on asset type
    if (node.assetType == ViewerAssetType::ShpSprite) {
        // Load the SHP
        if (node.mixChain.count > 0) {
            uint32_t size;
            void* data = extractFromMixChain(node, &size);
            if (data) {
                g_currentSHP = Shp_Load(data, size);
                free(data);
            }
        } else {
            g_currentSHP = Shp_LoadFile([node.fullPath UTF8String]);
        }

        if (g_currentSHP) {
            NSLog(@"Loaded SHP: %@ (%d frames)", node.name,
                  Shp_GetFrameCount(g_currentSHP));
        }
    } else if (node.assetType == ViewerAssetType::AudAudio) {
        loadAUD(node);
    } else if (node.assetType == ViewerAssetType::VqaVideo) {
        loadVQA(node);
    }
}

// Load and display current asset
static void loadCurrentAsset() {
    // Clean up previous assets
    if (g_currentSHP) {
        Shp_Free(g_currentSHP);
        g_currentSHP = nullptr;
    }
    if (g_currentAUD) {
        Aud_Free(g_currentAUD);
        g_currentAUD = nullptr;
    }
    stopVQA();    // Stop any playing video
    if (g_currentVQA) {
        delete g_currentVQA;
        g_currentVQA = nullptr;
    }
    stopAudio();  // Stop any playing audio
    freeVqaAudioBuffer();  // Free VQA audio buffer

    // Restore default palette after VQA playback
    if (g_paletteLoaded) {
        Renderer_SetPalette(&g_palette);
    }

    g_currentFrame = 0;

    if (g_allAssets.empty() || g_currentAssetIndex >= (int)g_allAssets.size())
        return;

    FileTreeNode* node = g_allAssets[g_currentAssetIndex];

    // Update review tab labels
    if (g_reviewNameLabel) {
        [g_reviewNameLabel setStringValue:node.name];
    }

    if (g_reviewInfoLabel) {
        NSString* info = [NSString stringWithFormat:
            @"Type: %s\nSize: %u bytes\nPath: %@",
            getTypeName(node.assetType), node.size, node.fullPath];
        [g_reviewInfoLabel setStringValue:info];
    }

    // Update status
    std::string key = [node.fullPath UTF8String];
    ReviewStatus status = PENDING;
    auto it = g_reviewStatus.find(key);
    if (it != g_reviewStatus.end()) status = it->second;

    if (g_reviewStatusLabel) {
        const char* statusStr = "PENDING";
        if (status == GOOD) statusStr = "GOOD";
        else if (status == BAD) statusStr = "BAD";
        [g_reviewStatusLabel setStringValue:
            [NSString stringWithUTF8String:statusStr]];
    }

    // Update progress
    if (g_progressLabel) {
        int reviewed = 0;
        for (auto& p : g_reviewStatus) {
            if (p.second != PENDING) reviewed++;
        }
        [g_progressLabel setStringValue:
            [NSString stringWithFormat:@"Asset %d of %zu (Reviewed: %d)",
             g_currentAssetIndex + 1, g_allAssets.size(), reviewed]];
    }

    // Clear info labels for types we're not displaying
    if (g_audioInfoLabel && node.assetType != ViewerAssetType::AudAudio) {
        [g_audioInfoLabel setStringValue:@""];
    }
    if (g_videoInfoLabel && node.assetType != ViewerAssetType::VqaVideo) {
        [g_videoInfoLabel setStringValue:@""];
    }
    // Enable play/stop buttons only for audio/video
    bool playable = (node.assetType == ViewerAssetType::AudAudio ||
                     node.assetType == ViewerAssetType::VqaVideo);
    if (!playable) {
        if (g_playButton) [g_playButton setEnabled:NO];
        if (g_stopButton) [g_stopButton setEnabled:NO];
    }

    // Load based on asset type
    if (node.assetType == ViewerAssetType::ShpSprite) {
        // Load the SHP
        if (node.mixChain.count > 0) {
            // Inside MIX - extract first
            uint32_t size;
            void* data = extractFromMixChain(node, &size);
            if (data) {
                g_currentSHP = Shp_Load(data, size);
                free(data);
            }
        } else {
            // Direct file
            g_currentSHP = Shp_LoadFile([node.fullPath UTF8String]);
        }

        if (g_currentSHP) {
            NSLog(@"Loaded SHP: %@ (%d frames)", node.name,
                  Shp_GetFrameCount(g_currentSHP));
        }
    } else if (node.assetType == ViewerAssetType::AudAudio) {
        loadAUD(node);
    } else if (node.assetType == ViewerAssetType::VqaVideo) {
        loadVQA(node);
    }
}

// Draw checkerboard background pattern
static void drawCheckerboard() {
    const int CHECKER_SIZE = 8;  // 8x8 pixel squares
    uint8_t light = 96;   // Light gray palette index (we'll set it up)
    uint8_t dark = 64;    // Dark gray palette index

    // Get framebuffer
    uint8_t* fb = Renderer_GetFramebuffer();
    if (!fb) return;

    for (int y = 0; y < 400; y++) {
        for (int x = 0; x < 640; x++) {
            int cx = x / CHECKER_SIZE;
            int cy = y / CHECKER_SIZE;
            fb[y * 640 + x] = ((cx + cy) & 1) ? light : dark;
        }
    }
}

// Draw speaker silhouette for AUD files
static void drawSpeakerIcon() {
    uint8_t* fb = Renderer_GetFramebuffer();
    if (!fb) return;

    // Center of display area
    int cx = 320;
    int cy = 200;
    int size = 80;  // Icon size

    uint8_t color = 15;  // White in standard palette

    // Speaker body (rectangle)
    int bodyW = size / 3;
    int bodyH = size / 2;
    int bodyX = cx - size/4;
    int bodyY = cy - bodyH/2;
    for (int y = bodyY; y < bodyY + bodyH; y++) {
        if (y < 0 || y >= 400) continue;
        for (int x = bodyX; x < bodyX + bodyW; x++) {
            if (x < 0 || x >= 640) continue;
            fb[y * 640 + x] = color;
        }
    }

    // Speaker cone (trapezoid extending right)
    int coneBaseY = cy - size/2;
    int coneH = size;
    for (int dy = 0; dy < coneH; dy++) {
        int y = coneBaseY + dy;
        if (y < 0 || y >= 400) continue;
        // Width expands from center outward
        float t = (float)dy / (float)(coneH - 1);
        float expand = fabs(t - 0.5f) * 2.0f;  // 0 at center, 1 at edges
        int startX = bodyX + bodyW;
        int endX = startX + (int)(size/2 * (0.3f + 0.7f * expand));
        for (int x = startX; x < endX && x < 640; x++) {
            if (x < 0) continue;
            fb[y * 640 + x] = color;
        }
    }

    // Sound waves (arcs)
    for (int wave = 0; wave < 3; wave++) {
        int radius = size/2 + 15 + wave * 18;
        int waveX = cx + size/4;
        for (int angle = -40; angle <= 40; angle++) {
            float rad = (float)angle * 3.14159f / 180.0f;
            int px = waveX + (int)(cosf(rad) * radius);
            int py = cy + (int)(sinf(rad) * radius);
            if (px >= 0 && px < 640 && py >= 0 && py < 400) {
                fb[py * 640 + px] = color;
                // Thicken the wave
                if (px+1 < 640) fb[py * 640 + px + 1] = color;
            }
        }
    }
}

// Update preview display
static void updatePreview() {
    if (!g_metalView) return;

    // Draw background based on mode
    switch (g_previewBg) {
        case BG_BLACK:
            Renderer_Clear(0);
            break;
        case BG_GRAY:
            Renderer_Clear(96);  // Medium gray
            break;
        case BG_CHECKER:
            drawCheckerboard();
            break;
    }

    if (g_currentSHP && g_paletteLoaded) {
        int frameCount = Shp_GetFrameCount(g_currentSHP);
        if (frameCount > 0) {
            int frame = g_currentFrame % frameCount;
            const ShpFrame* sf = Shp_GetFrame(g_currentSHP, frame);
            if (sf && sf->pixels) {
                int scaledW = sf->width * g_zoomLevel;
                int scaledH = sf->height * g_zoomLevel;

                // Center in preview area
                int x = (640 - scaledW) / 2;
                int y = (400 - scaledH) / 2;

                if (g_zoomLevel == 1) {
                    // 1x: Use standard blit
                    Renderer_Blit(sf->pixels, sf->width, sf->height, x, y, TRUE);
                } else {
                    // Zoomed: manual pixel scaling via framebuffer
                    uint8_t* fb = Renderer_GetFramebuffer();
                    if (fb) {
                        for (int dy = 0; dy < scaledH; dy++) {
                            int srcY = dy / g_zoomLevel;
                            int dstY = y + dy;
                            if (dstY < 0 || dstY >= 400) continue;
                            for (int dx = 0; dx < scaledW; dx++) {
                                int srcX = dx / g_zoomLevel;
                                int dstX = x + dx;
                                if (dstX < 0 || dstX >= 640) continue;
                                uint8_t pixel = sf->pixels[srcY * sf->width + srcX];
                                if (pixel != 0) {  // Transparency
                                    fb[dstY * 640 + dstX] = pixel;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else if (g_currentVQA && g_currentVQA->IsLoaded()) {
        // Render VQA frame
        const uint8_t* vqaFrame = g_currentVQA->GetFrameBuffer();
        const uint8_t* vqaPal = g_currentVQA->GetPalette();
        if (vqaFrame && vqaPal) {
            int w = g_currentVQA->GetWidth();
            int h = g_currentVQA->GetHeight();

            // Update renderer palette from VQA palette if it changed
            if (g_currentVQA->PaletteChanged()) {
                Palette pal;
                for (int i = 0; i < 256; i++) {
                    pal.colors[i][0] = vqaPal[i * 3 + 0];  // R
                    pal.colors[i][1] = vqaPal[i * 3 + 1];  // G
                    pal.colors[i][2] = vqaPal[i * 3 + 2];  // B
                }
                Renderer_SetPalette(&pal);
            }

            int scaledW = w * g_zoomLevel;
            int scaledH = h * g_zoomLevel;

            // Center in preview area
            int x = (640 - scaledW) / 2;
            int y = (400 - scaledH) / 2;

            if (g_zoomLevel == 1) {
                Renderer_Blit(vqaFrame, w, h, x, y, FALSE);
            } else {
                // Zoomed: manual pixel scaling
                uint8_t* fb = Renderer_GetFramebuffer();
                if (fb) {
                    for (int dy = 0; dy < scaledH; dy++) {
                        int srcY = dy / g_zoomLevel;
                        int dstY = y + dy;
                        if (dstY < 0 || dstY >= 400) continue;
                        for (int dx = 0; dx < scaledW; dx++) {
                            int srcX = dx / g_zoomLevel;
                            int dstX = x + dx;
                            if (dstX < 0 || dstX >= 640) continue;
                            fb[dstY * 640 + dstX] = vqaFrame[srcY * w + srcX];
                        }
                    }
                }
            }
        }
    } else if (g_currentAUD) {
        // Draw speaker icon for AUD files
        drawSpeakerIcon();
    }

    Renderer_Present();
}

// Animation timer callback
static void animationTick() {
    if (g_animating && g_currentSHP) {
        g_currentFrame++;
        updatePreview();
    }

    // VQA playback - advance frame if playing
    if (g_vqaPlaying && g_currentVQA && g_currentVQA->IsLoaded()) {
        if (!g_currentVQA->NextFrame()) {
            // End of video
            g_vqaPlaying = false;
            g_currentVQA->Stop();
        }
        updatePreview();
    }
}

// Tab 2: Category browser delegate for systematic asset review
@interface CategoryOutlineDelegate : NSObject <NSOutlineViewDataSource,
                                               NSOutlineViewDelegate>
@end

@implementation CategoryOutlineDelegate

- (NSInteger)outlineView:(NSOutlineView*)ov numberOfChildrenOfItem:(id)item {
    if (item == nil) {
        // Root level: show categories
        return (NSInteger)g_categories.size();
    }

    // Check if item is a category (NSNumber index)
    if ([item isKindOfClass:[NSNumber class]]) {
        NSInteger catIdx = [item integerValue];
        if (catIdx >= 0 && catIdx < (NSInteger)g_categories.size()) {
            return (NSInteger)g_categories[catIdx].assets.size();
        }
    }
    return 0;
}

- (id)outlineView:(NSOutlineView*)ov child:(NSInteger)index ofItem:(id)item {
    if (item == nil) {
        // Return category index wrapped in NSNumber
        return @(index);
    }

    if ([item isKindOfClass:[NSNumber class]]) {
        // Return the asset node directly (it's an Objective-C object)
        NSInteger catIdx = [item integerValue];
        if (catIdx >= 0 && catIdx < (NSInteger)g_categories.size()) {
            return g_categories[catIdx].assets[index];
        }
    }
    return nil;
}

- (BOOL)outlineView:(NSOutlineView*)ov isItemExpandable:(id)item {
    if ([item isKindOfClass:[NSNumber class]]) {
        NSInteger catIdx = [item integerValue];
        if (catIdx >= 0 && catIdx < (NSInteger)g_categories.size()) {
            return !g_categories[catIdx].assets.empty();
        }
    }
    return NO;
}

- (id)outlineView:(NSOutlineView*)ov
    objectValueForTableColumn:(NSTableColumn*)col byItem:(id)item {

    NSString* ident = col.identifier;

    // Category row
    if ([item isKindOfClass:[NSNumber class]]) {
        NSInteger catIdx = [item integerValue];
        if (catIdx >= 0 && catIdx < (NSInteger)g_categories.size()) {
            auto& cat = g_categories[catIdx];
            if ([ident isEqualToString:@"name"]) {
                return [NSString stringWithFormat:@"%s (%zu)",
                        cat.name, cat.assets.size()];
            } else if ([ident isEqualToString:@"type"]) {
                return @"Category";
            } else if ([ident isEqualToString:@"status"]) {
                // Count reviewed in category
                int reviewed = 0;
                for (auto* asset : cat.assets) {
                    std::string key = [asset.fullPath UTF8String];
                    auto it = g_reviewStatus.find(key);
                    if (it != g_reviewStatus.end() && it->second != PENDING)
                        reviewed++;
                }
                return [NSString stringWithFormat:@"%d/%zu",
                        reviewed, cat.assets.size()];
            }
        }
        return @"";
    }

    // Asset row
    FileTreeNode* node = (FileTreeNode*)item;
    if ([ident isEqualToString:@"name"]) {
        return node.name;
    } else if ([ident isEqualToString:@"type"]) {
        return [NSString stringWithUTF8String:getTypeName(node.assetType)];
    } else if ([ident isEqualToString:@"status"]) {
        std::string key = [node.fullPath UTF8String];
        auto it = g_reviewStatus.find(key);
        if (it != g_reviewStatus.end()) {
            if (it->second == GOOD) return @"GOOD";
            if (it->second == BAD) return @"BAD";
        }
        return @"pending";
    }
    return @"";
}

- (void)outlineViewSelectionDidChange:(NSNotification*)notif {
    NSOutlineView* ov = notif.object;
    NSInteger row = [ov selectedRow];
    if (row < 0) return;

    id item = [ov itemAtRow:row];

    // If category selected, find its index
    if ([item isKindOfClass:[NSNumber class]]) {
        g_currentCategory = (int)[item integerValue];
        g_currentCategoryAsset = 0;

        // Update info label with category description
        if (g_currentCategory >= 0 &&
            g_currentCategory < (int)g_categories.size()) {
            auto& cat = g_categories[g_currentCategory];
            if (g_reviewInfoLabel) {
                [g_reviewInfoLabel setStringValue:
                    [NSString stringWithFormat:@"%s\n\n%zu assets",
                     cat.description, cat.assets.size()]];
            }
            if (g_reviewNameLabel) {
                [g_reviewNameLabel setStringValue:
                    [NSString stringWithUTF8String:cat.name]];
            }
        }

        // Load first asset in category if available
        if (!g_categories[g_currentCategory].assets.empty()) {
            loadCategoryAsset();
            updatePreview();
        }
        return;
    }

    // Asset selected - find which category it's in
    FileTreeNode* node = (FileTreeNode*)item;
    for (size_t catIdx = 0; catIdx < g_categories.size(); catIdx++) {
        auto& cat = g_categories[catIdx];
        for (size_t assetIdx = 0; assetIdx < cat.assets.size(); assetIdx++) {
            if (cat.assets[assetIdx] == node) {
                g_currentCategory = (int)catIdx;
                g_currentCategoryAsset = (int)assetIdx;
                loadCategoryAsset();
                updatePreview();
                return;
            }
        }
    }
}

@end

// Helper to get unique capability categories
static std::vector<std::string> getCapabilityCategories() {
    std::vector<std::string> cats;
    std::set<std::string> seen;
    for (const auto& cap : g_capabilities) {
        if (seen.find(cap.category) == seen.end()) {
            cats.push_back(cap.category);
            seen.insert(cap.category);
        }
    }
    return cats;
}

// Get capabilities for a category
static std::vector<size_t> getCapabilitiesInCategory(const std::string& cat) {
    std::vector<size_t> indices;
    for (size_t i = 0; i < g_capabilities.size(); i++) {
        if (g_capabilities[i].category == cat) {
            indices.push_back(i);
        }
    }
    return indices;
}

// Tab 2: Capability Checklist delegate
@interface CapabilityOutlineDelegate : NSObject <NSOutlineViewDataSource,
                                                  NSOutlineViewDelegate>
@end

@implementation CapabilityOutlineDelegate

- (NSInteger)outlineView:(NSOutlineView*)ov numberOfChildrenOfItem:(id)item {
    if (item == nil) {
        // Root level: show capability categories
        return (NSInteger)getCapabilityCategories().size();
    }

    // Check if item is a category (NSString)
    if ([item isKindOfClass:[NSString class]]) {
        std::string cat = [item UTF8String];
        return (NSInteger)getCapabilitiesInCategory(cat).size();
    }
    return 0;
}

- (id)outlineView:(NSOutlineView*)ov child:(NSInteger)index ofItem:(id)item {
    if (item == nil) {
        // Return category name wrapped in NSString
        auto cats = getCapabilityCategories();
        if (index < (NSInteger)cats.size()) {
            return [NSString stringWithUTF8String:cats[index].c_str()];
        }
        return nil;
    }

    if ([item isKindOfClass:[NSString class]]) {
        // Return capability index wrapped in NSNumber
        std::string cat = [item UTF8String];
        auto indices = getCapabilitiesInCategory(cat);
        if (index < (NSInteger)indices.size()) {
            return @(indices[index]);
        }
    }
    return nil;
}

- (BOOL)outlineView:(NSOutlineView*)ov isItemExpandable:(id)item {
    if ([item isKindOfClass:[NSString class]]) {
        std::string cat = [item UTF8String];
        return !getCapabilitiesInCategory(cat).empty();
    }
    return NO;
}

- (id)outlineView:(NSOutlineView*)ov
    objectValueForTableColumn:(NSTableColumn*)col byItem:(id)item {

    NSString* ident = col.identifier;

    // Category row
    if ([item isKindOfClass:[NSString class]]) {
        std::string cat = [item UTF8String];
        auto indices = getCapabilitiesInCategory(cat);

        if ([ident isEqualToString:@"name"]) {
            return [NSString stringWithFormat:@"%s (%zu)",
                    cat.c_str(), indices.size()];
        } else if ([ident isEqualToString:@"status"]) {
            // Count status for category
            int working = 0, broken = 0, untested = 0;
            for (size_t idx : indices) {
                auto& cap = g_capabilities[idx];
                // Check persisted status first
                auto it = g_capabilityStatus.find(cap.id);
                CapabilityStatus st = (it != g_capabilityStatus.end())
                                       ? it->second : cap.status;
                if (st == CAP_WORKING) working++;
                else if (st == CAP_BROKEN) broken++;
                else untested++;
            }
            return [NSString stringWithFormat:@"%d/%d/%d",
                    working, broken, untested];
        }
        return @"";
    }

    // Capability row (NSNumber with index)
    if ([item isKindOfClass:[NSNumber class]]) {
        NSInteger idx = [item integerValue];
        if (idx >= 0 && idx < (NSInteger)g_capabilities.size()) {
            auto& cap = g_capabilities[idx];

            // Get status (check persisted first)
            auto it = g_capabilityStatus.find(cap.id);
            CapabilityStatus st = (it != g_capabilityStatus.end())
                                   ? it->second : cap.status;

            if ([ident isEqualToString:@"name"]) {
                return [NSString stringWithUTF8String:cap.name];
            } else if ([ident isEqualToString:@"status"]) {
                if (st == CAP_WORKING) return @"WORKING";
                if (st == CAP_BROKEN) return @"BROKEN";
                return @"untested";
            }
        }
    }
    return @"";
}

- (void)outlineViewSelectionDidChange:(NSNotification*)notif {
    NSOutlineView* ov = notif.object;
    NSInteger row = [ov selectedRow];
    if (row < 0) return;

    id item = [ov itemAtRow:row];

    // If category selected, show category info
    if ([item isKindOfClass:[NSString class]]) {
        std::string cat = [item UTF8String];
        auto indices = getCapabilitiesInCategory(cat);

        // Update labels with category overview
        if (g_reviewNameLabel) {
            [g_reviewNameLabel setStringValue:
                [NSString stringWithUTF8String:cat.c_str()]];
        }
        if (g_reviewInfoLabel) {
            int working = 0, broken = 0, untested = 0;
            for (size_t idx : indices) {
                auto it = g_capabilityStatus.find(g_capabilities[idx].id);
                CapabilityStatus st = (it != g_capabilityStatus.end())
                                       ? it->second
                                       : g_capabilities[idx].status;
                if (st == CAP_WORKING) working++;
                else if (st == CAP_BROKEN) broken++;
                else untested++;
            }
            [g_reviewInfoLabel setStringValue:
                [NSString stringWithFormat:
                    @"%zu capabilities\n\nWorking: %d\nBroken: %d\n"
                    @"Untested: %d",
                    indices.size(), working, broken, untested]];
        }
        return;
    }

    // Capability selected - show details
    if ([item isKindOfClass:[NSNumber class]]) {
        NSInteger idx = [item integerValue];
        if (idx >= 0 && idx < (NSInteger)g_capabilities.size()) {
            g_currentCapability = (int)idx;
            auto& cap = g_capabilities[idx];

            // Get status
            auto it = g_capabilityStatus.find(cap.id);
            CapabilityStatus st = (it != g_capabilityStatus.end())
                                   ? it->second : cap.status;

            if (g_reviewNameLabel) {
                [g_reviewNameLabel setStringValue:
                    [NSString stringWithUTF8String:cap.name]];
            }
            if (g_reviewInfoLabel) {
                NSString* statusStr = @"UNTESTED";
                if (st == CAP_WORKING) statusStr = @"WORKING";
                else if (st == CAP_BROKEN) statusStr = @"BROKEN";

                // Get notes
                auto notesIt = g_capabilityNotes.find(cap.id);
                NSString* notes = (notesIt != g_capabilityNotes.end())
                    ? [NSString stringWithUTF8String:notesIt->second.c_str()]
                    : @"(no notes)";

                [g_reviewInfoLabel setStringValue:
                    [NSString stringWithFormat:
                        @"Category: %s\nStatus: %@\n\n%s\n\n"
                        @"Test hint: %s\n\nNotes: %@",
                        cap.category, statusStr, cap.description,
                        cap.testHint, notes]];
            }
            if (g_reviewStatusLabel) {
                if (st == CAP_WORKING) {
                    [g_reviewStatusLabel setStringValue:@"WORKING"];
                } else if (st == CAP_BROKEN) {
                    [g_reviewStatusLabel setStringValue:@"BROKEN"];
                } else {
                    [g_reviewStatusLabel setStringValue:@"UNTESTED"];
                }
            }
        }
    }
}

@end

// Tab 1: File Browser delegate
@interface FileBrowserDelegate : NSObject <NSOutlineViewDataSource,
                                           NSOutlineViewDelegate>
@end

@implementation FileBrowserDelegate

- (NSInteger)outlineView:(NSOutlineView*)ov numberOfChildrenOfItem:(id)item {
    if (item == nil) {
        return g_rootNode ? (NSInteger)g_rootNode.children.count : 0;
    }
    FileTreeNode* node = (FileTreeNode*)item;
    return (NSInteger)node.children.count;
}

- (id)outlineView:(NSOutlineView*)ov child:(NSInteger)index ofItem:(id)item {
    if (item == nil) {
        return g_rootNode.children[index];
    }
    FileTreeNode* node = (FileTreeNode*)item;
    return node.children[index];
}

- (BOOL)outlineView:(NSOutlineView*)ov isItemExpandable:(id)item {
    FileTreeNode* node = (FileTreeNode*)item;
    return node.isContainer;
}

- (id)outlineView:(NSOutlineView*)ov
    objectValueForTableColumn:(NSTableColumn*)col byItem:(id)item {
    FileTreeNode* node = (FileTreeNode*)item;

    NSString* ident = col.identifier;
    if ([ident isEqualToString:@"name"]) {
        return node.name;
    } else if ([ident isEqualToString:@"type"]) {
        return [NSString stringWithUTF8String:getTypeName(node.assetType)];
    } else if ([ident isEqualToString:@"size"]) {
        if (node.size > 1024 * 1024) {
            return [NSString stringWithFormat:@"%.1f MB",
                   node.size / (1024.0 * 1024.0)];
        } else if (node.size > 1024) {
            return [NSString stringWithFormat:@"%.1f KB",
                   node.size / 1024.0];
        }
        return [NSString stringWithFormat:@"%u B", node.size];
    }
    return @"";
}

- (void)outlineViewSelectionDidChange:(NSNotification*)notif {
    NSOutlineView* ov = notif.object;
    NSInteger row = [ov selectedRow];
    if (row < 0) return;

    FileTreeNode* node = [ov itemAtRow:row];
    if (!node) return;

    // Update info label
    if (g_assetInfoLabel) {
        NSString* info = [NSString stringWithFormat:
            @"Name: %@\nType: %s\nSize: %u bytes\nCRC: %08X\n"
            @"MIX depth: %lu",
            node.name, getTypeName(node.assetType), node.size, node.crc,
            (unsigned long)node.mixChain.count];
        [g_assetInfoLabel setStringValue:info];
    }

    // Also load the asset for preview (SHP, AUD, VQA, etc.)
    stopAudio();
    stopVQA();
    if (g_currentSHP) {
        Shp_Free(g_currentSHP);
        g_currentSHP = nullptr;
    }
    g_currentFrame = 0;

    if (node.assetType == ViewerAssetType::ShpSprite) {
        if (node.mixChain.count > 0) {
            uint32_t size;
            void* data = extractFromMixChain(node, &size);
            if (data) {
                g_currentSHP = Shp_Load(data, size);
                free(data);
            }
        } else {
            g_currentSHP = Shp_LoadFile([node.fullPath UTF8String]);
        }
        if (g_currentSHP) {
            NSLog(@"File browser loaded SHP: %@ (%d frames)",
                  node.name, Shp_GetFrameCount(g_currentSHP));
        }
    } else if (node.assetType == ViewerAssetType::AudAudio) {
        loadAUD(node);
    } else if (node.assetType == ViewerAssetType::VqaVideo) {
        loadVQA(node);
    }

    updatePreview();
}

@end

// Metal view delegate
@interface PreviewViewDelegate : NSObject <MTKViewDelegate>
@end

@implementation PreviewViewDelegate

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
}

- (void)drawInMTKView:(MTKView*)view {
    (void)view;
    Renderer_Present();
}

@end

// Main app delegate
@interface AssetViewerDelegate : NSObject <NSApplicationDelegate,
                                           NSWindowDelegate>
@property (strong) NSWindow* window;
@property (strong) FileBrowserDelegate* browserDelegate;
@property (strong) PreviewViewDelegate* previewDelegate;
@end

@implementation AssetViewerDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notif {
    // Load filename database - try multiple paths
    NSArray* dbPaths = @[
        @"data/filenames.dat",
        @"../data/filenames.dat",
        [[NSBundle mainBundle] pathForResource:@"filenames" ofType:@"dat"]
            ?: @""
    ];

    for (NSString* dbPath in dbPaths) {
        if (dbPath.length > 0) {
            g_filenameDB.loadFromFile([dbPath UTF8String]);
        }
    }

    // Create window
    NSRect frame = NSMakeRect(100, 100, 1200, 800);
    NSWindowStyleMask style = NSWindowStyleMaskTitled |
                              NSWindowStyleMaskClosable |
                              NSWindowStyleMaskResizable |
                              NSWindowStyleMaskMiniaturizable;

    _window = [[NSWindow alloc] initWithContentRect:frame
                                          styleMask:style
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    [_window setTitle:@"Red Alert Asset Viewer"];
    [_window setDelegate:self];
    [_window setMinSize:NSMakeSize(800, 600)];

    NSView* content = [_window contentView];

    // Create tab view
    g_tabView = [[NSTabView alloc] initWithFrame:content.bounds];
    [g_tabView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [content addSubview:g_tabView];

    // Tab 1: File Browser
    NSTabViewItem* browserTab = [[NSTabViewItem alloc]
                                  initWithIdentifier:@"browser"];
    [browserTab setLabel:@"File Browser"];
    NSView* browserView = [[NSView alloc]
                            initWithFrame:NSMakeRect(0, 0, 1180, 740)];
    [browserTab setView:browserView];
    [self setupBrowserTab:browserView];
    [g_tabView addTabViewItem:browserTab];

    // Tab 2: Capability Checklist
    NSTabViewItem* capabilityTab = [[NSTabViewItem alloc]
                                     initWithIdentifier:@"capability"];
    [capabilityTab setLabel:@"Capability Checklist"];
    NSView* capabilityView = [[NSView alloc]
                               initWithFrame:NSMakeRect(0, 0, 1180, 740)];
    [capabilityTab setView:capabilityView];
    [self setupCapabilityTab:capabilityView];
    [g_tabView addTabViewItem:capabilityTab];

    // Tab 3: Asset Test (for testing capabilities with files)
    NSTabViewItem* testTab = [[NSTabViewItem alloc]
                               initWithIdentifier:@"test"];
    [testTab setLabel:@"Asset Test"];
    NSView* testView = [[NSView alloc]
                         initWithFrame:NSMakeRect(0, 0, 1180, 740)];
    [testTab setView:testView];
    [self setupReviewTab:testView];
    [g_tabView addTabViewItem:testTab];

    // Initialize renderer
    if (g_metalView) {
        _previewDelegate = [[PreviewViewDelegate alloc] init];
        g_metalView.delegate = _previewDelegate;
        Renderer_Init((__bridge void*)g_metalView);
    }

    // Start animation timer (~15 FPS for VQA compatibility)
    g_animTimer = [NSTimer scheduledTimerWithTimeInterval:0.067
                                                  repeats:YES
                                                    block:^(NSTimer* t) {
        animationTick();
    }];

    [_window makeKeyAndOrderFront:nil];

    NSLog(@"Asset Viewer ready.");
}

- (void)setupBrowserTab:(NSView*)tabView {
    CGFloat w = tabView.bounds.size.width;
    CGFloat h = tabView.bounds.size.height;
    CGFloat topBarHeight = 45;

    // Top bar with path and browse button (at top of view)
    NSTextField* pathTitle = [[NSTextField alloc]
                              initWithFrame:NSMakeRect(10, h - 30, 80, 20)];
    [pathTitle setStringValue:@"Root Path:"];
    [pathTitle setBezeled:NO];
    [pathTitle setEditable:NO];
    [pathTitle setDrawsBackground:NO];
    [pathTitle setAutoresizingMask:NSViewMinYMargin];
    [tabView addSubview:pathTitle];

    g_pathLabel = [[NSTextField alloc]
                   initWithFrame:NSMakeRect(95, h - 30, w - 200, 20)];
    [g_pathLabel setStringValue:@"(No directory selected)"];
    [g_pathLabel setBezeled:YES];
    [g_pathLabel setEditable:NO];
    [g_pathLabel setAutoresizingMask:NSViewWidthSizable | NSViewMinYMargin];
    [tabView addSubview:g_pathLabel];

    NSButton* browseBtn = [[NSButton alloc]
                           initWithFrame:NSMakeRect(w - 95, h - 33, 85, 26)];
    [browseBtn setTitle:@"Browse..."];
    [browseBtn setBezelStyle:NSBezelStyleRounded];
    [browseBtn setTarget:self];
    [browseBtn setAction:@selector(browseClicked:)];
    [browseBtn setAutoresizingMask:NSViewMinXMargin | NSViewMinYMargin];
    [tabView addSubview:browseBtn];

    // Split view: outline on left, info on right (below the top bar)
    NSSplitView* split = [[NSSplitView alloc]
                          initWithFrame:NSMakeRect(0, 0, w, h - topBarHeight)];
    [split setVertical:YES];
    [split setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [tabView addSubview:split];

    // Outline view in scroll view
    NSScrollView* scrollView = [[NSScrollView alloc]
                                initWithFrame:NSMakeRect(0, 0, w * 0.6, h-45)];
    [scrollView setHasVerticalScroller:YES];
    [scrollView setHasHorizontalScroller:YES];
    [scrollView setAutohidesScrollers:YES];

    g_outlineView = [[NSOutlineView alloc] initWithFrame:scrollView.bounds];
    [g_outlineView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    // Columns
    NSTableColumn* nameCol = [[NSTableColumn alloc]
                              initWithIdentifier:@"name"];
    [nameCol.headerCell setStringValue:@"Name"];
    [nameCol setWidth:300];
    [g_outlineView addTableColumn:nameCol];

    NSTableColumn* typeCol = [[NSTableColumn alloc]
                              initWithIdentifier:@"type"];
    [typeCol.headerCell setStringValue:@"Type"];
    [typeCol setWidth:100];
    [g_outlineView addTableColumn:typeCol];

    NSTableColumn* sizeCol = [[NSTableColumn alloc]
                              initWithIdentifier:@"size"];
    [sizeCol.headerCell setStringValue:@"Size"];
    [sizeCol setWidth:80];
    [g_outlineView addTableColumn:sizeCol];

    [g_outlineView setOutlineTableColumn:nameCol];

    _browserDelegate = [[FileBrowserDelegate alloc] init];
    [g_outlineView setDataSource:_browserDelegate];
    [g_outlineView setDelegate:_browserDelegate];

    [scrollView setDocumentView:g_outlineView];
    [split addSubview:scrollView];

    // Info panel on right
    NSView* infoPanel = [[NSView alloc]
                         initWithFrame:NSMakeRect(0, 0, w * 0.4, h - 45)];

    NSTextField* infoTitle = [[NSTextField alloc]
                              initWithFrame:NSMakeRect(10, h - 80, 200, 20)];
    [infoTitle setStringValue:@"Asset Information"];
    [infoTitle setFont:[NSFont boldSystemFontOfSize:14]];
    [infoTitle setBezeled:NO];
    [infoTitle setEditable:NO];
    [infoTitle setDrawsBackground:NO];
    [infoPanel addSubview:infoTitle];

    g_assetInfoLabel = [[NSTextField alloc]
                        initWithFrame:NSMakeRect(10, h - 220, 280, 120)];
    [g_assetInfoLabel setStringValue:@"Select an asset to view details"];
    [g_assetInfoLabel setBezeled:NO];
    [g_assetInfoLabel setEditable:NO];
    [g_assetInfoLabel setDrawsBackground:NO];
    [infoPanel addSubview:g_assetInfoLabel];

    [split addSubview:infoPanel];
    [split setPosition:w * 0.65 ofDividerAtIndex:0];
}

- (void)setupCapabilityTab:(NSView*)tabView {
    CGFloat w = tabView.bounds.size.width;
    CGFloat h = tabView.bounds.size.height;

    // Main horizontal split: capability list on left, details on right
    NSSplitView* mainSplit = [[NSSplitView alloc]
                               initWithFrame:NSMakeRect(0, 0, w, h)];
    [mainSplit setVertical:YES];
    [mainSplit setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [tabView addSubview:mainSplit];

    // Left panel: Capability outline view
    NSView* leftPanel = [[NSView alloc]
                         initWithFrame:NSMakeRect(0, 0, 450, h)];
    [leftPanel setAutoresizingMask:NSViewHeightSizable];
    [mainSplit addSubview:leftPanel];

    // Title for capabilities
    NSTextField* capTitle = [[NSTextField alloc]
                             initWithFrame:NSMakeRect(10, h - 30, 430, 20)];
    [capTitle setStringValue:
        @"Capability Checklist (Working/Broken/Untested)"];
    [capTitle setFont:[NSFont boldSystemFontOfSize:14]];
    [capTitle setBezeled:NO];
    [capTitle setEditable:NO];
    [capTitle setDrawsBackground:NO];
    [capTitle setAutoresizingMask:NSViewMinYMargin];
    [leftPanel addSubview:capTitle];

    // Capability outline view in scroll view
    NSScrollView* capScrollView = [[NSScrollView alloc]
                                   initWithFrame:NSMakeRect(0, 0, 450, h - 40)];
    [capScrollView setHasVerticalScroller:YES];
    [capScrollView setHasHorizontalScroller:YES];
    [capScrollView setAutohidesScrollers:YES];
    [capScrollView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    g_capabilityOutline = [[NSOutlineView alloc]
                           initWithFrame:capScrollView.bounds];
    [g_capabilityOutline setAutoresizingMask:NSViewWidthSizable|
                                             NSViewHeightSizable];

    // Columns for capability outline
    NSTableColumn* capNameCol = [[NSTableColumn alloc]
                                  initWithIdentifier:@"name"];
    [capNameCol.headerCell setStringValue:@"Capability"];
    [capNameCol setWidth:320];
    [g_capabilityOutline addTableColumn:capNameCol];

    NSTableColumn* capStatusCol = [[NSTableColumn alloc]
                                    initWithIdentifier:@"status"];
    [capStatusCol.headerCell setStringValue:@"Status"];
    [capStatusCol setWidth:100];
    [g_capabilityOutline addTableColumn:capStatusCol];

    [g_capabilityOutline setOutlineTableColumn:capNameCol];

    g_capabilityDelegate = [[CapabilityOutlineDelegate alloc] init];
    [g_capabilityOutline setDataSource:g_capabilityDelegate];
    [g_capabilityOutline setDelegate:g_capabilityDelegate];

    [capScrollView setDocumentView:g_capabilityOutline];
    [leftPanel addSubview:capScrollView];

    // Right panel: details and controls
    NSView* rightPanel = [[NSView alloc]
                          initWithFrame:NSMakeRect(0, 0, w - 460, h)];
    [rightPanel setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [mainSplit addSubview:rightPanel];

    [mainSplit setPosition:450 ofDividerAtIndex:0];

    CGFloat rw = w - 460;
    CGFloat y = h - 40;

    // Capability name
    NSTextField* capNameLabel = [[NSTextField alloc]
                                  initWithFrame:NSMakeRect(10, y, rw - 20, 24)];
    [capNameLabel setStringValue:@"(Select a capability)"];
    [capNameLabel setFont:[NSFont boldSystemFontOfSize:16]];
    [capNameLabel setBezeled:NO];
    [capNameLabel setEditable:NO];
    [capNameLabel setDrawsBackground:NO];
    [capNameLabel setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [rightPanel addSubview:capNameLabel];
    // Note: We'll reuse g_reviewNameLabel for this
    y -= 30;

    // Status label
    NSTextField* statusTitle = [[NSTextField alloc]
                                initWithFrame:NSMakeRect(10, y, 60, 20)];
    [statusTitle setStringValue:@"Status:"];
    [statusTitle setBezeled:NO];
    [statusTitle setEditable:NO];
    [statusTitle setDrawsBackground:NO];
    [rightPanel addSubview:statusTitle];

    NSTextField* statusValue = [[NSTextField alloc]
                                initWithFrame:NSMakeRect(70, y, 100, 20)];
    [statusValue setStringValue:@"UNTESTED"];
    [statusValue setFont:[NSFont boldSystemFontOfSize:12]];
    [statusValue setBezeled:NO];
    [statusValue setEditable:NO];
    [statusValue setDrawsBackground:NO];
    [rightPanel addSubview:statusValue];

    // Status buttons
    NSButton* workingBtn = [[NSButton alloc]
                            initWithFrame:NSMakeRect(180, y - 3, 100, 28)];
    [workingBtn setTitle:@"WORKING"];
    [workingBtn setBezelStyle:NSBezelStyleRounded];
    [workingBtn setTarget:self];
    [workingBtn setAction:@selector(markCapabilityWorking:)];
    [rightPanel addSubview:workingBtn];

    NSButton* brokenBtn = [[NSButton alloc]
                           initWithFrame:NSMakeRect(290, y - 3, 100, 28)];
    [brokenBtn setTitle:@"BROKEN"];
    [brokenBtn setBezelStyle:NSBezelStyleRounded];
    [brokenBtn setTarget:self];
    [brokenBtn setAction:@selector(markCapabilityBroken:)];
    [rightPanel addSubview:brokenBtn];

    NSButton* untestedBtn = [[NSButton alloc]
                             initWithFrame:NSMakeRect(400, y - 3, 100, 28)];
    [untestedBtn setTitle:@"UNTESTED"];
    [untestedBtn setBezelStyle:NSBezelStyleRounded];
    [untestedBtn setTarget:self];
    [untestedBtn setAction:@selector(markCapabilityUntested:)];
    [rightPanel addSubview:untestedBtn];
    y -= 50;

    // Capability details (description, test hint, notes)
    NSTextField* detailsLabel = [[NSTextField alloc]
                                 initWithFrame:NSMakeRect(10, y - 200, rw - 20, 200)];
    [detailsLabel setStringValue:
        @"Select a capability from the list to see details.\n\n"
        @"Mark capabilities as WORKING, BROKEN, or UNTESTED as you test.\n\n"
        @"Use the Asset Test tab to test specific file types."];
    [detailsLabel setBezeled:YES];
    [detailsLabel setEditable:NO];
    [detailsLabel setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [rightPanel addSubview:detailsLabel];
    y -= 220;

    // Notes field
    NSTextField* notesTitle = [[NSTextField alloc]
                               initWithFrame:NSMakeRect(10, y, 60, 20)];
    [notesTitle setStringValue:@"Notes:"];
    [notesTitle setBezeled:NO];
    [notesTitle setEditable:NO];
    [notesTitle setDrawsBackground:NO];
    [rightPanel addSubview:notesTitle];
    y -= 25;

    NSTextField* notesField = [[NSTextField alloc]
                               initWithFrame:NSMakeRect(10, y - 60, rw - 20, 60)];
    [notesField setStringValue:@""];
    [notesField setBezeled:YES];
    [notesField setEditable:YES];
    [notesField setAutoresizingMask:NSViewWidthSizable];
    [notesField setTarget:self];
    [notesField setAction:@selector(notesChanged:)];
    [rightPanel addSubview:notesField];

    // Summary at bottom (fixed position)
    NSTextField* summaryLabel = [[NSTextField alloc]
                                 initWithFrame:NSMakeRect(10, 10, rw - 20, 40)];
    [summaryLabel setStringValue:
        [NSString stringWithFormat:
            @"%zu capabilities | Working: 0 | Broken: 0 | Untested: %zu",
            g_capabilities.size(), g_capabilities.size()]];
    [summaryLabel setBezeled:NO];
    [summaryLabel setEditable:NO];
    [summaryLabel setDrawsBackground:NO];
    [summaryLabel setAutoresizingMask:NSViewWidthSizable];
    [rightPanel addSubview:summaryLabel];

    // Save/Load buttons (positioned above summary)
    NSButton* saveCapBtn = [[NSButton alloc]
                            initWithFrame:NSMakeRect(10, 55, 120, 28)];
    [saveCapBtn setTitle:@"Save Checklist"];
    [saveCapBtn setBezelStyle:NSBezelStyleRounded];
    [saveCapBtn setTarget:self];
    [saveCapBtn setAction:@selector(saveCapabilityChecklist:)];
    [rightPanel addSubview:saveCapBtn];

    NSButton* loadCapBtn = [[NSButton alloc]
                            initWithFrame:NSMakeRect(140, 55, 120, 28)];
    [loadCapBtn setTitle:@"Load Checklist"];
    [loadCapBtn setBezelStyle:NSBezelStyleRounded];
    [loadCapBtn setTarget:self];
    [loadCapBtn setAction:@selector(loadCapabilityChecklist:)];
    [rightPanel addSubview:loadCapBtn];
}

- (void)setupReviewTab:(NSView*)tabView {
    CGFloat w = tabView.bounds.size.width;
    CGFloat h = tabView.bounds.size.height;

    // Main horizontal split: categories on left, preview+controls on right
    NSSplitView* mainSplit = [[NSSplitView alloc]
                               initWithFrame:NSMakeRect(0, 0, w, h)];
    [mainSplit setVertical:YES];
    [mainSplit setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [tabView addSubview:mainSplit];

    // Left panel: Category outline view
    NSView* leftPanel = [[NSView alloc]
                         initWithFrame:NSMakeRect(0, 0, 350, h)];
    [leftPanel setAutoresizingMask:NSViewHeightSizable];
    [mainSplit addSubview:leftPanel];

    // Title for categories
    NSTextField* catTitle = [[NSTextField alloc]
                             initWithFrame:NSMakeRect(10, h - 30, 330, 20)];
    [catTitle setStringValue:@"Asset Categories"];
    [catTitle setFont:[NSFont boldSystemFontOfSize:14]];
    [catTitle setBezeled:NO];
    [catTitle setEditable:NO];
    [catTitle setDrawsBackground:NO];
    [catTitle setAutoresizingMask:NSViewMinYMargin];
    [leftPanel addSubview:catTitle];

    // Category outline view in scroll view
    NSScrollView* catScrollView = [[NSScrollView alloc]
                                   initWithFrame:NSMakeRect(0, 0, 350, h - 40)];
    [catScrollView setHasVerticalScroller:YES];
    [catScrollView setHasHorizontalScroller:YES];
    [catScrollView setAutohidesScrollers:YES];
    [catScrollView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

    g_categoryOutline = [[NSOutlineView alloc]
                         initWithFrame:catScrollView.bounds];
    [g_categoryOutline setAutoresizingMask:NSViewWidthSizable|
                                           NSViewHeightSizable];

    // Columns for category outline
    NSTableColumn* catNameCol = [[NSTableColumn alloc]
                                  initWithIdentifier:@"name"];
    [catNameCol.headerCell setStringValue:@"Name"];
    [catNameCol setWidth:200];
    [g_categoryOutline addTableColumn:catNameCol];

    NSTableColumn* catTypeCol = [[NSTableColumn alloc]
                                  initWithIdentifier:@"type"];
    [catTypeCol.headerCell setStringValue:@"Type"];
    [catTypeCol setWidth:70];
    [g_categoryOutline addTableColumn:catTypeCol];

    NSTableColumn* catStatusCol = [[NSTableColumn alloc]
                                    initWithIdentifier:@"status"];
    [catStatusCol.headerCell setStringValue:@"Status"];
    [catStatusCol setWidth:60];
    [g_categoryOutline addTableColumn:catStatusCol];

    [g_categoryOutline setOutlineTableColumn:catNameCol];

    g_categoryDelegate = [[CategoryOutlineDelegate alloc] init];
    [g_categoryOutline setDataSource:g_categoryDelegate];
    [g_categoryOutline setDelegate:g_categoryDelegate];

    [catScrollView setDocumentView:g_categoryOutline];
    [leftPanel addSubview:catScrollView];

    // Right panel: preview and controls
    NSView* rightPanel = [[NSView alloc]
                          initWithFrame:NSMakeRect(0, 0, w - 360, h)];
    [rightPanel setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [mainSplit addSubview:rightPanel];

    [mainSplit setPosition:350 ofDividerAtIndex:0];

    CGFloat rw = w - 360;

    // ========== LAYOUT: Labels ABOVE preview, Controls BELOW preview ==========
    // Fixed positions from top down, then controls from bottom up

    // --- TOP SECTION: Labels above preview ---
    CGFloat topY = h - 20;

    // Asset name (top)
    g_reviewNameLabel = [[NSTextField alloc]
                         initWithFrame:NSMakeRect(10, topY - 24, rw - 20, 24)];
    [g_reviewNameLabel setStringValue:@"(Select an asset from categories)"];
    [g_reviewNameLabel setFont:[NSFont boldSystemFontOfSize:16]];
    [g_reviewNameLabel setBezeled:NO];
    [g_reviewNameLabel setEditable:NO];
    [g_reviewNameLabel setDrawsBackground:NO];
    [g_reviewNameLabel setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [rightPanel addSubview:g_reviewNameLabel];

    // Asset info (type, size, frames)
    g_reviewInfoLabel = [[NSTextField alloc]
                         initWithFrame:NSMakeRect(10, topY - 44, rw - 20, 18)];
    [g_reviewInfoLabel setStringValue:@""];
    [g_reviewInfoLabel setBezeled:NO];
    [g_reviewInfoLabel setEditable:NO];
    [g_reviewInfoLabel setDrawsBackground:NO];
    [g_reviewInfoLabel setFont:[NSFont systemFontOfSize:11]];
    [g_reviewInfoLabel setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [rightPanel addSubview:g_reviewInfoLabel];

    // Audio info label
    g_audioInfoLabel = [[NSTextField alloc]
                        initWithFrame:NSMakeRect(10, topY - 62, 310, 16)];
    [g_audioInfoLabel setStringValue:@""];
    [g_audioInfoLabel setBezeled:NO];
    [g_audioInfoLabel setEditable:NO];
    [g_audioInfoLabel setDrawsBackground:NO];
    [g_audioInfoLabel setFont:[NSFont systemFontOfSize:10]];
    [rightPanel addSubview:g_audioInfoLabel];

    // Video info label (same row as audio)
    g_videoInfoLabel = [[NSTextField alloc]
                        initWithFrame:NSMakeRect(330, topY - 62, 310, 16)];
    [g_videoInfoLabel setStringValue:@""];
    [g_videoInfoLabel setBezeled:NO];
    [g_videoInfoLabel setEditable:NO];
    [g_videoInfoLabel setDrawsBackground:NO];
    [g_videoInfoLabel setFont:[NSFont systemFontOfSize:10]];
    [rightPanel addSubview:g_videoInfoLabel];

    // Keyboard shortcuts hint
    NSTextField* keysLabel = [[NSTextField alloc]
                              initWithFrame:NSMakeRect(10, topY - 80, rw - 20, 16)];
    [keysLabel setStringValue:
        @"Keys: ←→ Nav, G=Good, B=Bad, Space=Play/Pause, +/-=Zoom"];
    [keysLabel setBezeled:NO];
    [keysLabel setEditable:NO];
    [keysLabel setDrawsBackground:NO];
    [keysLabel setFont:[NSFont systemFontOfSize:10]];
    [keysLabel setTextColor:[NSColor secondaryLabelColor]];
    [rightPanel addSubview:keysLabel];

    // --- MIDDLE: Preview area (640x400) ---
    CGFloat previewTop = topY - 90;
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    g_metalView = [[MTKView alloc]
                   initWithFrame:NSMakeRect(10, previewTop - 400, 640, 400)
                   device:device];
    [g_metalView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [g_metalView setFramebufferOnly:YES];
    [g_metalView setPreferredFramesPerSecond:60];
    [g_metalView setAutoresizingMask:NSViewMinYMargin | NSViewMaxYMargin];
    g_metalView.layer.borderWidth = 1;
    g_metalView.layer.borderColor = [[NSColor grayColor] CGColor];
    [rightPanel addSubview:g_metalView];

    // --- BOTTOM SECTION: Controls below preview (build from bottom up) ---
    CGFloat ctrlY = 10;  // Start from bottom

    // Bottom row: Save button
    NSButton* saveBtn = [[NSButton alloc]
                         initWithFrame:NSMakeRect(10, ctrlY, 120, 28)];
    [saveBtn setTitle:@"Save Results"];
    [saveBtn setBezelStyle:NSBezelStyleRounded];
    [saveBtn setTarget:self];
    [saveBtn setAction:@selector(saveResults:)];
    [rightPanel addSubview:saveBtn];

    // Progress label (same row as save)
    g_progressLabel = [[NSTextField alloc]
                       initWithFrame:NSMakeRect(140, ctrlY + 6, 400, 18)];
    [g_progressLabel setStringValue:@"Load assets from File Browser tab"];
    [g_progressLabel setBezeled:NO];
    [g_progressLabel setEditable:NO];
    [g_progressLabel setDrawsBackground:NO];
    [g_progressLabel setFont:[NSFont systemFontOfSize:11]];
    [rightPanel addSubview:g_progressLabel];
    ctrlY += 35;

    // Row: Status and review buttons
    NSTextField* statusTitle = [[NSTextField alloc]
                                initWithFrame:NSMakeRect(10, ctrlY + 6, 55, 18)];
    [statusTitle setStringValue:@"Status:"];
    [statusTitle setBezeled:NO];
    [statusTitle setEditable:NO];
    [statusTitle setDrawsBackground:NO];
    [statusTitle setFont:[NSFont systemFontOfSize:12]];
    [rightPanel addSubview:statusTitle];

    g_reviewStatusLabel = [[NSTextField alloc]
                           initWithFrame:NSMakeRect(65, ctrlY + 6, 90, 18)];
    [g_reviewStatusLabel setStringValue:@"PENDING"];
    [g_reviewStatusLabel setFont:[NSFont boldSystemFontOfSize:12]];
    [g_reviewStatusLabel setBezeled:NO];
    [g_reviewStatusLabel setEditable:NO];
    [g_reviewStatusLabel setDrawsBackground:NO];
    [rightPanel addSubview:g_reviewStatusLabel];

    g_goodButton = [[NSButton alloc]
                    initWithFrame:NSMakeRect(160, ctrlY, 80, 28)];
    [g_goodButton setTitle:@"GOOD"];
    [g_goodButton setBezelStyle:NSBezelStyleRounded];
    [g_goodButton setTarget:self];
    [g_goodButton setAction:@selector(markGoodCategory:)];
    [rightPanel addSubview:g_goodButton];

    g_badButton = [[NSButton alloc]
                   initWithFrame:NSMakeRect(250, ctrlY, 80, 28)];
    [g_badButton setTitle:@"BAD"];
    [g_badButton setBezelStyle:NSBezelStyleRounded];
    [g_badButton setTarget:self];
    [g_badButton setAction:@selector(markBadCategory:)];
    [rightPanel addSubview:g_badButton];
    ctrlY += 35;

    // Row: Playback controls
    g_playButton = [[NSButton alloc]
                    initWithFrame:NSMakeRect(10, ctrlY, 80, 28)];
    [g_playButton setTitle:@"Play"];
    [g_playButton setBezelStyle:NSBezelStyleRounded];
    [g_playButton setTarget:self];
    [g_playButton setAction:@selector(playAudio:)];
    [g_playButton setEnabled:NO];
    [rightPanel addSubview:g_playButton];

    g_stopButton = [[NSButton alloc]
                    initWithFrame:NSMakeRect(100, ctrlY, 80, 28)];
    [g_stopButton setTitle:@"Stop"];
    [g_stopButton setBezelStyle:NSBezelStyleRounded];
    [g_stopButton setTarget:self];
    [g_stopButton setAction:@selector(stopAudio:)];
    [g_stopButton setEnabled:NO];
    [rightPanel addSubview:g_stopButton];

    // Navigation on same row
    NSButton* prevBtn = [[NSButton alloc]
                         initWithFrame:NSMakeRect(200, ctrlY, 80, 28)];
    [prevBtn setTitle:@"< Prev"];
    [prevBtn setBezelStyle:NSBezelStyleRounded];
    [prevBtn setTarget:self];
    [prevBtn setAction:@selector(prevCategoryAsset:)];
    [rightPanel addSubview:prevBtn];

    NSButton* nextBtn = [[NSButton alloc]
                         initWithFrame:NSMakeRect(290, ctrlY, 80, 28)];
    [nextBtn setTitle:@"Next >"];
    [nextBtn setBezelStyle:NSBezelStyleRounded];
    [nextBtn setTarget:self];
    [nextBtn setAction:@selector(nextCategoryAsset:)];
    [rightPanel addSubview:nextBtn];
    ctrlY += 35;

    // Row: Zoom controls + BG toggle
    NSButton* zoomOutBtn = [[NSButton alloc]
                            initWithFrame:NSMakeRect(10, ctrlY, 40, 28)];
    [zoomOutBtn setTitle:@"-"];
    [zoomOutBtn setBezelStyle:NSBezelStyleRounded];
    [zoomOutBtn setTarget:self];
    [zoomOutBtn setAction:@selector(zoomOut:)];
    [rightPanel addSubview:zoomOutBtn];

    g_zoomLabel = [[NSTextField alloc]
                   initWithFrame:NSMakeRect(55, ctrlY + 6, 70, 18)];
    [g_zoomLabel setStringValue:@"Zoom: 1x"];
    [g_zoomLabel setBezeled:NO];
    [g_zoomLabel setEditable:NO];
    [g_zoomLabel setDrawsBackground:NO];
    [g_zoomLabel setFont:[NSFont systemFontOfSize:11]];
    [rightPanel addSubview:g_zoomLabel];

    NSButton* zoomInBtn = [[NSButton alloc]
                           initWithFrame:NSMakeRect(125, ctrlY, 40, 28)];
    [zoomInBtn setTitle:@"+"];
    [zoomInBtn setBezelStyle:NSBezelStyleRounded];
    [zoomInBtn setTarget:self];
    [zoomInBtn setAction:@selector(zoomIn:)];
    [rightPanel addSubview:zoomInBtn];

    g_bgToggleButton = [[NSButton alloc]
                        initWithFrame:NSMakeRect(180, ctrlY, 100, 28)];
    [g_bgToggleButton setTitle:@"BG: Check"];
    [g_bgToggleButton setBezelStyle:NSBezelStyleRounded];
    [g_bgToggleButton setTarget:self];
    [g_bgToggleButton setAction:@selector(toggleBackground:)];
    [rightPanel addSubview:g_bgToggleButton];
}

- (void)browseClicked:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setCanChooseDirectories:YES];
    [panel setCanChooseFiles:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setMessage:@"Select root directory containing game assets"];

    if ([panel runModal] == NSModalResponseOK) {
        NSURL* url = panel.URL;
        NSString* path = [url path];

        [g_pathLabel setStringValue:path];

        // Clear existing data
        g_rootNode = nil;
        g_allAssets.clear();
        g_currentAssetIndex = 0;

        // Create root node and scan
        g_rootNode = [[FileTreeNode alloc] init];
        g_rootNode.name = [path lastPathComponent];
        g_rootNode.fullPath = path;
        g_rootNode.assetType = ViewerAssetType::Directory;
        g_rootNode.isContainer = YES;

        // Perform scan
        NSLog(@"Scanning directory: %@", path);
        scanDirectory(g_rootNode, path, 0);

        // Calculate directory sizes (recursive)
        calculateNodeSize(g_rootNode);

        NSLog(@"Scan complete. Found %zu viewable assets.", g_allAssets.size());

        // Reload outline view
        [g_outlineView reloadData];

        // Load palette if found
        loadDefaultPalette();

        // Categorize discovered assets into predefined categories
        categorizeAssets();
        g_currentCategory = 0;
        g_currentCategoryAsset = 0;

        // Reload category outline view
        if (g_categoryOutline) {
            [g_categoryOutline reloadData];
        }

        // Load first asset if any
        if (!g_allAssets.empty()) {
            loadCategoryAsset();
            updatePreview();
        }

        // Update progress
        if (g_progressLabel) {
            int nonEmpty = 0;
            for (const auto& cat : g_categories) {
                if (!cat.assets.empty()) nonEmpty++;
            }
            [g_progressLabel setStringValue:
                [NSString stringWithFormat:
                    @"%zu assets in %d categories (Reviewed: 0)",
                    g_allAssets.size(), nonEmpty]];
        }
    }
}

- (void)prevAsset:(id)sender {
    if (g_allAssets.empty()) return;
    g_currentAssetIndex--;
    if (g_currentAssetIndex < 0)
        g_currentAssetIndex = (int)g_allAssets.size() - 1;
    loadCurrentAsset();
    updatePreview();
}

- (void)nextAsset:(id)sender {
    if (g_allAssets.empty()) return;
    g_currentAssetIndex++;
    if (g_currentAssetIndex >= (int)g_allAssets.size())
        g_currentAssetIndex = 0;
    loadCurrentAsset();
    updatePreview();
}

- (void)markGood:(id)sender {
    if (g_allAssets.empty()) return;
    FileTreeNode* node = g_allAssets[g_currentAssetIndex];
    std::string key = [node.fullPath UTF8String];
    g_reviewStatus[key] = GOOD;
    loadCurrentAsset();  // Refresh status display
    [self nextAsset:nil];  // Auto-advance
}

- (void)markBad:(id)sender {
    if (g_allAssets.empty()) return;
    FileTreeNode* node = g_allAssets[g_currentAssetIndex];
    std::string key = [node.fullPath UTF8String];
    g_reviewStatus[key] = BAD;
    loadCurrentAsset();
    [self nextAsset:nil];
}

// Category-based navigation
- (void)prevCategoryAsset:(id)sender {
    if (g_currentCategory >= (int)g_categories.size()) return;
    auto& cat = g_categories[g_currentCategory];
    if (cat.assets.empty()) return;

    g_currentCategoryAsset--;
    if (g_currentCategoryAsset < 0)
        g_currentCategoryAsset = (int)cat.assets.size() - 1;

    loadCategoryAsset();
    updatePreview();

    // Update outline selection
    if (g_categoryOutline) {
        [g_categoryOutline reloadData];
    }
}

- (void)nextCategoryAsset:(id)sender {
    if (g_currentCategory >= (int)g_categories.size()) return;
    auto& cat = g_categories[g_currentCategory];
    if (cat.assets.empty()) return;

    g_currentCategoryAsset++;
    if (g_currentCategoryAsset >= (int)cat.assets.size())
        g_currentCategoryAsset = 0;

    loadCategoryAsset();
    updatePreview();

    // Update outline selection
    if (g_categoryOutline) {
        [g_categoryOutline reloadData];
    }
}

- (void)markGoodCategory:(id)sender {
    if (g_currentCategory >= (int)g_categories.size()) return;
    auto& cat = g_categories[g_currentCategory];
    if (cat.assets.empty() ||
        g_currentCategoryAsset >= (int)cat.assets.size()) return;

    FileTreeNode* node = cat.assets[g_currentCategoryAsset];
    std::string key = [node.fullPath UTF8String];
    g_reviewStatus[key] = GOOD;

    // Auto-advance
    [self nextCategoryAsset:nil];

    // Update outline to show new status
    if (g_categoryOutline) {
        [g_categoryOutline reloadData];
    }
}

- (void)markBadCategory:(id)sender {
    if (g_currentCategory >= (int)g_categories.size()) return;
    auto& cat = g_categories[g_currentCategory];
    if (cat.assets.empty() ||
        g_currentCategoryAsset >= (int)cat.assets.size()) return;

    FileTreeNode* node = cat.assets[g_currentCategoryAsset];
    std::string key = [node.fullPath UTF8String];
    g_reviewStatus[key] = BAD;

    // Auto-advance
    [self nextCategoryAsset:nil];

    // Update outline to show new status
    if (g_categoryOutline) {
        [g_categoryOutline reloadData];
    }
}

- (void)togglePlay:(id)sender {
    g_animating = !g_animating;
}

- (void)playAudio:(id)sender {
    // Play either audio or video depending on what's loaded
    if (g_currentAUD) {
        playCurrentAUD();
    } else if (g_currentVQA && g_currentVQA->IsLoaded()) {
        playCurrentVQA();
    }
}

- (void)stopAudio:(id)sender {
    stopAudio();
    stopVQA();
}

- (void)toggleBackground:(id)sender {
    // Cycle through background modes: checker -> black -> gray -> checker
    switch (g_previewBg) {
        case BG_CHECKER: g_previewBg = BG_BLACK; break;
        case BG_BLACK:   g_previewBg = BG_GRAY; break;
        case BG_GRAY:    g_previewBg = BG_CHECKER; break;
    }

    // Update button label
    if (g_bgToggleButton) {
        const char* label = "BG: Checker";
        if (g_previewBg == BG_BLACK) label = "BG: Black";
        else if (g_previewBg == BG_GRAY) label = "BG: Gray";
        [g_bgToggleButton setTitle:[NSString stringWithUTF8String:label]];
    }

    updatePreview();
}

- (void)zoomIn:(id)sender {
    if (g_zoomLevel < 8) {
        g_zoomLevel *= 2;
        if (g_zoomLabel) {
            [g_zoomLabel setStringValue:
                [NSString stringWithFormat:@"Zoom: %dx", g_zoomLevel]];
        }
        updatePreview();
    }
}

- (void)zoomOut:(id)sender {
    if (g_zoomLevel > 1) {
        g_zoomLevel /= 2;
        if (g_zoomLabel) {
            [g_zoomLabel setStringValue:
                [NSString stringWithFormat:@"Zoom: %dx", g_zoomLevel]];
        }
        updatePreview();
    }
}

// Capability status marking
- (void)markCapabilityWorking:(id)sender {
    if (g_currentCapability >= 0 &&
        g_currentCapability < (int)g_capabilities.size()) {
        auto& cap = g_capabilities[g_currentCapability];
        g_capabilityStatus[cap.id] = CAP_WORKING;
        if (g_capabilityOutline) {
            [g_capabilityOutline reloadData];
        }
        NSLog(@"Marked %s as WORKING", cap.name);
    }
}

- (void)markCapabilityBroken:(id)sender {
    if (g_currentCapability >= 0 &&
        g_currentCapability < (int)g_capabilities.size()) {
        auto& cap = g_capabilities[g_currentCapability];
        g_capabilityStatus[cap.id] = CAP_BROKEN;
        if (g_capabilityOutline) {
            [g_capabilityOutline reloadData];
        }
        NSLog(@"Marked %s as BROKEN", cap.name);
    }
}

- (void)markCapabilityUntested:(id)sender {
    if (g_currentCapability >= 0 &&
        g_currentCapability < (int)g_capabilities.size()) {
        auto& cap = g_capabilities[g_currentCapability];
        g_capabilityStatus[cap.id] = CAP_UNTESTED;
        if (g_capabilityOutline) {
            [g_capabilityOutline reloadData];
        }
        NSLog(@"Marked %s as UNTESTED", cap.name);
    }
}

- (void)notesChanged:(id)sender {
    NSTextField* field = (NSTextField*)sender;
    if (g_currentCapability >= 0 &&
        g_currentCapability < (int)g_capabilities.size()) {
        auto& cap = g_capabilities[g_currentCapability];
        g_capabilityNotes[cap.id] = [[field stringValue] UTF8String];
    }
}

- (void)saveCapabilityChecklist:(id)sender {
    NSSavePanel* panel = [NSSavePanel savePanel];
    [panel setNameFieldStringValue:@"capability_checklist.txt"];
    [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"txt"]]];

    if ([panel runModal] == NSModalResponseOK) {
        NSURL* url = panel.URL;

        FILE* f = fopen([[url path] UTF8String], "w");
        if (f) {
            fprintf(f, "# Red Alert Capability Checklist\n");
            fprintf(f, "# Generated by Asset Viewer\n\n");

            int working = 0, broken = 0, untested = 0;

            // Group by category
            for (const auto& catName : getCapabilityCategories()) {
                fprintf(f, "## %s\n", catName.c_str());

                for (size_t i = 0; i < g_capabilities.size(); i++) {
                    auto& cap = g_capabilities[i];
                    if (cap.category != catName) continue;

                    auto it = g_capabilityStatus.find(cap.id);
                    CapabilityStatus st = (it != g_capabilityStatus.end())
                                           ? it->second : CAP_UNTESTED;

                    const char* statusStr = "UNTESTED";
                    if (st == CAP_WORKING) {
                        statusStr = "WORKING";
                        working++;
                    } else if (st == CAP_BROKEN) {
                        statusStr = "BROKEN";
                        broken++;
                    } else {
                        untested++;
                    }

                    fprintf(f, "- [%s] %s: %s\n", statusStr, cap.id, cap.name);

                    // Include notes if any
                    auto notesIt = g_capabilityNotes.find(cap.id);
                    if (notesIt != g_capabilityNotes.end() &&
                        !notesIt->second.empty()) {
                        fprintf(f, "  Notes: %s\n", notesIt->second.c_str());
                    }
                }
                fprintf(f, "\n");
            }

            fprintf(f, "## Summary\n");
            fprintf(f, "Total: %zu capabilities\n", g_capabilities.size());
            fprintf(f, "Working: %d\n", working);
            fprintf(f, "Broken: %d\n", broken);
            fprintf(f, "Untested: %d\n", untested);

            fclose(f);
            NSLog(@"Saved capability checklist to %@", [url path]);
        }
    }
}

- (void)loadCapabilityChecklist:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"txt"]]];

    if ([panel runModal] == NSModalResponseOK) {
        NSURL* url = panel.URL;

        FILE* f = fopen([[url path] UTF8String], "r");
        if (f) {
            char line[512];
            while (fgets(line, sizeof(line), f)) {
                // Parse lines like: - [WORKING] mix.open_encrypted: ...
                char status[32], capId[64];
                if (sscanf(line, "- [%31[^]]] %63[^:]:", status, capId) == 2) {
                    CapabilityStatus st = CAP_UNTESTED;
                    if (strcmp(status, "WORKING") == 0) st = CAP_WORKING;
                    else if (strcmp(status, "BROKEN") == 0) st = CAP_BROKEN;

                    g_capabilityStatus[capId] = st;
                }
                // Parse notes lines
                char notes[256];
                if (sscanf(line, "  Notes: %255[^\n]", notes) == 1) {
                    // Find the last capability we parsed
                    // (simple approach - not perfect but works)
                }
            }
            fclose(f);

            if (g_capabilityOutline) {
                [g_capabilityOutline reloadData];
            }
            NSLog(@"Loaded capability checklist from %@", [url path]);
        }
    }
}

- (void)saveResults:(id)sender {
    NSSavePanel* panel = [NSSavePanel savePanel];
    [panel setNameFieldStringValue:@"asset_review.txt"];
    [panel setAllowedContentTypes:@[[UTType typeWithFilenameExtension:@"txt"]]];

    if ([panel runModal] == NSModalResponseOK) {
        NSURL* url = panel.URL;

        FILE* f = fopen([[url path] UTF8String], "w");
        if (f) {
            fprintf(f, "# Asset Review Results\n");
            fprintf(f, "# Generated by Red Alert Asset Viewer\n\n");

            int goodCount = 0, badCount = 0, pendingCount = 0;

            fprintf(f, "## BAD Assets (need fixing):\n");
            for (auto& p : g_reviewStatus) {
                if (p.second == BAD) {
                    fprintf(f, "- %s\n", p.first.c_str());
                    badCount++;
                }
            }

            fprintf(f, "\n## GOOD Assets:\n");
            for (auto& p : g_reviewStatus) {
                if (p.second == GOOD) {
                    fprintf(f, "- %s\n", p.first.c_str());
                    goodCount++;
                }
            }

            // Count pending
            for (FileTreeNode* node : g_allAssets) {
                std::string key = [node.fullPath UTF8String];
                if (g_reviewStatus.find(key) == g_reviewStatus.end() ||
                    g_reviewStatus[key] == PENDING) {
                    pendingCount++;
                }
            }

            fprintf(f, "\n## Summary:\n");
            fprintf(f, "Total assets: %zu\n", g_allAssets.size());
            fprintf(f, "Good: %d\n", goodCount);
            fprintf(f, "Bad: %d\n", badCount);
            fprintf(f, "Pending: %d\n", pendingCount);

            fclose(f);
            NSLog(@"Results saved to %@", [url path]);
        }
    }
}

- (void)keyDown:(NSEvent*)event {
    unichar c = [[event characters] characterAtIndex:0];

    // Check which tab is active
    BOOL onReviewTab = [[g_tabView selectedTabViewItem].identifier
                        isEqualToString:@"review"];

    switch (c) {
        case NSLeftArrowFunctionKey:
            if (onReviewTab)
                [self prevCategoryAsset:nil];
            else
                [self prevAsset:nil];
            break;
        case NSRightArrowFunctionKey:
            if (onReviewTab)
                [self nextCategoryAsset:nil];
            else
                [self nextAsset:nil];
            break;
        case 'g': case 'G':
            if (onReviewTab)
                [self markGoodCategory:nil];
            else
                [self markGood:nil];
            break;
        case 'b': case 'B':
            if (onReviewTab)
                [self markBadCategory:nil];
            else
                [self markBad:nil];
            break;
        case ' ':
            [self togglePlay:nil];
            break;
        case 's': case 'S':
            [self saveResults:nil];
            break;
    }
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    [NSApp terminate:nil];
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notif {
    if (g_animTimer) {
        [g_animTimer invalidate];
        g_animTimer = nil;
    }
    if (g_currentSHP) {
        Shp_Free(g_currentSHP);
        g_currentSHP = nullptr;
    }
    Renderer_Shutdown();

    // Unmount any ISOs we mounted
    unmountAllISOs();
}

@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        AssetViewerDelegate* delegate = [[AssetViewerDelegate alloc] init];
        [NSApp setDelegate:delegate];

        // Create menu bar
        NSMenu* menuBar = [[NSMenu alloc] init];
        NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
        [menuBar addItem:appMenuItem];

        NSMenu* appMenu = [[NSMenu alloc] init];
        [appMenu addItemWithTitle:@"Quit Asset Viewer"
                           action:@selector(terminate:)
                    keyEquivalent:@"q"];
        [appMenuItem setSubmenu:appMenu];

        [NSApp setMainMenu:menuBar];

        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
    return 0;
}
