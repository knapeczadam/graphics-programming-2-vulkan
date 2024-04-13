#pragma once

// Project headers
#include "lve_device.h"

// Vulkan headers
#include <vulkan/vulkan.h>

// Standard headers
#include <memory>
#include <string>
#include <vector>

namespace lve
{
    class lve_swap_chain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        lve_swap_chain(lve_device &device_ref, VkExtent2D window_extent);
        lve_swap_chain(lve_device &device_ref, VkExtent2D window_extent, std::shared_ptr<lve_swap_chain> const &previous);
        ~lve_swap_chain();

        lve_swap_chain(lve_swap_chain const &)            = delete;
        lve_swap_chain &operator=(lve_swap_chain const &) = delete;

        auto get_frame_buffer(int index) -> VkFramebuffer { return swap_chain_framebuffers_[index]; }
        auto get_render_pass() -> VkRenderPass { return render_pass_; }
        auto get_image_view(int index) -> VkImageView { return swap_chain_image_views_[index]; }
        auto image_count() -> size_t { return swap_chain_images_.size(); }
        auto get_swap_chain_image_format() -> VkFormat { return swap_chain_image_format_; }
        auto get_swap_chain_extent() -> VkExtent2D { return swap_chain_extent_; }
        auto width() -> uint32_t { return swap_chain_extent_.width; }
        auto height() -> uint32_t { return swap_chain_extent_.height; }

        auto extent_aspect_ratio() -> float;

        auto find_depth_format() -> VkFormat;

        auto acquire_next_image(uint32_t *image_index) -> VkResult;
        auto submit_command_buffers(VkCommandBuffer const *buffers, uint32_t *image_index) -> VkResult;

        auto compare_swap_formats(lve_swap_chain const &swap_chain) const -> bool
        {
            return swap_chain.swap_chain_depth_format_ == swap_chain_depth_format_ and swap_chain.swap_chain_image_format_ == swap_chain_image_format_;
        }

    private:
        void init();
        void create_swap_chain();
        void create_image_views();
        void create_depth_resources();
        void create_render_pass();
        void create_framebuffers();
        void create_sync_objects();

        // Helper functions
        auto choose_swap_surface_format(std::vector<VkSurfaceFormatKHR> const &available_formats) -> VkSurfaceFormatKHR;
        auto choose_swap_present_mode(std::vector<VkPresentModeKHR> const &available_present_modes) -> VkPresentModeKHR;
        auto choose_swap_extent(VkSurfaceCapabilitiesKHR const &capabilities) -> VkExtent2D;

        VkFormat   swap_chain_image_format_;
        VkFormat   swap_chain_depth_format_;
        VkExtent2D swap_chain_extent_;

        std::vector<VkFramebuffer> swap_chain_framebuffers_;
        VkRenderPass               render_pass_;

        std::vector<VkImage>        depth_images_;
        std::vector<VkDeviceMemory> depth_image_memories_;
        std::vector<VkImageView>    depth_image_views_;
        std::vector<VkImage>        swap_chain_images_;
        std::vector<VkImageView>    swap_chain_image_views_;

        lve_device &device_;
        VkExtent2D window_extent_;

        VkSwapchainKHR swap_chain_;
        std::shared_ptr<lve_swap_chain> old_swap_chain_;

        std::vector<VkSemaphore> image_available_semaphores_;
        std::vector<VkSemaphore> render_finished_semaphores_;
        std::vector<VkFence>     in_flight_fences_;
        std::vector<VkFence>     images_in_flight_;
        size_t                   current_frame_               = 0;
    };
}
