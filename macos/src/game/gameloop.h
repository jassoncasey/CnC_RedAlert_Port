/**
 * Red Alert macOS Port - Game Loop
 *
 * Core game loop timing and state management.
 * Matches original game's frame-based timing model.
 */

#ifndef GAME_GAMELOOP_H
#define GAME_GAMELOOP_H

#include "compat/windows.h"
#include <cstdint>

// Original game ran at 15 FPS (TIMER_SECOND / 15 ticks per frame)
// We'll allow configurable frame rate
#define DEFAULT_GAME_FPS      15
#define MAX_GAME_FPS          60

// Frame timing (in game ticks, where 60 ticks = 1 second like original)
#define TICKS_PER_SECOND      60
#define TICKS_PER_MINUTE      (TICKS_PER_SECOND * 60)

// Game state
enum GameState {
    GAME_STATE_INIT,        // Initializing
    GAME_STATE_MENU,        // In menus
    GAME_STATE_PLAYING,     // Active gameplay
    GAME_STATE_PAUSED,      // Paused
    GAME_STATE_QUIT         // Shutting down
};

// Frame statistics
struct FrameStats {
    uint32_t frameCount;        // Total frames rendered
    uint32_t gameFrame;         // Game logic frame (at game FPS)
    float    currentFPS;        // Measured render FPS
    float    avgFrameTime;      // Average frame time in ms
    uint32_t lastSecondFrames;  // Frames in last second
    DWORD    lastSecondTime;    // Time of last FPS calculation
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize game loop
 */
void GameLoop_Init(void);

/**
 * Shutdown game loop
 */
void GameLoop_Shutdown(void);

/**
 * Run one iteration of the game loop
 * Called from the render callback (60 FPS).
 * Internally handles game logic timing.
 *
 * @return TRUE if game should continue, FALSE to quit
 */
BOOL GameLoop_RunFrame(void);

/**
 * Get current game state
 */
GameState GameLoop_GetState(void);

/**
 * Set game state
 */
void GameLoop_SetState(GameState state);

/**
 * Get current game frame number
 */
uint32_t GameLoop_GetFrame(void);

/**
 * Get frame statistics
 */
const FrameStats* GameLoop_GetStats(void);

/**
 * Set game speed (0 = fastest, higher = slower)
 * Original game used values 0-7, default 4.
 */
void GameLoop_SetSpeed(int speed);

/**
 * Get current game speed
 */
int GameLoop_GetSpeed(void);

/**
 * Pause/unpause the game
 */
void GameLoop_Pause(BOOL pause);

/**
 * Check if game is paused
 */
BOOL GameLoop_IsPaused(void);

/**
 * Request game quit
 */
void GameLoop_Quit(void);

/**
 * Check if quit was requested
 */
BOOL GameLoop_ShouldQuit(void);

// Callback types for game loop events
typedef void (*GameUpdateCallback)(uint32_t frame, float deltaTime);
typedef void (*GameRenderCallback)(void);

/**
 * Set update callback (called at game FPS rate)
 */
void GameLoop_SetUpdateCallback(GameUpdateCallback callback);

/**
 * Set render callback (called every frame)
 */
void GameLoop_SetRenderCallback(GameRenderCallback callback);

#ifdef __cplusplus
}
#endif

#endif // GAME_GAMELOOP_H
