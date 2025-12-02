/**
 * Red Alert macOS Port - Entry Point
 *
 * AppKit application shell with Metal rendering, input, and game loop.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "graphics/metal/renderer.h"
#include "input/input.h"
#include "game/gameloop.h"
#include "game/map.h"
#include "game/units.h"
#include "game/sprites.h"
#include "game/sounds.h"
#include "game/terrain.h"
#include "game/ai.h"
#include "game/mission.h"
#include "audio/audio.h"
#include "video/music.h"
#include "ui/menu.h"
#include "ui/game_ui.h"
#include "compat/assets.h"
#include "assets/assetloader.h"
#include "assets/shpfile.h"
#include <cmath>

// Game window dimensions (original Red Alert resolution)
static constexpr int WINDOW_WIDTH = 640;
static constexpr int WINDOW_HEIGHT = 400;

// Window reference for fullscreen toggle
static NSWindow* g_mainWindow = nil;

// Animation state for demo
static float g_animPhase = 0.0f;
static int g_bounceX = 100;
static int g_bounceY = 100;

// Audio test tones
static AudioSample* g_testTones[4] = {nullptr, nullptr, nullptr, nullptr};
static const uint32_t g_toneFreqs[4] = {262, 330, 392, 523}; // C4, E4, G4, C5

// Gameplay state
static bool g_inGameplay = false;
static int g_selectionStartX = -1;
static int g_selectionStartY = -1;
static bool g_isSelecting = false;
static bool g_attackMoveMode = false;  // A key: next click = attack-move

// Mission result state
typedef enum {
    MISSION_ONGOING = 0,
    MISSION_VICTORY,
    MISSION_DEFEAT
} MissionResult;
static MissionResult g_missionResult = MISSION_ONGOING;
static int g_resultDisplayTimer = 0;   // Frames to show result screen
static int g_gameFrameCount = 0;       // Frame counter (triggers, timing)

// Assets loaded flag
static bool g_assetsLoaded = false;

// Toggle fullscreen mode
static void ToggleFullscreen(void) {
    if (g_mainWindow) {
        [g_mainWindow toggleFullScreen:nil];
    }
}

// Current mission data
static MissionData g_currentMission;

// Start a mission from data
static void StartMission(const MissionData* mission) {
    g_inGameplay = true;
    g_missionResult = MISSION_ONGOING;
    g_resultDisplayTimer = 0;
    g_gameFrameCount = 0;

    // Initialize game UI
    GameUI_Init();

    // Start the mission (spawns map, units, buildings)
    Mission_Start(mission);

    // Center viewport on player base
    // Find player construction yard to center on
    int centerX = 150, centerY = 450;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        bool isPlayerConYard = bld && bld->team == TEAM_PLAYER &&
                              bld->type == BUILDING_CONSTRUCTION;
        if (isPlayerConYard) {
            centerX = bld->cellX * CELL_SIZE + (bld->width * CELL_SIZE) / 2;
            centerY = bld->cellY * CELL_SIZE + (bld->height * CELL_SIZE) / 2;
            break;
        }
    }
    Map_CenterViewport(centerX, centerY);

    NSLog(@"Mission started: %s", mission->name);
}

// Start a demo mission (skirmish mode)
static void StartDemoMission(void) {
    // Load demo mission data
    Mission_GetDemo(&g_currentMission);

    // Start it
    StartMission(&g_currentMission);
}

// Pending campaign info (set before showing briefing)
static int g_pendingCampaign = 0;
static int g_pendingDifficulty = 0;

// Actually start the mission after briefing
static void OnBriefingConfirmed(void) {
    NSLog(@"Briefing confirmed, starting mission");
    // Restore game palette (briefing may have changed it)
    Renderer_LoadPalette("SNOW.PAL");
    StartMission(&g_currentMission);
}

// Try to load mission from common paths
static bool TryLoadMission(MissionData* mission, const char* missionName) {
    // Search paths for mission INI files
    const char* searchPaths[] = {
        "/tmp/ra_extract/%s.INI",
        "../assets/%s.INI",
        "../../assets/%s.INI",
        "/Users/jasson/workspace/CnC_Red_Alert/assets/%s.INI",
        nullptr
    };

    char path[512];
    for (int i = 0; searchPaths[i]; i++) {
        snprintf(path, sizeof(path), searchPaths[i], missionName);
        if (Mission_LoadFromINI(mission, path)) {
            NSLog(@"Loaded mission from: %s", path);
            return true;
        }
    }
    return false;
}

// Start a campaign mission (shows briefing first)
static void StartCampaignMission(int campaign, int difficulty) {
    // Set mission name based on campaign and difficulty
    bool isAllied = (campaign == CAMPAIGN_ALLIED);
    const char* campaignName = isAllied ? "Allied" : "Soviet";
    const char* diffName = (difficulty == DIFFICULTY_EASY) ? "Easy" :
                           (difficulty == DIFFICULTY_HARD) ? "Hard" : "Normal";

    NSLog(@"Preparing %s Campaign on %s difficulty", campaignName, diffName);

    // Store pending campaign info
    g_pendingCampaign = campaign;
    g_pendingDifficulty = difficulty;

    // Build mission filename
    // Soviet: SCG01EA, SCG02EA, ...
    // Allied: SCU01EA, SCU02EA, ...
    const char* prefix = (campaign == CAMPAIGN_ALLIED) ? "SCU" : "SCG";
    char missionName[32];
    snprintf(missionName, sizeof(missionName), "%s01EA", prefix);

    // Try to load real mission
    bool loaded = TryLoadMission(&g_currentMission, missionName);
    if (!loaded) {
        NSLog(@"Could not load mission %s, falling back to demo", missionName);
        Mission_GetDemo(&g_currentMission);
        // Update mission name to reflect campaign
        snprintf(g_currentMission.name, sizeof(g_currentMission.name),
                 "%s Mission 1 (Demo)", campaignName);
    } else {
        NSLog(@"Mission loaded: %s", g_currentMission.name);
        int mw = g_currentMission.mapWidth, mh = g_currentMission.mapHeight;
        int mx = g_currentMission.mapX, my = g_currentMission.mapY;
        NSLog(@"  Map: %dx%d at (%d,%d)", mw, mh, mx, my);
        const char* hasTerr = g_currentMission.terrainType ? "YES" : "NO";
        NSLog(@"  Terrain data: %s", hasTerr);
        int uc = g_currentMission.unitCount;
        int bc = g_currentMission.buildingCount;
        NSLog(@"  Units: %d, Buildings: %d", uc, bc);
    }

    // Adjust starting credits based on difficulty
    int credits = g_currentMission.startCredits;
    if (difficulty == DIFFICULTY_EASY) {
        g_currentMission.startCredits = (credits * 150) / 100;
    } else if (difficulty == DIFFICULTY_HARD) {
        g_currentMission.startCredits = (credits * 60) / 100;
    }

    // Use mission description as briefing text, or generate default
    const char* briefingText = g_currentMission.description;
    if (!briefingText || strlen(briefingText) < 10) {
        if (isAllied) {
            briefingText = "Commander, Soviet forces invaded Eastern Europe. "
                           "Establish a base and rescue Allied scientists. "
                           "Eliminate all Soviet forces. Good luck.";
        } else {
            briefingText = "Comrade, the capitalist West threatens our Union. "
                           "Crush Allied forces and secure our borders. "
                           "Show them the might of the Red Army!";
        }
    }

    // Set briefing data and show briefing screen
    Menu_SetBriefing(g_currentMission.name, briefingText);
    Menu_SetBriefingConfirmCallback(OnBriefingConfirmed);
    Menu_SetCurrentScreen(MENU_SCREEN_BRIEFING);
}

// ============================================================================
// GameUpdate Helpers
// ============================================================================

// Handle menu screen updates. Returns true if in menu (skip game logic).
static bool UpdateMenuScreen(float /*deltaTime*/) {
    MenuScreen currentScreen = Menu_GetCurrentScreen();
    if (currentScreen == MENU_SCREEN_NONE) return false;

    Menu* activeMenu = nullptr;
    switch (currentScreen) {
        case MENU_SCREEN_MAIN:
            activeMenu = Menu_GetMainMenu();
            break;
        case MENU_SCREEN_CAMPAIGN_SELECT:
            activeMenu = Menu_GetCampaignMenu();
            break;
        case MENU_SCREEN_DIFFICULTY_SELECT:
            activeMenu = Menu_GetDifficultyMenu();
            break;
        case MENU_SCREEN_OPTIONS:
            activeMenu = Menu_GetOptionsMenu();
            break;
        case MENU_SCREEN_CREDITS: {
            bool esc = Input_WasKeyPressed(VK_ESCAPE);
            bool enter = Input_WasKeyPressed(VK_RETURN);
            bool space = Input_WasKeyPressed(VK_SPACE);
            bool click = (Input_GetMouseButtons() & INPUT_MOUSE_LEFT);
            if (esc || enter || space || click) {
                Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            }
            break;
        }
        case MENU_SCREEN_BRIEFING:
            Menu_UpdateBriefing();
            break;
        case MENU_SCREEN_VIDEO:
            Menu_UpdateVideo();
            break;
        default:
            break;
    }

    if (activeMenu) Menu_Update(activeMenu);

    // ESC goes back from submenus
    if (Input_WasKeyPressed(VK_ESCAPE)) {
        if (currentScreen == MENU_SCREEN_OPTIONS ||
            currentScreen == MENU_SCREEN_CAMPAIGN_SELECT) {
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
        } else if (currentScreen == MENU_SCREEN_DIFFICULTY_SELECT) {
            Menu_SetCurrentScreen(MENU_SCREEN_CAMPAIGN_SELECT);
        }
    }
    return true;
}

