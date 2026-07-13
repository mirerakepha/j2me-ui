#include "retro_core.h"
#include "libretro.h"
#include <dlfcn.h>
#include <cstring>


std::vector<int16_t> g_pending_audio;

namespace{
    RetroCore* g_active_core = nullptr;
     bool noopRumble(unsigned, retro_rumble_effect, uint16_t) { return true; }
}

RetroCore::RetroCore() {
    frame_.format = RETRO_PIXEL_FORMAT_RGB565;
}

RetroCore::~RetroCore() {
    if (game_loaded_ && fn_unload_game_) fn_unload_game_();
    if (handle_) {
        if (fn_deinit_) fn_deinit_();
        dlclose(handle_);
    }
    if (g_active_core == this) g_active_core = nullptr;
}

bool RetroCore::load(const std::string& core_path, const std::string& system_dir, const std::string& resolution, const std::string& rotate) {
    system_dir_ = system_dir;
    resolution_ = resolution;
    rotate_ = rotate;

    handle_ = dlopen(core_path.c_str(), RTLD_LAZY | RTLD_LOCAL);

    if (!handle_) {
        std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return false;
    }

    #define RESOLVE(member, symbol) \
        member = reinterpret_cast<decltype(member)>(dlsym(handle_, symbol)); \
        if (!member) { std::fprintf(stderr, "missing symbol: %s\n", symbol); return false; }    

    RESOLVE(fn_init_, "retro_init")
    RESOLVE(fn_deinit_, "retro_deinit")
    RESOLVE(fn_get_av_info_, "retro_get_system_av_info")
    RESOLVE(fn_run_, "retro_run")
    RESOLVE(fn_load_game_, "retro_load_game")
    RESOLVE(fn_unload_game_, "retro_unload_game")
    RESOLVE(fn_set_environment_, "retro_set_environment")
    RESOLVE(fn_set_video_refresh_, "retro_set_video_refresh")
    RESOLVE(fn_set_audio_sample_, "retro_set_audio_sample")
    RESOLVE(fn_set_audio_sample_batch_, "retro_set_audio_sample_batch")
    RESOLVE(fn_set_input_poll_, "retro_set_input_poll")
    RESOLVE(fn_set_input_state_, "retro_set_input_state")
    #undef RESOLVE

    g_active_core = this;

    fn_set_environment_(&RetroCore::environmentCallback);
    fn_set_video_refresh_(&RetroCore::videoRefreshCallback);
    fn_set_audio_sample_(&RetroCore::audioSampleCallback);
    fn_set_audio_sample_batch_(&RetroCore::audioSampleBatchCallback);
    fn_set_input_poll_(&RetroCore::inputPollCallback);
    fn_set_input_state_(&RetroCore::inputStateCallback);

    fn_init_();
    return true;
}

bool RetroCore::loadGame(const std::string& game_path) {
    retro_game_info info{};
    info.path = game_path.c_str();
    info.data = nullptr;

    info.size = 0;
    info.meta = nullptr;

    if (!fn_load_game_(&info)) {
        std::fprintf(stderr, "retro_game_load failed for %s\n", game_path.c_str());
        return false;
    }
    fn_get_av_info_(&av_info_);
    game_loaded_ = true;

    return true;
}

void RetroCore::runFrame() {
    fn_run_();
}


bool RetroCore::environmentCallback(unsigned cmd, void* data) {
    switch (cmd) {
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
                                                     *reinterpret_cast<const char**>(data) = g_active_core -> system_dir_.c_str();
                                                     return true;
                                                 }
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
                                                   *reinterpret_cast<const char**>(data) = g_active_core->system_dir_.c_str();
                                                   return true;
                                               }

    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
                                                 auto fmt = *reinterpret_cast<const retro_pixel_format*>(data);
                                                 if (fmt == RETRO_PIXEL_FORMAT_XRGB8888 || fmt == RETRO_PIXEL_FORMAT_RGB565) {
                                                     g_active_core -> frame_.format = fmt;
                                                     return true;
                                                 }
                                                 return false;
                                             }
    
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        auto* var = reinterpret_cast<retro_variable*>(data);
        if (var->key && std::strcmp(var->key, "freej2me_resolution") == 0) {
            std::fprintf(stderr, "CORE ASKED FOR RESOLUTION, ANSWERING: %s\n", g_active_core->resolution_.c_str());
            var->value = g_active_core->resolution_.c_str();
            return true;
        }
        if (var->key && std::strcmp(var->key, "freej2me_rotate") == 0) {
            std::fprintf(stderr, "CORE ASKED FOR ROTATE, ANSWERING: %s\n", g_active_core->rotate_.c_str());
            var->value = g_active_core->rotate_.c_str();
            return true;
        }
        return false;
    }

    case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
            // We have no real rumble hardware to drive, but the core
            // calls this pointer completely unconditionally in its main
            // loop's "no rumble active" branch — leaving it null
            // segfaults on literally the first frame. A harmless no-op
            // that just returns true satisfies it safely.
            auto* iface = reinterpret_cast<retro_rumble_interface*>(data);
            iface->set_rumble_state = &noopRumble;
            return true;
    }

    case RETRO_ENVIRONMENT_SET_GEOMETRY: {
            auto* geom = reinterpret_cast<const retro_game_geometry*>(data);
            g_active_core->av_info_.geometry = *geom;
            g_active_core->geometry_dirty_ = true;
            return true;
    }
    
    default:
        return false;
    
    }
}


void RetroCore::videoRefreshCallback(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!data) return;
    auto& f = g_active_core->frame_;
    f.pixels = data;
    f.width = width;
    f.height = height;
    f.pitch = pitch;
}


void RetroCore::audioSampleCallback(int16_t left, int16_t right) {
    g_pending_audio.push_back(left);
    g_pending_audio.push_back(right);
}

size_t RetroCore::audioSampleBatchCallback(const int16_t* data, size_t frames) {
    size_t old_size = g_pending_audio.size();
    g_pending_audio.resize(old_size + frames * 2);
    std::memcpy(g_pending_audio.data() + old_size, data, frames * 2 * sizeof(int16_t));
    return frames;
}

void RetroCore::inputPollCallback() {

}

int16_t RetroCore::inputStateCallback(unsigned port, unsigned device, unsigned /*index*/, unsigned id) {
    if (port != 0 || device != RETRO_DEVICE_JOYPAD) return 0;
    if (id >= JOY_COUNT) return 0;
    return g_active_core->button[id] ? 1 : 0;
}











