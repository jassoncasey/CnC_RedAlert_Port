/**
 * Test VQA audio extraction - decodes audio from a VQA file and outputs WAV
 */
#include "video/vqa.h"
#include "assets/assetloader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Write WAV file
bool WriteWAV(const char* path, const int16_t* samples, uint32_t sampleCount,
              uint32_t sampleRate, uint8_t channels) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    uint32_t dataSize = sampleCount * sizeof(int16_t) * channels;
    uint32_t fileSize = 36 + dataSize;
    uint16_t blockAlign = channels * 2;
    uint32_t byteRate = sampleRate * blockAlign;

    // RIFF header
    fwrite("RIFF", 4, 1, f);
    fwrite(&fileSize, 4, 1, f);
    fwrite("WAVE", 4, 1, f);

    // fmt chunk
    fwrite("fmt ", 4, 1, f);
    uint32_t fmtSize = 16;
    fwrite(&fmtSize, 4, 1, f);
    uint16_t audioFormat = 1;  // PCM
    fwrite(&audioFormat, 2, 1, f);
    uint16_t numChannels = channels;
    fwrite(&numChannels, 2, 1, f);
    fwrite(&sampleRate, 4, 1, f);
    fwrite(&byteRate, 4, 1, f);
    fwrite(&blockAlign, 2, 1, f);
    uint16_t bitsPerSample = 16;
    fwrite(&bitsPerSample, 2, 1, f);

    // data chunk
    fwrite("data", 4, 1, f);
    fwrite(&dataSize, 4, 1, f);
    fwrite(samples, sizeof(int16_t), sampleCount * channels, f);

    fclose(f);
    return true;
}

int main(int argc, char* argv[]) {
    const char* vqaName = argc > 1 ? argv[1] : "PROLOG.VQA";
    const char* outputPath = argc > 2 ? argv[2] : "/tmp/vqa_audio.wav";

    printf("Initializing asset loader...\n");
    if (!Assets_Init()) {
        printf("ERROR: Failed to initialize asset loader\n");
        return 1;
    }

    printf("Loading VQA: %s\n", vqaName);
    uint32_t dataSize = 0;
    void* data = Assets_LoadVQA(vqaName, &dataSize);

    if (!data || dataSize == 0) {
        printf("ERROR: Failed to load VQA file\n");
        Assets_Shutdown();
        return 1;
    }

    printf("VQA file size: %u bytes\n", dataSize);

    VQAPlayer player;
    if (!player.Load((uint8_t*)data, dataSize)) {
        printf("ERROR: Failed to parse VQA file\n");
        free(data);
        Assets_Shutdown();
        return 1;
    }

    printf("VQA loaded: %dx%d, %d frames, %d fps\n",
           player.GetWidth(), player.GetHeight(),
           player.GetFrameCount(), player.GetFPS());

    if (!player.HasAudio()) {
        printf("ERROR: VQA has no audio\n");
        free(data);
        Assets_Shutdown();
        return 1;
    }

    printf("Audio: %d Hz, %d channels, %d bits\n",
           player.GetAudioSampleRate(),
           player.GetAudioChannels(),
           player.GetAudioBitsPerSample());

    // Collect all audio
    std::vector<int16_t> allAudio;
    int16_t tempBuffer[16384];

    player.Play();

    int frameCount = player.GetFrameCount();
    for (int i = 0; i < frameCount; i++) {
        if (!player.NextFrame()) {
            printf("ERROR: Failed to decode frame %d\n", i);
            break;
        }

        int samples = player.GetAudioSamples(tempBuffer, 16384);
        if (samples > 0) {
            allAudio.insert(allAudio.end(), tempBuffer, tempBuffer + samples);
        }

        if (i % 100 == 0) {
            printf("Frame %d/%d, total samples: %zu\n",
                   i, frameCount, allAudio.size());
        }
    }

    printf("Total audio samples: %zu\n", allAudio.size());

    if (allAudio.empty()) {
        printf("ERROR: No audio samples extracted\n");
        free(data);
        Assets_Shutdown();
        return 1;
    }

    // Write WAV
    printf("Writing: %s\n", outputPath);
    if (WriteWAV(outputPath, allAudio.data(), allAudio.size(),
                 player.GetAudioSampleRate(), 1)) {
        float duration = (float)allAudio.size() / player.GetAudioSampleRate();
        printf("SUCCESS: WAV written (%.2f seconds)\n", duration);
    } else {
        printf("ERROR: Failed to write WAV\n");
    }

    free(data);
    Assets_Shutdown();
    return 0;
}
