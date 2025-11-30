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

/**
 * Draw a line (Bresenham's algorithm)
 */
void Renderer_DrawLine(int x1, int y1, int x2, int y2, uint8_t colorIndex);

/**
 * Draw a rectangle outline
 */
void Renderer_DrawRect(int x, int y, int width, int height, uint8_t colorIndex);

/**
 * Draw a horizontal line (optimized)
 */
void Renderer_HLine(int x1, int x2, int y, uint8_t colorIndex);

/**
 * Draw a vertical line (optimized)
 */
void Renderer_VLine(int x, int y1, int y2, uint8_t colorIndex);

/**
 * Draw a circle outline
 */
void Renderer_DrawCircle(int cx, int cy, int radius, uint8_t colorIndex);

/**
 * Draw a filled circle
 */
void Renderer_FillCircle(int cx, int cy, int radius, uint8_t colorIndex);

/**
 * Blit a sprite (with optional transparency)
 * Transparency: palette index 0 is transparent if trans=TRUE
 *
 * @param srcData   Source sprite data (palette indices)
 * @param srcWidth  Source width
 * @param srcHeight Source height
 * @param destX     Destination X
 * @param destY     Destination Y
 * @param trans     If TRUE, color index 0 is transparent
 */
void Renderer_Blit(const uint8_t* srcData, int srcWidth, int srcHeight,
                   int destX, int destY, BOOL trans);

/**
 * Blit a portion of a sprite
 */
void Renderer_BlitRegion(const uint8_t* srcData, int srcWidth, int srcHeight,
                         int srcX, int srcY, int regionWidth, int regionHeight,
                         int destX, int destY, BOOL trans);

/**
 * Scale and blit a sprite
 */
void Renderer_ScaleBlit(const uint8_t* srcData, int srcWidth, int srcHeight,
                        int destX, int destY, int destWidth, int destHeight,
                        BOOL trans);

/**
 * Apply a color remap to a region
 * @param remap  256-byte remap table
 */
void Renderer_Remap(int x, int y, int width, int height, const uint8_t* remap);

/**
 * Draw text (simple bitmap font)
 * Returns width of rendered text in pixels.
 * Note: This is a stub - real font rendering requires font data.
 */
int Renderer_DrawText(const char* text, int x, int y, uint8_t fgColor, uint8_t bgColor);

/**
 * Set clipping rectangle
 */
void Renderer_SetClipRect(int x, int y, int width, int height);

/**
 * Reset clipping to full screen
 */
void Renderer_ResetClip(void);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_METAL_RENDERER_H
