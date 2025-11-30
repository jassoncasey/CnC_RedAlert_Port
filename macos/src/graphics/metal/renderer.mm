/**
 * Red Alert macOS Port - Metal Renderer Implementation
 *
 * Software framebuffer rendered via Metal.
 */

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

#include "renderer.h"
#include <cstring>
#include <cstdlib>

// Shader source for fullscreen quad
static const char* shaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

// Fullscreen triangle - 3 vertices cover the screen
vertex VertexOut vertexShader(uint vertexID [[vertex_id]]) {
    VertexOut out;

    // Generate fullscreen triangle vertices
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };

    float2 texCoords[3] = {
        float2(0.0, 1.0),
        float2(2.0, 1.0),
        float2(0.0, -1.0)
    };

    out.position = float4(positions[vertexID], 0.0, 1.0);
    out.texCoord = texCoords[vertexID];

    return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]],
                                texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::nearest, min_filter::nearest);
    return colorTexture.sample(textureSampler, in.texCoord);
}
)";

// Renderer state
static struct {
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLRenderPipelineState> pipelineState;
    id<MTLTexture> framebufferTexture;
    MTKView* view;

    uint8_t* framebuffer;       // 8-bit indexed framebuffer
    uint32_t* rgbaBuffer;       // RGBA conversion buffer
    Palette palette;            // Current palette

    bool initialized;
} g_renderer = {};

BOOL Renderer_Init(void* metalView) {
    if (g_renderer.initialized) {
        return TRUE;
    }

    MTKView* view = (__bridge MTKView*)metalView;
    if (!view || !view.device) {
        NSLog(@"Renderer_Init: Invalid Metal view");
        return FALSE;
    }

    g_renderer.view = view;
    g_renderer.device = view.device;

    // Create command queue
    g_renderer.commandQueue = [g_renderer.device newCommandQueue];
    if (!g_renderer.commandQueue) {
        NSLog(@"Renderer_Init: Failed to create command queue");
        return FALSE;
    }

    // Compile shaders
    NSError* error = nil;
    id<MTLLibrary> library = [g_renderer.device newLibraryWithSource:@(shaderSource)
                                                              options:nil
                                                                error:&error];
    if (!library) {
        NSLog(@"Renderer_Init: Failed to compile shaders: %@", error);
        return FALSE;
    }

    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

    // Create pipeline
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    pipelineDesc.colorAttachments[0].pixelFormat = view.colorPixelFormat;

    g_renderer.pipelineState = [g_renderer.device newRenderPipelineStateWithDescriptor:pipelineDesc
                                                                                  error:&error];
    if (!g_renderer.pipelineState) {
        NSLog(@"Renderer_Init: Failed to create pipeline state: %@", error);
        return FALSE;
    }

    // Create framebuffer texture
    MTLTextureDescriptor* texDesc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                     width:FRAMEBUFFER_WIDTH
                                    height:FRAMEBUFFER_HEIGHT
                                 mipmapped:NO];
    texDesc.usage = MTLTextureUsageShaderRead;

    g_renderer.framebufferTexture = [g_renderer.device newTextureWithDescriptor:texDesc];
    if (!g_renderer.framebufferTexture) {
        NSLog(@"Renderer_Init: Failed to create framebuffer texture");
        return FALSE;
    }

    // Allocate CPU-side buffers
    size_t pixelCount = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;
    g_renderer.framebuffer = (uint8_t*)calloc(pixelCount, sizeof(uint8_t));
    g_renderer.rgbaBuffer = (uint32_t*)calloc(pixelCount, sizeof(uint32_t));

    if (!g_renderer.framebuffer || !g_renderer.rgbaBuffer) {
        NSLog(@"Renderer_Init: Failed to allocate framebuffer");
        Renderer_Shutdown();
        return FALSE;
    }

    // Initialize with default palette
    StubAssets_CreatePalette(&g_renderer.palette);

    // Enable continuous rendering
    view.paused = NO;
    view.enableSetNeedsDisplay = NO;

    g_renderer.initialized = true;
    NSLog(@"Renderer initialized: %dx%d framebuffer", FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

    return TRUE;
}

