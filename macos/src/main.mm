/**
 * Red Alert macOS Port - Entry Point
 *
 * Minimal AppKit application shell.
 * Creates window, runs event loop, handles Cmd+Q.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

// Game window dimensions (original Red Alert resolution)
static constexpr int WINDOW_WIDTH = 640;
static constexpr int WINDOW_HEIGHT = 400;

#pragma mark - Application Delegate

@interface RAAppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
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

    // Set up Metal view (placeholder - black screen for now)
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        NSLog(@"Metal is not supported on this device");
        [NSApp terminate:nil];
        return;
    }

    MTKView *metalView = [[MTKView alloc] initWithFrame:frame device:device];
    metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);  // Black
    metalView.enableSetNeedsDisplay = NO;
    metalView.paused = YES;  // We'll drive rendering manually later

    [self.window setContentView:metalView];
    [self.window makeKeyAndOrderFront:nil];

    NSLog(@"Red Alert initialized - Metal device: %@", device.name);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    (void)notification;
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
