#include "shading_mode_controller.h"

// Project includes
#include "src/engine/frame_info.h"

// Standard includes
#include <iostream>

#define GREEN_TEXT(text) "\033[1;32m" text "\033[0m"
#define MAGENTA_TEXT(text) "\033[1;35m" text "\033[0m"

namespace dae
{
    void shading_mode_controller::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_1 and action == GLFW_PRESS)
        {
            frame_info::instance().shading_mode = (frame_info::instance().shading_mode + 1) % 4;

            std::string m_ShadingModeString;
            switch (frame_info::instance().shading_mode)
            {
            case 0:
                m_ShadingModeString = "OBSERVED AREA";
                break;
            case 1:
                m_ShadingModeString = "DIFFUSE";
                break;
            case 2:
                m_ShadingModeString = "SPECULAR";
                break;
            case 3:
                m_ShadingModeString = "COMBINED";
                break;
            }
            std::cout << GREEN_TEXT("* Shading Mode = ") << MAGENTA_TEXT("" + m_ShadingModeString + "") << '\n';
        }
        if (key == GLFW_KEY_2 and action == GLFW_PRESS)
        {
            frame_info::instance().use_normal = not frame_info::instance().use_normal;
            
            std::string on_off = frame_info::instance().use_normal ? "ON" : "OFF";
            std::cout << GREEN_TEXT("* NormalMap ") << MAGENTA_TEXT("" + on_off + "") << '\n';
        }
    }
}
