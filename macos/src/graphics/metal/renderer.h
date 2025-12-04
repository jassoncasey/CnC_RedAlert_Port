/**
 * Red Alert macOS Port - Metal Renderer
 *
 * Compatibility header - forwards to ra-media library.
 */

#ifndef GRAPHICS_METAL_RENDERER_H
#define GRAPHICS_METAL_RENDERER_H

#include "compat/windows.h"
#include "compat/assets.h"
#include <wwd/renderer.h>

// Framebuffer dimensions aliases
#define FRAMEBUFFER_WIDTH  WWD_FRAMEBUFFER_WIDTH
#define FRAMEBUFFER_HEIGHT WWD_FRAMEBUFFER_HEIGHT

#ifdef __cplusplus
extern "C" {
#endif

// Function aliases - map game API to ra-media API
static inline BOOL Renderer_Init(void* metalView) {
    return Wwd_Renderer_Init(metalView);
}

static inline void Renderer_Shutdown(void) {
    Wwd_Renderer_Shutdown();
}

static inline uint8_t* Renderer_GetFramebuffer(void) {
    return Wwd_Renderer_GetFramebuffer();
}

static inline int Renderer_GetWidth(void) {
    return Wwd_Renderer_GetWidth();
}

static inline int Renderer_GetHeight(void) {
    return Wwd_Renderer_GetHeight();
}

static inline void Renderer_SetPalette(const Palette* palette) {
    Wwd_Renderer_SetPalette((const WwdPalette*)palette);
}

static inline void Renderer_Present(void) {
    Wwd_Renderer_Present();
}

static inline void Renderer_Clear(uint8_t colorIndex) {
    Wwd_Renderer_Clear(colorIndex);
}

static inline void Renderer_FillRect(int x, int y, int width, int height,
                                     uint8_t colorIndex) {
    Wwd_Renderer_FillRect(x, y, width, height, colorIndex);
}

static inline void Renderer_PutPixel(int x, int y, uint8_t colorIndex) {
    Wwd_Renderer_PutPixel(x, y, colorIndex);
}

static inline uint8_t Renderer_GetPixel(int x, int y) {
    return Wwd_Renderer_GetPixel(x, y);
}

static inline void Renderer_DrawLine(int x1, int y1, int x2, int y2,
                                     uint8_t colorIndex) {
    Wwd_Renderer_DrawLine(x1, y1, x2, y2, colorIndex);
}

static inline void Renderer_DrawRect(int x, int y, int width, int height,
                                     uint8_t colorIndex) {
    Wwd_Renderer_DrawRect(x, y, width, height, colorIndex);
}

static inline void Renderer_HLine(int x1, int x2, int y, uint8_t colorIndex) {
    Wwd_Renderer_HLine(x1, x2, y, colorIndex);
}

static inline void Renderer_VLine(int x, int y1, int y2, uint8_t colorIndex) {
    Wwd_Renderer_VLine(x, y1, y2, colorIndex);
}

static inline void Renderer_DrawCircle(int cx, int cy, int radius,
                                       uint8_t colorIndex) {
    Wwd_Renderer_DrawCircle(cx, cy, radius, colorIndex);
}

static inline void Renderer_FillCircle(int cx, int cy, int radius,
                                       uint8_t colorIndex) {
    Wwd_Renderer_FillCircle(cx, cy, radius, colorIndex);
}

static inline void Renderer_Blit(const uint8_t* srcData, int srcWidth,
                                 int srcHeight, int destX, int destY,
                                 BOOL trans) {
    Wwd_Renderer_Blit(srcData, srcWidth, srcHeight, destX, destY, trans);
}

static inline void Renderer_BlitRegion(const uint8_t* srcData, int srcWidth,
                                       int srcHeight, int srcX, int srcY,
                                       int regionWidth, int regionHeight,
                                       int destX, int destY, BOOL trans) {
    Wwd_Renderer_BlitRegion(srcData, srcWidth, srcHeight, srcX, srcY,
                           regionWidth, regionHeight, destX, destY, trans);
}

static inline void Renderer_ScaleBlit(const uint8_t* srcData, int srcWidth,
                                      int srcHeight, int destX, int destY,
                                      int destWidth, int destHeight,
                                      BOOL trans) {
    Wwd_Renderer_ScaleBlit(srcData, srcWidth, srcHeight, destX, destY,
                          destWidth, destHeight, trans);
}

static inline void Renderer_Remap(int x, int y, int width, int height,
                                  const uint8_t* remap) {
    Wwd_Renderer_Remap(x, y, width, height, remap);
}

static inline void Renderer_DimRect(int x, int y, int width, int height,
                                    int amount) {
    Wwd_Renderer_DimRect(x, y, width, height, amount);
}

static inline void Renderer_SetAlpha(int x, int y, int width, int height,
                                     uint8_t alpha) {
    Wwd_Renderer_SetAlpha(x, y, width, height, alpha);
}

static inline void Renderer_ClearAlpha(void) {
    Wwd_Renderer_ClearAlpha();
}

static inline uint8_t* Renderer_GetAlphaBuffer(void) {
    return Wwd_Renderer_GetAlphaBuffer();
}

static inline int Renderer_DrawText(const char* text, int x, int y,
                                    uint8_t fgColor, uint8_t bgColor) {
    return Wwd_Renderer_DrawText(text, x, y, fgColor, bgColor);
}

static inline void Renderer_SetClipRect(int x, int y, int width, int height) {
    Wwd_Renderer_SetClipRect(x, y, width, height);
}

static inline void Renderer_ResetClip(void) {
    Wwd_Renderer_ResetClip();
}

static inline void Renderer_BlitSprite(const uint8_t* pixels, int width,
                                       int height, int destX, int destY,
                                       int offsetX, int offsetY, BOOL trans) {
    Wwd_Renderer_BlitSprite(pixels, width, height, destX, destY,
                           offsetX, offsetY, trans);
}

static inline void Renderer_BlitRemapped(const uint8_t* srcData, int srcWidth,
                                         int srcHeight, int destX, int destY,
                                         BOOL trans, const uint8_t* remap) {
    Wwd_Renderer_BlitRemapped(srcData, srcWidth, srcHeight, destX, destY,
                             trans, remap);
}

static inline void Renderer_BlitSpriteRemapped(const uint8_t* pixels,
                                               int width, int height,
                                               int destX, int destY,
                                               int offsetX, int offsetY,
                                               BOOL trans,
                                               const uint8_t* remap) {
    Wwd_Renderer_BlitSpriteRemapped(pixels, width, height, destX, destY,
                                   offsetX, offsetY, trans, remap);
}

/**
 * Load a game palette from AssetLoader and set it as current
 * Note: This is game-specific and remains in the game layer.
 * @param name  Palette filename (e.g., "SNOW.PAL", "TEMPERAT.PAL")
 * @return TRUE on success
 */
BOOL Renderer_LoadPalette(const char* name);

#ifdef __cplusplus
}
#endif

#endif // GRAPHICS_METAL_RENDERER_H
