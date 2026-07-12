#include "retro_core.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <cstdio>

struct KeyMap { SDL_Scancode key; JoypadButton button; };

static const KeyMap kKeyMap[] = {
    { SDL_SCANCODE_UP, JOY_UP },
    { SDL_SCANCODE_DOWN, JOY_DOWN },
    { SDL_SCANCODE_LEFT, JOY_LEFT },
    { SDL_SCANCODE_RIGHT, JOY_RIGHT },
    { SDL_SCANCODE_RETURN, JOY_A }, // FIRE center key
    { SDL_SCANCODE_Q, JOY_L },
    { SDL_SCANCODE_W, JOY_R },
    { SDL_SCANCODE_ESCAPE, JOY_START },

};

static Uint32 toSdlPixelFormat(retro_pixel_format fmt) {
    switch (fmt) {
        case RETRO_PIXEL_FORMAT_XRGB8888: return SDL_PIXELFORMAT_ARGB8888;
        case RETRO_PIXEL_FORMAT_RGB565: return SDL_PIXELFORMAT_RGB565;
        default: return SDL_PIXELFORMAT_RGB565;
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: %s <core.so> <system_dir> <game.jar>\n", argv[0]);
        return 1;
    }
    const std::string core_path = argv[1];
    const std::string system_dir = argv[2];
    const std::string game_path = argv[3];

    RetroCore core;

    if (!core.load(core_path, system_dir)) return 1;
    if (!core.loadGame(game_path)) return 1;

    const auto& geom = core.avInfo().geometry;
    const double fps = core.avInfo().timing.fps > 0 ? core.avInfo().timing.fps : 60.0;
    const int sample_rate = static_cast<int>(core.avInfo().timing.sample_rate > 0 ? core.avInfo().timing.sample_rate : 44100);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const int scale = 3;

    SDL_Window* window = SDL_CreateWindow(
            "J2ME UI",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            geom.base_width * scale, geom.base_height * scale,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
            );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, geom.base_width, geom.base_height);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_Texture* texture = SDL_CreateTexture(
            renderer, toSdlPixelFormat(core.lastFrame().format),
            SDL_TEXTUREACCESS_STREAMING, geom.max_width ? geom.max_width : geom.base_width,
            geom.max_height ? geom.max_height : geom.base_height
            );

    SDL_AudioSpec want{}, have{};
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.channels = 1024;

    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audio_dev) SDL_PauseAudioDevice(audio_dev, 0);

    const Uint32 frame_delay_ms = static_cast<Uint32>(1000.0 / fps);
    bool running = true;

    while (running) {

        Uint32 frame_start = SDL_GetTicks();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false; 
            if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
                bool pressed = (ev.type == SDL_KEYDOWN);
                for (const auto& km : kKeyMap) {
                    if (ev.key.keysym.scancode == km.key) core.button[km.button] = pressed;
                }
            }
        }

        core.runFrame();

        const RetroFrame& f = core.lastFrame();
        if (f.pixels) {
            SDL_UpdateTexture(texture, nullptr, f.pixels, static_cast<int>(f.pitch));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        if (audio_dev && !g_pending_audio.empty()) {
            SDL_QueueAudio(audio_dev, g_pending_audio.data(),
                    static_cast<Uint32>(g_pending_audio.size() * sizeof(int16_t)));
            g_pending_audio.clear();
        }

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < frame_delay_ms) SDL_Delay(frame_delay_ms - elapsed);
    }

    if (audio_dev) SDL_CloseAudioDevice(audio_dev);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;    

}
