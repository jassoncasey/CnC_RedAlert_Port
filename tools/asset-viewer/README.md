# Asset Viewer

Visual inspection tool for Westwood game assets.

## Features

- Native macOS folder picker
- Recursive directory scanning
- Recursive MIX file scanning (MIX within MIX)
- Filename database for CRC-to-name mapping
- Tree view of discovered assets
- Sprite/animation preview with Metal renderer
- Audio playback
- VQA video playback
- Palette inspection

## Supported Formats

| Format | Description |
|--------|-------------|
| MIX | Archive container (encrypted/unencrypted) |
| SHP | Sprite sheets (LCW compressed) |
| PAL | 256-color palettes (6-bit VGA) |
| AUD | Audio (IMA ADPCM, uncompressed) |
| VQA | Video (vector quantized animation) |
| TMP | Terrain templates |
| INI | Configuration files |

## Building

```bash
make
./build/asset-viewer
```

## Dependencies

- ra-media (../libs/ra-media)
- libwestwood (via submodules/westwood-formats)
- macOS 14+ (Sonoma)
- ARM64 (Apple Silicon)

## Usage

1. Launch the viewer
2. Click "Select Directory" to choose asset location
3. Browse the tree view to explore assets
4. Select an asset to preview it
5. Use Tab 2 for systematic asset review

## Architecture

The viewer uses ra-media for rendering and audio playback, and the
game's asset loaders for file format parsing. Future versions may
migrate to libwestwood for format decoding.

## License

GPL v3 - Part of the Red Alert macOS port.
