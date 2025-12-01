/**
 * Test modular exponentiation with small numbers we can verify manually
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

// Copy the BigInt320 class from mixkey.cpp for testing
class BigInt320 {
public:
    static constexpr int WORDS = 10;
    uint32_t data[WORDS];

    BigInt320() { memset(data, 0, sizeof(data)); }
    BigInt320(uint32_t val) { memset(data, 0, sizeof(data)); data[0] = val; }

    void FromBytesLE(const uint8_t* bytes, size_t len) {
        memset(data, 0, sizeof(data));
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            data[wordIdx] |= ((uint32_t)bytes[i]) << (byteIdx * 8);
        }
    }

    void ToBytesLE(uint8_t* bytes, size_t len) const {
        memset(bytes, 0, len);
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            bytes[i] = (data[wordIdx] >> (byteIdx * 8)) & 0xFF;
        }
    }

    void FromBytes(const uint8_t* bytes, size_t len) {
        memset(data, 0, sizeof(data));
        size_t offset = 0;
        if (len > WORDS * 4) {
            offset = len - WORDS * 4;
            len = WORDS * 4;
        }
        for (size_t i = 0; i < len; i++) {
            size_t wordIdx = (len - 1 - i) / 4;
            size_t byteIdx = (len - 1 - i) % 4;
            data[wordIdx] |= ((uint32_t)bytes[offset + i]) << (byteIdx * 8);
        }
    }

    int Compare(const BigInt320& other) const {
        for (int i = WORDS - 1; i >= 0; i--) {
            if (data[i] < other.data[i]) return -1;
            if (data[i] > other.data[i]) return 1;
        }
        return 0;
    }

    void Sub(const BigInt320& other) {
        uint64_t borrow = 0;
        for (int i = 0; i < WORDS; i++) {
            uint64_t diff = (uint64_t)data[i] - (uint64_t)other.data[i] - borrow;
            data[i] = (uint32_t)diff;
            borrow = (diff >> 63) & 1;
        }
    }

    bool IsZero() const {
        for (int i = 0; i < WORDS; i++) {
            if (data[i] != 0) return false;
        }
        return true;
    }

    bool GetBit(int pos) const {
        if (pos < 0 || pos >= WORDS * 32) return false;
        return (data[pos / 32] >> (pos % 32)) & 1;
    }

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

    void Print(const char* label) const {
        printf("%s: ", label);
        for (int i = WORDS - 1; i >= 0; i--) {
            printf("%08X", data[i]);
        }
        printf("\n");
    }
};

// Modular multiplication using shift-and-subtract
static void ModMul(BigInt320& result, const BigInt320& a, const BigInt320& b, const BigInt320& m) {
    uint32_t product[BigInt320::WORDS * 2];
    memset(product, 0, sizeof(product));

    // Multiply a * b
    for (int i = 0; i < BigInt320::WORDS; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < BigInt320::WORDS; j++) {
            uint64_t prod = (uint64_t)a.data[i] * (uint64_t)b.data[j] + product[i + j] + carry;
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

    int modBits = m.HighBit();

    if (productBits < 0 || modBits < 0) {
        memset(result.data, 0, sizeof(result.data));
        return;
    }

    // Shift-and-subtract division
    while (productBits >= modBits) {
        int shift = productBits - modBits;
        int wordShift = shift / 32;
        int bitShift = shift % 32;

        bool canSubtract = false;

        if (wordShift < BigInt320::WORDS) {
            uint64_t mShifted[BigInt320::WORDS + 1];
            memset(mShifted, 0, sizeof(mShifted));

            for (int i = 0; i < BigInt320::WORDS; i++) {
                mShifted[i] |= ((uint64_t)m.data[i] << bitShift);
                if (i + 1 <= BigInt320::WORDS) {
                    mShifted[i + 1] |= (bitShift > 0) ? ((uint64_t)m.data[i] >> (32 - bitShift)) : 0;
                }
            }

            canSubtract = true;
            for (int i = BigInt320::WORDS; i >= 0; i--) {
                uint32_t pVal = (wordShift + i < BigInt320::WORDS * 2) ? product[wordShift + i] : 0;
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
                uint64_t borrow = 0;
                for (int i = 0; i <= BigInt320::WORDS; i++) {
                    int idx = wordShift + i;
                    if (idx >= BigInt320::WORDS * 2) break;
                    uint64_t diff = (uint64_t)product[idx] - (uint32_t)mShifted[i] - borrow;
                    product[idx] = (uint32_t)diff;
                    borrow = (diff >> 63) & 1;
                }
                for (int i = wordShift + BigInt320::WORDS + 1; i < BigInt320::WORDS * 2 && borrow; i++) {
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

        if (!canSubtract && productBits >= modBits) {
            productBits--;
        }
    }

    for (int i = 0; i < BigInt320::WORDS; i++) {
        result.data[i] = product[i];
    }

    while (result.Compare(m) >= 0) {
        result.Sub(m);
    }
}

// Modular exponentiation
static void ModExp(BigInt320& result, const BigInt320& base, const BigInt320& exp, const BigInt320& m) {
    BigInt320 b = base;
    result = BigInt320(1);

    int bits = exp.HighBit();
    if (bits < 0) {
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

int main() {
    printf("Modular Exponentiation Tests\n");
    printf("============================\n\n");

    // Test 1: Simple case - 2^3 mod 7 = 8 mod 7 = 1
    {
        printf("Test 1: 2^3 mod 7 = ?\n");
        BigInt320 base(2);
        BigInt320 exp(3);
        BigInt320 mod(7);
        BigInt320 result;
        ModExp(result, base, exp, mod);
        printf("Expected: 1, Got: %u\n\n", result.data[0]);
    }

    // Test 2: 3^5 mod 13 = 243 mod 13 = 243 - 18*13 = 243 - 234 = 9
    {
        printf("Test 2: 3^5 mod 13 = ?\n");
        BigInt320 base(3);
        BigInt320 exp(5);
        BigInt320 mod(13);
        BigInt320 result;
        ModExp(result, base, exp, mod);
        printf("Expected: 9, Got: %u\n\n", result.data[0]);
    }

    // Test 3: 7^65537 mod 11
    // 7^1 mod 11 = 7
    // 7^2 mod 11 = 49 mod 11 = 5
    // 7^4 mod 11 = 25 mod 11 = 3
    // 7^8 mod 11 = 9 mod 11 = 9
    // 7^16 mod 11 = 81 mod 11 = 4
    // ...This is tedious to calculate by hand
    // But 65537 = 2^16 + 1, so 7^65537 = 7^(2^16) * 7^1 mod 11
    {
        printf("Test 3: 7^65537 mod 11 = ?\n");
        BigInt320 base(7);
        BigInt320 exp(65537);
        BigInt320 mod(11);
        BigInt320 result;
        ModExp(result, base, exp, mod);
        printf("Got: %u\n", result.data[0]);
        // Calculate 7^10 mod 11 = 1 (Fermat's little theorem: a^(p-1) = 1 mod p)
        // 65537 mod 10 = 7, so 7^65537 mod 11 = 7^7 mod 11 = 7^4 * 7^2 * 7 = 3 * 5 * 7 = 105 mod 11 = 6
        printf("Expected: 6\n\n");
    }

    // Test 4: Larger modulus with known result
    {
        printf("Test 4: 12345^65537 mod 1000003 = ?\n");
        BigInt320 base(12345);
        BigInt320 exp(65537);
        BigInt320 mod(1000003);
        BigInt320 result;
        ModExp(result, base, exp, mod);
        printf("Got: %u\n", result.data[0]);
        // We'll verify this externally
    }

    // Test 5: Very simple - 1^e mod m = 1
    {
        printf("\nTest 5: 1^65537 mod (any) = 1\n");
        BigInt320 base(1);
        BigInt320 exp(65537);
        BigInt320 mod(12345);
        BigInt320 result;
        ModExp(result, base, exp, mod);
        printf("Expected: 1, Got: %u\n", result.data[0]);
    }

    // Test 6: Check with 40-byte modulus (actual key)
    {
        printf("\nTest 6: Testing with actual modulus size\n");

        uint8_t modulusBytes[40] = {
            0x51, 0xbc, 0xda, 0x08, 0x6d, 0x39, 0xfc, 0xe4,
            0x56, 0x51, 0x60, 0xd6, 0x51, 0x71, 0x3f, 0xa2,
            0xe8, 0xaa, 0x54, 0xfa, 0x66, 0x82, 0xb0, 0x4a,
            0xab, 0xdd, 0x0e, 0x6a, 0xf8, 0xb0, 0xc1, 0xe6,
            0xd1, 0xfb, 0x4f, 0x3d, 0xaa, 0x43, 0x7f, 0x15
        };

        BigInt320 modulus;
        modulus.FromBytes(modulusBytes, 40);

        // Test: 1^65537 mod n should be 1
        BigInt320 base(1);
        BigInt320 exp(65537);
        BigInt320 result;
        ModExp(result, base, exp, modulus);

        printf("1^65537 mod n: ");
        if (result.data[0] == 1 && result.data[1] == 0) {
            printf("PASS (= 1)\n");
        } else {
            printf("FAIL (got %u)\n", result.data[0]);
        }

        // Test: 2^65537 mod n
        BigInt320 two(2);
        ModExp(result, two, exp, modulus);
        result.Print("2^65537 mod n");
    }

    printf("\n============================\n");
    printf("Tests complete\n");

    return 0;
}
