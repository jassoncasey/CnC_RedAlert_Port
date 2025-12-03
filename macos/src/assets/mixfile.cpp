/**
 * Red Alert macOS Port - MIX File Reader Implementation
 *
 * Supports both encrypted and unencrypted MIX files.
 *
 * MIX file format (unencrypted):
 *   Header:
 *     short count     - Number of files in archive
 *     long size       - Total size of data section
 *   Index (count entries):
 *     long crc        - CRC hash of filename
 *     long offset     - Offset from start of data section
 *     long size       - Size of file
 *   Data section:
 *     Raw file data
 *
 * MIX file format (encrypted - RA new format):
 *   4 bytes: flags (0x00020000 = encrypted header)
 *   80 bytes: RSA-encrypted key block
 *   6 bytes: Blowfish-encrypted header (file count + data size)
 *   N*12 bytes: Blowfish-encrypted index
 *   Data section (unencrypted)
 */

#include "mixfile.h"
#include "crypto/blowfish.h"
#include "crypto/mixkey.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>

// MIX file header (6 bytes)
#pragma pack(push, 1)
struct MixHeader {
    int16_t count;      // Number of files
    int32_t dataSize;   // Total data size
};

// Index entry (12 bytes)
struct MixEntry {
    uint32_t crc;       // CRC of filename
    uint32_t offset;    // Offset in data section
    uint32_t size;      // File size
};
#pragma pack(pop)

// Internal MIX file structure
struct MixFile {
    // File-based access
    FILE* file;

    // Memory-based access
    const uint8_t* memData;
    uint32_t memSize;
    bool ownsMemData;

    // Common fields
    MixHeader header;
    MixEntry* entries;
    uint32_t dataStart;     // Offset to start of data section
    char filename[256];
    bool encrypted;         // Whether file has encrypted header
    bool isMemory;          // True if opened from memory
};

// MIX file flags
constexpr uint32_t MIX_FLAG_CHECKSUM  = 0x00010000;
constexpr uint32_t MIX_FLAG_ENCRYPTED = 0x00020000;

// Classic Westwood hash function (used by C&C and RA1)
// This is NOT a CRC - it's a rotate-left-and-add hash
uint32_t Mix_CalculateCRC(const char* name) {
    // Get length and calculate padding
    int len = 0;
    while (name[len]) len++;

    int padding = (len % 4 != 0) ? (4 - len % 4) : 0;
    int paddedLen = len + padding;

    // Convert to uppercase and pad with zeros
    uint8_t buffer[256];
    for (int i = 0; i < len; i++) {
        buffer[i] = (uint8_t)toupper((unsigned char)name[i]);
    }
    for (int i = len; i < paddedLen; i++) {
        buffer[i] = 0;
    }

    // Hash: rotate left 1 bit and add, processing 4 bytes at a time
    uint32_t result = 0;
    for (int i = 0; i < paddedLen; i += 4) {
        uint32_t val = buffer[i] |
                       ((uint32_t)buffer[i + 1] << 8) |
                       ((uint32_t)buffer[i + 2] << 16) |
                       ((uint32_t)buffer[i + 3] << 24);
        result = ((result << 1) | (result >> 31)) + val;
    }

    return result;
}

// Binary search for entry by CRC
// NOTE: MIX entries are sorted by SIGNED int32 comparison, not unsigned!
static MixEntry* FindEntry(MixFile* mix, uint32_t crc) {
    if (!mix || !mix->entries) return nullptr;

    // Entries are sorted by signed CRC for binary search
    int lo = 0;
    int hi = mix->header.count - 1;
    int32_t targetSigned = (int32_t)crc;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int32_t entrySigned = (int32_t)mix->entries[mid].crc;

        if (entrySigned == targetSigned) {
            return &mix->entries[mid];
        } else if (entrySigned < targetSigned) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    return nullptr;
}

