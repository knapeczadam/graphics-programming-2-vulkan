/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */
#include "buffer.h"

// Project includes
#include "src/vulkan/device.h"

// Standard includes
#include <cassert>
#include <cstring>

namespace dae
{
    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instance_size The size of an instance
     * @param min_offset_alignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    auto buffer::get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) -> VkDeviceSize
    {
        if (min_offset_alignment > 0)
        {
            return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return instance_size;
    }

    buffer::buffer(
        VkDeviceSize          instance_size,
        uint32_t              instance_count,
        VkBufferUsageFlags    usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        VkDeviceSize          min_offset_alignment)
        : device_ptr_{&device::instance()},
          instance_count_{instance_count},
          instance_size_{instance_size},
          usage_flags_{usage_flags},
          memory_property_flags_{memory_property_flags}
    {
        alignment_size_ = get_alignment(instance_size, min_offset_alignment);
        buffer_size_ = alignment_size_ * instance_count;
        device_ptr_->create_buffer(buffer_size_, usage_flags, memory_property_flags, buffer_, memory_);
    }

    buffer::~buffer()
    {
        unmap();
        vkDestroyBuffer(device_ptr_->logical_device(), buffer_, nullptr);
        vkFreeMemory(device_ptr_->logical_device(), memory_, nullptr);
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */
    auto buffer::map(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        assert(buffer_ and memory_ and "Called map on buffer before create");
        return vkMapMemory(device_ptr_->logical_device(), memory_, offset, size, 0, &mapped_);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void buffer::unmap()
    {
        if (mapped_)
        {
            vkUnmapMemory(device_ptr_->logical_device(), memory_);
            mapped_ = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void buffer::write_to_buffer(void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mapped_ and "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mapped_, data, buffer_size_);
        }
        else
        {
            char *mem_offset = (char*)mapped_;
            mem_offset += offset;
            memcpy(mem_offset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    auto buffer::flush(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size   = size;
        return vkFlushMappedMemoryRanges(device_ptr_->logical_device(), 1, &mapped_range);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    auto buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size   = size;
        return vkInvalidateMappedMemoryRanges(device_ptr_->logical_device(), 1, &mapped_range);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    auto buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) -> VkDescriptorBufferInfo
    {
        return VkDescriptorBufferInfo{
            buffer_,
            offset,
            size,
        };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    void buffer::write_to_index(void *data, int index)
    {
        write_to_buffer(data, instance_size_, index * alignment_size_);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    auto buffer::flush_index(int index) -> VkResult { return flush(alignment_size_, index * alignment_size_); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    auto buffer::descriptor_info_for_index(int index) -> VkDescriptorBufferInfo
    {
        return descriptor_info(alignment_size_, index * alignment_size_);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    auto buffer::invalidate_index(int index) -> VkResult
    {
        return invalidate(alignment_size_, index * alignment_size_);
    }
}