// Shutdown gameplay and return to menu
static void ExitToMenu(void) {
    g_inGameplay = false;
    g_missionResult = MISSION_ONGOING;
    AI_Shutdown();
    GameUI_Shutdown();
    Map_Shutdown();
    Units_Shutdown();
    Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
}

// Handle map scrolling (keyboard and mouse edge)
static void UpdateMapScrolling(void) {
    int scrollSpeed = 8;

    // Keyboard scrolling
    if (Input_IsKeyDown(VK_UP) || Input_IsKeyDown('W'))
        Map_ScrollViewport(0, -scrollSpeed);
    if (Input_IsKeyDown(VK_DOWN) || Input_IsKeyDown('S'))
        Map_ScrollViewport(0, scrollSpeed);
    if (Input_IsKeyDown(VK_LEFT) || Input_IsKeyDown('A'))
        Map_ScrollViewport(-scrollSpeed, 0);
    if (Input_IsKeyDown(VK_RIGHT) || Input_IsKeyDown('D'))
        Map_ScrollViewport(scrollSpeed, 0);

    // Mouse edge scrolling
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    if (mx < 10) Map_ScrollViewport(-scrollSpeed, 0);
    if (mx > WINDOW_WIDTH - 10) Map_ScrollViewport(scrollSpeed, 0);
    if (my < 10) Map_ScrollViewport(0, -scrollSpeed);
    if (my > WINDOW_HEIGHT - 10) Map_ScrollViewport(0, scrollSpeed);
}