// Helper to read encrypted MIX file
static MixFile* OpenEncryptedMix(FILE* f, uint32_t flags,
                                 const char* filename) {
    (void)flags;  // Currently unused

    // Read the 80-byte RSA-encrypted key block
    uint8_t encryptedKey[MIXKEY_ENCRYPTED_SIZE];
    if (fread(encryptedKey, MIXKEY_ENCRYPTED_SIZE, 1, f) != 1) {
        return nullptr;
    }

    // Decrypt the key using RSA
    uint8_t blowfishKey[MIXKEY_DECRYPTED_SIZE];
    if (!MixKey_DecryptKey(encryptedKey, blowfishKey)) {
        return nullptr;
    }

    // Initialize Blowfish with the decrypted key
    Blowfish bf;
    bf.SetKey(blowfishKey, MIXKEY_DECRYPTED_SIZE);

    // Read and decrypt the header (first 8 bytes, but header is 6 bytes)
    // We read 8 bytes because Blowfish works in 8-byte blocks
    uint8_t headerBlock[8];
    if (fread(headerBlock, 8, 1, f) != 1) {
        return nullptr;
    }
    bf.DecryptBlock(headerBlock);

    // Parse header (6 bytes: 2-byte count + 4-byte size)
    MixHeader header;
    header.count = (int16_t)(headerBlock[0] | (headerBlock[1] << 8));
    int b2 = headerBlock[2], b3 = headerBlock[3];
    int b4 = headerBlock[4], b5 = headerBlock[5];
    header.dataSize = (int32_t)(b2 | (b3 << 8) | (b4 << 16) | (b5 << 24));

    // Sanity check
    if (header.count < 0 || header.count > 10000) {
        return nullptr;
    }

    // Calculate index size (12 bytes per entry)
    size_t indexSize = header.count * sizeof(MixEntry);

    // We already consumed 2 bytes of the index (from the 8-byte block)
    // Calculate remaining bytes needed (including padding to 8-byte boundary)
    size_t totalHeaderIndex = 6 + indexSize;  // Header + index
    size_t blocksNeeded = (totalHeaderIndex + 7) / 8;
    size_t totalEncrypted = blocksNeeded * 8;
    size_t remaining = totalEncrypted - 8;  // We already read first 8 bytes

    // Allocate buffer for remaining encrypted data
    uint8_t* encryptedData = new uint8_t[remaining + 8];  // Extra space
    memcpy(encryptedData, headerBlock, 8);  // Copy first block

    if (remaining > 0) {
        if (fread(encryptedData + 8, remaining, 1, f) != 1) {
            delete[] encryptedData;
            return nullptr;
        }

        // Decrypt remaining blocks
        for (size_t i = 8; i < remaining + 8; i += 8) {
            bf.DecryptBlock(encryptedData + i);
        }
    }

    // Create MixFile structure
    MixFile* mix = new MixFile;
    memset(mix, 0, sizeof(MixFile));
    mix->file = f;
    mix->header = header;
    mix->encrypted = true;
    strncpy(mix->filename, filename, sizeof(mix->filename) - 1);

    // Copy index entries from decrypted data (starts at offset 6)
    mix->entries = new MixEntry[header.count];
    memcpy(mix->entries, encryptedData + 6, indexSize);

    delete[] encryptedData;

    // Data starts after flags(4) + key(80) + encrypted_header_index
    uint32_t dstart = 4 + MIXKEY_ENCRYPTED_SIZE + (uint32_t)totalEncrypted;
    mix->dataStart = dstart;

    return mix;
}

// Helper to open encrypted MIX from memory
static MixFile* OpenEncryptedMixMemory(const uint8_t* data, uint32_t size,
                                       bool ownsData) {
    if (size < 4 + MIXKEY_ENCRYPTED_SIZE + 8) {
        return nullptr;  // Too small
    }

    uint32_t pos = 4;  // Skip flags word

    // Get the RSA-encrypted key block
    const uint8_t* encryptedKey = data + pos;
    pos += MIXKEY_ENCRYPTED_SIZE;

    // Decrypt the key using RSA
    uint8_t blowfishKey[MIXKEY_DECRYPTED_SIZE];
    if (!MixKey_DecryptKey(encryptedKey, blowfishKey)) {
        return nullptr;
    }

    // Initialize Blowfish with the decrypted key
    Blowfish bf;
    bf.SetKey(blowfishKey, MIXKEY_DECRYPTED_SIZE);

    // Read and decrypt the header (first 8 bytes)
    if (pos + 8 > size) return nullptr;
    uint8_t headerBlock[8];
    memcpy(headerBlock, data + pos, 8);
    bf.DecryptBlock(headerBlock);
    pos += 8;

    // Parse header
    MixHeader header;
    header.count = (int16_t)(headerBlock[0] | (headerBlock[1] << 8));
    int b2 = headerBlock[2], b3 = headerBlock[3];
    int b4 = headerBlock[4], b5 = headerBlock[5];
    header.dataSize = (int32_t)(b2 | (b3 << 8) | (b4 << 16) | (b5 << 24));

    if (header.count < 0 || header.count > 10000) {
        return nullptr;
    }

    // Calculate total encrypted size
    size_t indexSize = header.count * sizeof(MixEntry);
    size_t totalHeaderIndex = 6 + indexSize;
    size_t blocksNeeded = (totalHeaderIndex + 7) / 8;
    size_t totalEncrypted = blocksNeeded * 8;
    size_t remaining = totalEncrypted - 8;

    // Allocate buffer for all encrypted data
    uint8_t* encryptedData = new uint8_t[totalEncrypted];
    memcpy(encryptedData, headerBlock, 8);

    if (remaining > 0) {
        if (pos + remaining > size) {
            delete[] encryptedData;
            return nullptr;
        }
        memcpy(encryptedData + 8, data + pos, remaining);
        pos += remaining;

        // Decrypt remaining blocks
        for (size_t i = 8; i < totalEncrypted; i += 8) {
            bf.DecryptBlock(encryptedData + i);
        }
    }

    // Create MixFile structure
    MixFile* mix = new MixFile;
    memset(mix, 0, sizeof(MixFile));
    mix->memData = data;
    mix->memSize = size;
    mix->ownsMemData = ownsData;
    mix->isMemory = true;
    mix->header = header;
    mix->encrypted = true;
    strncpy(mix->filename, "(memory)", sizeof(mix->filename) - 1);

    // Copy index entries
    mix->entries = new MixEntry[header.count];
    memcpy(mix->entries, encryptedData + 6, indexSize);

    delete[] encryptedData;

    // Data starts after: flags(4) + key(80) + encrypted_header_index
    mix->dataStart = 4 + MIXKEY_ENCRYPTED_SIZE + (uint32_t)totalEncrypted;

    return mix;
}

