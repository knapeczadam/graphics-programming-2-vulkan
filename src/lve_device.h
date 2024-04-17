#pragma once

#include "lve_window.h"

// std lib headers
#include <string>
#include <vector>

namespace dae
{
    struct swap_chain_support_details
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
    };

    struct queue_family_indices
    {
        uint32_t graphics_family;
        uint32_t present_family;
        bool     graphics_family_has_value = false;
        bool     present_family_has_value  = false;
        
        [[nodiscard]] auto is_complete() const -> bool { return graphics_family_has_value and present_family_has_value; }
    };

    class lve_device
    {
    public:
#ifdef NDEBUG
        const bool enable_validation_layers = false;
#else
        const bool enable_validation_layers = true;
#endif

        lve_device(lve_window &window);
        ~lve_device();

        // Not copyable or movable
        lve_device(const lve_device &)       = delete;
        void operator=(const lve_device &)   = delete;
        lve_device(lve_device &&)            = delete;
        lve_device &operator=(lve_device &&) = delete;

        [[nodiscard]] auto get_command_pool() const -> VkCommandPool { return command_pool_; }
        [[nodiscard]] auto device() const -> VkDevice { return device_; }
        [[nodiscard]] auto surface() const -> VkSurfaceKHR { return surface_; }
        [[nodiscard]] auto graphics_queue() const -> VkQueue { return graphics_queue_; }
        [[nodiscard]] auto present_queue() const -> VkQueue { return present_queue_; }

        auto get_swap_chain_support() -> swap_chain_support_details { return query_swap_chain_support(physical_device_); }
        auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) -> uint32_t;
        auto find_physical_queue_families() -> queue_family_indices { return find_queue_families(physical_device_); }
        auto find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;

        // Buffer Helper Functions
        void create_buffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &buffer_memory);
        auto begin_single_time_commands() -> VkCommandBuffer;
        void end_single_time_commands(VkCommandBuffer command_buffer);
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
        auto find_queue_families(VkPhysicalDevice device) -> queue_family_indices;
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void has_gflw_required_instance_extensions();
        auto check_device_extension_support(VkPhysicalDevice device) -> bool;
        auto query_swap_chain_support(VkPhysicalDevice device) -> swap_chain_support_details;

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
}
