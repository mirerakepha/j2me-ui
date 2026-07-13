#pragma once

#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include "libretro.h"

enum JoypadButton {
    JOY_B = 0, JOY_Y = 1, JOY_SELECT = 2, JOY_START = 3,
    JOY_UP = 4, JOY_DOWN = 5, JOY_LEFT = 6, JOY_RIGHT = 7,
    JOY_A = 8, JOY_X = 9, JOY_L = 10, JOY_R = 11,
    JOY_L2 = 12, JOY_R2 = 13, JOY_L3 = 14, JOY_R3 = 15,
    JOY_COUNT = 16
};

struct RetroFrame {
    const void* pixels = nullptr;
    unsigned width = 0;
    unsigned height = 0;
    size_t pitch = 0;
    retro_pixel_format format = RETRO_PIXEL_FORMAT_RGB565;
};

class RetroCore {
    public:
        RetroCore();
        ~RetroCore();

        RetroCore(const RetroCore&) = delete;
        RetroCore& operator=(const RetroCore&) = delete;

        bool load(const std::string& core_path, const std::string& system_dir, const std::string& resolution, const std::string& rotate);
        bool loadGame(const std::string& game_path);

        bool geometryDirty() const { return geometry_dirty_; }
        void clearGeometryDirty() { geometry_dirty_ = false; }

        void runFrame();

        const retro_system_av_info& avInfo() const {return av_info_;}
        const RetroFrame& lastFrame() const { return frame_;}

        bool button[JOY_COUNT] = {};


    private:
        static bool environmentCallback(unsigned cmd, void* data);
        static void videoRefreshCallback(const void* data, unsigned width, unsigned height, size_t pitch);
        static void audioSampleCallback(int16_t left, int16_t right);
        static size_t audioSampleBatchCallback(const int16_t* data, size_t frames);
        static void inputPollCallback();
        static int16_t inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id);

        void* handle_ = nullptr;
        bool game_loaded_ = false;
        bool geometry_dirty_ = false;
        retro_system_av_info av_info_{};
        RetroFrame frame_;
        std::string system_dir_;
        std::string resolution_;
        std::string rotate_;

        retro_init_t                 fn_init_ = nullptr;
        retro_deinit_t               fn_deinit_ = nullptr;
        retro_get_system_av_info_t   fn_get_av_info_ = nullptr;
        retro_run_t                  fn_run_ = nullptr;
        retro_load_game_t            fn_load_game_ = nullptr;
        retro_unload_game_t          fn_unload_game_ = nullptr;
        retro_set_environment_t      fn_set_environment_ = nullptr;
        retro_set_video_refresh_t    fn_set_video_refresh_ = nullptr;
        retro_set_audio_sample_t     fn_set_audio_sample_ = nullptr;
        retro_set_audio_sample_batch_t  fn_set_audio_sample_batch_ = nullptr;
        retro_set_input_poll_t       fn_set_input_poll_ = nullptr;
        retro_set_input_state_t      fn_set_input_state_ = nullptr;
};

extern std::vector<int16_t> g_pending_audio;



