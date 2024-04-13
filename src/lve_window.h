#pragma once

// GLFW includes
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Standard includes
#include <string>

namespace lve
{
    class lve_window
    {
    public:
        lve_window(int width, int height, std::string name);
        ~lve_window();

        lve_window(const lve_window &)            = delete;
        lve_window(lve_window &&)                 = delete;
        lve_window &operator=(const lve_window &) = delete;
        lve_window &operator=(lve_window &&)      = delete;

        [[nodiscard]] auto should_close() const -> bool;
        [[nodiscard]] auto get_extent() const -> VkExtent2D { return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)};}
        [[nodiscard]] auto was_window_resized() const -> bool { return frame_buffer_resized_; }
        [[nodiscard]] auto get_glfw_window() const -> GLFWwindow* { return window_ptr_; }
        void reset_window_resized_flag() { frame_buffer_resized_ = false; }
        
        void create_window_surface(VkInstance instance, VkSurfaceKHR *surface_ptr);

    private:
        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);
        void init_window();

    private:
        GLFWwindow *window_ptr_ = nullptr;
        int width_;
        int height_;
        bool frame_buffer_resized_ = false;
        std::string window_name_;
    };
}
