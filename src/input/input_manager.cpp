#include "input_manager.h"

// Standard includes
#include <iostream>

#include "src/engine/frame_info.h"

namespace dae
{
    void input_manager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_1 and action == GLFW_PRESS)
        {
            frame_info::instance().ubo_ptr->use_normal = not frame_info::instance().ubo_ptr->use_normal;
        }
        if (key == GLFW_KEY_2 and action == GLFW_PRESS)
        {
            frame_info::instance().ubo_ptr->shading_mode = (frame_info::instance().ubo_ptr->shading_mode + 1) % 4;
        }
    }
}
