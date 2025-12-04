# wwd-media

Shared media library for Red Alert ports and tools.

## Components

| Component | Description |
|-----------|-------------|
| **Renderer** | Metal-based 8-bit palettized framebuffer (640x400) |
| **Audio** | CoreAudio mixer with 16 simultaneous channels |
| **VQA** | Westwood VQA video decoder (IMA ADPCM audio) |

## Building

```bash
make
```

Produces `build/libwwd-media.a` static library.

## API

Headers are in `include/ra/`:

| Header | Purpose |
|--------|---------|
| `types.h` | Common types (WwdBool, WwdPalette, WwdAudioSample) |
| `renderer.h` | Metal renderer API (Wwd_Renderer_*) |
| `audio.h` | CoreAudio playback API (Wwd_Audio_*) |
| `vqa.h` | VQA decoder class and C interface |

## Usage

Include the appropriate header and link against `libwwd-media.a`:

```cpp
#include <wwd/renderer.h>
#include <wwd/audio.h>
#include <wwd/vqa.h>

// Initialize
Wwd_Renderer_Init(metalView);
Wwd_Audio_Init();

// Use
WwdPalette palette;
Wwd_Renderer_SetPalette(&palette);
Wwd_Renderer_Present();

WwdAudioSample sample;
Wwd_Audio_Play(&sample, 255, 0, WWD_FALSE);
```

## Dependencies

- macOS 14+ (Sonoma)
- ARM64 (Apple Silicon)
- Metal framework
- CoreAudio framework

## License

GPL v3 - Part of the Red Alert macOS port.