// Handle left-click selection. Returns true if UI consumed click.
static bool UpdateUnitSelection(int mx, int my) {
    static bool wasLeftDown = false;
    uint8_t buttons = Input_GetMouseButtons();
    bool leftDown = (buttons & INPUT_MOUSE_LEFT) != 0;

    // UI input first
    if (leftDown && !wasLeftDown) {
        if (GameUI_HandleInput(mx, my, TRUE, FALSE)) {
            wasLeftDown = leftDown;
            return true;
        }
    }

    // Selection handling
    if (leftDown && !wasLeftDown) {
        g_selectionStartX = mx;
        g_selectionStartY = my;
        g_isSelecting = true;
    } else if (!leftDown && wasLeftDown && g_isSelecting) {
        int x1 = g_selectionStartX, y1 = g_selectionStartY;
        int x2 = mx, y2 = my;

        if (abs(x2 - x1) < 5 && abs(y2 - y1) < 5) {
            int unitId = Units_GetAtScreen(mx, my);
            if (unitId >= 0) Units_Select(unitId, Input_IsKeyDown(VK_SHIFT));
            else Units_DeselectAll();
        } else {
            Units_SelectInRect(x1, y1, x2, y2, TEAM_PLAYER);
        }
        g_isSelecting = false;
    }
    wasLeftDown = leftDown;
    return false;
}

// Handle right-click commands
static void UpdateRightClickCommands(int mx, int my) {
    static bool wasRightDown = false;
    uint8_t buttons = Input_GetMouseButtons();
    bool rightDown = (buttons & INPUT_MOUSE_RIGHT) != 0;

    if (rightDown && !wasRightDown) {
        int worldX, worldY;
        Map_ScreenToWorld(mx, my, &worldX, &worldY);
        int targetId = Units_GetAtScreen(mx, my);
        Unit* target = Units_Get(targetId);
        bool ctrlDown = Input_IsKeyDown(VK_CONTROL);

        for (int i = 0; i < MAX_UNITS; i++) {
            Unit* unit = Units_Get(i);
            if (!unit || !unit->selected) continue;

            if (ctrlDown) {
                Units_CommandForceAttack(i, worldX, worldY);
            } else if (g_attackMoveMode) {
                Units_CommandAttackMove(i, worldX, worldY);
            } else if (target && target->team == TEAM_ENEMY) {
                Units_CommandAttack(i, targetId);
            } else {
                Units_CommandMove(i, worldX, worldY);
            }
        }
        g_attackMoveMode = false;
    }
    wasRightDown = rightDown;
}

// Handle hotkey commands (S, A, G)
static void UpdateHotkeyCommands(void) {
    // Stop (S - but not if holding WASD)
    if (Input_WasKeyPressed('S') && !Input_IsKeyDown('W')) {
        for (int i = 0; i < MAX_UNITS; i++) {
            Unit* unit = Units_Get(i);
            if (unit && unit->selected) Units_CommandStop(i);
        }
    }

    // Attack-move mode (A - but not if holding WASD)
    if (Input_WasKeyPressed('A') && !Input_IsKeyDown('W') &&
        !Input_IsKeyDown('S') && !Input_IsKeyDown('D')) {
        g_attackMoveMode = true;
    }

    // Guard (G)
    if (Input_WasKeyPressed('G')) {
        for (int i = 0; i < MAX_UNITS; i++) {
            Unit* unit = Units_Get(i);
            if (unit && unit->selected) Units_CommandGuard(i);
        }
    }
}

// Check mission triggers and victory/defeat conditions
static void UpdateMissionState(void) {
    if (g_missionResult != MISSION_ONGOING) return;

    // Process triggers
    int triggerResult = Mission_ProcessTriggers(&g_currentMission,
                                                 g_gameFrameCount);
    if (triggerResult == 1) {
        g_missionResult = MISSION_VICTORY;
        g_resultDisplayTimer = 180;
        NSLog(@"VICTORY via trigger!");
    } else if (triggerResult == -1) {
        g_missionResult = MISSION_DEFEAT;
        g_resultDisplayTimer = 180;
        NSLog(@"DEFEAT via trigger!");
    }

    // Check victory/defeat
    if (g_missionResult == MISSION_ONGOING) {
        int result = Mission_CheckVictory(&g_currentMission, g_gameFrameCount);
        if (result == 1) {
            g_missionResult = MISSION_VICTORY;
            g_resultDisplayTimer = 180;
            NSLog(@"VICTORY!");
        } else if (result == -1) {
            g_missionResult = MISSION_DEFEAT;
            g_resultDisplayTimer = 180;
            NSLog(@"DEFEAT!");
        }
    }
}

// Handle result screen display. Returns true if should exit to menu.
static bool UpdateResultScreen(void) {
    if (g_missionResult == MISSION_ONGOING || g_resultDisplayTimer <= 0)
        return false;

    g_resultDisplayTimer--;
    if (g_resultDisplayTimer < 120) {
        if (Input_WasKeyPressed(VK_ESCAPE) || Input_WasKeyPressed(VK_RETURN) ||
            Input_WasKeyPressed(VK_SPACE) ||
            (Input_GetMouseButtons() & INPUT_MOUSE_LEFT)) {
            return true;
        }
    }
    return false;
}

