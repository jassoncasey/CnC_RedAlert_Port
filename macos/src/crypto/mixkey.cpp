/**
 * Red Alert macOS Port - MIX Key Decryption Implementation
 *
 * Implements RSA decryption using Westwood's public key.
 * The key is embedded in the original game's binary.
 *
 * Public key (base64): AihRvNoIbTn85FZRYNZRcT+i6KpU+maCsEqr3Q5q+LDB5tH7Tz2qQ38V
 * Exponent: 65537 (0x10001)
 */

#include "mixkey.h"
#include <cstring>
#include <cstdio>

// Simple big integer class for RSA (320-bit precision for 40-byte blocks)
// We need 320 bits because the modulus is 40 bytes = 320 bits
class BigInt320 {
public:
    static constexpr int WORDS = 10;  // 10 x 32-bit = 320 bits
    uint32_t data[WORDS];

    BigInt320() {
        memset(data, 0, sizeof(data));
    }

    BigInt320(uint32_t val) {
        memset(data, 0, sizeof(data));
        data[0] = val;
    }

    // Copy from byte array (little-endian - Westwood format)
    void FromBytesLE(const uint8_t* bytes, size_t len) {
        memset(data, 0, sizeof(data));
        // Read bytes directly as little-endian
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            data[wordIdx] |= ((uint32_t)bytes[i]) << (byteIdx * 8);
        }
    }

    // Copy to byte array (little-endian - Westwood format)
    void ToBytesLE(uint8_t* bytes, size_t len) const {
        memset(bytes, 0, len);
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            bytes[i] = (data[wordIdx] >> (byteIdx * 8)) & 0xFF;
        }
    }

    // Copy from byte array (big-endian - DER format for key)
    void FromBytes(const uint8_t* bytes, size_t len) {
        memset(data, 0, sizeof(data));
        size_t offset = 0;
        if (len > WORDS * 4) {
            offset = len - WORDS * 4;
            len = WORDS * 4;
        }

        // Read bytes in big-endian order into little-endian words
        for (size_t i = 0; i < len; i++) {
            size_t wordIdx = (len - 1 - i) / 4;
            size_t byteIdx = (len - 1 - i) % 4;
            data[wordIdx] |= ((uint32_t)bytes[offset + i]) << (byteIdx * 8);
        }
    }

    // Copy to byte array (big-endian)
    void ToBytes(uint8_t* bytes, size_t len) const {
        memset(bytes, 0, len);
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = (len - 1 - i) / 4;
            size_t byteIdx = (len - 1 - i) % 4;
            if (wordIdx < WORDS) {
                bytes[i] = (data[wordIdx] >> (byteIdx * 8)) & 0xFF;
            }
        }
    }

    // Compare: -1 if this < other, 0 if equal, 1 if this > other
    int Compare(const BigInt320& other) const {
        for (int i = WORDS - 1; i >= 0; i--) {
            if (data[i] < other.data[i]) return -1;
            if (data[i] > other.data[i]) return 1;
        }
        return 0;
    }

    // Add another BigInt
    void Add(const BigInt320& other) {
        uint64_t carry = 0;
        for (int i = 0; i < WORDS; i++) {
            uint64_t sum = (uint64_t)data[i] + (uint64_t)other.data[i] + carry;
            data[i] = (uint32_t)sum;
            carry = sum >> 32;
        }
    }

    // Subtract another BigInt (assumes this >= other)
    void Sub(const BigInt320& other) {
        uint64_t borrow = 0;
        for (int i = 0; i < WORDS; i++) {
            uint64_t a = (uint64_t)data[i];
            uint64_t b = (uint64_t)other.data[i];
            uint64_t diff = a - b - borrow;
            data[i] = (uint32_t)diff;
            borrow = (diff >> 63) & 1;
        }
    }

    // Left shift by 1 bit
    void ShiftLeft1() {
        uint32_t carry = 0;
        for (int i = 0; i < WORDS; i++) {
            uint32_t newCarry = data[i] >> 31;
            data[i] = (data[i] << 1) | carry;
            carry = newCarry;
        }
    }

    // Right shift by 1 bit
    void ShiftRight1() {
        uint32_t carry = 0;
        for (int i = WORDS - 1; i >= 0; i--) {
            uint32_t newCarry = data[i] & 1;
            data[i] = (data[i] >> 1) | (carry << 31);
            carry = newCarry;
        }
    }

    // Check if zero
    bool IsZero() const {
        for (int i = 0; i < WORDS; i++) {
            if (data[i] != 0) return false;
        }
        return true;
    }

    // Get bit at position
    bool GetBit(int pos) const {
        if (pos < 0 || pos >= WORDS * 32) return false;
        return (data[pos / 32] >> (pos % 32)) & 1;
    }

    // Get highest set bit position (-1 if zero)
    int HighBit() const {
        for (int i = WORDS - 1; i >= 0; i--) {
            if (data[i] != 0) {
                for (int j = 31; j >= 0; j--) {
                    if ((data[i] >> j) & 1) {
                        return i * 32 + j;
                    }
                }
            }
        }
        return -1;
    }
};

