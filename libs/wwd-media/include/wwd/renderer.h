/**
 * wwd-media - Metal Renderer
 *
 * Renders an 8-bit paletted framebuffer to screen via Metal.
 * Designed for classic game graphics with palette-based rendering.
 */

#ifndef WWD_RENDERER_H
#define WWD_RENDERER_H

#include "wwd/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Metal renderer
 * Must be called after the window/view is created.
 *
 * @param metalView  Pointer to MTKView (as void* for C compatibility)
 * @return WWD_TRUE on success
 */
WwdBool Wwd_Renderer_Init(void* metalView);

/**
 * Shutdown the renderer
 */
void Wwd_Renderer_Shutdown(void);

/**
 * Get pointer to the 8-bit framebuffer
 * Pixels are palette indices (0-255).
 */
uint8_t* Wwd_Renderer_GetFramebuffer(void);

/**
 * Get framebuffer width
 */
int Wwd_Renderer_GetWidth(void);

/**
 * Get framebuffer height
 */
int Wwd_Renderer_GetHeight(void);

/**
 * Set the current palette for rendering
 */
void Wwd_Renderer_SetPalette(const WwdPalette* palette);

/**
 * Present the framebuffer to screen
 * Converts 8-bit indexed to RGBA using current palette,
 * uploads to GPU, and renders.
 */
void Wwd_Renderer_Present(void);

/**
 * Clear the framebuffer to a specific palette index
 */
void Wwd_Renderer_Clear(uint8_t colorIndex);

/**
 * Draw a filled rectangle (in palette indices)
 */
void Wwd_Renderer_FillRect(int x, int y, int width, int height,
                          uint8_t colorIndex);

/**
 * Put a single pixel
 */
void Wwd_Renderer_PutPixel(int x, int y, uint8_t colorIndex);

/**
 * Get a single pixel
 */
uint8_t Wwd_Renderer_GetPixel(int x, int y);

/**
 * Draw a line (Bresenham's algorithm)
 */
void Wwd_Renderer_DrawLine(int x1, int y1, int x2, int y2, uint8_t colorIndex);

/**
 * Draw a rectangle outline
 */
void Wwd_Renderer_DrawRect(int x, int y, int width, int height,
                          uint8_t colorIndex);

/**
 * Draw a horizontal line (optimized)
 */
void Wwd_Renderer_HLine(int x1, int x2, int y, uint8_t colorIndex);

/**
 * Draw a vertical line (optimized)
 */
void Wwd_Renderer_VLine(int x, int y1, int y2, uint8_t colorIndex);

/**
 * Draw a circle outline
 */
void Wwd_Renderer_DrawCircle(int cx, int cy, int radius, uint8_t colorIndex);

/**
 * Draw a filled circle
 */
void Wwd_Renderer_FillCircle(int cx, int cy, int radius, uint8_t colorIndex);

/**
 * Blit a sprite (with optional transparency)
 * Transparency: palette index 0 is transparent if trans=WWD_TRUE
 *
 * @param srcData   Source sprite data (palette indices)
 * @param srcWidth  Source width
 * @param srcHeight Source height
 * @param destX     Destination X
 * @param destY     Destination Y
 * @param trans     If WWD_TRUE, color index 0 is transparent
 */
void Wwd_Renderer_Blit(const uint8_t* srcData, int srcWidth, int srcHeight,
                      int destX, int destY, WwdBool trans);

/**
 * Blit a portion of a sprite
 */
void Wwd_Renderer_BlitRegion(const uint8_t* srcData, int srcWidth, int srcHeight,
                            int srcX, int srcY, int regionWidth, int regionHeight,
                            int destX, int destY, WwdBool trans);

/**
 * Scale and blit a sprite
 */
void Wwd_Renderer_ScaleBlit(const uint8_t* srcData, int srcWidth, int srcHeight,
                           int destX, int destY, int destWidth, int destHeight,
                           WwdBool trans);

/**
 * Apply a color remap to a region
 * @param remap  256-byte remap table
 */
void Wwd_Renderer_Remap(int x, int y, int width, int height,
                       const uint8_t* remap);

/**
 * Dim a rectangular region (fog of war effect)
 * Darkens existing pixels by shifting to darker palette entries.
 * @param amount  Dimming amount (0=none, 1=slight, 2=heavy)
 */
void Wwd_Renderer_DimRect(int x, int y, int width, int height, int amount);

/**
 * Set alpha (transparency) for a rectangular region.
 * Used for fog of war: 255=fully visible, 0=fully dark/hidden.
 * The alpha blends the rendered color toward black.
 * @param alpha  Alpha value (0=black, 128=50% dim, 255=fully visible)
 */
void Wwd_Renderer_SetAlpha(int x, int y, int width, int height, uint8_t alpha);

/**
 * Clear alpha buffer to fully opaque (255).
 */
void Wwd_Renderer_ClearAlpha(void);

/**
 * Get pointer to the alpha buffer.
 * Same dimensions as framebuffer.
 */
uint8_t* Wwd_Renderer_GetAlphaBuffer(void);

/**
 * Draw text (simple bitmap font)
 * Returns width of rendered text in pixels.
 * Note: This is a stub - real font rendering requires font data.
 */
int Wwd_Renderer_DrawText(const char* text, int x, int y,
                         uint8_t fgColor, uint8_t bgColor);

/**
 * Set clipping rectangle
 */
void Wwd_Renderer_SetClipRect(int x, int y, int width, int height);

/**
 * Reset clipping to full screen
 */
void Wwd_Renderer_ResetClip(void);

/**
 * Blit an SHP frame directly (convenience wrapper)
 * @param pixels     8-bit indexed pixel data
 * @param width      Frame width
 * @param height     Frame height
 * @param destX      Destination X
 * @param destY      Destination Y
 * @param offsetX    Sprite hotspot offset X (subtracted from destX)
 * @param offsetY    Sprite hotspot offset Y (subtracted from destY)
 * @param trans      If WWD_TRUE, color index 0 is transparent
 */
void Wwd_Renderer_BlitSprite(const uint8_t* pixels, int width, int height,
                            int destX, int destY, int offsetX, int offsetY,
                            WwdBool trans);

/**
 * Blit with color remapping (for team colors)
 * @param srcData    8-bit indexed pixel data
 * @param srcWidth   Source width
 * @param srcHeight  Source height
 * @param destX      Destination X
 * @param destY      Destination Y
 * @param trans      If WWD_TRUE, color index 0 is transparent
 * @param remap      256-byte remap table (NULL = no remapping)
 */
void Wwd_Renderer_BlitRemapped(const uint8_t* srcData, int srcWidth,
                              int srcHeight, int destX, int destY,
                              WwdBool trans, const uint8_t* remap);

/**
 * Blit sprite with remapping (convenience wrapper with hotspot)
 */
void Wwd_Renderer_BlitSpriteRemapped(const uint8_t* pixels, int width,
                                    int height, int destX, int destY,
                                    int offsetX, int offsetY,
                                    WwdBool trans, const uint8_t* remap);

#ifdef __cplusplus
}
#endif

#endif // WWD_RENDERER_H
