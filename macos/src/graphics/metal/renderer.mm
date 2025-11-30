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
