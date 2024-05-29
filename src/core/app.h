#pragma once

// Project includes
#include "src/core/game_object.h"
#include "src/core/window.h"
#include "src/engine/descriptors.h"
#include "src/engine/device.h"
#include "src/engine/renderer.h"

// Standard includes
#include <memory>

namespace dae
{
    class app
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;
        static constexpr float max_frame_time = 60.0f;

        app();
        ~app();

        app(app const &)            = delete;
        app &operator=(app const &) = delete;

        void run();

    private:
        void load_game_objects();
        
    private:
        window                      window_{width, height, "Hello Vulkan!"};
        device                      device_{window_};
        renderer                    renderer_{window_, device_};

        std::unique_ptr<descriptor_pool> global_pool_{};
        game_object::map                 game_objects_;
    };
}
