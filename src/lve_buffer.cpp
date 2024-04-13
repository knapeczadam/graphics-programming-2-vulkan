/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */
#include "lve_buffer.h"

// Standard includes
#include <cassert>
#include <cstring>

namespace lve
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
    auto lve_buffer::get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) -> VkDeviceSize
    {
        if (min_offset_alignment > 0)
        {
            return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return instance_size;
    }

    lve_buffer::lve_buffer(
        lve_device            &device,
        VkDeviceSize          instanceSize,
        uint32_t              instanceCount,
        VkBufferUsageFlags    usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize          minOffsetAlignment)
        : lve_device_{device},
          instance_count_{instanceCount},
          instance_size_{instanceSize},
          usage_flags_{usageFlags},
          memory_property_flags_{memoryPropertyFlags}
    {
        alignment_size_ = get_alignment(instanceSize, minOffsetAlignment);
        buffer_size_ = alignment_size_ * instanceCount;
        device.create_buffer(buffer_size_, usageFlags, memoryPropertyFlags, buffer_, memory_);
    }

    lve_buffer::~lve_buffer()
    {
        unmap();
        vkDestroyBuffer(lve_device_.device(), buffer_, nullptr);
        vkFreeMemory(lve_device_.device(), memory_, nullptr);
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
    auto lve_buffer::map(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        assert(buffer_ and memory_ and "Called map on buffer before create");
        return vkMapMemory(lve_device_.device(), memory_, offset, size, 0, &mapped_);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void lve_buffer::unmap()
    {
        if (mapped_)
        {
            vkUnmapMemory(lve_device_.device(), memory_);
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
    void lve_buffer::write_to_buffer(void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mapped_ and "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mapped_, data, buffer_size_);
        }
        else
        {
            char *memOffset = (char*)mapped_;
            memOffset += offset;
            memcpy(memOffset, data, size);
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
    auto lve_buffer::flush(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory_;
        mappedRange.offset = offset;
        mappedRange.size   = size;
        return vkFlushMappedMemoryRanges(lve_device_.device(), 1, &mappedRange);
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
    auto lve_buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) -> VkResult
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory_;
        mappedRange.offset = offset;
        mappedRange.size   = size;
        return vkInvalidateMappedMemoryRanges(lve_device_.device(), 1, &mappedRange);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    auto lve_buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) -> VkDescriptorBufferInfo
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
    void lve_buffer::write_to_index(void *data, int index)
    {
        write_to_buffer(data, instance_size_, index * alignment_size_);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    auto lve_buffer::flush_index(int index) -> VkResult { return flush(alignment_size_, index * alignment_size_); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    auto lve_buffer::descriptor_info_for_index(int index) -> VkDescriptorBufferInfo
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
    auto lve_buffer::invalidate_index(int index) -> VkResult
    {
        return invalidate(alignment_size_, index * alignment_size_);
    }
} // namespace lve
