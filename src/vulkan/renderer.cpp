#include "renderer.h"

// Project includes
#include "src/engine/window.h"
#include "src/vulkan/device.h"

// Standard includes
#include <array>
#include <stdexcept>

namespace dae
{
    renderer::~renderer()
    {
        free_command_buffers();    
    }

    auto renderer::begin_frame() -> VkCommandBuffer
    {
        assert(not is_frame_started_ and "Can't call begin_frame while already in progess");
        
        auto result = swap_chain_->acquire_next_image(&current_image_index_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swap_chain();
            return nullptr;
        }
        
        if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error{"Failed to acquire swap chain image!"};
        }

        is_frame_started_ = true;

        auto command_buffer = current_command_buffer();
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to begin recording command buffer"};
        }

        return command_buffer;
    }

    void renderer::end_frame()
    {
        assert(is_frame_started_ and "Can't call end_frame while frame is not in progress");
        auto command_buffer = current_command_buffer();
        
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to record command buffer!"};
        }
        
        auto result = swap_chain_->submit_command_buffers(&command_buffer, &current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or window_ptr_->was_window_resized())
        {
            window_ptr_->reset_window_resized_flag();
            recreate_swap_chain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to present swap chain image!"};
        }

        is_frame_started_ = false;
        current_frame_index_ = (current_frame_index_ + 1) % swap_chain::MAX_FRAMES_IN_FLIGHT;
    }

    void renderer::begin_swap_chain_render_pass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ and "Can't call begin_swap_chain_render_pass if frame is not in progesss");
        assert(command_buffer == current_command_buffer() and "Can't begin render pass on command buffer from a different frame");
        
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass  = swap_chain_->render_pass();
        render_pass_info.framebuffer = swap_chain_->get_frame_buffer(current_image_index_);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swap_chain_->swap_chain_extent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color            = {{0.01f, 0.01f, 0.1f, 0.1f}};
        clear_values[1].depthStencil     = {1.0f, 0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues    = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE); // we don't have secondary command buffer

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(swap_chain_->swap_chain_extent().width);
        viewport.height   = static_cast<float>(swap_chain_->swap_chain_extent().height);\
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swap_chain_->swap_chain_extent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void renderer::end_swap_chain_render_pass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ and "Can't call end_swap_chain_render_pass if frame is not in progesss");
        assert(command_buffer == current_command_buffer() and "Can't end render pass on command buffer from a different frame");
        
        vkCmdEndRenderPass(command_buffer);
    }

    renderer::renderer()
        : window_ptr_{&window::instance()}
        , device_ptr_{&device::instance()}
    {
        recreate_swap_chain();
        create_command_buffers();
    }

    void renderer::create_command_buffers()
    {
        command_buffers_.resize(swap_chain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool        = device_ptr_->command_pool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(device_ptr_->logical_device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to allocate command buffers!"};
        }
    }

    void renderer::free_command_buffers()
    {
        vkFreeCommandBuffers(
            device_ptr_->logical_device(),
            device_ptr_->command_pool(),
            static_cast<uint32_t>(command_buffers_.size()),
            command_buffers_.data());
        command_buffers_.clear();
    }

    void renderer::recreate_swap_chain()
    {
        auto extent = window_ptr_->get_extent();
        while (extent.width == 0 or extent.height == 0)
        {
            extent = window_ptr_->get_extent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device_ptr_->logical_device());
        if (swap_chain_ == nullptr)
        {
            swap_chain_ = std::make_unique<swap_chain>(extent);
        }
        else
        {
            std::shared_ptr<swap_chain> old_swap_chain = std::move(swap_chain_);
            swap_chain_ = std::make_unique<swap_chain>(extent, old_swap_chain);

            if (not old_swap_chain->compare_swap_formats(*swap_chain_))
            {
                throw std::runtime_error{"Swap chain image (or depth) format has changed!"};
            }
        }
    }
}
