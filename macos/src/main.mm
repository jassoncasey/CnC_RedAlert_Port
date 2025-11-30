/**
 * Red Alert macOS Port - Entry Point
 *
 * AppKit application shell with Metal rendering and input handling.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "graphics/metal/renderer.h"
#include "input/input.h"
#include "compat/assets.h"

// Game window dimensions (original Red Alert resolution)
static constexpr int WINDOW_WIDTH = 640;
static constexpr int WINDOW_HEIGHT = 400;

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
    // Handle modifier key changes
    static uint16_t lastModifiers = 0;
    uint16_t currentModifiers = (uint16_t)event.modifierFlags;

    // Detect which modifier changed
    uint16_t changed = lastModifiers ^ currentModifiers;
    BOOL pressed = (currentModifiers & changed) != 0;

    // Map modifier flag to keycode
    if (changed & NSEventModifierFlagShift) {
        if (pressed) Input_HandleKeyDown(56, currentModifiers);  // kVK_Shift
        else Input_HandleKeyUp(56, currentModifiers);
    }
    if (changed & NSEventModifierFlagControl) {
        if (pressed) Input_HandleKeyDown(59, currentModifiers);  // kVK_Control
        else Input_HandleKeyUp(59, currentModifiers);
    }
    if (changed & NSEventModifierFlagOption) {
        if (pressed) Input_HandleKeyDown(58, currentModifiers);  // kVK_Option
        else Input_HandleKeyUp(58, currentModifiers);
    }
    if (changed & NSEventModifierFlagCommand) {
        if (pressed) Input_HandleKeyDown(55, currentModifiers);  // kVK_Command
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
    // Flip Y coordinate (macOS has origin at bottom-left)
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
@property (nonatomic) BOOL showInputDebug;
@end

@implementation RAViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    (void)view;
    (void)size;
}

- (void)drawInMTKView:(MTKView *)view {
    (void)view;

    // Process input at start of frame
    Input_Update();

    // Draw test pattern with input visualization
    [self drawTestPattern];

    // Present the framebuffer
    Renderer_Present();
}

- (void)drawTestPattern {
    // Clear to dark gray
    Renderer_Clear(8);

    // Draw colored rectangles using palette colors
    Renderer_FillRect(50, 50, 100, 80, 4);    // Dark red
    Renderer_FillRect(200, 50, 100, 80, 2);   // Dark green
    Renderer_FillRect(350, 50, 100, 80, 1);   // Dark blue
    Renderer_FillRect(500, 50, 100, 80, 14);  // Yellow

    // Bright variants
    Renderer_FillRect(50, 150, 100, 80, 12);  // Bright red
    Renderer_FillRect(200, 150, 100, 80, 10); // Bright green
    Renderer_FillRect(350, 150, 100, 80, 9);  // Bright blue
    Renderer_FillRect(500, 150, 100, 80, 11); // Bright cyan

    // Draw mouse cursor indicator (white square at mouse position)
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    Renderer_FillRect(mx - 5, my - 5, 10, 10, 15);  // White

    // Draw mouse button indicators
    uint8_t buttons = Input_GetMouseButtons();
    if (buttons & INPUT_MOUSE_LEFT) {
        Renderer_FillRect(50, 280, 40, 30, 12);   // Red when left pressed
    } else {
        Renderer_FillRect(50, 280, 40, 30, 4);    // Dark red otherwise
    }

    if (buttons & INPUT_MOUSE_RIGHT) {
        Renderer_FillRect(100, 280, 40, 30, 9);   // Blue when right pressed
    } else {
        Renderer_FillRect(100, 280, 40, 30, 1);   // Dark blue otherwise
    }

    if (buttons & INPUT_MOUSE_MIDDLE) {
        Renderer_FillRect(150, 280, 40, 30, 10);  // Green when middle pressed
    } else {
        Renderer_FillRect(150, 280, 40, 30, 2);   // Dark green otherwise
    }

    // Draw key indicators for common keys (WASD, Space, Escape)
    int keyY = 330;

    // W key
    if (Input_IsKeyDown('W')) {
        Renderer_FillRect(270, keyY, 30, 25, 15);
    } else {
        Renderer_FillRect(270, keyY, 30, 25, 7);
    }

    // A key
    if (Input_IsKeyDown('A')) {
        Renderer_FillRect(230, keyY + 30, 30, 25, 15);
    } else {
        Renderer_FillRect(230, keyY + 30, 30, 25, 7);
    }

    // S key
    if (Input_IsKeyDown('S')) {
        Renderer_FillRect(270, keyY + 30, 30, 25, 15);
    } else {
        Renderer_FillRect(270, keyY + 30, 30, 25, 7);
    }

    // D key
    if (Input_IsKeyDown('D')) {
        Renderer_FillRect(310, keyY + 30, 30, 25, 15);
    } else {
        Renderer_FillRect(310, keyY + 30, 30, 25, 7);
    }

    // Space bar
    if (Input_IsKeyDown(VK_SPACE)) {
        Renderer_FillRect(370, keyY + 30, 100, 25, 15);
    } else {
        Renderer_FillRect(370, keyY + 30, 100, 25, 7);
    }

    // Escape
    if (Input_IsKeyDown(VK_ESCAPE)) {
        Renderer_FillRect(500, keyY, 50, 25, 12);
    } else {
        Renderer_FillRect(500, keyY, 50, 25, 4);
    }

    // Arrow keys
    if (Input_IsKeyDown(VK_UP)) {
        Renderer_FillRect(550, keyY + 30, 30, 25, 15);
    } else {
        Renderer_FillRect(550, keyY + 30, 30, 25, 7);
    }

    if (Input_IsKeyDown(VK_LEFT)) {
        Renderer_FillRect(510, keyY + 60, 30, 25, 15);
    } else {
        Renderer_FillRect(510, keyY + 60, 30, 25, 7);
    }

    if (Input_IsKeyDown(VK_DOWN)) {
        Renderer_FillRect(550, keyY + 60, 30, 25, 15);
    } else {
        Renderer_FillRect(550, keyY + 60, 30, 25, 7);
    }

    if (Input_IsKeyDown(VK_RIGHT)) {
        Renderer_FillRect(590, keyY + 60, 30, 25, 15);
    } else {
        Renderer_FillRect(590, keyY + 60, 30, 25, 7);
    }
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

    [self.window setTitle:@"Red Alert - Input Test"];
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

    // Initialize systems
    Input_Init();
    StubAssets_Init();

    if (!Renderer_Init((__bridge void*)metalView)) {
        NSLog(@"Failed to initialize renderer");
        [NSApp terminate:nil];
        return;
    }

    // Set up view delegate for rendering
    self.viewDelegate = [[RAViewDelegate alloc] init];
    metalView.delegate = self.viewDelegate;

    [self.window setContentView:metalView];
    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:metalView];

    NSLog(@"Red Alert initialized - Metal device: %@", device.name);
    NSLog(@"Press WASD, arrow keys, Space, Escape. Move mouse. Click buttons.");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    (void)notification;
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

        // Create menu bar with Quit option
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
