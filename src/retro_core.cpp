#include "retro_core.h"
#include "libretro.h"
#include <dlfcn.h>
#include <cstring>

std::vector<int16_t> g_pending_audio;

namespace{
    RetroCore* g_active_core = nullptr;
}

RetroCore::Retrocore() {
    frame_.format = RETRO_PIXEL_FORMAT_RGB565;
}

RetroCore::~RetroCore() {
    if (game_loaded && fn_unload_game_) fn_unload_game_();
    if (handle_) {
        if (fn_deinit_) fn_deinit();
        dlclose(handle_);
    }
    if (g_active_core == this) g_active_core = nullptr;
}
