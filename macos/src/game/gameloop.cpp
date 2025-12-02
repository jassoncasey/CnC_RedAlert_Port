/**
 * Red Alert macOS Port - Game Loop Implementation
 *
 * Fixed timestep game loop with variable render rate.
 */

#include "gameloop.h"
#include "compat/windows.h"
#include <cstdio>

// Global frame counter (used by save/load system)
uint32_t Frame = 0;

// Game loop state
static struct {
    GameState state;
    FrameStats stats;

    // Timing
    int gameSpeed;              // 0-7 (0 = fastest)
    DWORD lastUpdateTime;       // Last game update time
    DWORD accumulator;          // Accumulated time for fixed timestep
    DWORD tickInterval;         // Milliseconds per game tick

    // Callbacks
    GameUpdateCallback updateCallback;
    GameRenderCallback renderCallback;

    // Flags
    bool paused;
    bool quitRequested;
    bool initialized;
} g_loop = {};

// Calculate tick interval based on game speed
static DWORD CalculateTickInterval(int speed) {
    // Original game: speed 0 = ~15 FPS, speed 4 = ~10 FPS, speed 7 = ~7 FPS
    // Base interval is 1000ms / 15 = 66ms for speed 0
    // Each speed level adds ~10ms
    int baseFPS = DEFAULT_GAME_FPS;
    int adjustedFPS = baseFPS - speed;
    if (adjustedFPS < 5) adjustedFPS = 5;  // Minimum 5 FPS
    return 1000 / adjustedFPS;
}

void GameLoop_Init(void) {
    if (g_loop.initialized) return;

    g_loop.state = GAME_STATE_INIT;
    g_loop.gameSpeed = 4;  // Default speed (middle)
    g_loop.tickInterval = CalculateTickInterval(g_loop.gameSpeed);
    g_loop.lastUpdateTime = GetTickCount();
    g_loop.accumulator = 0;

    // Initialize stats
    g_loop.stats.frameCount = 0;
    g_loop.stats.gameFrame = 0;
    g_loop.stats.currentFPS = 0.0f;
    g_loop.stats.avgFrameTime = 0.0f;
    g_loop.stats.lastSecondFrames = 0;
    g_loop.stats.lastSecondTime = GetTickCount();

    g_loop.paused = false;
    g_loop.quitRequested = false;
    g_loop.updateCallback = nullptr;
    g_loop.renderCallback = nullptr;

    g_loop.initialized = true;
}

void GameLoop_Shutdown(void) {
    g_loop.initialized = false;
    g_loop.state = GAME_STATE_QUIT;
}

BOOL GameLoop_RunFrame(void) {
    if (!g_loop.initialized || g_loop.quitRequested) {
        return FALSE;
    }

    DWORD currentTime = GetTickCount();
    DWORD deltaTime = currentTime - g_loop.lastUpdateTime;
    g_loop.lastUpdateTime = currentTime;

    // Cap delta time to prevent spiral of death
    if (deltaTime > 250) {
        deltaTime = 250;
    }

    // Update FPS counter
    g_loop.stats.frameCount++;
    g_loop.stats.lastSecondFrames++;

    if (currentTime - g_loop.stats.lastSecondTime >= 1000) {
        g_loop.stats.currentFPS = (float)g_loop.stats.lastSecondFrames;
        g_loop.stats.avgFrameTime = 1000.0f / g_loop.stats.currentFPS;
        g_loop.stats.lastSecondFrames = 0;
        g_loop.stats.lastSecondTime = currentTime;
    }

    // Fixed timestep update (game logic)
    // Always call update for input handling, even when paused
    if (g_loop.state == GAME_STATE_PLAYING) {
        g_loop.accumulator += deltaTime;

        // Process game updates at fixed rate
        while (g_loop.accumulator >= g_loop.tickInterval) {
            g_loop.accumulator -= g_loop.tickInterval;
            if (!g_loop.paused) {
                g_loop.stats.gameFrame++;
            }

            // Call update callback (handles input even when paused)
            if (g_loop.updateCallback) {
                float dt = (float)g_loop.tickInterval / 1000.0f;
                g_loop.updateCallback(g_loop.stats.gameFrame, dt);
            }
        }
    }

    // Render (always, regardless of pause state)
    if (g_loop.renderCallback) {
        g_loop.renderCallback();
    }

    return TRUE;
}

GameState GameLoop_GetState(void) {
    return g_loop.state;
}

void GameLoop_SetState(GameState state) {
    GameState oldState = g_loop.state;
    g_loop.state = state;

    // Reset timing when entering playing state
    if (state == GAME_STATE_PLAYING && oldState != GAME_STATE_PLAYING) {
        g_loop.lastUpdateTime = GetTickCount();
        g_loop.accumulator = 0;
    }
}

uint32_t GameLoop_GetFrame(void) {
    return g_loop.stats.gameFrame;
}

const FrameStats* GameLoop_GetStats(void) {
    return &g_loop.stats;
}

void GameLoop_SetSpeed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 7) speed = 7;
    g_loop.gameSpeed = speed;
    g_loop.tickInterval = CalculateTickInterval(speed);
}

int GameLoop_GetSpeed(void) {
    return g_loop.gameSpeed;
}

void GameLoop_Pause(BOOL pause) {
    g_loop.paused = pause ? true : false;

    // Reset accumulator when unpausing to prevent catch-up
    if (!g_loop.paused) {
        g_loop.lastUpdateTime = GetTickCount();
        g_loop.accumulator = 0;
    }
}

BOOL GameLoop_IsPaused(void) {
    return g_loop.paused ? TRUE : FALSE;
}

void GameLoop_Quit(void) {
    g_loop.quitRequested = true;
    g_loop.state = GAME_STATE_QUIT;
}

BOOL GameLoop_ShouldQuit(void) {
    return g_loop.quitRequested ? TRUE : FALSE;
}

void GameLoop_SetUpdateCallback(GameUpdateCallback callback) {
    g_loop.updateCallback = callback;
}

void GameLoop_SetRenderCallback(GameRenderCallback callback) {
    g_loop.renderCallback = callback;
}
