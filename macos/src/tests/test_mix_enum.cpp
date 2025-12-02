/**
 * Test: Enumerate files in MIX by dumping all entries
 *
 * Since MIX files store entries by CRC only, we need to either:
 * 1. Brute force known filenames
 * 2. Access the raw entry list and dump by CRC
 *
 * This tool adds a function to enumerate all entries.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

// MIX file structures (duplicated from mixfile.cpp for direct access)
#pragma pack(push, 1)
struct MixHeader {
    int16_t count;
    int32_t dataSize;
};

struct MixEntry {
    uint32_t crc;
    uint32_t offset;
    uint32_t size;
};
#pragma pack(pop)

// Westwood hash function
uint32_t hashFilename(const char* name) {
    int len = 0;
    while (name[len]) len++;

    int padding = (len % 4 != 0) ? (4 - len % 4) : 0;
    int paddedLen = len + padding;

    uint8_t buffer[256];
    for (int i = 0; i < len; i++) {
        buffer[i] = (uint8_t)toupper((unsigned char)name[i]);
    }
    for (int i = len; i < paddedLen; i++) {
        buffer[i] = 0;
    }

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

// Known filenames to match against CRCs
static const char* knownFiles[] = {
    // Core files
    "RULES.INI", "REDALERT.INI", "AI.INI", "ART.INI", "SOUND.INI",
    "TUTORIAL.INI", "GAME.DAT",
    // Allied missions
    "SCG01EA.INI", "SCG02EA.INI", "SCG03EA.INI", "SCG04EA.INI",
    "SCG05EA.INI", "SCG06EA.INI", "SCG07EA.INI", "SCG08EA.INI",
    "SCG09EA.INI", "SCG10EA.INI", "SCG11EA.INI", "SCG12EA.INI",
    "SCG13EA.INI", "SCG14EA.INI",
    // Soviet missions
    "SCU01EA.INI", "SCU02EA.INI", "SCU03EA.INI", "SCU04EA.INI",
    "SCU05EA.INI", "SCU06EA.INI", "SCU07EA.INI", "SCU08EA.INI",
    "SCU09EA.INI", "SCU10EA.INI", "SCU11EA.INI", "SCU12EA.INI",
    "SCU13EA.INI", "SCU14EA.INI",
    // MIX file references (nested archives)
    "GENERAL.MIX", "CONQUER.MIX", "LOCAL.MIX", "HIRES.MIX", "LORES.MIX",
    "SPEECH.MIX", "SOUNDS.MIX", "MOVIES.MIX", "ALLIES.MIX", "SOVIET.MIX",
    "SNOW.MIX", "TEMPERAT.MIX", "DESERT.MIX", "INTERIOR.MIX",
    "MAIN.MIX", "INSTALL.MIX", "SETUP.MIX", "SCORES.MIX",
    // Other possible files
    "CONQUER.ENG", "THEME.INI", "MISSION.INI",
    "DATA.MIX", "BRIEFING.MIX", "CAMPAIGN.MIX",
    nullptr
};

const char* lookupCRC(uint32_t crc) {
    for (int i = 0; knownFiles[i] != nullptr; i++) {
        if (hashFilename(knownFiles[i]) == crc) {
            return knownFiles[i];
        }
    }
    return nullptr;
}

void dumpMixContents(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("ERROR: Could not open %s\n", path);
        return;
    }

    // Read first 4 bytes to check format
    uint32_t firstWord;
    if (fread(&firstWord, 4, 1, f) != 1) {
        printf("ERROR: Could not read header\n");
        fclose(f);
        return;
    }

    // Check if encrypted (flags in first 4 bytes)
    bool encrypted = false;
    long headerOffset = 0;

    uint16_t firstWord16 = firstWord & 0xFFFF;
    if (firstWord16 == 0) {
        // RA format - check flags
        uint16_t flags = (firstWord >> 16) & 0xFFFF;
        encrypted = (flags & 0x2) != 0;
        headerOffset = 4;
        printf("Format: Red Alert (flags=0x%04x, encrypted=%d)\n", flags, encrypted);
    } else {
        // Classic C&C format
        printf("Format: Classic C&C\n");
    }

    if (encrypted) {
        printf("ERROR: Encrypted MIX files require full decryption (not implemented in this tool)\n");
        fclose(f);
        return;
    }

    // Seek to header
    fseek(f, headerOffset, SEEK_SET);

    // Read header
    MixHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) {
        printf("ERROR: Could not read MIX header\n");
        fclose(f);
        return;
    }

    printf("File count: %d\n", header.count);
    printf("Data size: %d bytes\n", header.dataSize);

    if (header.count < 0 || header.count > 10000) {
        printf("ERROR: Invalid file count (probably encrypted)\n");
        fclose(f);
        return;
    }

    // Read entries
    MixEntry* entries = new MixEntry[header.count];
    if (fread(entries, sizeof(MixEntry), header.count, f) != (size_t)header.count) {
        printf("ERROR: Could not read entries\n");
        delete[] entries;
        fclose(f);
        return;
    }

    printf("\n--- Entries ---\n");
    printf("%5s  %10s  %10s  %10s  %s\n", "Index", "CRC", "Offset", "Size", "Filename");
    printf("%s\n", "--------------------------------------------------------------");

    for (int i = 0; i < header.count; i++) {
        const char* name = lookupCRC(entries[i].crc);
        printf("%5d  0x%08x  %10u  %10u  %s\n",
               i,
               entries[i].crc,
               entries[i].offset,
               entries[i].size,
               name ? name : "(unknown)");
    }

    // Try to extract any known files
    printf("\n--- Extracting known files ---\n");
    long dataStart = ftell(f);  // Current position is start of data

    for (int i = 0; i < header.count; i++) {
        const char* name = lookupCRC(entries[i].crc);
        if (name) {
            // Extract this file
            char outPath[512];
            snprintf(outPath, sizeof(outPath), "/tmp/ra_extract/%s", name);

            fseek(f, dataStart + entries[i].offset, SEEK_SET);
            void* data = malloc(entries[i].size);
            if (fread(data, 1, entries[i].size, f) == entries[i].size) {
                system("mkdir -p /tmp/ra_extract");
                FILE* out = fopen(outPath, "wb");
                if (out) {
                    fwrite(data, 1, entries[i].size, out);
                    fclose(out);
                    printf("Extracted: %s (%u bytes)\n", name, entries[i].size);
                }
            }
            free(data);
        }
    }

    delete[] entries;
    fclose(f);
}

int main(int argc, char** argv) {
    const char* mixPath = argc > 1 ? argv[1] : "/Volumes/CD1/INSTALL/REDALERT.MIX";

    printf("=== MIX File Enumeration ===\n");
    printf("Path: %s\n\n", mixPath);

    dumpMixContents(mixPath);

    // Also try MAIN.MIX which often contains the actual game data
    printf("\n\n=== Trying MAIN.MIX ===\n");
    dumpMixContents("/Volumes/CD1/INSTALL/MAIN.MIX");

    printf("\n=== Trying loose files in INSTALL ===\n");
    system("ls -la /Volumes/CD1/INSTALL/*.INI 2>/dev/null || echo 'No INI files in INSTALL'");
    system("ls -la /Volumes/CD1/INSTALL/*.MIX 2>/dev/null | head -20");

    return 0;
}
