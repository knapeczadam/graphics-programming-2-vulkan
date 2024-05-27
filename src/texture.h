#pragma once

#include "device.h"

// Project includes
#include "device.h"

// Standard includes
#include <string>

// Vulkan includes
#include <vulkan/vulkan_core.h>

namespace dae
{
    class texture
    {
    public:
        texture(device &device, std::string const &file_path);
        ~texture();

        texture(texture const &)            = delete;
        texture(texture &&)                 = delete;
        texture &operator=(texture const &) = delete;
        texture &operator=(texture &&)      = delete;

        [[nodiscard]] auto sampler() const -> VkSampler { return sampler_; }
        [[nodiscard]] auto image_view() const -> VkImageView { return image_view_; }
        [[nodiscard]] auto image_layout() const -> VkImageLayout { return image_layout_; }

    private:
        void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
        void generate_mipmaps();

    private:
        device         &device_;
        VkImage        image_;
        VkDeviceMemory image_memory_;
        VkImageView    image_view_;
        VkSampler      sampler_;
        VkFormat       image_format_;
        VkImageLayout  image_layout_;

        int width_;
        int height_;
        int mip_levels_;
    };
}
