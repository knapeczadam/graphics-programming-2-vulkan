#pragma once

// Project includes
#include "src/vulkan/swap_chain.h"
#include "src/utility/singleton.h"

// Standard includes
#include <cassert>
#include <memory>
#include <vector>


namespace dae
{
    // Forward declarations
    class window;
    class device;
    
    class renderer final : public singleton<renderer>
    {
    public:
        ~renderer() override;

        renderer(renderer const &other)            = delete;
        renderer(renderer &&other)                 = delete;
        renderer &operator=(renderer const &other) = delete;
        renderer &operator=(renderer &&other)      = delete;

        void init(window *window_ptr, device *device_ptr);

        [[nodiscard]] auto swap_chain_render_pass() const -> VkRenderPass { return swap_chain_->render_pass(); }
        [[nodiscard]] auto aspect_ratio() const -> float { return swap_chain_->extent_aspect_ratio(); }
        [[nodiscard]] auto is_frame_in_progress() const -> bool { return is_frame_started_; }
        [[nodiscard]] auto current_command_buffer() const -> VkCommandBuffer
        {
            assert(is_frame_started_ and "Cannot get command buffer when frame not in progress!");
            return command_buffers_[current_frame_index_];
        }

        [[nodiscard]] auto frame_index() const -> int
        {
            assert(is_frame_started_ and "Cannot get frame index when frame not in progress!");
            return current_frame_index_;
        }

        auto begin_frame() -> VkCommandBuffer;
        void end_frame();
        void begin_swap_chain_render_pass(VkCommandBuffer command_buffer);
        void end_swap_chain_render_pass(VkCommandBuffer command_buffer);

    private:
        friend class singleton<renderer>;
        renderer() = default;
        
        void create_command_buffers();
        void free_command_buffers();
        void recreate_swap_chain();
        
    private:
        window                      *window_ptr_ = nullptr;
        device                      *device_ptr_ = nullptr;
        std::unique_ptr<swap_chain> swap_chain_;
        std::vector<VkCommandBuffer> command_buffers_;

        uint32_t current_image_index_ = {};
        int      current_frame_index_ = {};
        bool     is_frame_started_    = {};
    };
}