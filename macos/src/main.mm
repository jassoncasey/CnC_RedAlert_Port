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
#include "compat/assets.h"
#include <cmath>

// Game window dimensions (original Red Alert resolution)
static constexpr int WINDOW_WIDTH = 640;
static constexpr int WINDOW_HEIGHT = 400;

// Animation state for demo
static float g_animPhase = 0.0f;
static int g_bounceX = 100;
static int g_bounceY = 100;
static int g_bounceVX = 3;
static int g_bounceVY = 2;

#pragma mark - Game Callbacks

// Called at game logic rate (15 FPS default)
void GameUpdate(uint32_t frame, float deltaTime) {
    (void)deltaTime;

    // Update animation phase
    g_animPhase += 0.1f;

    // Update bouncing box
    g_bounceX += g_bounceVX;
    g_bounceY += g_bounceVY;

    if (g_bounceX <= 0 || g_bounceX >= WINDOW_WIDTH - 50) {
        g_bounceVX = -g_bounceVX;
    }
    if (g_bounceY <= 0 || g_bounceY >= WINDOW_HEIGHT - 50) {
        g_bounceVY = -g_bounceVY;
    }

    // Handle game input
    if (Input_WasKeyPressed(VK_ESCAPE)) {
        GameLoop_Quit();
    }

    // Speed controls (+ and -)
    if (Input_WasKeyPressed(VK_ADD) || Input_WasKeyPressed('=')) {
        int speed = GameLoop_GetSpeed();
        if (speed > 0) GameLoop_SetSpeed(speed - 1);
    }
    if (Input_WasKeyPressed(VK_SUBTRACT) || Input_WasKeyPressed('-')) {
        int speed = GameLoop_GetSpeed();
        if (speed < 7) GameLoop_SetSpeed(speed + 1);
    }

    // Pause (P key)
    if (Input_WasKeyPressed('P')) {
        GameLoop_Pause(!GameLoop_IsPaused());
    }

    // Log every 60 game frames (roughly every 4 seconds at speed 4)
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

// Called every render frame (60 FPS)
void GameRender(void) {
    const FrameStats* stats = GameLoop_GetStats();

    // Clear to dark gray
    Renderer_Clear(8);

    // === MILESTONE 9: Drawing Primitives Demo ===

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

    // Controls help
    Renderer_DrawText("ESC=QUIT  P=PAUSE  +/-=SPEED", 350, 375, 7, 0);
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
    int x = (int)location.x;
    int y = WINDOW_HEIGHT - (int)location.y - 1;
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
                              NSWindowStyleMaskMiniaturizable;

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];

    [self.window setTitle:@"Red Alert - Rendering Test"];
    [self.window center];

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
    GameLoop_Shutdown();
    Renderer_Shutdown();
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
