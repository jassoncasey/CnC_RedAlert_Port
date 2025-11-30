/**
 * Red Alert macOS Port - MIX File Reader Implementation
 *
 * MIX file format (unencrypted, as used in RA):
 *   Header:
 *     short count     - Number of files in archive
 *     long size       - Total size of data section
 *   Index (count entries):
 *     long crc        - CRC hash of filename
 *     long offset     - Offset from start of data section
 *     long size       - Size of file
 *   Data section:
 *     Raw file data
 */

#include "mixfile.h"
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
    FILE* file;
    MixHeader header;
    MixEntry* entries;
    uint32_t dataStart;     // Offset to start of data section
    char filename[256];
};

// Westwood CRC table (generated from polynomial)
static uint32_t g_crcTable[256];
static bool g_crcTableInit = false;

static void InitCRCTable(void) {
    if (g_crcTableInit) return;

    for (int i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        g_crcTable[i] = crc;
    }
    g_crcTableInit = true;
}

uint32_t Mix_CalculateCRC(const char* name) {
    InitCRCTable();

    // Westwood uses upper-case for CRC calculation
    uint32_t crc = 0;
    int len = 0;

    // Calculate CRC on uppercase filename
    while (name[len]) {
        char c = (char)toupper(name[len]);
        uint32_t index = ((crc >> 24) ^ c) & 0xFF;
        crc = (crc << 8) ^ g_crcTable[index];
        len++;
    }

    // Pad with zeros to make length multiple of 4
    while (len % 4 != 0) {
        uint32_t index = (crc >> 24) & 0xFF;
        crc = (crc << 8) ^ g_crcTable[index];
        len++;
    }

    return crc;
}

// Binary search for entry by CRC
static MixEntry* FindEntry(MixFile* mix, uint32_t crc) {
    if (!mix || !mix->entries) return nullptr;

    // Entries are sorted by CRC for binary search
    int lo = 0;
    int hi = mix->header.count - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (mix->entries[mid].crc == crc) {
            return &mix->entries[mid];
        } else if (mix->entries[mid].crc < crc) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    return nullptr;
}

MixFileHandle Mix_Open(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return nullptr;
    }

    // Check for encrypted header (first 2 bytes would be large)
    uint16_t firstWord;
    if (fread(&firstWord, sizeof(firstWord), 1, f) != 1) {
        fclose(f);
        return nullptr;
    }
    fseek(f, 0, SEEK_SET);

    // For now, only support unencrypted MIX files
    // Encrypted files have a different header structure
    if (firstWord == 0) {
        // Potentially new-style encrypted MIX, skip for now
        fclose(f);
        return nullptr;
    }

    MixFile* mix = new MixFile;
    memset(mix, 0, sizeof(MixFile));
    mix->file = f;
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
    mix->dataStart = sizeof(MixHeader) + (uint32_t)indexSize;

    return mix;
}

void Mix_Close(MixFileHandle mix) {
    if (mix) {
        if (mix->file) {
            fclose(mix->file);
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

uint32_t Mix_ReadFileByCRC(MixFileHandle mix, uint32_t crc, void* buffer, uint32_t bufSize) {
    if (!mix || !buffer) return 0;

    MixEntry* entry = FindEntry(mix, crc);
    if (!entry) return 0;

    uint32_t readSize = (entry->size < bufSize) ? entry->size : bufSize;

    // Seek to file position
    fseek(mix->file, mix->dataStart + entry->offset, SEEK_SET);

    // Read data
    size_t bytesRead = fread(buffer, 1, readSize, mix->file);
    return (uint32_t)bytesRead;
}

uint32_t Mix_ReadFile(MixFileHandle mix, const char* name, void* buffer, uint32_t bufSize) {
    return Mix_ReadFileByCRC(mix, Mix_CalculateCRC(name), buffer, bufSize);
}

void* Mix_AllocReadFile(MixFileHandle mix, const char* name, uint32_t* outSize) {
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
