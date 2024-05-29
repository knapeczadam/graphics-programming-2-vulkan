#pragma once

// Standard includes
#include <string>

// Vulkan includes
#include <vulkan/vulkan.h>

namespace dae
{
    // Forward declarations
    class device;
    
    class texture
    {
    public:
        texture(device *device_ptr, std::string const &file_path, VkFormat format);
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
        device         *device_ptr_;
        VkImage        image_        = VK_NULL_HANDLE;
        VkDeviceMemory image_memory_ = VK_NULL_HANDLE;
        VkImageView    image_view_   = VK_NULL_HANDLE;
        VkSampler      sampler_      = VK_NULL_HANDLE;
        VkFormat       image_format_ = VK_FORMAT_UNDEFINED;
        VkImageLayout  image_layout_ = VK_IMAGE_LAYOUT_UNDEFINED;

        int width_      = 0;
        int height_     = 0;
        int mip_levels_ = 0;
    };
}
