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

// Called every render frame (60 FPS)
void GameRender(void) {
    const FrameStats* stats = GameLoop_GetStats();

    // Clear to dark gray
    Renderer_Clear(8);

    // Draw bouncing box (changes color based on game frame)
    uint8_t boxColor = 1 + (stats->gameFrame % 14);  // Cycle through colors 1-14
    Renderer_FillRect(g_bounceX, g_bounceY, 50, 50, boxColor);

    // Draw static colored rectangles
    Renderer_FillRect(50, 50, 80, 60, 4);    // Dark red
    Renderer_FillRect(150, 50, 80, 60, 2);   // Dark green
    Renderer_FillRect(250, 50, 80, 60, 1);   // Dark blue
    Renderer_FillRect(350, 50, 80, 60, 14);  // Yellow

    // Draw mouse cursor
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    Renderer_FillRect(mx - 3, my - 3, 6, 6, 15);  // White cursor

    // Draw mouse button indicators
    uint8_t buttons = Input_GetMouseButtons();
    Renderer_FillRect(50, 250, 30, 25, (buttons & INPUT_MOUSE_LEFT) ? 12 : 4);
    Renderer_FillRect(90, 250, 30, 25, (buttons & INPUT_MOUSE_RIGHT) ? 9 : 1);
    Renderer_FillRect(130, 250, 30, 25, (buttons & INPUT_MOUSE_MIDDLE) ? 10 : 2);

    // Draw speed indicator (bar graph)
    int speed = GameLoop_GetSpeed();
    for (int i = 0; i <= 7; i++) {
        uint8_t barColor = (i <= speed) ? 10 : 2;  // Green if active
        Renderer_FillRect(450 + i * 20, 250, 15, 25, barColor);
    }

    // Draw pause indicator
    if (GameLoop_IsPaused()) {
        // Draw "PAUSED" indicator (two bars)
        Renderer_FillRect(280, 180, 20, 60, 15);
        Renderer_FillRect(320, 180, 20, 60, 15);
    }

    // Draw WASD keys
    int keyY = 300;
    Renderer_FillRect(270, keyY, 30, 25, Input_IsKeyDown('W') ? 15 : 7);
    Renderer_FillRect(230, keyY + 30, 30, 25, Input_IsKeyDown('A') ? 15 : 7);
    Renderer_FillRect(270, keyY + 30, 30, 25, Input_IsKeyDown('S') ? 15 : 7);
    Renderer_FillRect(310, keyY + 30, 30, 25, Input_IsKeyDown('D') ? 15 : 7);

    // Space bar
    Renderer_FillRect(370, keyY + 30, 80, 25, Input_IsKeyDown(VK_SPACE) ? 15 : 7);

    // Arrow keys
    Renderer_FillRect(520, keyY, 30, 25, Input_IsKeyDown(VK_UP) ? 15 : 7);
    Renderer_FillRect(480, keyY + 30, 30, 25, Input_IsKeyDown(VK_LEFT) ? 15 : 7);
    Renderer_FillRect(520, keyY + 30, 30, 25, Input_IsKeyDown(VK_DOWN) ? 15 : 7);
    Renderer_FillRect(560, keyY + 30, 30, 25, Input_IsKeyDown(VK_RIGHT) ? 15 : 7);

    // FPS display area (just a box, no text yet)
    Renderer_FillRect(10, 10, 80, 25, 0);

    // Frame counter visualization (bar that fills based on render frame)
    int barWidth = (stats->frameCount % 60) * 2;
    Renderer_FillRect(100, 10, barWidth, 10, 10);
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

    [self.window setTitle:@"Red Alert - Game Loop Test"];
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
