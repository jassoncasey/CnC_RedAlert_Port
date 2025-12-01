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
#include "audio/audio.h"
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

// Assets loaded flag
static bool g_assetsLoaded = false;

// Toggle fullscreen mode
static void ToggleFullscreen(void) {
    if (g_mainWindow) {
        [g_mainWindow toggleFullScreen:nil];
    }
}

// Start a demo mission
static void StartDemoMission(void) {
    g_inGameplay = true;

    // Initialize map and units
    Map_Init();
    Units_Init();
    GameUI_Init();

    // Generate demo map
    Map_GenerateDemo();

    // Spawn player units (Allies)
    Units_Spawn(UNIT_TANK_MEDIUM, TEAM_PLAYER, 100, 400);
    Units_Spawn(UNIT_TANK_MEDIUM, TEAM_PLAYER, 140, 420);
    Units_Spawn(UNIT_TANK_LIGHT, TEAM_PLAYER, 180, 400);
    Units_Spawn(UNIT_TANK_LIGHT, TEAM_PLAYER, 180, 440);
    Units_Spawn(UNIT_RIFLE, TEAM_PLAYER, 80, 450);
    Units_Spawn(UNIT_RIFLE, TEAM_PLAYER, 100, 450);
    Units_Spawn(UNIT_RIFLE, TEAM_PLAYER, 120, 450);
    Units_Spawn(UNIT_ROCKET, TEAM_PLAYER, 60, 430);
    Units_Spawn(UNIT_HARVESTER, TEAM_PLAYER, 200, 500);

    // Spawn enemy units (Soviet)
    Units_Spawn(UNIT_TANK_HEAVY, TEAM_ENEMY, 1200, 300);
    Units_Spawn(UNIT_TANK_MEDIUM, TEAM_ENEMY, 1150, 350);
    Units_Spawn(UNIT_TANK_MEDIUM, TEAM_ENEMY, 1250, 350);
    Units_Spawn(UNIT_RIFLE, TEAM_ENEMY, 1100, 400);
    Units_Spawn(UNIT_RIFLE, TEAM_ENEMY, 1130, 400);
    Units_Spawn(UNIT_RIFLE, TEAM_ENEMY, 1160, 400);
    Units_Spawn(UNIT_ROCKET, TEAM_ENEMY, 1200, 250);

    // Spawn buildings
    Buildings_Spawn(BUILDING_CONSTRUCTION, TEAM_PLAYER, 2, 15);
    Buildings_Spawn(BUILDING_POWER, TEAM_PLAYER, 6, 16);
    Buildings_Spawn(BUILDING_BARRACKS, TEAM_PLAYER, 2, 19);
    Buildings_Spawn(BUILDING_REFINERY, TEAM_PLAYER, 6, 19);

    Buildings_Spawn(BUILDING_CONSTRUCTION, TEAM_ENEMY, 55, 10);
    Buildings_Spawn(BUILDING_POWER, TEAM_ENEMY, 59, 11);
    Buildings_Spawn(BUILDING_TURRET, TEAM_ENEMY, 52, 14);
    Buildings_Spawn(BUILDING_TURRET, TEAM_ENEMY, 58, 8);

    // Center viewport on player base
    Map_CenterViewport(150, 450);

    NSLog(@"Demo mission started!");
}

#pragma mark - Game Callbacks

