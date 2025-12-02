/**
 * Red Alert macOS Port - Sound Manager Implementation
 */

#include "sounds.h"
#include "map.h"
#include "assets/assetloader.h"
#include "assets/audfile.h"
#include "audio/audio.h"
#include <cstdio>
#include <cstring>

// Sound effect to AUD filename mapping
// Names from SOUNDS.MIX in Red Alert archives
// Try different name variations if primary not found
static const char* g_soundNames[SFX_COUNT] = {
    nullptr,            // SFX_NONE
    // Combat sounds
    "GUN5.AUD",         // SFX_GUN_SHOT - machine gun
    "CANNON1.AUD",      // SFX_CANNON - tank cannon
    "MISSLAU1.AUD",     // SFX_ROCKET - missile launch (alt: ROCKET1)
    "EXPNEW04.AUD",     // SFX_EXPLOSION_SM - small explosion (alt: XPLOS)
    "EXPNEW14.AUD",     // SFX_EXPLOSION_LG - large explosion (alt: XPLOBIG4)
    // Unit sounds
    "AWAIT1.AUD",       // SFX_UNIT_SELECT - try ROGER, YESSIR, AWAIT
    "MOVOUT1.AUD",      // SFX_UNIT_MOVE - moving out
    "ACKNO.AUD",        // SFX_UNIT_ATTACK - acknowledged
    "SCREAM1.AUD",      // SFX_UNIT_DIE - death scream
    // Building sounds
    "BUILD5.AUD",       // SFX_BUILD_COMPLETE - construction complete
    "CASH.AUD",         // SFX_SELL - cash sound (alt: CASHTURN)
    "LOPOWER.AUD",      // SFX_POWER_DOWN - low power (alt: POWRDN1)
    // UI sounds
    "BEEPY2.AUD",       // SFX_CLICK - button beep (alt: BUTTON)
    "RADARON2.AUD",     // SFX_RADAR_ON - radar online (alt: RADAR1)
    "MONEY1.AUD",       // SFX_MONEY - credits (alt: CREDIT1)
};

// Loaded audio samples
static AudioSample* g_sounds[SFX_COUNT] = {nullptr};
static bool g_soundsInitialized = false;
static int g_soundsLoaded = 0;

BOOL Sounds_Init(void) {
    if (g_soundsInitialized) return TRUE;

    fprintf(stderr, "Sounds: Loading sound effects...\n");

    for (int i = 1; i < SFX_COUNT; i++) {
        if (g_soundNames[i]) {
            AudData* aud = Assets_LoadAUD(g_soundNames[i]);
            if (aud && aud->samples && aud->sampleCount > 0) {
                // Create AudioSample from AUD data
                // AudioSample expects 16-bit data as bytes
                g_sounds[i] = (AudioSample*)malloc(sizeof(AudioSample));
                if (g_sounds[i]) {
                    uint32_t dataSize = aud->sampleCount * sizeof(int16_t);
                    g_sounds[i]->data = (uint8_t*)malloc(dataSize);
                    if (g_sounds[i]->data) {
                        memcpy(g_sounds[i]->data, aud->samples, dataSize);
                        g_sounds[i]->dataSize = dataSize;
                        g_sounds[i]->sampleRate = aud->sampleRate;
                        g_sounds[i]->channels = aud->channels;
                        g_sounds[i]->bitsPerSample = 16;
                        g_soundsLoaded++;
                        fprintf(stderr, "  Loaded %s (%u samples)\n",
                               g_soundNames[i], aud->sampleCount);
                    } else {
                        free(g_sounds[i]);
                        g_sounds[i] = nullptr;
                    }
                }
                Aud_Free(aud);
            } else {
                fprintf(stderr, "  MISSING: %s\n", g_soundNames[i]);
                if (aud) Aud_Free(aud);
            }
        }
    }

    g_soundsInitialized = true;
    fprintf(stderr, "Sounds: Loaded %d sound effects\n", g_soundsLoaded);
    return TRUE;
}

void Sounds_Shutdown(void) {
    for (int i = 0; i < SFX_COUNT; i++) {
        if (g_sounds[i]) {
            if (g_sounds[i]->data) {
                free(g_sounds[i]->data);
            }
            free(g_sounds[i]);
            g_sounds[i] = nullptr;
        }
    }
    g_soundsInitialized = false;
    g_soundsLoaded = 0;
}

BOOL Sounds_Available(void) {
    return g_soundsInitialized && g_soundsLoaded > 0;
}

void Sounds_Play(SoundEffect sfx, uint8_t volume, int8_t pan) {
    if (sfx <= SFX_NONE || sfx >= SFX_COUNT) return;
    if (!g_sounds[sfx]) return;

    Audio_Play(g_sounds[sfx], volume, pan, FALSE);
}

void Sounds_PlayAt(SoundEffect sfx, int worldX, int worldY, uint8_t volume) {
    if (sfx <= SFX_NONE || sfx >= SFX_COUNT) return;
    if (!g_sounds[sfx]) return;

    // Calculate pan based on world position relative to viewport center
    Viewport* vp = Map_GetViewport();
    if (!vp) {
        Sounds_Play(sfx, volume, 0);
        return;
    }

    int viewCenterX = vp->x + vp->width / 2;
    int relX = worldX - viewCenterX;

    // Pan range: -128 (left) to +127 (right)
    // Map relative X to pan value, clamped
    int pan = relX / 4; // Scale down
    if (pan < -128) pan = -128;
    if (pan > 127) pan = 127;

    // Attenuate volume based on distance from view center
    int viewCenterY = vp->y + vp->height / 2;
    int dx = worldX - viewCenterX;
    int dy = worldY - viewCenterY;
    int distSq = dx * dx + dy * dy;

    // Max audible distance squared (about 2 screens away)
    int maxDistSq = (vp->width * 2) * (vp->width * 2);

    if (distSq > maxDistSq) {
        return; // Too far away, don't play
    }

    // Attenuate volume by distance
    int attenVolume = volume * (maxDistSq - distSq) / maxDistSq;
    if (attenVolume < 10) attenVolume = 10; // Minimum audible volume
    if (attenVolume > 255) attenVolume = 255;

    Audio_Play(g_sounds[sfx], (uint8_t)attenVolume, (int8_t)pan, FALSE);
}

int Sounds_GetLoadedCount(void) {
    return g_soundsLoaded;
}