#pragma mark - Game Callbacks

// Called at game logic rate (15 FPS default)
void GameUpdate(uint32_t frame, float deltaTime) {
    Music_Update((int)(deltaTime * 1000));

    // Menu mode
    if (UpdateMenuScreen(deltaTime)) return;

    // Gameplay mode
    if (g_inGameplay) {
        if (Input_WasKeyPressed('P')) GameLoop_Pause(!GameLoop_IsPaused());
        if (Input_WasKeyPressed('F')) ToggleFullscreen();

        // ESC handling
        if (Input_WasKeyPressed(VK_ESCAPE)) {
            if (g_attackMoveMode) g_attackMoveMode = false;
            else { ExitToMenu(); return; }
        }

        UpdateMapScrolling();

        int mx = Input_GetMouseX(), my = Input_GetMouseY();
        if (UpdateUnitSelection(mx, my)) return;
        UpdateRightClickCommands(mx, my);
        UpdateHotkeyCommands();

        // Update game systems
        Map_Update();
        Units_Update();
        GameUI_Update();
        AI_Update();
        g_gameFrameCount++;

        UpdateMissionState();
        if (UpdateResultScreen()) { ExitToMenu(); return; }
        return;
    }

    // Demo mode fallback
    g_animPhase += 0.1f;
    if (Input_WasKeyPressed(VK_ESCAPE)) {
        if (!GameUI_HandleEscape()) Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
    }
    if (Input_WasKeyPressed('P')) GameLoop_Pause(!GameLoop_IsPaused());
    if (Input_WasKeyPressed('F')) ToggleFullscreen();

    if (frame % 60 == 0) {
        const FrameStats* stats = GameLoop_GetStats();
        NSLog(@"Game frame %u, Render FPS: %.1f, Speed: %d%s",
              frame, stats->currentFPS, GameLoop_GetSpeed(),
              GameLoop_IsPaused() ? " [PAUSED]" : "");
    }
}

// Test sprite data (16x16 simple icon)
static const uint8_t g_testSprite[16 * 16] = {
    0,0,0,0,0,4,4,4,4,4,4,0,0,0,0,0,
    0,0,0,4,4,4,12,12,12,12,4,4,4,0,0,0,
    0,0,4,4,12,12,12,15,15,12,12,12,4,4,0,0,
    0,4,4,12,12,15,15,15,15,15,15,12,12,4,4,0,
    0,4,12,12,15,15,15,15,15,15,15,15,12,12,4,0,
    4,4,12,15,15,15,15,15,15,15,15,15,15,12,4,4,
    4,12,12,15,15,15,0,0,0,0,15,15,15,12,12,4,
    4,12,15,15,15,15,0,0,0,0,15,15,15,15,12,4,
    4,12,15,15,15,15,0,0,0,0,15,15,15,15,12,4,
    4,12,12,15,15,15,0,0,0,0,15,15,15,12,12,4,
    4,4,12,15,15,15,15,15,15,15,15,15,15,12,4,4,
    0,4,12,12,15,15,15,15,15,15,15,15,12,12,4,0,
    0,4,4,12,12,15,15,15,15,15,15,12,12,4,4,0,
    0,0,4,4,12,12,12,15,15,12,12,12,4,4,0,0,
    0,0,0,4,4,4,12,12,12,12,4,4,4,0,0,0,
    0,0,0,0,0,4,4,4,4,4,4,0,0,0,0,0,
};

// Render credits screen
static void RenderCredits(void) {
    // Set menu palette for proper colors
    Palette menuPal;
    StubAssets_CreatePalette(&menuPal);
    Renderer_SetPalette(&menuPal);

    Renderer_Clear(0);
    Renderer_DrawText("CREDITS", 280, 50, 15, 0);
    Renderer_HLine(100, 540, 70, 7);

    Renderer_DrawText("COMMAND & CONQUER: RED ALERT", 180, 100, 14, 0);
    Renderer_DrawText("ORIGINAL GAME BY WESTWOOD STUDIOS", 150, 130, 7, 0);

    Renderer_DrawText("MACOS PORT", 270, 180, 10, 0);
    Renderer_DrawText("MILESTONE 13 - GAMEPLAY", 195, 210, 7, 0);

    Renderer_DrawText("BUILT WITH:", 260, 260, 7, 0);
    Renderer_DrawText("- METAL FOR GRAPHICS", 220, 285, 7, 0);
    Renderer_DrawText("- COREAUDIO FOR SOUND", 220, 305, 7, 0);
    Renderer_DrawText("- APPKIT FOR WINDOWING", 220, 325, 7, 0);

    Renderer_DrawText("PRESS ESCAPE OR ENTER TO RETURN", 170, 370, 14, 0);
}

// Called every render frame (60 FPS)
// ============================================================================
// GameRender Helpers
// ============================================================================

