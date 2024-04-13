#include "lve_window.h"

// Standard includes
#include <stdexcept>

namespace lve
{
    lve_window::lve_window(int width, int height, std::string name)
        : width_(width)
        , height_(height)
        , window_name_(std::move(name))
    {
        init_window();
    }

    lve_window::~lve_window()
    {
        glfwDestroyWindow(window_ptr_);
        glfwTerminate();
    }

    void lve_window::create_window_surface(VkInstance instance, VkSurfaceKHR *surface_ptr)
    {
        if (glfwCreateWindowSurface(instance, window_ptr_, nullptr, surface_ptr) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create window surface!"};
            
        }
    }

    void lve_window::framebuffer_resize_callback(GLFWwindow *window, int width, int height)
    {
        auto updated_window = reinterpret_cast<lve_window*>(glfwGetWindowUserPointer(window));
        updated_window->frame_buffer_resized_ = true;
        updated_window->width_ = width;
        updated_window->height_ = height;
        
    }

    auto lve_window::should_close() const -> bool
    {
        return glfwWindowShouldClose(window_ptr_);
    }

    void lve_window::init_window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window_ptr_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_ptr_, this);
        glfwSetFramebufferSizeCallback(window_ptr_, framebuffer_resize_callback);
    }
}