MixFileHandle Mix_OpenMemory(const void* data, uint32_t size, BOOL ownsData) {
    if (!data || size < 6) {
        return nullptr;
    }

    const uint8_t* origData = (const uint8_t*)data;
    const uint8_t* ptr = origData;

    // OpenRA format detection:
    // - Read first 2 bytes as uint16
    // - If != 0, it's classic C&C format (header starts at offset 0)
    // - If == 0, read next 2 bytes as flags:
    //   - If flags & 0x2 (encrypted), handle encryption
    //   - Otherwise, header starts at offset 4
    uint16_t firstWord16 = ptr[0] | (ptr[1] << 8);

    bool isCncMix = (firstWord16 != 0);
    uint32_t headerOffset = 0;

    if (!isCncMix) {
        // RA/TS/RA2 format - read flags at offset 2
        if (size < 4) return nullptr;
        uint16_t flags = ptr[2] | (ptr[3] << 8);

        bool isEncrypted = (flags & 0x2) != 0;

        if (isEncrypted) {
            return OpenEncryptedMixMemory(origData, size, ownsData);
        }

        // Not encrypted - header starts at offset 4
        headerOffset = 4;
    }

    ptr = origData + headerOffset;
    uint32_t remainingSize = size - headerOffset;

    // Unencrypted MIX file
    if (remainingSize < sizeof(MixHeader)) {
        return nullptr;
    }

    MixFile* mix = new MixFile;
    memset(mix, 0, sizeof(MixFile));
    mix->memData = origData;
    mix->memSize = size;
    mix->ownsMemData = ownsData;
    mix->isMemory = true;
    mix->encrypted = false;
    strncpy(mix->filename, "(memory)", sizeof(mix->filename) - 1);

    // Read header
    memcpy(&mix->header, ptr, sizeof(MixHeader));
    ptr += sizeof(MixHeader);

    if (mix->header.count < 0 || mix->header.count > 10000) {
        delete mix;
        return nullptr;
    }

    // Read index
    size_t indexSize = mix->header.count * sizeof(MixEntry);
    mix->entries = new MixEntry[mix->header.count];
    memcpy(mix->entries, ptr, indexSize);

    // Data starts after header and index (relative to original data start)
    mix->dataStart = headerOffset + sizeof(MixHeader) + (uint32_t)indexSize;

    return mix;
}

MixFileHandle Mix_Open(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return nullptr;
    }

    // OpenRA format detection:
    // - Read first 2 bytes as uint16
    // - If != 0, it's classic C&C format (header starts at offset 0)
    // - If == 0, read next 2 bytes as flags:
    //   - If flags & 0x2 (encrypted), handle encryption
    //   - Otherwise, header starts at offset 4
    uint16_t firstWord16;
    if (fread(&firstWord16, sizeof(firstWord16), 1, f) != 1) {
        fclose(f);
        return nullptr;
    }

    bool isCncMix = (firstWord16 != 0);
    long headerOffset = 0;

    if (!isCncMix) {
        // RA/TS/RA2 format - read flags
        uint16_t flags;
        if (fread(&flags, sizeof(flags), 1, f) != 1) {
            fclose(f);
            return nullptr;
        }

        bool isEncrypted = (flags & 0x2) != 0;

        if (isEncrypted) {
            // Seek back, use encrypted handler (expects offset 0)
            fseek(f, 0, SEEK_SET);
            uint32_t fullFlags;
            if (fread(&fullFlags, sizeof(fullFlags), 1, f) != 1) {
                fclose(f);
                return nullptr;
            }
            MixFile* mix = OpenEncryptedMix(f, fullFlags, filename);
            if (!mix) {
                fclose(f);
                return nullptr;
            }
            return mix;
        }

        // Not encrypted - header starts at offset 4 (after the flags word)
        headerOffset = 4;
    }

    // Seek to header position
    fseek(f, headerOffset, SEEK_SET);

    // Unencrypted MIX file
    MixFile* mix = new MixFile;
    memset(mix, 0, sizeof(MixFile));
    mix->file = f;
    mix->encrypted = false;
    strncpy(mix->filename, filename, sizeof(mix->filename) - 1);

    // Read header
    if (fread(&mix->header, sizeof(MixHeader), 1, f) != 1) {
        delete mix;
        fclose(f);
        return nullptr;
    }

    // Sanity check
    if (mix->header.count < 0 || mix->header.count > 10000) {
        // Likely encrypted or invalid
        delete mix;
        fclose(f);
        return nullptr;
    }

    // Allocate and read index
    mix->entries = new MixEntry[mix->header.count];
    size_t indexSize = mix->header.count * sizeof(MixEntry);

    if (fread(mix->entries, indexSize, 1, f) != 1) {
        delete[] mix->entries;
        delete mix;
        fclose(f);
        return nullptr;
    }

    // Data section starts after header and index
    mix->dataStart = (uint32_t)ftell(f);

    return mix;
}

