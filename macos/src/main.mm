/**
 * Red Alert macOS Port - Entry Point
 *
 * AppKit application shell with Metal rendering.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "graphics/metal/renderer.h"
#include "compat/assets.h"

// Game window dimensions (original Red Alert resolution)
static constexpr int WINDOW_WIDTH = 640;
static constexpr int WINDOW_HEIGHT = 400;

#pragma mark - MTKView Delegate

@interface RAViewDelegate : NSObject <MTKViewDelegate>
@end

@implementation RAViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    (void)view;
    (void)size;
    // Handle resize if needed
}

- (void)drawInMTKView:(MTKView *)view {
    (void)view;
    // Present the framebuffer
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

    [self.window setTitle:@"Red Alert"];
    [self.window center];

    // Set up Metal view
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        NSLog(@"Metal is not supported on this device");
        [NSApp terminate:nil];
        return;
    }

    MTKView *metalView = [[MTKView alloc] initWithFrame:frame device:device];
    metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    metalView.preferredFramesPerSecond = 60;

    // Initialize renderer
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

    // Initialize stub assets
    StubAssets_Init();

    // Draw test pattern to verify rendering works
    [self drawTestPattern];

    NSLog(@"Red Alert initialized - Metal device: %@", device.name);
}

- (void)drawTestPattern {
    // Clear to black
    Renderer_Clear(0);

    // Draw colored rectangles using palette colors
    // Red rectangle (palette index 4 = dark red)
    Renderer_FillRect(50, 50, 100, 80, 4);

    // Green rectangle (palette index 2 = dark green)
    Renderer_FillRect(200, 50, 100, 80, 2);

    // Blue rectangle (palette index 1 = dark blue)
    Renderer_FillRect(350, 50, 100, 80, 1);

    // Bright variants
    Renderer_FillRect(50, 180, 100, 80, 12);   // Bright red
    Renderer_FillRect(200, 180, 100, 80, 10);  // Bright green
    Renderer_FillRect(350, 180, 100, 80, 9);   // Bright blue

    // Yellow and cyan
    Renderer_FillRect(500, 50, 100, 80, 14);   // Yellow
    Renderer_FillRect(500, 180, 100, 80, 11);  // Bright cyan

    // Gray gradient at bottom
    for (int i = 0; i < 16; i++) {
        int x = 40 + i * 35;
        Renderer_FillRect(x, 300, 30, 60, i);
    }

    // White text area placeholder
    Renderer_FillRect(200, 370, 240, 20, 15);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    (void)notification;
    Renderer_Shutdown();
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
