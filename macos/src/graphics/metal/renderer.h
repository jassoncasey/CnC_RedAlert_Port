/**
 * Red Alert macOS Port - Metal Renderer
 *
 * Renders an 8-bit paletted framebuffer to screen via Metal.
 */

#ifndef GRAPHICS_METAL_RENDERER_H
#define GRAPHICS_METAL_RENDERER_H

#include "compat/windows.h"
#include "compat/assets.h"
#include <cstdint>

// Framebuffer dimensions (original Red Alert was 640x400 in high-res mode)
#define FRAMEBUFFER_WIDTH  640
#define FRAMEBUFFER_HEIGHT 400

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Metal renderer
 * Must be called after the window/view is created.
 *
 * @param metalView  Pointer to MTKView (as void* for C compatibility)
 * @return TRUE on success
 */
BOOL Renderer_Init(void* metalView);

/**
 * Shutdown the renderer
 */
void Renderer_Shutdown(void);

/**
 * Get pointer to the 8-bit framebuffer
 * Pixels are palette indices (0-255).
 */
uint8_t* Renderer_GetFramebuffer(void);

/**
 * Get framebuffer width
 */
int Renderer_GetWidth(void);

/**
 * Get framebuffer height
 */
int Renderer_GetHeight(void);

/**
 * Set the current palette for rendering
 */
void Renderer_SetPalette(const Palette* palette);

/**
 * Present the framebuffer to screen
 * Converts 8-bit indexed to RGBA using current palette,
 * uploads to GPU, and renders.
 */
void Renderer_Present(void);

/**
 * Clear the framebuffer to a specific palette index
 */
void Renderer_Clear(uint8_t colorIndex);

/**
 * Draw a filled rectangle (in palette indices)
 */
void Renderer_FillRect(int x, int y, int width, int height, uint8_t colorIndex);

/**
 * Put a single pixel
 */
void Renderer_PutPixel(int x, int y, uint8_t colorIndex);

/**
 * Get a single pixel
 */
uint8_t Renderer_GetPixel(int x, int y);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_METAL_RENDERER_H