// Modular multiplication: result = (a * b) mod m
// Uses shift-and-subtract division for modular reduction
static void ModMul(BigInt320& result, const BigInt320& a,
                   const BigInt320& b, const BigInt320& m) {
    // Use double-width intermediate for multiplication
    uint32_t product[BigInt320::WORDS * 2];
    memset(product, 0, sizeof(product));

    // Multiply a * b
    for (int i = 0; i < BigInt320::WORDS; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < BigInt320::WORDS; j++) {
            uint64_t ai = (uint64_t)a.data[i];
            uint64_t bj = (uint64_t)b.data[j];
            uint64_t prod = ai * bj + product[i + j] + carry;
            product[i + j] = (uint32_t)prod;
            carry = prod >> 32;
        }
        product[i + BigInt320::WORDS] = (uint32_t)carry;
    }

    // Find highest bit in product
    int productBits = -1;
    for (int i = BigInt320::WORDS * 2 - 1; i >= 0; i--) {
        if (product[i] != 0) {
            for (int j = 31; j >= 0; j--) {
                if ((product[i] >> j) & 1) {
                    productBits = i * 32 + j;
                    break;
                }
            }
            break;
        }
    }

    // Find highest bit in modulus
    int modBits = m.HighBit();

    if (productBits < 0 || modBits < 0) {
        // Product or modulus is zero
        memset(result.data, 0, sizeof(result.data));
        return;
    }

    // Shift-and-subtract division
    // We need to reduce product mod m
    // Start from the highest bit and work down
    while (productBits >= modBits) {
        int shift = productBits - modBits;
        int wordShift = shift / 32;
        int bitShift = shift % 32;

        // Check if product >= m << shift
        // Build shifted m temporarily for comparison
        bool canSubtract = false;

        // Simple comparison: check the high words
        if (wordShift < BigInt320::WORDS) {
            // Compare product[wordShift..] with m << bitShift
            uint64_t mShifted[BigInt320::WORDS + 1];
            memset(mShifted, 0, sizeof(mShifted));

            for (int i = 0; i < BigInt320::WORDS; i++) {
                mShifted[i] |= ((uint64_t)m.data[i] << bitShift);
                if (i + 1 <= BigInt320::WORDS) {
                    uint64_t hi = (bitShift > 0)
                        ? ((uint64_t)m.data[i] >> (32 - bitShift)) : 0;
                    mShifted[i + 1] |= hi;
                }
            }

            // Compare product[wordShift..] with mShifted
            canSubtract = true;
            for (int i = BigInt320::WORDS; i >= 0; i--) {
                int pIdx = wordShift + i;
                int maxI = BigInt320::WORDS * 2;
                uint32_t pVal = (pIdx < maxI) ? product[pIdx] : 0;
                uint32_t mVal = (uint32_t)mShifted[i];
                if (pVal < mVal) {
                    canSubtract = false;
                    break;
                }
                if (pVal > mVal) {
                    break;
                }
            }

            if (canSubtract) {
                // Subtract m << shift from product
                uint64_t borrow = 0;
                for (int i = 0; i <= BigInt320::WORDS; i++) {
                    int idx = wordShift + i;
                    if (idx >= BigInt320::WORDS * 2) break;
                    uint64_t pv = (uint64_t)product[idx];
                    uint64_t mv = (uint32_t)mShifted[i];
                    uint64_t diff = pv - mv - borrow;
                    product[idx] = (uint32_t)diff;
                    borrow = (diff >> 63) & 1;
                }
                // Propagate borrow
                int start = wordShift + BigInt320::WORDS + 1;
                for (int i = start; i < BigInt320::WORDS * 2 && borrow; i++) {
                    uint64_t diff = (uint64_t)product[i] - borrow;
                    product[i] = (uint32_t)diff;
                    borrow = (diff >> 63) & 1;
                }
            }
        }

        // Recalculate productBits
        productBits = -1;
        for (int i = BigInt320::WORDS * 2 - 1; i >= 0; i--) {
            if (product[i] != 0) {
                for (int j = 31; j >= 0; j--) {
                    if ((product[i] >> j) & 1) {
                        productBits = i * 32 + j;
                        break;
                    }
                }
                break;
            }
        }

        // Safety: avoid infinite loop if no subtract
        if (!canSubtract && productBits >= modBits) {
            productBits--;
        }
    }

    // Copy result
    for (int i = 0; i < BigInt320::WORDS; i++) {
        result.data[i] = product[i];
    }

    // Final reduction if needed
    while (result.Compare(m) >= 0) {
        result.Sub(m);
    }
}

