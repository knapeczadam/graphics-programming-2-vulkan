#pragma once

// Project includes
#include "lve_camera.h"
#include "lve_game_object.h"

// Vulkan includes
#include <vulkan/vulkan.h>


namespace lve
{
    constexpr int MAX_LIGHTS = 10;
    
    struct point_light
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };
    
    struct global_ubo
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::vec4 ambient_light_color{1.0f, 1.0f, 1.0f, 0.02f};
        point_light point_lights[MAX_LIGHTS];
        int num_lights;
    };
    
    struct frame_info
    {
        int                  frame_index;
        float                frame_time;
        VkCommandBuffer      command_buffer;
        lve_camera           &camera;
        VkDescriptorSet      global_descriptor_set;
        lve_game_object::map &game_objects;
    };
}
