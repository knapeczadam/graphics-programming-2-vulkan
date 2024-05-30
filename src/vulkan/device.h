#pragma once

// Project includes
#include "src/utility/singleton.h"

// std lib headers
#include <vector>

// vulkan headers
#include <vulkan/vulkan.h>

namespace dae
{
    // Forward declarations
    class window;
    
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

    class device final : public singleton<device>
    {
    public:
#ifdef NDEBUG
        bool const enable_validation_layers = false;
#else
        bool const enable_validation_layers = true;
#endif

        ~device() override;

        // Not copyable or movable
        device(device const &other)            = delete;
        device(device &&other)                 = delete;
        device &operator=(device const &other) = delete;
        device &operator=(device &&other)      = delete;

        [[nodiscard]] auto command_pool() const -> VkCommandPool { return command_pool_; }
        [[nodiscard]] auto logical_device() const -> VkDevice { return device_; }
        [[nodiscard]] auto physical_device() const -> VkPhysicalDevice { return physical_device_; }
        [[nodiscard]] auto surface() const -> VkSurfaceKHR { return surface_; }
        [[nodiscard]] auto graphics_queue() const -> VkQueue { return graphics_queue_; }
        [[nodiscard]] auto present_queue() const -> VkQueue { return present_queue_; }

        auto get_swap_chain_support() -> swap_chain_support_details { return query_swap_chain_support(physical_device_); }
        auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) -> uint32_t;
        auto find_physical_queue_families() -> queue_family_indices { return find_queue_families(physical_device_); }
        auto find_supported_format(std::vector<VkFormat> const &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;

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
            VkImageCreateInfo const &image_info,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &image_memory);

        VkPhysicalDeviceProperties properties;

    private:
        friend class singleton<device>;
        device();

    private:
        void create_instance();
        void setup_debug_messenger();
        void create_surface();
        void pick_physical_device();
        void create_logical_device();
        void create_command_pool();

        // helper functions
        auto is_device_suitable(VkPhysicalDevice device) -> bool;
        auto required_extensions() -> std::vector<const char*>;
        auto check_validation_layer_support() -> bool;
        auto find_queue_families(VkPhysicalDevice device) -> queue_family_indices;
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void has_gflw_required_instance_extensions();
        auto check_device_extension_support(VkPhysicalDevice device) -> bool;
        auto query_swap_chain_support(VkPhysicalDevice device) -> swap_chain_support_details;

        VkInstance               instance_        = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
        VkPhysicalDevice         physical_device_ = VK_NULL_HANDLE;
        window                   *window_ptr_     = nullptr;
        VkCommandPool            command_pool_    = VK_NULL_HANDLE;

        VkDevice     device_         = VK_NULL_HANDLE;
        VkSurfaceKHR surface_        = VK_NULL_HANDLE;
        VkQueue      graphics_queue_ = VK_NULL_HANDLE;
        VkQueue      present_queue_  = VK_NULL_HANDLE;

        const std::vector<const char*> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