// Called at game logic rate (15 FPS default)
void GameUpdate(uint32_t frame, float deltaTime) {
    (void)deltaTime;

    // Handle menus
    MenuScreen currentScreen = Menu_GetCurrentScreen();
    if (currentScreen != MENU_SCREEN_NONE) {
        // Update active menu
        Menu* activeMenu = nullptr;
        switch (currentScreen) {
            case MENU_SCREEN_MAIN:
                activeMenu = Menu_GetMainMenu();
                break;
            case MENU_SCREEN_OPTIONS:
                activeMenu = Menu_GetOptionsMenu();
                break;
            case MENU_SCREEN_CREDITS:
                // Show credits then return to main on any key or mouse click
                if (Input_WasKeyPressed(VK_ESCAPE) || Input_WasKeyPressed(VK_RETURN) ||
                    Input_WasKeyPressed(VK_SPACE) || (Input_GetMouseButtons() & INPUT_MOUSE_LEFT)) {
                    Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
                }
                break;
            default:
                break;
        }

        if (activeMenu) {
            Menu_Update(activeMenu);
        }

        // ESC goes back from submenus
        if (Input_WasKeyPressed(VK_ESCAPE)) {
            if (currentScreen == MENU_SCREEN_OPTIONS) {
                Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            }
        }
        return; // Don't process game logic while in menus
    }

    // === GAMEPLAY MODE ===
    if (g_inGameplay) {
        // ESC returns to main menu
        if (Input_WasKeyPressed(VK_ESCAPE)) {
            g_inGameplay = false;
            GameUI_Shutdown();
            Map_Shutdown();
            Units_Shutdown();
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            return;
        }

        // Map scrolling with arrow keys or WASD
        int scrollSpeed = 8;
        if (Input_IsKeyDown(VK_UP) || Input_IsKeyDown('W')) {
            Map_ScrollViewport(0, -scrollSpeed);
        }
        if (Input_IsKeyDown(VK_DOWN) || Input_IsKeyDown('S')) {
            Map_ScrollViewport(0, scrollSpeed);
        }
        if (Input_IsKeyDown(VK_LEFT) || Input_IsKeyDown('A')) {
            Map_ScrollViewport(-scrollSpeed, 0);
        }
        if (Input_IsKeyDown(VK_RIGHT) || Input_IsKeyDown('D')) {
            Map_ScrollViewport(scrollSpeed, 0);
        }

        // Mouse edge scrolling
        int mx = Input_GetMouseX();
        int my = Input_GetMouseY();
        if (mx < 10) Map_ScrollViewport(-scrollSpeed, 0);
        if (mx > WINDOW_WIDTH - 10) Map_ScrollViewport(scrollSpeed, 0);
        if (my < 10) Map_ScrollViewport(0, -scrollSpeed);
        if (my > WINDOW_HEIGHT - 10) Map_ScrollViewport(0, scrollSpeed);

        // Unit selection with left mouse button
        uint8_t buttons = Input_GetMouseButtons();
        static bool wasLeftDown = false;
        bool leftDown = (buttons & INPUT_MOUSE_LEFT) != 0;
        bool rightDown = (buttons & INPUT_MOUSE_RIGHT) != 0;

        // Handle UI input first (sidebar, radar)
        if (leftDown && !wasLeftDown) {
            if (GameUI_HandleInput(mx, my, TRUE, FALSE)) {
                // UI consumed the click, don't process as game input
                wasLeftDown = leftDown;
                return;
            }
        }

        if (leftDown && !wasLeftDown) {
            // Start selection
            g_selectionStartX = mx;
            g_selectionStartY = my;
            g_isSelecting = true;
        } else if (!leftDown && wasLeftDown && g_isSelecting) {
            // End selection
            int x1 = g_selectionStartX;
            int y1 = g_selectionStartY;
            int x2 = mx;
            int y2 = my;

            // If click (small drag), try to select single unit
            if (abs(x2 - x1) < 5 && abs(y2 - y1) < 5) {
                int unitId = Units_GetAtScreen(mx, my);
                if (unitId >= 0) {
                    Units_Select(unitId, Input_IsKeyDown(VK_SHIFT));
                } else {
                    Units_DeselectAll();
                }
            } else {
                // Box selection
                Units_SelectInRect(x1, y1, x2, y2, TEAM_PLAYER);
            }
            g_isSelecting = false;
        }
        wasLeftDown = leftDown;

        // Right click commands
        static bool wasRightDown = false;

        if (rightDown && !wasRightDown) {
            int worldX, worldY;
            Map_ScreenToWorld(mx, my, &worldX, &worldY);

            // Check if clicking on enemy unit
            int targetId = Units_GetAtScreen(mx, my);
            Unit* target = Units_Get(targetId);

            // Command selected units
            for (int i = 0; i < MAX_UNITS; i++) {
                Unit* unit = Units_Get(i);
                if (unit && unit->selected) {
                    if (target && target->team == TEAM_ENEMY) {
                        // Attack command
                        Units_CommandAttack(i, targetId);
                    } else {
                        // Move command
                        Units_CommandMove(i, worldX, worldY);
                    }
                }
            }
        }
        wasRightDown = rightDown;

        // Stop command (S key)
        if (Input_WasKeyPressed('S') && !Input_IsKeyDown('W')) {
            for (int i = 0; i < MAX_UNITS; i++) {
                Unit* unit = Units_Get(i);
                if (unit && unit->selected) {
                    Units_CommandStop(i);
                }
            }
        }

        // Update game systems
        Map_Update();
        Units_Update();
        GameUI_Update();

        // Pause (P key)
        if (Input_WasKeyPressed('P')) {
            GameLoop_Pause(!GameLoop_IsPaused());
        }

        // Fullscreen (F key)
        if (Input_WasKeyPressed('F')) {
            ToggleFullscreen();
        }

        return;
    }

    // === OLD DEMO MODE (when not in gameplay) ===
    g_animPhase += 0.1f;

    // Handle game input
    if (Input_WasKeyPressed(VK_ESCAPE)) {
        // First try to cancel placement mode
        if (!GameUI_HandleEscape()) {
            // If not in placement mode, go to menu
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
        }
    }

    // Pause (P key)
    if (Input_WasKeyPressed('P')) {
        GameLoop_Pause(!GameLoop_IsPaused());
    }

    // Fullscreen (F key)
    if (Input_WasKeyPressed('F')) {
        ToggleFullscreen();
    }

    // Log every 60 game frames
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
void GameRender(void) {
    const FrameStats* stats = GameLoop_GetStats();

    // Handle menu rendering
    MenuScreen currentScreen = Menu_GetCurrentScreen();
    if (currentScreen != MENU_SCREEN_NONE) {
        Menu* activeMenu = nullptr;
        switch (currentScreen) {
            case MENU_SCREEN_MAIN:
                activeMenu = Menu_GetMainMenu();
                break;
            case MENU_SCREEN_OPTIONS:
                activeMenu = Menu_GetOptionsMenu();
                break;
            case MENU_SCREEN_CREDITS:
                RenderCredits();
                return;
            default:
                break;
        }

        if (activeMenu) {
            Menu_Render(activeMenu);
        }
        return;
    }

    // === GAMEPLAY RENDERING ===
    if (g_inGameplay) {
        Renderer_Clear(0);

        // Clip to game view area (don't draw over sidebar)
        Renderer_SetClipRect(0, 16, 560, 368);

        // Render map terrain
        Map_Render();

        // Render units and buildings
        Units_Render();

        // Reset clip for UI
        Renderer_ResetClip();

        // Render game UI (sidebar, radar, selection panel)
        GameUI_Render();

        // Draw selection box if dragging
        if (g_isSelecting) {
            int mx = Input_GetMouseX();
            int my = Input_GetMouseY();
            int x1 = g_selectionStartX;
            int y1 = g_selectionStartY;
            int x2 = mx;
            int y2 = my;

            // Normalize
            if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
            if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

            Renderer_DrawRect(x1, y1, x2 - x1, y2 - y1, 15);
        }

        // Draw mouse cursor
        int mx = Input_GetMouseX();
        int my = Input_GetMouseY();
        Renderer_DrawLine(mx - 8, my, mx + 8, my, 15);
        Renderer_DrawLine(mx, my - 8, mx, my + 8, 15);

        // Draw HUD (top bar - game view only, sidebar has its own)
        const FrameStats* stats = GameLoop_GetStats();
        Renderer_FillRect(0, 0, 560, 16, 0);
        Renderer_DrawText("RED ALERT - DEMO", 10, 3, 14, 0);

        // Unit count
        char hudText[64];
        snprintf(hudText, sizeof(hudText), "P:%d E:%d",
                 Units_CountByTeam(TEAM_PLAYER), Units_CountByTeam(TEAM_ENEMY));
        Renderer_DrawText(hudText, 200, 3, 10, 0);

        // Selected count
        int selected = Units_GetSelectedCount();
        if (selected > 0) {
            snprintf(hudText, sizeof(hudText), "SEL:%d", selected);
            Renderer_DrawText(hudText, 300, 3, 15, 0);
        }

        // FPS
        snprintf(hudText, sizeof(hudText), "%.0f", stats->currentFPS);
        Renderer_DrawText(hudText, 540, 3, 7, 0);

        // Pause overlay
        if (GameLoop_IsPaused()) {
            Renderer_FillRect(220, 180, 120, 40, 0);
            Renderer_DrawRect(220, 180, 120, 40, 15);
            Renderer_DrawText("PAUSED", 245, 195, 15, 0);
        }

        // Controls help at bottom (only in game view area)
        Renderer_FillRect(0, 384, 560, 16, 0);
        Renderer_DrawText("WASD=SCROLL LMB=SELECT RMB=CMD ESC=MENU", 20, 387, 7, 0);

        return;
    }

    // Clear to dark gray
    Renderer_Clear(8);

    // === OLD DEMO (when not in gameplay) ===

    // Draw title text
    Renderer_DrawText("RED ALERT MACOS PORT", 200, 10, 15, 0);
    Renderer_DrawText("MILESTONE 9: RENDERING", 195, 25, 14, 0);

    // Draw lines radiating from center
    int cx = 100, cy = 100;
    for (int angle = 0; angle < 360; angle += 30) {
        float rad = angle * 3.14159f / 180.0f;
        int ex = cx + (int)(50 * cosf(rad));
        int ey = cy + (int)(50 * sinf(rad));
        uint8_t color = 1 + (angle / 30) % 14;
        Renderer_DrawLine(cx, cy, ex, ey, color);
    }

    // Draw circles
    Renderer_DrawCircle(250, 100, 40, 12);  // Red outline
    Renderer_FillCircle(250, 100, 30, 4);   // Dark red fill
    Renderer_DrawCircle(350, 100, 40, 10);  // Green outline
    Renderer_FillCircle(350, 100, 30, 2);   // Dark green fill
    Renderer_DrawCircle(450, 100, 40, 9);   // Blue outline
    Renderer_FillCircle(450, 100, 30, 1);   // Dark blue fill

    // Draw rectangle outlines
    Renderer_DrawRect(520, 60, 60, 80, 14);  // Yellow outline
    Renderer_DrawRect(525, 65, 50, 70, 6);   // Cyan outline

    // Draw horizontal and vertical lines
    Renderer_HLine(50, 590, 160, 7);   // Gray horizontal
    Renderer_VLine(320, 170, 240, 7);  // Gray vertical center

    // Draw sprites with transparency (index 0 = transparent)
    Renderer_Blit(g_testSprite, 16, 16, 50, 180, TRUE);
    Renderer_Blit(g_testSprite, 16, 16, 80, 180, TRUE);
    Renderer_Blit(g_testSprite, 16, 16, 110, 180, TRUE);

    // Draw scaled sprite
    Renderer_ScaleBlit(g_testSprite, 16, 16, 150, 170, 48, 48, TRUE);

    // Bouncing box (changes color based on game frame)
    uint8_t boxColor = 1 + (stats->gameFrame % 14);
    Renderer_FillRect(g_bounceX, g_bounceY, 30, 30, boxColor);
    Renderer_DrawRect(g_bounceX - 2, g_bounceY - 2, 34, 34, 15);

    // Draw mouse cursor with crosshair
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    Renderer_DrawLine(mx - 10, my, mx + 10, my, 15);
    Renderer_DrawLine(mx, my - 10, mx, my + 10, 15);
    Renderer_FillCircle(mx, my, 3, 12);

    // Mouse button indicators
    uint8_t buttons = Input_GetMouseButtons();
    Renderer_FillCircle(60, 260, 12, (buttons & INPUT_MOUSE_LEFT) ? 12 : 4);
    Renderer_FillCircle(100, 260, 12, (buttons & INPUT_MOUSE_RIGHT) ? 9 : 1);
    Renderer_FillCircle(140, 260, 12, (buttons & INPUT_MOUSE_MIDDLE) ? 10 : 2);
    Renderer_DrawText("L", 56, 255, 15, 0);
    Renderer_DrawText("R", 96, 255, 15, 0);
    Renderer_DrawText("M", 135, 255, 15, 0);

    // Speed indicator with text
    int speed = GameLoop_GetSpeed();
    Renderer_DrawText("SPEED:", 450, 180, 15, 0);
    for (int i = 0; i <= 7; i++) {
        uint8_t barColor = (i <= speed) ? 10 : 2;
        Renderer_FillRect(450 + i * 15, 195, 12, 20, barColor);
    }

    // Pause indicator with text
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
    Renderer_FillRect(510, keyY, 25, 20, Input_IsKeyDown(VK_UP) ? 15 : 2);
    Renderer_FillRect(475, keyY + 25, 25, 20, Input_IsKeyDown(VK_LEFT) ? 15 : 2);
    Renderer_FillRect(510, keyY + 25, 25, 20, Input_IsKeyDown(VK_DOWN) ? 15 : 2);
    Renderer_FillRect(545, keyY + 25, 25, 20, Input_IsKeyDown(VK_RIGHT) ? 15 : 2);

    // Space bar
    Renderer_FillRect(370, keyY + 25, 80, 20, Input_IsKeyDown(VK_SPACE) ? 15 : 2);
    Renderer_DrawText("SPACE", 385, keyY + 30, 0, 0);

    // FPS display
    Renderer_FillRect(10, 370, 120, 20, 0);
    Renderer_DrawText("FPS:", 15, 375, 15, 0);
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "%.1f", stats->currentFPS);
    Renderer_DrawText(fpsText, 55, 375, 10, 0);

    // Audio section
    Renderer_DrawText("AUDIO:", 10, 230, 15, 0);

    // Volume bar
    uint8_t vol = Audio_GetMasterVolume();
    int volBarWidth = (vol * 60) / 255;
    Renderer_FillRect(10, 245, 60, 10, 2);
    Renderer_FillRect(10, 245, volBarWidth, 10, 10);
    Renderer_DrawRect(10, 245, 60, 10, 7);

    // Sound buttons (1-4)
    Renderer_DrawText("1234=PLAY", 10, 260, 7, 0);
    for (int i = 0; i < 4; i++) {
        uint8_t btnColor = Input_IsKeyDown('1' + i) ? 14 : 6;
        Renderer_FillRect(80 + i * 20, 258, 15, 12, btnColor);
        char num[2] = {static_cast<char>('1' + i), '\0'};
        Renderer_DrawText(num, 84 + i * 20, 260, 0, 0);
    }

    // Audio info
    char audioText[32];
    snprintf(audioText, sizeof(audioText), "VOL:%d", vol);
    Renderer_DrawText(audioText, 10, 275, 7, 0);
    snprintf(audioText, sizeof(audioText), "PLAYING:%d", Audio_GetPlayingCount());
    Renderer_DrawText(audioText, 80, 275, 7, 0);

    // Controls help
    Renderer_DrawText("ESC=QUIT P=PAUSE +/-=SPEED F=FULLSCREEN", 180, 375, 7, 0);
    Renderer_DrawText("1234=SOUND M=MUTE []=VOL", 210, 385, 7, 0);
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
    int x = (int)(location.x * WINDOW_WIDTH / viewSize.width);
    int y = (int)((viewSize.height - location.y) * WINDOW_HEIGHT / viewSize.height);

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

    // Process input
    Input_Update();

    // Run game loop iteration
    if (!GameLoop_RunFrame()) {
        // Quit requested
        [NSApp terminate:nil];
    }

    // Present to screen
    Renderer_Present();
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
    [self.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

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

    RAMetalView *metalView = [[RAMetalView alloc] initWithFrame:frame device:device];
    metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    metalView.preferredFramesPerSecond = 60;

    // Enable mouse tracking
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc]
        initWithRect:metalView.bounds
             options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect)
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
            NSLog(@"Terrain system initialized (%d templates)", Terrain_GetLoadedCount());
        }
    } else {
        NSLog(@"Warning: AssetLoader failed to initialize (game archives not found)");
    }

    // Initialize audio
    if (!Audio_Init()) {
        NSLog(@"Warning: Audio initialization failed");
    }

    // Create test tones for audio demo
    for (int i = 0; i < 4; i++) {
        g_testTones[i] = Audio_CreateTestTone(g_toneFreqs[i], 200);
    }

    // Initialize menu system
    Menu_Init();
    Menu_SetNewGameCallback(StartDemoMission);
    Menu_SetCurrentScreen(MENU_SCREEN_MAIN);

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

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
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
        NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit Red Alert"
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
