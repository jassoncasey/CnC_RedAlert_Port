/**
 * Test TMP terrain tile parsing via libwestwood
 */

#include <cstdio>
#include <cstdint>
#include <westwood/tmp.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <tmp_file>\n", argv[0]);
        return 1;
    }

    auto result = wwd::TmpReader::open(argv[1]);
    if (!result) {
        printf("FAIL: Could not open TMP file: %s\n", argv[1]);
        return 1;
    }

    auto& reader = *result;
    const auto& info = reader->info();
    const auto& tiles = reader->tiles();

    printf("TMP File: %s\n", argv[1]);
    printf("  Format: %s\n",
           info.format == wwd::TmpFormat::RA ? "RA" :
           info.format == wwd::TmpFormat::TD ? "TD" :
           info.format == wwd::TmpFormat::TS ? "TS" : "RA2");
    printf("  Tile size: %dx%d\n", info.tile_width, info.tile_height);
    printf("  Tile count: %d\n", info.tile_count);
    printf("  Empty tiles: %d\n", info.empty_count);
    printf("  Valid tiles: %d\n", info.tile_count - info.empty_count);
    printf("  Index range: 0x%04X - 0x%04X\n",
           info.index_start, info.index_end);
    printf("  Image start: 0x%04X\n", info.image_start);

    // Expected tile size for RA: 24x24 = 576 bytes
    uint32_t expected_size = info.tile_width * info.tile_height;
    printf("  Expected tile size: %u bytes\n", expected_size);

    // Validate tiles
    int valid_count = 0;
    int decode_ok = 0;
    int decode_fail = 0;

    for (size_t i = 0; i < tiles.size(); i++) {
        const auto& tile = tiles[i];
        if (tile.valid) {
            valid_count++;
            auto data = reader->decode_tile(i);
            if (data.size() == expected_size) {
                decode_ok++;
            } else {
                decode_fail++;
                printf("  Tile %zu: expected %u bytes, got %zu\n",
                       i, expected_size, data.size());
            }
        }
    }

    printf("\nResults:\n");
    printf("  Valid tiles found: %d\n", valid_count);
    printf("  Tiles decoded OK: %d\n", decode_ok);
    printf("  Tiles decode failed: %d\n", decode_fail);

    if (decode_fail == 0 && decode_ok > 0) {
        printf("\nPASS: All tiles decoded correctly\n");
        return 0;
    } else if (decode_ok == 0) {
        printf("\nFAIL: No tiles decoded\n");
        return 1;
    } else {
        printf("\nFAIL: Some tiles failed to decode\n");
        return 1;
    }
}
