#pragma once

#include "device.h"

namespace dae
{
    class buffer
    {
    public:
        buffer(
            device &device,
            VkDeviceSize instance_size,
            uint32_t instance_count,
            VkBufferUsageFlags usage_flags,
            VkMemoryPropertyFlags memory_property_flags,
            VkDeviceSize min_offset_alignment = 1);
        ~buffer();

        buffer(buffer const &) = delete;
        buffer &operator=(buffer const &) = delete;

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
        [[nodiscard]] auto get_mapped_memory() const -> void * { return mapped_; }
        [[nodiscard]] auto get_instance_count() const -> uint32_t { return instance_count_; }
        [[nodiscard]] auto get_instance_size() const -> VkDeviceSize { return instance_size_; }
        [[nodiscard]] auto get_alignment_size() const -> VkDeviceSize { return instance_size_; }
        [[nodiscard]] auto get_usage_flags() const -> VkBufferUsageFlags { return usage_flags_; }
        [[nodiscard]] auto get_memory_property_flags() const -> VkMemoryPropertyFlags { return memory_property_flags_; }
        [[nodiscard]] auto get_buffer_size() const -> VkDeviceSize { return buffer_size_; }

    private:
        static auto get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) -> VkDeviceSize;

        device         &device_;
        void           *mapped_      = nullptr;
        VkBuffer       buffer_       = VK_NULL_HANDLE;
        VkDeviceMemory memory_       = VK_NULL_HANDLE;

        VkDeviceSize          buffer_size_;
        uint32_t              instance_count_;
        VkDeviceSize          instance_size_;
        VkDeviceSize          alignment_size_;
        VkBufferUsageFlags    usage_flags_;
        VkMemoryPropertyFlags memory_property_flags_;
    };
}