void Mix_Close(MixFileHandle mix) {
    if (mix) {
        if (mix->file) {
            fclose(mix->file);
        }
        if (mix->isMemory && mix->ownsMemData && mix->memData) {
            delete[] mix->memData;
        }
        if (mix->entries) {
            delete[] mix->entries;
        }
        delete mix;
    }
}

int Mix_GetFileCount(MixFileHandle mix) {
    return mix ? mix->header.count : 0;
}

BOOL Mix_FileExists(MixFileHandle mix, const char* name) {
    return Mix_FileExistsByCRC(mix, Mix_CalculateCRC(name));
}

BOOL Mix_FileExistsByCRC(MixFileHandle mix, uint32_t crc) {
    return FindEntry(mix, crc) != nullptr;
}

uint32_t Mix_GetFileSize(MixFileHandle mix, const char* name) {
    MixEntry* entry = FindEntry(mix, Mix_CalculateCRC(name));
    return entry ? entry->size : 0;
}

uint32_t Mix_ReadFileByCRC(MixFileHandle mix, uint32_t crc,
                           void* buffer, uint32_t bufSize) {
    if (!mix || !buffer) return 0;

    MixEntry* entry = FindEntry(mix, crc);
    if (!entry) return 0;

    uint32_t readSize = (entry->size < bufSize) ? entry->size : bufSize;

    if (mix->isMemory) {
        // Memory-based read
        uint32_t offset = mix->dataStart + entry->offset;
        if (offset + readSize > mix->memSize) {
            return 0;  // Out of bounds
        }
        memcpy(buffer, mix->memData + offset, readSize);
        return readSize;
    } else {
        // File-based read
        fseek(mix->file, mix->dataStart + entry->offset, SEEK_SET);
        size_t bytesRead = fread(buffer, 1, readSize, mix->file);
        return (uint32_t)bytesRead;
    }
}

uint32_t Mix_ReadFile(MixFileHandle mix, const char* name,
                      void* buffer, uint32_t bufSize) {
    return Mix_ReadFileByCRC(mix, Mix_CalculateCRC(name), buffer, bufSize);
}

void* Mix_AllocReadFile(MixFileHandle mix, const char* name,
                        uint32_t* outSize) {
    if (!mix) return nullptr;

    MixEntry* entry = FindEntry(mix, Mix_CalculateCRC(name));
    if (!entry) return nullptr;

    void* buffer = malloc(entry->size);
    if (!buffer) return nullptr;

    uint32_t bytesRead = Mix_ReadFile(mix, name, buffer, entry->size);
    if (bytesRead != entry->size) {
        free(buffer);
        return nullptr;
    }

    if (outSize) *outSize = entry->size;
    return buffer;
}

BOOL Mix_GetEntryByIndex(MixFileHandle mix, int index,
                         uint32_t* outCRC, uint32_t* outSize) {
    if (!mix || !mix->entries) return FALSE;
    if (index < 0 || index >= mix->header.count) return FALSE;

    if (outCRC) *outCRC = mix->entries[index].crc;
    if (outSize) *outSize = mix->entries[index].size;
    return TRUE;
}

void* Mix_AllocReadFileByCRC(MixFileHandle mix, uint32_t crc,
                             uint32_t* outSize) {
    if (!mix) return nullptr;

    MixEntry* entry = FindEntry(mix, crc);
    if (!entry) return nullptr;

    void* buffer = malloc(entry->size);
    if (!buffer) return nullptr;

    uint32_t bytesRead = Mix_ReadFileByCRC(mix, crc, buffer, entry->size);
    if (bytesRead != entry->size) {
        free(buffer);
        return nullptr;
    }

    if (outSize) *outSize = entry->size;
    return buffer;
}
