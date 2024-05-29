#pragma once

// Project includes
#include "src/core/game_object.h"
#include "src/engine/window.h"

namespace dae
{
    enum key_mappings
    {
        move_left     = GLFW_KEY_A,
        move_right    = GLFW_KEY_D,
        move_forward  = GLFW_KEY_W,
        move_backward = GLFW_KEY_S,
        move_up       = GLFW_KEY_E,
        move_down     = GLFW_KEY_Q,
        look_left     = GLFW_KEY_LEFT,
        look_right    = GLFW_KEY_RIGHT,
        look_up       = GLFW_KEY_UP,
        look_down     = GLFW_KEY_DOWN
        
    };
    class movement_controller
    {
    public:
        void move(GLFWwindow *window_ptr, float dt, game_object &game_object);
        
    private:
        float move_speed = 3.0f;
        float look_speed = 5.5;

        double last_mouse_x_ = 0;
        double last_mouse_y_ = 0;
    };
}
