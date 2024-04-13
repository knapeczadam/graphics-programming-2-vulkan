#include "keyboard_movement_controller.h"

namespace lve
{
    void keyboard_movement_controller::move_in_plane_xy(GLFWwindow* window_ptr, float dt, lve_game_object& game_object)
    {
        glm::vec3 rotate {0};
        if (glfwGetKey(window_ptr, key_mappings::look_right) == GLFW_PRESS)
        {
            rotate.y += 1.0f;
        }
        if (glfwGetKey(window_ptr, key_mappings::look_left) == GLFW_PRESS)
        {
            rotate.y -= 1.0f;
        }
        if (glfwGetKey(window_ptr, key_mappings::look_up) == GLFW_PRESS)
        {
            rotate.x += 1.0f;
        }
        if (glfwGetKey(window_ptr, key_mappings::look_down) == GLFW_PRESS)
        {
            rotate.x -= 1.0f;
        }

        if (glm::dot(rotate, rotate) > glm::epsilon<float>())
        {
            game_object.transform.rotation += look_speed * dt * glm::normalize(rotate);
        }

        game_object.transform.rotation.x = glm::clamp(game_object.transform.rotation.x, -1.5f, 1.5f);
        game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y, glm::two_pi<float>());

        float yaw = game_object.transform.rotation.y;
        glm::vec3 const forward_dir = {glm::sin(yaw), 0.0f, glm::cos(yaw)};
        glm::vec3 const right_dir   = {forward_dir.z, 0.0f, -forward_dir.x};
        glm::vec3 const up_dir      = {0.0f, -1.0f, 0.0f};

        glm::vec3 move_dir {0.0f};
        if (glfwGetKey(window_ptr, key_mappings::move_forward) == GLFW_PRESS)
        {
            move_dir += forward_dir;
        }
        if (glfwGetKey(window_ptr, key_mappings::move_backward) == GLFW_PRESS)
        {
            move_dir -= forward_dir;
        }
        if (glfwGetKey(window_ptr, key_mappings::move_right) == GLFW_PRESS)
        {
            move_dir += right_dir;
        }
        if (glfwGetKey(window_ptr, key_mappings::move_left) == GLFW_PRESS)
        {
            move_dir -= right_dir;
        }
        if (glfwGetKey(window_ptr, key_mappings::move_up) == GLFW_PRESS)
        {
            move_dir += up_dir;
        }
        if (glfwGetKey(window_ptr, key_mappings::move_down) == GLFW_PRESS)
        {
            move_dir -= up_dir;
        }

        if (glm::dot(move_dir, move_dir) > glm::epsilon<float>())
        {
            game_object.transform.translation += move_speed * dt * glm::normalize(move_dir);
        }
        
    }
}
