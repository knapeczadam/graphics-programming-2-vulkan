#pragma once

// Project includes
#include "lve_camera.h"

// Vulkan includes
#include <vulkan/vulkan.h>

namespace lve
{
    struct frame_info
    {
        int             frame_index;
        float           frame_time;
        VkCommandBuffer command_buffer;
        lve_camera      &camera;
        VkDescriptorSet global_descriptor_set;
    };
}