// Render menu screens. Returns true if menu was rendered.
static bool RenderMenuScreen(void) {
    MenuScreen screen = Menu_GetCurrentScreen();
    if (screen == MENU_SCREEN_NONE) return false;

    switch (screen) {
        case MENU_SCREEN_CREDITS:   RenderCredits();        return true;
        case MENU_SCREEN_BRIEFING:  Menu_RenderBriefing();  return true;
        case MENU_SCREEN_VIDEO:     Menu_RenderVideo();     return true;
        default: break;
    }

    Menu* activeMenu = nullptr;
    switch (screen) {
        case MENU_SCREEN_MAIN:
            activeMenu = Menu_GetMainMenu(); break;
        case MENU_SCREEN_CAMPAIGN_SELECT:
            activeMenu = Menu_GetCampaignMenu(); break;
        case MENU_SCREEN_DIFFICULTY_SELECT:
            activeMenu = Menu_GetDifficultyMenu(); break;
        case MENU_SCREEN_OPTIONS:
            activeMenu = Menu_GetOptionsMenu(); break;
        default: break;
    }
    if (activeMenu) Menu_Render(activeMenu);
    return true;
}

// Render selection box while dragging
static void RenderSelectionBox(void) {
    if (!g_isSelecting) return;

    int mx = Input_GetMouseX(), my = Input_GetMouseY();
    int x1 = g_selectionStartX, y1 = g_selectionStartY;
    int x2 = mx, y2 = my;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    Renderer_DrawRect(x1, y1, x2 - x1, y2 - y1, 15);
}

// Render mouse cursor crosshair
static void RenderCursor(void) {
    int mx = Input_GetMouseX(), my = Input_GetMouseY();
    Renderer_DrawLine(mx - 8, my, mx + 8, my, 15);
    Renderer_DrawLine(mx, my - 8, mx, my + 8, 15);
}

// Render top HUD bar
static void RenderGameHUD(void) {
    const FrameStats* stats = GameLoop_GetStats();
    Renderer_FillRect(0, 0, 560, 16, 0);

    char hudText[64];
    snprintf(hudText, sizeof(hudText), "%s", g_currentMission.name);
    Renderer_DrawText(hudText, 10, 3, 14, 0);

    snprintf(hudText, sizeof(hudText), "P:%d E:%d",
             Units_CountByTeam(TEAM_PLAYER), Units_CountByTeam(TEAM_ENEMY));
    Renderer_DrawText(hudText, 200, 3, 10, 0);

    int selected = Units_GetSelectedCount();
    if (selected > 0) {
        snprintf(hudText, sizeof(hudText), "SEL:%d", selected);
        Renderer_DrawText(hudText, 300, 3, 15, 0);
    }

    snprintf(hudText, sizeof(hudText), "%.0f", stats->currentFPS);
    Renderer_DrawText(hudText, 540, 3, 7, 0);
}

// Render pause overlay
static void RenderPauseOverlay(void) {
    if (!GameLoop_IsPaused()) return;
    Renderer_FillRect(220, 180, 120, 40, 0);
    Renderer_DrawRect(220, 180, 120, 40, 15);
    Renderer_DrawText("PAUSED", 245, 195, 15, 0);
}

// Render victory/defeat overlay
static void RenderResultOverlay(void) {
    if (g_missionResult == MISSION_ONGOING) return;

    // Scanline effect
    for (int y = 0; y < 400; y += 2) Renderer_HLine(0, 560, y, 0);

    int boxW = 300, boxH = 100;
    int boxX = (560 - boxW) / 2, boxY = (400 - boxH) / 2 - 20;

    Renderer_FillRect(boxX, boxY, boxW, boxH, 0);
    Renderer_DrawRect(boxX, boxY, boxW, boxH, 15);
    Renderer_DrawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, 7);

    int tx = boxX + 50, ty1 = boxY + 20, ty2 = boxY + 45;
    if (g_missionResult == MISSION_VICTORY) {
        Renderer_DrawText("MISSION ACCOMPLISHED", tx, ty1, 10, 0);
        Renderer_DrawText("You defeated the enemy!", tx - 10, ty2, 15, 0);
    } else {
        Renderer_DrawText("MISSION FAILED", tx + 25, ty1, 4, 0);
        Renderer_DrawText("Your forces destroyed.", tx - 5, ty2, 15, 0);
    }
    Renderer_DrawText("Press any key...", boxX + 75, boxY + 75, 7, 0);
}

// Render bottom controls help bar
static void RenderControlsHelp(void) {
    Renderer_FillRect(0, 384, 560, 16, 0);
    if (g_attackMoveMode) {
        Renderer_DrawText("A-MOVE: RMB=TARGET ESC=CANCEL", 20, 387, 14, 0);
    } else {
        Renderer_DrawText("WASD=SCROLL A=MOVE G=GUARD S=STOP", 20, 387, 7, 0);
    }
}

// Render gameplay mode
static void RenderGameplay(void) {
    Renderer_Clear(0);
    Renderer_SetClipRect(0, 16, 560, 368);
    Map_Render();
    Units_Render();
    Renderer_ResetClip();

    GameUI_Render();
    RenderSelectionBox();
    RenderCursor();
    RenderGameHUD();
    RenderPauseOverlay();
    RenderResultOverlay();
    RenderControlsHelp();
}

