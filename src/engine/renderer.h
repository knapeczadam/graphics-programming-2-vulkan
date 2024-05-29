#pragma once

// Project includes
#include "src/core/window.h"
#include "src/engine/device.h"
#include "src/engine/swap_chain.h"

// Standard includes
#include <cassert>
#include <memory>
#include <vector>

namespace dae
{
    class renderer
    {
    public:
        renderer(window &window, device &device);
        ~renderer();

        renderer(renderer const &)            = delete;
        renderer &operator=(renderer const &) = delete;

        [[nodiscard]] auto get_swap_chain_render_pass() const -> VkRenderPass { return swap_chain_->get_render_pass(); }
        [[nodiscard]] auto get_aspect_ratio() const -> float { return swap_chain_->extent_aspect_ratio(); }
        [[nodiscard]] auto is_frame_in_progress() const -> bool { return is_frame_started_; }
        [[nodiscard]] auto get_current_command_buffer() const -> VkCommandBuffer
        {
            assert(is_frame_started_ and "Cannot get command buffer when frame not in progress!");
            return command_buffers_[current_frame_index_];
        }

        [[nodiscard]] auto get_frame_index() const -> int
        {
            assert(is_frame_started_ and "Cannot get frame index when frame not in progress!");
            return current_frame_index_;
        }

        auto begin_frame() -> VkCommandBuffer;
        void end_frame();
        void begin_swap_chain_render_pass(VkCommandBuffer command_buffer);
        void end_swap_chain_render_pass(VkCommandBuffer command_buffer);

    private:
        void create_command_buffers();
        void free_command_buffers();
        void recreate_swap_chain();
        
    private:
        window                      &window_;
        device                      &device_;
        std::unique_ptr<swap_chain> swap_chain_;
        std::vector<VkCommandBuffer>    command_buffers_;

        uint32_t current_image_index_ = {};
        int      current_frame_index_ = {};
        bool     is_frame_started_    = {};
        
        
    };
}