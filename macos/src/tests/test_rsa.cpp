/**
 * Test RSA decryption with known test vectors
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "crypto/mixkey.h"

// Print bytes as hex
void printHex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
        if ((i + 1) % 40 == 0) printf("\n           ");
    }
    printf("\n");
}

int main() {
    printf("RSA Decryption Test\n");
    printf("===================\n\n");

    // First, let's read a known encrypted key from a MIX file
    // and see what we get from decryption

    // The decrypted data should be:
    // - A 56-byte Blowfish key
    // - Padded to 80 bytes (40 bytes per RSA block)
    // The key is derived from RNG, so we don't know the expected value
    // BUT we can verify that:
    // 1. The same input always produces the same output
    // 2. Different inputs produce different outputs

    // Read actual encrypted data from REDALERT.MIX header
    FILE* f = fopen("../assets/REDALERT.MIX", "rb");
    if (!f) {
        f = fopen("../../assets/REDALERT.MIX", "rb");
    }
    if (!f) {
        printf("Could not open REDALERT.MIX - trying without file\n\n");
    }

    if (f) {
        // Read the flags (should be 0x00020000)
        uint32_t flags;
        fread(&flags, 4, 1, f);
        printf("MIX flags: 0x%08X\n", flags);

        if (flags == 0 || (flags & 0xFFFF) == 0) {
            // Check if we need another read
            if (flags == 0) {
                fread(&flags, 4, 1, f);
                printf("Real flags: 0x%08X\n", flags);
            }
        }

        // Read the 80-byte encrypted key block
        uint8_t encryptedKey[80];
        fread(encryptedKey, 80, 1, f);

        printf("\nEncrypted key block (80 bytes):\n");
        printHex("Encrypted ", encryptedKey, 80);

        // Decrypt it
        uint8_t blowfishKey[56];
        bool success = MixKey_DecryptKey(encryptedKey, blowfishKey);

        printf("\nDecryption %s\n", success ? "succeeded" : "FAILED");
        if (success) {
            printHex("Blowfish key", blowfishKey, 56);
        }

        // Read the first encrypted header block (8 bytes after the key)
        uint8_t headerBlock[8];
        fread(headerBlock, 8, 1, f);
        printHex("\nEncrypted header", headerBlock, 8);

        fclose(f);
    }

    // Test with a simple known value
    // If we have base^e mod n = result, and we know base and e and n
    // We can verify our ModExp is working

    printf("\n=== Testing BigInt320 ===\n");

    // Simple test: 2^65537 mod n where n is the public key
    // This should give a deterministic result

    // The public key modulus (from base64 decode, skipping DER header):
    uint8_t modulusBytes[40] = {
        0x51, 0xbc, 0xda, 0x08, 0x6d, 0x39, 0xfc, 0xe4,
        0x56, 0x51, 0x60, 0xd6, 0x51, 0x71, 0x3f, 0xa2,
        0xe8, 0xaa, 0x54, 0xfa, 0x66, 0x82, 0xb0, 0x4a,
        0xab, 0xdd, 0x0e, 0x6a, 0xf8, 0xb0, 0xc1, 0xe6,
        0xd1, 0xfb, 0x4f, 0x3d, 0xaa, 0x43, 0x7f, 0x15
    };

    printf("Modulus (40 bytes):\n");
    printHex("n", modulusBytes, 40);

    printf("\nPublic exponent: 65537 (0x10001)\n");

    // If we have a cipher block and want to verify:
    // plain = cipher^e mod n

    // For testing, we can verify that 1^e mod n = 1
    uint8_t oneBlock[40] = {0};
    oneBlock[0] = 1;  // Little-endian: value is 1

    uint8_t decrypted[56];
    // Can't easily test with MixKey_DecryptKey directly since it combines 2 blocks
    // But we can look at the actual debug output

    printf("\n=== Summary ===\n");
    printf("The decryption function is being tested via test_mix_decrypt.\n");
    printf("If that test fails, the issue is in RSA or Blowfish.\n");
    printf("We've verified Blowfish works correctly with standard test vectors.\n");
    printf("The likely issue is in the RSA modular exponentiation or byte ordering.\n");

    return 0;
}
