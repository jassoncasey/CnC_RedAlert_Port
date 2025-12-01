/**
 * Red Alert macOS Port - Blowfish Cipher
 *
 * Public domain Blowfish implementation for MIX file decryption.
 * Based on Bruce Schneier's algorithm.
 */

#ifndef CRYPTO_BLOWFISH_H
#define CRYPTO_BLOWFISH_H

#include <cstdint>
#include <cstddef>

class Blowfish {
public:
    static constexpr int BLOCK_SIZE = 8;      // 64-bit blocks
    static constexpr int MAX_KEY_SIZE = 56;   // Maximum key length in bytes

    Blowfish();
    ~Blowfish();

    /**
     * Initialize cipher with a key
     * @param key     Key data (1-56 bytes)
     * @param keyLen  Key length in bytes
     */
    void SetKey(const uint8_t* key, size_t keyLen);

    /**
     * Encrypt a single 8-byte block
     * @param block   8 bytes to encrypt (modified in place)
     */
    void EncryptBlock(uint8_t* block);

    /**
     * Decrypt a single 8-byte block
     * @param block   8 bytes to decrypt (modified in place)
     */
    void DecryptBlock(uint8_t* block);

    /**
     * Encrypt data using ECB mode
     * @param data    Data to encrypt (must be multiple of 8 bytes)
     * @param len     Data length (must be multiple of 8)
     */
    void Encrypt(uint8_t* data, size_t len);

    /**
     * Decrypt data using ECB mode
     * @param data    Data to decrypt (must be multiple of 8 bytes)
     * @param len     Data length (must be multiple of 8)
     */
    void Decrypt(uint8_t* data, size_t len);

    /**
     * Check if key has been set
     */
    bool IsKeyed() const { return isKeyed_; }

private:
    static constexpr int ROUNDS = 16;
    static constexpr int SUBKEYS = ROUNDS + 2;

    // S-boxes (4 x 256 entries)
    uint32_t S_[4][256];

    // P-array (subkeys)
    uint32_t P_[SUBKEYS];

    bool isKeyed_;

    // Core Blowfish function
    uint32_t F(uint32_t x);

    // Encrypt left/right pair
    void EncryptPair(uint32_t& left, uint32_t& right);

    // Decrypt left/right pair
    void DecryptPair(uint32_t& left, uint32_t& right);

    // Initial P-array and S-box values (derived from pi)
    static const uint32_t P_INIT[SUBKEYS];
    static const uint32_t S_INIT[4][256];
};

#endif // CRYPTO_BLOWFISH_H