// Render demo mode graphics test
static void RenderDemoMode(void) {
    const FrameStats* stats = GameLoop_GetStats();
    Renderer_Clear(8);

    // Title
    Renderer_DrawText("RED ALERT MACOS PORT", 200, 10, 15, 0);
    Renderer_DrawText("MILESTONE 9: RENDERING", 195, 25, 14, 0);

    // Radial lines
    int cx = 100, cy = 100;
    for (int angle = 0; angle < 360; angle += 30) {
        float rad = angle * 3.14159f / 180.0f;
        int ex = cx + (int)(50 * cosf(rad));
        int ey = cy + (int)(50 * sinf(rad));
        Renderer_DrawLine(cx, cy, ex, ey, 1 + (angle / 30) % 14);
    }

    // Circles
    Renderer_DrawCircle(250, 100, 40, 12);
    Renderer_FillCircle(250, 100, 30, 4);
    Renderer_DrawCircle(350, 100, 40, 10);
    Renderer_FillCircle(350, 100, 30, 2);
    Renderer_DrawCircle(450, 100, 40, 9);
    Renderer_FillCircle(450, 100, 30, 1);

    // Rectangles
    Renderer_DrawRect(520, 60, 60, 80, 14);
    Renderer_DrawRect(525, 65, 50, 70, 6);

    // Lines
    Renderer_HLine(50, 590, 160, 7);
    Renderer_VLine(320, 170, 240, 7);

    // Sprites
    Renderer_Blit(g_testSprite, 16, 16, 50, 180, TRUE);
    Renderer_Blit(g_testSprite, 16, 16, 80, 180, TRUE);
    Renderer_Blit(g_testSprite, 16, 16, 110, 180, TRUE);
    Renderer_ScaleBlit(g_testSprite, 16, 16, 150, 170, 48, 48, TRUE);

    // Bouncing box
    uint8_t boxClr = 1 + (stats->gameFrame % 14);
    Renderer_FillRect(g_bounceX, g_bounceY, 30, 30, boxClr);
    Renderer_DrawRect(g_bounceX - 2, g_bounceY - 2, 34, 34, 15);

    // Cursor
    int mx = Input_GetMouseX(), my = Input_GetMouseY();
    Renderer_DrawLine(mx - 10, my, mx + 10, my, 15);
    Renderer_DrawLine(mx, my - 10, mx, my + 10, 15);
    Renderer_FillCircle(mx, my, 3, 12);

    // Mouse buttons
    uint8_t buttons = Input_GetMouseButtons();
    Renderer_FillCircle(60, 260, 12, (buttons & INPUT_MOUSE_LEFT) ? 12 : 4);
    Renderer_FillCircle(100, 260, 12, (buttons & INPUT_MOUSE_RIGHT) ? 9 : 1);
    Renderer_FillCircle(140, 260, 12, (buttons & INPUT_MOUSE_MIDDLE) ? 10 : 2);
    Renderer_DrawText("L", 56, 255, 15, 0);
    Renderer_DrawText("R", 96, 255, 15, 0);
    Renderer_DrawText("M", 135, 255, 15, 0);

    // Speed indicator
    int speed = GameLoop_GetSpeed();
    Renderer_DrawText("SPEED:", 450, 180, 15, 0);
    for (int i = 0; i <= 7; i++)
        Renderer_FillRect(450 + i * 15, 195, 12, 20, (i <= speed) ? 10 : 2);

    // Pause
    if (GameLoop_IsPaused()) {
        Renderer_FillRect(260, 130, 120, 30, 0);
        Renderer_DrawRect(260, 130, 120, 30, 15);
        Renderer_DrawText("PAUSED", 285, 140, 15, 0);
    }

    // Key indicators
    int keyY = 290;
    Renderer_DrawText("WASD:", 220, keyY - 15, 7, 0);
    Renderer_FillRect(270, keyY, 25, 20, Input_IsKeyDown('W') ? 15 : 2);
    Renderer_FillRect(235, keyY + 25, 25, 20, Input_IsKeyDown('A') ? 15 : 2);
    Renderer_FillRect(270, keyY + 25, 25, 20, Input_IsKeyDown('S') ? 15 : 2);
    Renderer_FillRect(305, keyY + 25, 25, 20, Input_IsKeyDown('D') ? 15 : 2);
    Renderer_DrawText("W", 278, keyY + 5, 0, 0);
    Renderer_DrawText("A", 243, keyY + 30, 0, 0);
    Renderer_DrawText("S", 278, keyY + 30, 0, 0);
    Renderer_DrawText("D", 313, keyY + 30, 0, 0);

    // Arrow keys
    Renderer_DrawText("ARROWS:", 460, keyY - 15, 7, 0);
    uint8_t upC = Input_IsKeyDown(VK_UP) ? 15 : 2;
    uint8_t dnC = Input_IsKeyDown(VK_DOWN) ? 15 : 2;
    uint8_t ltC = Input_IsKeyDown(VK_LEFT) ? 15 : 2;
    uint8_t rtC = Input_IsKeyDown(VK_RIGHT) ? 15 : 2;
    Renderer_FillRect(510, keyY, 25, 20, upC);
    Renderer_FillRect(475, keyY + 25, 25, 20, ltC);
    Renderer_FillRect(510, keyY + 25, 25, 20, dnC);
    Renderer_FillRect(545, keyY + 25, 25, 20, rtC);

    // Space bar
    uint8_t spC = Input_IsKeyDown(VK_SPACE) ? 15 : 2;
    Renderer_FillRect(370, keyY + 25, 80, 20, spC);
    Renderer_DrawText("SPACE", 385, keyY + 30, 0, 0);

    // FPS
    Renderer_FillRect(10, 370, 120, 20, 0);
    Renderer_DrawText("FPS:", 15, 375, 15, 0);
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "%.1f", stats->currentFPS);
    Renderer_DrawText(fpsText, 55, 375, 10, 0);

    // Audio
    Renderer_DrawText("AUDIO:", 10, 230, 15, 0);
    uint8_t vol = Audio_GetMasterVolume();
    Renderer_FillRect(10, 245, 60, 10, 2);
    Renderer_FillRect(10, 245, (vol * 60) / 255, 10, 10);
    Renderer_DrawRect(10, 245, 60, 10, 7);

    Renderer_DrawText("1234=PLAY", 10, 260, 7, 0);
    for (int i = 0; i < 4; i++) {
        uint8_t kc = Input_IsKeyDown('1' + i) ? 14 : 6;
        Renderer_FillRect(80 + i * 20, 258, 15, 12, kc);
        char num[2] = {static_cast<char>('1' + i), '\0'};
        Renderer_DrawText(num, 84 + i * 20, 260, 0, 0);
    }

    char audioText[32];
    snprintf(audioText, sizeof(audioText), "VOL:%d", vol);
    Renderer_DrawText(audioText, 10, 275, 7, 0);
    snprintf(audioText, sizeof(audioText), "PLAY:%d", Audio_GetPlayingCount());
    Renderer_DrawText(audioText, 80, 275, 7, 0);

    // Help
    Renderer_DrawText("ESC=QUIT P=PAUSE +/-=SPEED F=FULL", 180, 375, 7, 0);
    Renderer_DrawText("1234=SOUND M=MUTE []=VOL", 210, 385, 7, 0);
}

