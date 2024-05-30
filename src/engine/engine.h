#pragma once

// Project includes
#include "src/core/game_object.h"
#include "src/vulkan/descriptors.h"

// Standard includes
#include <functional>
#include <memory>

namespace dae
{
    // Forward declarations
    class window;
    class device;
    class renderer;
    
    class engine final
    {
    public:
        static constexpr int width = 800;
        static constexpr int height = 600;
        static constexpr float max_frame_time = 60.0f;

        engine();
        ~engine() = default;

        engine(engine const &other)            = delete;
        engine(engine &&other)                 = delete;
        engine &operator=(engine const &other) = delete;
        engine &operator=(engine &&other)      = delete;

        void run(std::function<void()> const &load);

    private:
        window   *window_ptr_   = nullptr;
        device   *device_ptr_   = nullptr;
        renderer *renderer_ptr_ = nullptr;
        
        std::unique_ptr<descriptor_pool> global_pool_{};
    };
}