void Renderer_Shutdown(void) {
    if (g_renderer.framebuffer) {
        free(g_renderer.framebuffer);
        g_renderer.framebuffer = nullptr;
    }
    if (g_renderer.rgbaBuffer) {
        free(g_renderer.rgbaBuffer);
        g_renderer.rgbaBuffer = nullptr;
    }

    g_renderer.pipelineState = nil;
    g_renderer.framebufferTexture = nil;
    g_renderer.commandQueue = nil;
    g_renderer.device = nil;
    g_renderer.view = nil;

    g_renderer.initialized = false;
}

uint8_t* Renderer_GetFramebuffer(void) {
    return g_renderer.framebuffer;
}

int Renderer_GetWidth(void) {
    return FRAMEBUFFER_WIDTH;
}

int Renderer_GetHeight(void) {
    return FRAMEBUFFER_HEIGHT;
}

void Renderer_SetPalette(const Palette* palette) {
    if (palette) {
        memcpy(&g_renderer.palette, palette, sizeof(Palette));
    }
}

void Renderer_Present(void) {
    if (!g_renderer.initialized || !g_renderer.view) {
        return;
    }

    // Convert indexed framebuffer to RGBA
    size_t pixelCount = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t idx = g_renderer.framebuffer[i];
        uint8_t r = g_renderer.palette.colors[idx][0];
        uint8_t g = g_renderer.palette.colors[idx][1];
        uint8_t b = g_renderer.palette.colors[idx][2];
        // RGBA8 format: 0xAABBGGRR (little-endian)
        g_renderer.rgbaBuffer[i] = 0xFF000000 | (b << 16) | (g << 8) | r;
    }

    // Upload to texture
    MTLRegion region = MTLRegionMake2D(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    [g_renderer.framebufferTexture replaceRegion:region
                                     mipmapLevel:0
                                       withBytes:g_renderer.rgbaBuffer
                                     bytesPerRow:FRAMEBUFFER_WIDTH * sizeof(uint32_t)];

    // Get drawable
    id<CAMetalDrawable> drawable = g_renderer.view.currentDrawable;
    MTLRenderPassDescriptor* passDesc = g_renderer.view.currentRenderPassDescriptor;

    if (!drawable || !passDesc) {
        return;
    }

    // Create command buffer and encoder
    id<MTLCommandBuffer> commandBuffer = [g_renderer.commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];

    [encoder setRenderPipelineState:g_renderer.pipelineState];
    [encoder setFragmentTexture:g_renderer.framebufferTexture atIndex:0];

    // Draw fullscreen triangle (3 vertices)
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

    [encoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

void Renderer_Clear(uint8_t colorIndex) {
    if (g_renderer.framebuffer) {
        memset(g_renderer.framebuffer, colorIndex, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
    }
}

void Renderer_FillRect(int x, int y, int width, int height, uint8_t colorIndex) {
    if (!g_renderer.framebuffer) return;

    // Clip to framebuffer bounds
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > FRAMEBUFFER_WIDTH) { width = FRAMEBUFFER_WIDTH - x; }
    if (y + height > FRAMEBUFFER_HEIGHT) { height = FRAMEBUFFER_HEIGHT - y; }

    if (width <= 0 || height <= 0) return;

    for (int row = 0; row < height; row++) {
        uint8_t* dest = g_renderer.framebuffer + (y + row) * FRAMEBUFFER_WIDTH + x;
        memset(dest, colorIndex, width);
    }
}

void Renderer_PutPixel(int x, int y, uint8_t colorIndex) {
    if (!g_renderer.framebuffer) return;
    if (x < 0 || x >= FRAMEBUFFER_WIDTH || y < 0 || y >= FRAMEBUFFER_HEIGHT) return;

    g_renderer.framebuffer[y * FRAMEBUFFER_WIDTH + x] = colorIndex;
}

uint8_t Renderer_GetPixel(int x, int y) {
    if (!g_renderer.framebuffer) return 0;
    if (x < 0 || x >= FRAMEBUFFER_WIDTH || y < 0 || y >= FRAMEBUFFER_HEIGHT) return 0;

    return g_renderer.framebuffer[y * FRAMEBUFFER_WIDTH + x];
}

// Clipping rectangle (default: full screen)
static int g_clipX = 0;
static int g_clipY = 0;
static int g_clipWidth = FRAMEBUFFER_WIDTH;
static int g_clipHeight = FRAMEBUFFER_HEIGHT;

void Renderer_SetClipRect(int x, int y, int width, int height) {
    g_clipX = (x < 0) ? 0 : x;
    g_clipY = (y < 0) ? 0 : y;
    g_clipWidth = (x + width > FRAMEBUFFER_WIDTH) ? FRAMEBUFFER_WIDTH - g_clipX : width;
    g_clipHeight = (y + height > FRAMEBUFFER_HEIGHT) ? FRAMEBUFFER_HEIGHT - g_clipY : height;
}

void Renderer_ResetClip(void) {
    g_clipX = 0;
    g_clipY = 0;
    g_clipWidth = FRAMEBUFFER_WIDTH;
    g_clipHeight = FRAMEBUFFER_HEIGHT;
}

// Helper: clip-aware pixel plot
static inline void ClippedPixel(int x, int y, uint8_t color) {
    if (x >= g_clipX && x < g_clipX + g_clipWidth &&
        y >= g_clipY && y < g_clipY + g_clipHeight) {
        g_renderer.framebuffer[y * FRAMEBUFFER_WIDTH + x] = color;
    }
}

void Renderer_HLine(int x1, int x2, int y, uint8_t colorIndex) {
    if (!g_renderer.framebuffer) return;
    if (y < g_clipY || y >= g_clipY + g_clipHeight) return;

    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }

    if (x1 < g_clipX) x1 = g_clipX;
    if (x2 >= g_clipX + g_clipWidth) x2 = g_clipX + g_clipWidth - 1;

    if (x1 > x2) return;

    uint8_t* dest = g_renderer.framebuffer + y * FRAMEBUFFER_WIDTH + x1;
    memset(dest, colorIndex, x2 - x1 + 1);
}

void Renderer_VLine(int x, int y1, int y2, uint8_t colorIndex) {
    if (!g_renderer.framebuffer) return;
    if (x < g_clipX || x >= g_clipX + g_clipWidth) return;

    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

    if (y1 < g_clipY) y1 = g_clipY;
    if (y2 >= g_clipY + g_clipHeight) y2 = g_clipY + g_clipHeight - 1;

    if (y1 > y2) return;

    uint8_t* dest = g_renderer.framebuffer + y1 * FRAMEBUFFER_WIDTH + x;
    for (int y = y1; y <= y2; y++) {
        *dest = colorIndex;
        dest += FRAMEBUFFER_WIDTH;
    }
}

void Renderer_DrawLine(int x1, int y1, int x2, int y2, uint8_t colorIndex) {
    if (!g_renderer.framebuffer) return;

    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        ClippedPixel(x1, y1, colorIndex);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void Renderer_DrawRect(int x, int y, int width, int height, uint8_t colorIndex) {
    if (width <= 0 || height <= 0) return;

    Renderer_HLine(x, x + width - 1, y, colorIndex);                 // Top
    Renderer_HLine(x, x + width - 1, y + height - 1, colorIndex);    // Bottom
    Renderer_VLine(x, y, y + height - 1, colorIndex);                // Left
    Renderer_VLine(x + width - 1, y, y + height - 1, colorIndex);    // Right
}

void Renderer_DrawCircle(int cx, int cy, int radius, uint8_t colorIndex) {
    if (!g_renderer.framebuffer || radius <= 0) return;

    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        ClippedPixel(cx + x, cy + y, colorIndex);
        ClippedPixel(cx + y, cy + x, colorIndex);
        ClippedPixel(cx - y, cy + x, colorIndex);
        ClippedPixel(cx - x, cy + y, colorIndex);
        ClippedPixel(cx - x, cy - y, colorIndex);
        ClippedPixel(cx - y, cy - x, colorIndex);
        ClippedPixel(cx + y, cy - x, colorIndex);
        ClippedPixel(cx + x, cy - y, colorIndex);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void Renderer_FillCircle(int cx, int cy, int radius, uint8_t colorIndex) {
    if (!g_renderer.framebuffer || radius <= 0) return;

    // Filled circle using horizontal lines
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        Renderer_HLine(cx - x, cx + x, cy + y, colorIndex);
        Renderer_HLine(cx - x, cx + x, cy - y, colorIndex);
        Renderer_HLine(cx - y, cx + y, cy + x, colorIndex);
        Renderer_HLine(cx - y, cx + y, cy - x, colorIndex);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void Renderer_Blit(const uint8_t* srcData, int srcWidth, int srcHeight,
                   int destX, int destY, BOOL trans) {
    if (!g_renderer.framebuffer || !srcData) return;

    for (int sy = 0; sy < srcHeight; sy++) {
        int dy = destY + sy;
        if (dy < g_clipY || dy >= g_clipY + g_clipHeight) continue;

        for (int sx = 0; sx < srcWidth; sx++) {
            int dx = destX + sx;
            if (dx < g_clipX || dx >= g_clipX + g_clipWidth) continue;

            uint8_t pixel = srcData[sy * srcWidth + sx];
            if (trans && pixel == 0) continue;  // Transparent

            g_renderer.framebuffer[dy * FRAMEBUFFER_WIDTH + dx] = pixel;
        }
    }
}

void Renderer_BlitRegion(const uint8_t* srcData, int srcWidth, int srcHeight,
                         int srcX, int srcY, int regionWidth, int regionHeight,
                         int destX, int destY, BOOL trans) {
    if (!g_renderer.framebuffer || !srcData) return;

    for (int ry = 0; ry < regionHeight; ry++) {
        int sy = srcY + ry;
        int dy = destY + ry;

        if (sy < 0 || sy >= srcHeight) continue;
        if (dy < g_clipY || dy >= g_clipY + g_clipHeight) continue;

        for (int rx = 0; rx < regionWidth; rx++) {
            int sx = srcX + rx;
            int dx = destX + rx;

            if (sx < 0 || sx >= srcWidth) continue;
            if (dx < g_clipX || dx >= g_clipX + g_clipWidth) continue;

            uint8_t pixel = srcData[sy * srcWidth + sx];
            if (trans && pixel == 0) continue;

            g_renderer.framebuffer[dy * FRAMEBUFFER_WIDTH + dx] = pixel;
        }
    }
}

void Renderer_ScaleBlit(const uint8_t* srcData, int srcWidth, int srcHeight,
                        int destX, int destY, int destWidth, int destHeight,
                        BOOL trans) {
    if (!g_renderer.framebuffer || !srcData) return;
    if (destWidth <= 0 || destHeight <= 0) return;

    // Fixed-point scaling
    int xRatio = (srcWidth << 16) / destWidth;
    int yRatio = (srcHeight << 16) / destHeight;

    for (int dy = 0; dy < destHeight; dy++) {
        int screenY = destY + dy;
        if (screenY < g_clipY || screenY >= g_clipY + g_clipHeight) continue;

        int srcY = (dy * yRatio) >> 16;
        if (srcY >= srcHeight) srcY = srcHeight - 1;

        for (int dx = 0; dx < destWidth; dx++) {
            int screenX = destX + dx;
            if (screenX < g_clipX || screenX >= g_clipX + g_clipWidth) continue;

            int srcX = (dx * xRatio) >> 16;
            if (srcX >= srcWidth) srcX = srcWidth - 1;

            uint8_t pixel = srcData[srcY * srcWidth + srcX];
            if (trans && pixel == 0) continue;

            g_renderer.framebuffer[screenY * FRAMEBUFFER_WIDTH + screenX] = pixel;
        }
    }
}

void Renderer_Remap(int x, int y, int width, int height, const uint8_t* remap) {
    if (!g_renderer.framebuffer || !remap) return;

    // Clip
    int x1 = (x < g_clipX) ? g_clipX : x;
    int y1 = (y < g_clipY) ? g_clipY : y;
    int x2 = x + width;
    int y2 = y + height;
    if (x2 > g_clipX + g_clipWidth) x2 = g_clipX + g_clipWidth;
    if (y2 > g_clipY + g_clipHeight) y2 = g_clipY + g_clipHeight;

    for (int py = y1; py < y2; py++) {
        uint8_t* row = g_renderer.framebuffer + py * FRAMEBUFFER_WIDTH + x1;
        for (int px = x1; px < x2; px++) {
            *row = remap[*row];
            row++;
        }
    }
}

// Simple 8x8 bitmap font for basic text rendering
// Each character is 8 bytes, one byte per row
static const uint8_t g_font8x8[128][8] = {
    // Only define printable ASCII (32-126)
    // Space (32)
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['!'] = {0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00},
    ['"'] = {0x6C,0x6C,0x24,0x00,0x00,0x00,0x00,0x00},
    ['#'] = {0x6C,0xFE,0x6C,0x6C,0xFE,0x6C,0x00,0x00},
    ['$'] = {0x10,0x7C,0xD0,0x7C,0x16,0x7C,0x10,0x00},
    ['%'] = {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00},
    ['0'] = {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0x00},
    ['1'] = {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['2'] = {0x7C,0xC6,0x06,0x1C,0x30,0x60,0xFE,0x00},
    ['3'] = {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00},
    ['4'] = {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00},
    ['5'] = {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00},
    ['6'] = {0x7C,0xC6,0xC0,0xFC,0xC6,0xC6,0x7C,0x00},
    ['7'] = {0xFE,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
    ['8'] = {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00},
    ['9'] = {0x7C,0xC6,0xC6,0x7E,0x06,0xC6,0x7C,0x00},
    [':'] = {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    ['A'] = {0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00},
    ['B'] = {0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00},
    ['C'] = {0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00},
    ['D'] = {0xF8,0xCC,0xC6,0xC6,0xC6,0xCC,0xF8,0x00},
    ['E'] = {0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00},
    ['F'] = {0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00},
    ['G'] = {0x7C,0xC6,0xC0,0xCE,0xC6,0xC6,0x7E,0x00},
    ['H'] = {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00},
    ['I'] = {0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['J'] = {0x06,0x06,0x06,0x06,0xC6,0xC6,0x7C,0x00},
    ['K'] = {0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00},
    ['L'] = {0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00},
    ['M'] = {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00},
    ['N'] = {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00},
    ['O'] = {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    ['P'] = {0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00},
    ['Q'] = {0x7C,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x06},
    ['R'] = {0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00},
    ['S'] = {0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00},
    ['T'] = {0xFE,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    ['U'] = {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00},
    ['V'] = {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00},
    ['W'] = {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
    ['X'] = {0xC6,0x6C,0x38,0x38,0x6C,0xC6,0xC6,0x00},
    ['Y'] = {0xC6,0xC6,0x6C,0x38,0x18,0x18,0x18,0x00},
    ['Z'] = {0xFE,0x0C,0x18,0x30,0x60,0xC0,0xFE,0x00},
    ['-'] = {0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00},
    ['+'] = {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    ['='] = {0x00,0x00,0xFE,0x00,0xFE,0x00,0x00,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    [','] = {0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00},
    ['('] = {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    [')'] = {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    ['/'] = {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},
};

int Renderer_DrawText(const char* text, int x, int y, uint8_t fgColor, uint8_t bgColor) {
    if (!g_renderer.framebuffer || !text) return 0;

    int startX = x;

    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c >= 128) c = '?';  // Replace non-ASCII

        const uint8_t* glyph = g_font8x8[c];

        for (int row = 0; row < 8; row++) {
            int py = y + row;
            if (py < g_clipY || py >= g_clipY + g_clipHeight) continue;

            uint8_t bits = glyph[row];
            for (int col = 0; col < 8; col++) {
                int px = x + col;
                if (px < g_clipX || px >= g_clipX + g_clipWidth) continue;

                if (bits & (0x80 >> col)) {
                    g_renderer.framebuffer[py * FRAMEBUFFER_WIDTH + px] = fgColor;
                } else if (bgColor != 0) {
                    g_renderer.framebuffer[py * FRAMEBUFFER_WIDTH + px] = bgColor;
                }
            }
        }

        x += 8;
        text++;
    }

    return x - startX;
}
