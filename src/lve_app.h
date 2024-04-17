#pragma once

// Project includes
#include "lve_window.h"
#include "lve_device.h"
#include "lve_renderer.h"
#include "lve_game_object.h"
#include "lve_descriptors.h"

// Standard includes
#include <memory>
#include <vector>

namespace dae
{
    class lve_app
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;
        static constexpr float max_frame_time = 60.0f;

        lve_app();
        ~lve_app();

        lve_app(lve_app const &)            = delete;
        lve_app &operator=(lve_app const &) = delete;

        void run();

    private:
        void load_game_objects();
        
    private:
        lve_window                      window_{width, height, "Hello Vulkan!"};
        lve_device                      device_{window_};
        lve_renderer                    renderer_{window_, device_};

        std::unique_ptr<lve_descriptor_pool> global_pool_{};
        lve_game_object::map                 game_objects_;
    };
}
