#pragma once

// Vulkan includes
#include <vulkan/vulkan.h>

namespace dae
{
    // Forward declarations
    class device;
    
    class buffer final
    {
    public:
        buffer(
            device *device_ptr,
            VkDeviceSize instance_size,
            uint32_t instance_count,
            VkBufferUsageFlags usage_flags,
            VkMemoryPropertyFlags memory_property_flags,
            VkDeviceSize min_offset_alignment = 1);
        ~buffer();

        buffer(buffer const &other)            = delete;
        buffer &operator=(buffer const &other) = delete;

        auto map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;
        void unmap();

        void write_to_buffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        auto flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;
        auto descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkDescriptorBufferInfo;
        auto invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;

        void write_to_index(void *data, int index);
        auto flush_index(int index) -> VkResult;
        auto descriptor_info_for_index(int index) -> VkDescriptorBufferInfo;
        auto invalidate_index(int index) -> VkResult;

        [[nodiscard]] auto get_buffer() const -> VkBuffer { return buffer_; }
        [[nodiscard]] auto mapped_memory() const -> void * { return mapped_; }
        [[nodiscard]] auto instance_count() const -> uint32_t { return instance_count_; }
        [[nodiscard]] auto instance_size() const -> VkDeviceSize { return instance_size_; }
        [[nodiscard]] auto alignment_size() const -> VkDeviceSize { return instance_size_; }
        [[nodiscard]] auto usage_flags() const -> VkBufferUsageFlags { return usage_flags_; }
        [[nodiscard]] auto memory_property_flags() const -> VkMemoryPropertyFlags { return memory_property_flags_; }
        [[nodiscard]] auto buffer_size() const -> VkDeviceSize { return buffer_size_; }

    private:
        static auto get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) -> VkDeviceSize;

        device         *device_ptr_ = nullptr;
        void           *mapped_      = nullptr;
        VkBuffer       buffer_       = VK_NULL_HANDLE;
        VkDeviceMemory memory_       = VK_NULL_HANDLE;

        VkDeviceSize          buffer_size_           = 0;
        uint32_t              instance_count_        = 0;
        VkDeviceSize          instance_size_         = 0;
        VkDeviceSize          alignment_size_        = 0;
        VkBufferUsageFlags    usage_flags_           = 0 ;
        VkMemoryPropertyFlags memory_property_flags_ = 0;
    };
}
