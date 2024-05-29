#include "window.h"

// Standard includes
#include <stdexcept>

namespace dae
{
    window::window(int width, int height, std::string name)
        : width_(width)
        , height_(height)
        , window_name_(std::move(name))
    {
        init_window();
    }

    window::~window()
    {
        glfwDestroyWindow(window_ptr_);
        glfwTerminate();
    }

    void window::create_window_surface(VkInstance instance, VkSurfaceKHR *surface_ptr)
    {
        if (glfwCreateWindowSurface(instance, window_ptr_, nullptr, surface_ptr) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create window surface!"};
            
        }
    }

    void window::framebuffer_resize_callback(GLFWwindow *window_ptr, int width, int height)
    {
        auto updated_window = reinterpret_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        updated_window->frame_buffer_resized_ = true;
        updated_window->width_ = width;
        updated_window->height_ = height;
        
    }

    auto window::should_close() const -> bool
    {
        return glfwWindowShouldClose(window_ptr_);
    }

    void window::init_window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // we are not using OpenGL, hence the GLFW_NO_API
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window_ptr_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_ptr_, this);
        glfwSetFramebufferSizeCallback(window_ptr_, framebuffer_resize_callback);
    }
}
