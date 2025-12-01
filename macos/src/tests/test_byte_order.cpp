/**
 * Test byte order handling for RSA key and cipher blocks
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

class BigInt320 {
public:
    static constexpr int WORDS = 10;
    uint32_t data[WORDS];

    BigInt320() { memset(data, 0, sizeof(data)); }
    BigInt320(uint32_t val) { memset(data, 0, sizeof(data)); data[0] = val; }

    // Little-endian: byte[0] = LSB
    void FromBytesLE(const uint8_t* bytes, size_t len) {
        memset(data, 0, sizeof(data));
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            data[wordIdx] |= ((uint32_t)bytes[i]) << (byteIdx * 8);
        }
    }

    // Big-endian: byte[0] = MSB
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

    void ToBytesLE(uint8_t* bytes, size_t len) const {
        memset(bytes, 0, len);
        for (size_t i = 0; i < len && i < WORDS * 4; i++) {
            size_t wordIdx = i / 4;
            size_t byteIdx = i % 4;
            bytes[i] = (data[wordIdx] >> (byteIdx * 8)) & 0xFF;
        }
    }

    void Print(const char* label) const {
        printf("%s (internal, word[0]=LSB): ", label);
        for (int i = 0; i < WORDS; i++) {
            printf("%08X ", data[i]);
        }
        printf("\n");
    }

    void PrintHex(const char* label) const {
        printf("%s (big-endian hex): ", label);
        for (int i = WORDS - 1; i >= 0; i--) {
            printf("%08X", data[i]);
        }
        printf("\n");
    }
};

void printBytes(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    printf("Byte Order Tests\n");
    printf("================\n\n");

    // Test 1: Small number - verify conversion
    {
        printf("Test 1: Small number 0x12345678\n");
        BigInt320 num(0x12345678);
        num.Print("Internal");

        uint8_t bytes[4];
        num.ToBytesLE(bytes, 4);
        printBytes("LE bytes", bytes, 4);
        printf("Expected LE: 78 56 34 12\n\n");
    }

    // Test 2: Public key modulus from base64
    {
        printf("Test 2: Public key modulus (40 bytes, big-endian DER)\n");

        // These are the 40 bytes after the DER header (02 28)
        uint8_t modulusBytes[40] = {
            0x51, 0xbc, 0xda, 0x08, 0x6d, 0x39, 0xfc, 0xe4,  // MSB
            0x56, 0x51, 0x60, 0xd6, 0x51, 0x71, 0x3f, 0xa2,
            0xe8, 0xaa, 0x54, 0xfa, 0x66, 0x82, 0xb0, 0x4a,
            0xab, 0xdd, 0x0e, 0x6a, 0xf8, 0xb0, 0xc1, 0xe6,
            0xd1, 0xfb, 0x4f, 0x3d, 0xaa, 0x43, 0x7f, 0x15   // LSB
        };

        printBytes("Original DER bytes (BE)", modulusBytes, 40);

        BigInt320 modulus;
        modulus.FromBytes(modulusBytes, 40);
        modulus.Print("After FromBytes()");
        modulus.PrintHex("As big-endian");

        printf("\nVerification:\n");
        printf("First byte (0x51) should be in MSB position\n");
        printf("Last byte (0x15) should be in LSB position (word[0] bits 0-7)\n");
        printf("word[0] & 0xFF = 0x%02X (expected 0x15)\n", modulus.data[0] & 0xFF);
        printf("word[9] >> 24 = 0x%02X (expected 0x51)\n", (modulus.data[9] >> 24) & 0xFF);
    }

    printf("\n");

    // Test 3: Cipher block - little endian
    {
        printf("Test 3: Cipher block (40 bytes, little-endian as per wiki)\n");

        // Sample cipher block - first 40 bytes from REDALERT.MIX
        uint8_t cipherBytes[40] = {
            0x04, 0x70, 0x41, 0xE4, 0xBB, 0x12, 0x9B, 0x19,  // LSB first
            0x7E, 0xFB, 0x40, 0x86, 0xDD, 0x97, 0x4D, 0x11,
            0x14, 0x98, 0x81, 0x0B, 0xDE, 0xCE, 0xD3, 0x6B,
            0xEB, 0x6B, 0xFB, 0xFB, 0x4F, 0x4B, 0xB0, 0x13,
            0x92, 0x0F, 0xD8, 0x38, 0xF0, 0xE4, 0x43, 0x45   // MSB last (LE)
        };

        printBytes("Cipher bytes (LE)", cipherBytes, 40);

        BigInt320 cipher;
        cipher.FromBytesLE(cipherBytes, 40);
        cipher.Print("After FromBytesLE()");
        cipher.PrintHex("As big-endian");

        printf("\nVerification:\n");
        printf("First byte (0x04) should be in LSB position (word[0] bits 0-7)\n");
        printf("word[0] & 0xFF = 0x%02X (expected 0x04)\n", cipher.data[0] & 0xFF);
    }

    printf("\n================\n");
    printf("Byte order tests complete\n");

    return 0;
}
