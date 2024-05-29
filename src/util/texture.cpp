#include "texture.h"

// Project includes
#include "engine/buffer.h"

// Standard includes
#include <cmath>
#include <stdexcept>

// STB includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace dae
{
    texture::texture(device &device, std::string const &file_path, VkFormat format)
        : device_{device}
        , image_format_{format}
    {
        int text_channels, bytes_per_pixel;

        stbi_uc *pixels = stbi_load(file_path.c_str(), &width_, &height_, &text_channels, STBI_rgb_alpha);
        mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(width_, height_)))) + 1;

        buffer staging_buffer{
            device_,
            4,
            static_cast<uint32_t>(width_ * height_),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        staging_buffer.map();
        staging_buffer.write_to_buffer(pixels);

        VkImageCreateInfo image_info{};
        image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType     = VK_IMAGE_TYPE_2D;
        image_info.format        = image_format_;
        image_info.extent.width  = static_cast<uint32_t>(width_);
        image_info.extent.height = static_cast<uint32_t>(height_);
        image_info.extent.depth  = 1;
        image_info.mipLevels     = mip_levels_;
        image_info.arrayLayers   = 1;
        image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        device_.create_image_with_info(image_info,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, image_memory_);
        transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device_.copy_buffer_to_image(staging_buffer.get_buffer(), image_, static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1);
        generate_mipmaps();

        image_layout_ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter               = VK_FILTER_LINEAR;
        sampler_info.minFilter               = VK_FILTER_LINEAR;
        sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable        = VK_FALSE;
        sampler_info.maxAnisotropy           = 1.0f;
        sampler_info.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable           = VK_FALSE;
        sampler_info.compareOp               = VK_COMPARE_OP_NEVER;
        sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.minLod                  = 0.0f;
        sampler_info.maxLod                  = static_cast<float>(mip_levels_);
        sampler_info.mipLodBias              = 0.0f;
        sampler_info.anisotropyEnable        = VK_TRUE;
        sampler_info.maxAnisotropy           = 4.0f;

        vkCreateSampler(device_.get_logical_device(), &sampler_info, nullptr, &sampler_);

        VkImageViewCreateInfo view_info{};
        view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image                           = image_;
        view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format                          = image_format_;
        view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel   = 0;
        view_info.subresourceRange.levelCount     = mip_levels_;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = 1;
        view_info.components.r                    = VK_COMPONENT_SWIZZLE_R;
        view_info.components.g                    = VK_COMPONENT_SWIZZLE_G;
        view_info.components.b                    = VK_COMPONENT_SWIZZLE_B;
        view_info.components.a                    = VK_COMPONENT_SWIZZLE_A;

        vkCreateImageView(device_.get_logical_device(), &view_info, nullptr, &image_view_);

        stbi_image_free(pixels);
    }

    texture::~texture()
    {
        vkDestroyImage(device_.get_logical_device(), image_, nullptr);
        vkFreeMemory(device_.get_logical_device(), image_memory_, nullptr);
        vkDestroyImageView(device_.get_logical_device(), image_view_, nullptr);
        vkDestroySampler(device_.get_logical_device(), sampler_, nullptr);
    }

    void texture::transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout)
    {
        VkCommandBuffer command_buffer = device_.begin_single_time_commands();
        
        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = old_layout;
        barrier.newLayout                       = new_layout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image_;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = mip_levels_;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED and new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

         vkCmdPipelineBarrier(
            command_buffer,
            source_stage,
            destination_stage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        device_.end_single_time_commands(command_buffer);
    }

    void texture::generate_mipmaps()
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(device_.get_physical_device(), image_format_, &format_properties);

        if (not (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer command_buffer = device_.begin_single_time_commands();

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image                           = image_;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.subresourceRange.levelCount     = 1;

        int32_t mip_width  = width_;
        int32_t mip_height = height_;

        for (uint32_t i = 1; i < mip_levels_; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                command_buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mip_width, mip_height, 1};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;
            blit.dstOffsets[0]                 = {0, 0, 0};
            blit.dstOffsets[1]                 = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;

            vkCmdBlitImage(
                command_buffer,
                image_,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image_,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                command_buffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);

            if (mip_width > 1) mip_width /= 2;
            if (mip_height > 1) mip_height /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mip_levels_ - 1;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        device_.end_single_time_commands(command_buffer);
    }
}
