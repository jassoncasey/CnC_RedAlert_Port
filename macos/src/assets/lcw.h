/**
 * Red Alert macOS Port - LCW Compression
 *
 * Lempel-Castle-Welch compression used in Westwood games.
 * Also known as Format80 compression.
 */

#ifndef ASSETS_LCW_H
#define ASSETS_LCW_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decompress LCW/Format80 compressed data
 * @param src Source compressed data
 * @param dst Destination buffer (must be pre-allocated)
 * @param srcSize Size of compressed data
 * @param dstSize Size of destination buffer
 * @return Number of bytes written to dst, or -1 on error
 */
int LCW_Decompress(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize);

/**
 * Decode Base64 data
 * @param src Base64 encoded string
 * @param srcLen Length of source string
 * @param dst Destination buffer
 * @param dstSize Size of destination buffer
 * @return Number of bytes decoded, or -1 on error
 */
int Base64_Decode(const char* src, int srcLen, uint8_t* dst, int dstSize);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_LCW_H