// Called at render rate (60 FPS)
void GameRender(void) {
    if (RenderMenuScreen()) return;
    if (g_inGameplay) { RenderGameplay(); return; }
    RenderDemoMode();
}

#pragma mark - Custom View for Input

@interface RAMetalView : MTKView
@end

@implementation RAMetalView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)event {
    Input_HandleKeyDown(event.keyCode, (uint16_t)event.modifierFlags);
}

- (void)keyUp:(NSEvent *)event {
    Input_HandleKeyUp(event.keyCode, (uint16_t)event.modifierFlags);
}

- (void)flagsChanged:(NSEvent *)event {
    static uint16_t lastModifiers = 0;
    uint16_t currentModifiers = (uint16_t)event.modifierFlags;
    uint16_t changed = lastModifiers ^ currentModifiers;
    BOOL pressed = (currentModifiers & changed) != 0;

    if (changed & NSEventModifierFlagShift) {
        if (pressed) Input_HandleKeyDown(56, currentModifiers);
        else Input_HandleKeyUp(56, currentModifiers);
    }
    if (changed & NSEventModifierFlagControl) {
        if (pressed) Input_HandleKeyDown(59, currentModifiers);
        else Input_HandleKeyUp(59, currentModifiers);
    }
    if (changed & NSEventModifierFlagOption) {
        if (pressed) Input_HandleKeyDown(58, currentModifiers);
        else Input_HandleKeyUp(58, currentModifiers);
    }
    if (changed & NSEventModifierFlagCommand) {
        if (pressed) Input_HandleKeyDown(55, currentModifiers);
        else Input_HandleKeyUp(55, currentModifiers);
    }

    lastModifiers = currentModifiers;
}

- (void)mouseDown:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_LEFT, TRUE);
}

- (void)mouseUp:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_LEFT, FALSE);
}

- (void)rightMouseDown:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_RIGHT, TRUE);
}

- (void)rightMouseUp:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_RIGHT, FALSE);
}

- (void)otherMouseDown:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_MIDDLE, TRUE);
}

- (void)otherMouseUp:(NSEvent *)event {
    (void)event;
    Input_HandleMouseButton(INPUT_MOUSE_MIDDLE, FALSE);
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    NSSize viewSize = self.bounds.size;

    // Scale mouse coordinates to game resolution (640x400)
    float sw = viewSize.width, sh = viewSize.height;
    int x = (int)(location.x * WINDOW_WIDTH / sw);
    int y = (int)((sh - location.y) * WINDOW_HEIGHT / sh);

    // Clamp to valid range
    if (x < 0) x = 0;
    if (x >= WINDOW_WIDTH) x = WINDOW_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= WINDOW_HEIGHT) y = WINDOW_HEIGHT - 1;

    Input_HandleMouseMove(x, y);
}

- (void)mouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

@end

#pragma mark - MTKView Delegate

@interface RAViewDelegate : NSObject <MTKViewDelegate>
@end

@implementation RAViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    (void)view;
    (void)size;
}

- (void)drawInMTKView:(MTKView *)view {
    (void)view;

    // Run game loop iteration (processes key presses set by NSEvent handlers)
    if (!GameLoop_RunFrame()) {
        // Quit requested
        [NSApp terminate:nil];
    }

    // Present to screen
    Renderer_Present();

    // Clear per-frame input state AFTER processing
    // Key events are set by keyDown: before this callback
    // They must persist until GameLoop_RunFrame processes them
    Input_Update();
}

@end

#pragma mark - Application Delegate

@interface RAAppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) RAViewDelegate *viewDelegate;
@end

