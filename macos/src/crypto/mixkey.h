/**
 * Red Alert macOS Port - MIX Key Decryption
 *
 * RSA decryption for encrypted MIX file headers.
 * Uses Westwood's public key to decrypt the Blowfish key.
 */

#ifndef CRYPTO_MIXKEY_H
#define CRYPTO_MIXKEY_H

#include <cstdint>
#include <cstddef>

/**
 * Decrypt the encrypted key block from a MIX file header
 *
 * The encrypted block is 80 bytes (2 x 40-byte RSA blocks).
 * After decryption, we get a 56-byte Blowfish key.
 *
 * @param encryptedKey   80 bytes of RSA-encrypted key data
 * @param blowfishKey    Output buffer for 56-byte Blowfish key
 * @return true if decryption succeeded
 */
bool MixKey_DecryptKey(const uint8_t* encryptedKey, uint8_t* blowfishKey);

/**
 * Get the size of an encrypted key block
 *
 * The RSA modulus is 312 bits, so each block is 39 bytes (not 40).
 * Two 39-byte encrypted blocks = 78 bytes total.
 * Each decrypts to 38 bytes, giving 76 bytes output.
 * We take the first 56 bytes as the Blowfish key.
 */
constexpr size_t MIXKEY_RSA_BLOCK_SIZE = 39;   // RSA block size (312 bits / 8, rounded up)
constexpr size_t MIXKEY_ENCRYPTED_SIZE = 80;   // Actually 78 used, but file has 80 bytes
constexpr size_t MIXKEY_DECRYPTED_SIZE = 56;   // Blowfish key size

#endif // CRYPTO_MIXKEY_H
