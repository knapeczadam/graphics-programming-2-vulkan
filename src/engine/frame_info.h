#pragma once

// Project includes
#include "src/core/game_object.h"
#include "src/engine/camera.h"

// Vulkan includes
#include <vulkan/vulkan.h>

#include "src/utility/singleton.h"

namespace dae
{
    constexpr int MAX_LIGHTS = 10;
    
    struct point_light
    {
        glm::vec4 position {}; // ignore w
        glm::vec4 color    {}; // w is intensity
    };
    
    struct global_ubo
    {
        glm::mat4 projection          {1.0f};
        glm::mat4 view                {1.0f};
        glm::mat4 inverse_view        {1.0f};
        glm::vec4 ambient_light_color {1.0f, 1.0f, 1.0f, 0.02f};
        point_light point_lights[MAX_LIGHTS];
        int num_lights;
        bool use_normal;
        int shading_mode;
    };
    
    class frame_info final : public singleton<frame_info>
    {
    public:
        ~frame_info() override = default;

        frame_info(frame_info const &other)            = delete;
        frame_info(frame_info &&other)                 = delete;
        frame_info &operator=(frame_info const &other) = delete;
        frame_info &operator=(frame_info &&other)      = delete;
        
        int                       frame_index;
        VkCommandBuffer           command_buffer;
        camera                    *camera_ptr;
        VkDescriptorSet           global_descriptor_set;
        std::vector<game_object*> game_objects;
        global_ubo                *ubo_ptr;
        
    private:
        friend class singleton<frame_info>;
        frame_info() = default;
    };
}
