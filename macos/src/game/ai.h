/**
 * Red Alert macOS Port - Basic AI System
 *
 * Simple AI opponent that builds a base and attacks the player.
 */

#ifndef GAME_AI_H
#define GAME_AI_H

#include "compat/windows.h"
#include "units.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// AI difficulty levels
typedef enum {
    AI_EASY = 0,
    AI_MEDIUM,
    AI_HARD
} AIDifficulty;

// AI state
typedef enum {
    AI_STATE_BUILDING,      // Building up base
    AI_STATE_PREPARING,     // Building army
    AI_STATE_ATTACKING,     // Sending attack wave
    AI_STATE_DEFENDING      // Defending base
} AIState;

/**
 * Initialize the AI system
 */
void AI_Init(void);

/**
 * Shutdown the AI system
 */
void AI_Shutdown(void);

/**
 * Set AI difficulty level
 */
void AI_SetDifficulty(AIDifficulty difficulty);

/**
 * Set AI starting credits
 */
void AI_SetCredits(int credits);

/**
 * Get AI credits
 */
int AI_GetCredits(void);

/**
 * Update AI logic (called each game tick)
 */
void AI_Update(void);

/**
 * Check if AI has a specific building type
 */
BOOL AI_HasBuilding(BuildingType type);

/**
 * Count AI units of a specific type
 */
int AI_CountUnits(UnitType type);

/**
 * Calculate threat score for a target unit (AI-3: Threat assessment)
 * Higher scores = more valuable/dangerous targets
 * @param target Target unit to evaluate
 * @param attacker The unit evaluating the target
 * @return Threat score (0-1000)
 */
int AI_CalcThreatScore(Unit* target, Unit* attacker);

/**
 * Hunt mode targeting - find best target for hunt mission (AI-1)
 * Searches wider range than normal auto-acquire
 * @param unit The hunting unit
 * @return Unit ID of best target, or -1 if none
 */
int AI_FindHuntTarget(int unitId);

/**
 * Set hunt mode for an AI unit
 * @param unitId Unit to put in hunt mode
 * @param enabled Whether hunt mode is enabled
 */
void AI_SetHuntMode(int unitId, BOOL enabled);

/**
 * Check if unit is in hunt mode
 */
BOOL AI_IsHunting(int unitId);

#ifdef __cplusplus
}
#endif

#endif // GAME_AI_H
