/**
 * Red Alert macOS Port - VQA Video Player
 *
 * Compatibility header - forwards to ra-media library.
 */

#ifndef VIDEO_VQA_H
#define VIDEO_VQA_H

#include <wwd/vqa.h>

// Type aliases for game code compatibility
typedef WwdVqaHeader VQAHeader;
typedef WwdVqaPlayer VQAPlayer;
typedef WwdVqaState VQAState;

// Constants are already compatible (defined in ra/vqa.h)
// VQA_ID_*, VQAHDF_*, VQA_MAX_WIDTH, VQA_MAX_HEIGHT

#endif // VIDEO_VQA_H