@implementation RAAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    (void)notification;

    // Create window
    NSRect frame = NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    NSWindowStyleMask style = NSWindowStyleMaskTitled |
                              NSWindowStyleMaskClosable |
                              NSWindowStyleMaskMiniaturizable |
                              NSWindowStyleMaskResizable;

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];

    [self.window setTitle:@"Red Alert"];
    [self.window center];

    // Enable fullscreen support
    NSWindowCollectionBehavior fs = NSWindowCollectionBehaviorFullScreenPrimary;
    [self.window setCollectionBehavior:fs];

    // Set minimum window size to maintain aspect ratio readability
    [self.window setMinSize:NSMakeSize(640, 400)];

    // Store reference for fullscreen toggle
    g_mainWindow = self.window;

    // Set up Metal view
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        NSLog(@"Metal is not supported on this device");
        [NSApp terminate:nil];
        return;
    }

    RAMetalView *metalView;
    metalView = [[RAMetalView alloc] initWithFrame:frame device:device];
    metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    metalView.preferredFramesPerSecond = 60;

    // Enable mouse tracking
    NSTrackingAreaOptions opts = NSTrackingMouseMoved |
        NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc]
        initWithRect:metalView.bounds
             options:opts
               owner:metalView
            userInfo:nil];
    [metalView addTrackingArea:trackingArea];

    // Initialize all systems
    Input_Init();
    StubAssets_Init();
    GameLoop_Init();

    if (!Renderer_Init((__bridge void*)metalView)) {
        NSLog(@"Failed to initialize renderer");
        [NSApp terminate:nil];
        return;
    }

    // Initialize asset loader and load game assets
    if (Assets_Init()) {
        NSLog(@"AssetLoader initialized");
        // Load game palette
        if (Renderer_LoadPalette("SNOW.PAL")) {
            NSLog(@"Loaded SNOW.PAL palette");
        }
        // Initialize sprite system (loads all unit/building sprites)
        if (Sprites_Init()) {
            NSLog(@"Sprite system initialized");
            g_assetsLoaded = true;
        }
        // Initialize sound system (loads all game sound effects)
        if (Sounds_Init()) {
            NSLog(@"Sound system initialized");
        }
        // Initialize terrain system (loads terrain tiles)
        if (Terrain_Init()) {
            int cnt = Terrain_GetLoadedCount();
            NSLog(@"Terrain initialized (%d templates)", cnt);
        }
    } else {
        NSLog(@"AssetLoader failed (game archives not found)");
    }

    // Initialize audio
    if (!Audio_Init()) {
        NSLog(@"Warning: Audio initialization failed");
    }

    // Initialize music system (after audio)
    Music_Init();

    // Create test tones for audio demo
    for (int i = 0; i < 4; i++) {
        g_testTones[i] = Audio_CreateTestTone(g_toneFreqs[i], 200);
    }

    // Initialize menu system
    Menu_Init();
    Menu_SetNewGameCallback(StartDemoMission);
    Menu_SetStartCampaignCallback(StartCampaignMission);

    // Check if movies are available and play intro
    if (Assets_HasMovies()) {
        NSLog(@"Movies available - will play intro video");
        // Play intro video, then show main menu
        Menu_PlayVideo("PROLOG.VQA", []() {
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            // Start menu music (Hell March is the classic intro track)
            Music_PlayRandom();
            NSLog(@"Intro video complete, showing main menu");
        }, TRUE);
    } else {
        // No movies - go straight to main menu
        Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
        // Start menu music
        Music_PlayRandom();
    }

    // Set up game loop callbacks
    GameLoop_SetUpdateCallback(GameUpdate);
    GameLoop_SetRenderCallback(GameRender);
    GameLoop_SetState(GAME_STATE_PLAYING);

    // Set up view delegate for rendering
    self.viewDelegate = [[RAViewDelegate alloc] init];
    metalView.delegate = self.viewDelegate;

    [self.window setContentView:metalView];
    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:metalView];

    NSLog(@"Red Alert initialized - Metal device: %@", device.name);
    NSLog(@"Controls: WASD/Arrows=move, +/-=speed, P=pause, ESC=quit");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    (void)notification;

    // Clear window reference
    g_mainWindow = nil;

    // Free test tones
    for (int i = 0; i < 4; i++) {
        if (g_testTones[i]) {
            Audio_FreeTestTone(g_testTones[i]);
            g_testTones[i] = nullptr;
        }
    }

    // Shutdown sprite, sound, and terrain systems
    Terrain_Shutdown();
    Sounds_Shutdown();
    Sprites_Shutdown();
    g_assetsLoaded = false;

    Menu_Shutdown();
    Music_Shutdown();
    GameLoop_Shutdown();
    Audio_Shutdown();
    Renderer_Shutdown();
    Assets_Shutdown();
    Input_Shutdown();
    StubAssets_Shutdown();
    NSLog(@"Red Alert shutting down");
}

@end

#pragma mark - Main

int main(int argc, const char *argv[]) {
    (void)argc;
    (void)argv;

    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Create menu bar
        NSMenu *menuBar = [[NSMenu alloc] init];
        NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
        [menuBar addItem:appMenuItem];

        NSMenu *appMenu = [[NSMenu alloc] init];
        NSMenuItem *quitItem = [[NSMenuItem alloc]
            initWithTitle:@"Quit Red Alert"
                   action:@selector(terminate:)
            keyEquivalent:@"q"];
        [appMenu addItem:quitItem];
        [appMenuItem setSubmenu:appMenu];

        [app setMainMenu:menuBar];

        // Set delegate and run
        RAAppDelegate *delegate = [[RAAppDelegate alloc] init];
        [app setDelegate:delegate];
        [app activateIgnoringOtherApps:YES];
        [app run];
    }

    return 0;
}