// Modular exponentiation: result = base^exp mod m
static void ModExp(BigInt320& result, const BigInt320& base,
                   const BigInt320& exp, const BigInt320& m) {
    BigInt320 b = base;
    result = BigInt320(1);

    int bits = exp.HighBit();
    if (bits < 0) {
        // exp is zero, result is 1
        return;
    }

    for (int i = 0; i <= bits; i++) {
        if (exp.GetBit(i)) {
            ModMul(result, result, b, m);
        }
        if (i < bits) {
            ModMul(b, b, b, m);
        }
    }
}

// Westwood's public key (decoded from base64)
// Base64: AihRvNoIbTn85FZRYNZRcT+i6KpU+maCsEqr3Q5q+LDB5tH7Tz2qQ38V
// This decodes to a DER-encoded integer
// The "02 28" prefix means integer of 40 bytes (0x28 = 40)
// However, the actual modulus is only 312 bits (39 bytes significant)
static const uint8_t PUBLIC_KEY_DER[] = {
    0x02, 0x28,  // DER: INTEGER, 40 bytes (but leading byte is padding)
    0x51, 0xbc, 0xda, 0x08, 0x6d, 0x39, 0xfc, 0xe4,
    0x56, 0x51, 0x60, 0xd6, 0x51, 0x71, 0x3f, 0xa2,
    0xe8, 0xaa, 0x54, 0xfa, 0x66, 0x82, 0xb0, 0x4a,
    0xab, 0xdd, 0x0e, 0x6a, 0xf8, 0xb0, 0xc1, 0xe6,
    0xd1, 0xfb, 0x4f, 0x3d, 0xaa, 0x43, 0x7f, 0x15
};

// Public exponent: 65537 (0x10001)
static const uint32_t PUBLIC_EXPONENT = 65537;

// Get bit length of big number stored in uint32_t array
static uint32_t GetBitLength(const uint32_t* n, uint32_t wordLen) {
    // Find highest non-zero word
    uint32_t len = wordLen;
    while (len > 0 && n[len - 1] == 0) len--;
    if (len == 0) return 0;

    // Find highest bit in that word
    uint32_t bitLen = len * 32;
    uint32_t mask = 0x80000000;
    while ((mask & n[len - 1]) == 0) {
        mask >>= 1;
        bitLen--;
    }
    return bitLen;
}

bool MixKey_DecryptKey(const uint8_t* encryptedKey, uint8_t* blowfishKey) {
    if (!encryptedKey || !blowfishKey) {
        return false;
    }

    // Parse the public key modulus (skip DER header "02 28")
    BigInt320 modulus;
    modulus.FromBytes(PUBLIC_KEY_DER + 2, 40);

    // Get the bit length of the modulus (should be 312)
    uint32_t modulusBitLen = GetBitLength(modulus.data, BigInt320::WORDS);

    // Calculate block sizes per OpenRA algorithm:
    // a = (modulusBitLen - 1) / 8  (bytes per decrypted block)
    // Block input size = a + 1
    uint32_t a = (modulusBitLen - 1) / 8;  // = 38 for 312-bit modulus
    uint32_t blockInSize = a + 1;           // = 39 bytes per encrypted block
    uint32_t blockOutSize = a;              // = 38 bytes per decrypted block

    // We need enough decrypted data for 56 bytes
    // pre_len = (55 / a + 1) * (a + 1) = (55/38 + 1) * 39 = 2 * 39 = 78
    uint32_t numBlocks = 55 / a + 1;        // = 2 blocks

    // Exponent
    BigInt320 exp(PUBLIC_EXPONENT);

    // Decrypt blocks
    // Input: numBlocks * blockInSize = 2 * 39 = 78 bytes
    // Output: numBlocks * blockOutSize = 2 * 38 = 76 bytes
    uint8_t decrypted[256];
    memset(decrypted, 0, sizeof(decrypted));

    uint32_t srcOffset = 0;
    uint32_t dstOffset = 0;

    for (uint32_t block = 0; block < numBlocks; block++) {
        // Read blockInSize bytes as little-endian big integer
        BigInt320 cipher;
        cipher.FromBytesLE(encryptedKey + srcOffset, blockInSize);

        // RSA decrypt: plain = cipher^exp mod modulus
        BigInt320 plain;
        ModExp(plain, cipher, exp, modulus);

        // Write blockOutSize bytes as little-endian
        plain.ToBytesLE(decrypted + dstOffset, blockOutSize);

        srcOffset += blockInSize;
        dstOffset += blockOutSize;
    }

    // Copy first 56 bytes as Blowfish key
    memcpy(blowfishKey, decrypted, MIXKEY_DECRYPTED_SIZE);

    return true;
}
