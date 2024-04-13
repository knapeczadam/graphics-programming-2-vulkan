#pragma once

#include "lve_window.h"

// std lib headers
#include <string>
#include <vector>

namespace lve
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        auto isComplete() const -> bool { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class lve_device
    {
    public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
        const bool enable_validation_layers = true;
#endif

        lve_device(lve_window &window);
        ~lve_device();

        // Not copyable or movable
        lve_device(const lve_device &)       = delete;
        void operator=(const lve_device &)          = delete;
        lve_device(lve_device &&)            = delete;
        lve_device &operator=(lve_device &&) = delete;

        auto get_command_pool() -> VkCommandPool { return command_pool_; }
        auto device() -> VkDevice { return device_; }
        auto surface() -> VkSurfaceKHR { return surface_; }
        auto graphics_queue() -> VkQueue { return graphics_queue_; }
        auto present_queue() -> VkQueue { return present_queue_; }

        auto get_swap_chain_support() -> SwapChainSupportDetails { return query_swap_chain_support(physical_device_); }
        auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) -> uint32_t;
        auto find_physical_queue_families() -> QueueFamilyIndices { return find_queue_families(physical_device_); }
        auto find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;

        // Buffer Helper Functions
        void create_buffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &buffer_memory);
        auto begin_single_time_commands() -> VkCommandBuffer;
        void end_single_time_commands(VkCommandBuffer commandBuffer);
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

        void create_image_with_info(
            const VkImageCreateInfo &image_info,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &image_memory);

        VkPhysicalDeviceProperties properties;

    private:
        void create_instance();
        void setup_debug_messenger();
        void create_surface();
        void pick_physical_device();
        void create_logical_device();
        void create_command_pool();

        // helper functions
        auto is_device_suitable(VkPhysicalDevice device) -> bool;
        auto get_required_extensions() -> std::vector<const char*>;
        auto check_validation_layer_support() -> bool;
        auto find_queue_families(VkPhysicalDevice device) -> QueueFamilyIndices;
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void has_gflw_required_instance_extensions();
        auto check_device_extension_support(VkPhysicalDevice device) -> bool;
        auto query_swap_chain_support(VkPhysicalDevice device) -> SwapChainSupportDetails;

        VkInstance instance_;
        VkDebugUtilsMessengerEXT debug_messenger_;
        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
        lve_window &window_;
        VkCommandPool command_pool_;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphics_queue_;
        VkQueue present_queue_;

        const std::vector<const char*> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
} // namespace lve
