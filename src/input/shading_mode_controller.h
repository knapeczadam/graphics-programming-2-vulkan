#pragma once

// Project includes
#include "src/engine/window.h"

namespace dae
{
    class shading_mode_controller final
    {
    public:
        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    };
}
