/**
 * Test Blowfish cipher with known test vectors
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "crypto/blowfish.h"

// Test vectors from Bruce Schneier's test cases
// https://www.schneier.com/code/vectors.txt

struct TestVector {
    const char* key;
    size_t keyLen;
    uint8_t plaintext[8];
    uint8_t ciphertext[8];
};

// Standard Blowfish test vectors
static const TestVector vectors[] = {
    // Key: 0x00000000, Plaintext: 0x0000000000000000, Ciphertext: 0x4EF997456198DD78
    { "\x00\x00\x00\x00", 4,
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x4E, 0xF9, 0x97, 0x45, 0x61, 0x98, 0xDD, 0x78} },

    // Key: 0xFFFFFFFF, Plaintext: 0xFFFFFFFFFFFFFFFF, Ciphertext: 0x51866FD5B85ECB8A
    { "\xFF\xFF\xFF\xFF", 4,
      {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
      {0x51, 0x86, 0x6F, 0xD5, 0xB8, 0x5E, 0xCB, 0x8A} },

    // Key: "TESTKEY", Plaintext: "testdata"
    // This is a custom test case
};

void printHex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    printf("Blowfish Test Vectors\n");
    printf("=====================\n\n");

    int passed = 0;
    int failed = 0;

    for (size_t t = 0; t < sizeof(vectors) / sizeof(vectors[0]); t++) {
        const TestVector& v = vectors[t];

        printf("Test %zu:\n", t + 1);

        Blowfish bf;
        bf.SetKey((const uint8_t*)v.key, v.keyLen);

        // Test encryption
        uint8_t block[8];
        memcpy(block, v.plaintext, 8);

        printf("  Key length: %zu bytes\n", v.keyLen);
        printHex("  Plaintext ", v.plaintext, 8);
        printHex("  Expected  ", v.ciphertext, 8);

        bf.EncryptBlock(block);
        printHex("  Got       ", block, 8);

        if (memcmp(block, v.ciphertext, 8) == 0) {
            printf("  Encrypt: PASS\n");
            passed++;
        } else {
            printf("  Encrypt: FAIL\n");
            failed++;
        }

        // Test decryption
        bf.DecryptBlock(block);
        printHex("  Decrypted ", block, 8);

        if (memcmp(block, v.plaintext, 8) == 0) {
            printf("  Decrypt: PASS\n");
            passed++;
        } else {
            printf("  Decrypt: FAIL\n");
            failed++;
        }

        printf("\n");
    }

    // Test with a longer key - from OpenSSL test vectors
    printf("Test with longer key:\n");
    {
        // Key: "abcdefghijklmnop" (16 bytes)
        // Plaintext: 0x424C4F5746495348 ("BLOWFISH")
        const uint8_t key[] = "abcdefghijklmnop";
        uint8_t block[8] = {'B', 'L', 'O', 'W', 'F', 'I', 'S', 'H'};

        Blowfish bf;
        bf.SetKey(key, 16);

        printHex("  Key       ", key, 16);
        printHex("  Plaintext ", block, 8);

        uint8_t original[8];
        memcpy(original, block, 8);

        bf.EncryptBlock(block);
        printHex("  Encrypted ", block, 8);

        bf.DecryptBlock(block);
        printHex("  Decrypted ", block, 8);

        if (memcmp(block, original, 8) == 0) {
            printf("  Round-trip: PASS\n");
            passed++;
        } else {
            printf("  Round-trip: FAIL\n");
            failed++;
        }
    }

    printf("\n=====================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);

    return failed > 0 ? 1 : 0;
}
