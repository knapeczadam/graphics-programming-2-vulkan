#include "movement_controller.h"

namespace dae
{
    void movement_controller::move(GLFWwindow* window_ptr, float dt, game_object& game_object)
    {
        double mouse_x, mouse_y;
        glm::vec3 rotate{0};
        
        if (glfwGetWindowAttrib(window_ptr, GLFW_HOVERED))
        {
            glfwGetCursorPos(window_ptr, &mouse_x, &mouse_y);
            double delta_x = mouse_x - last_mouse_x_;
            double delta_y = mouse_y - last_mouse_y_;
            last_mouse_x_ = mouse_x;
            last_mouse_y_ = mouse_y;

            rotate.x += look_speed * delta_y * dt * -1;
            rotate.y += look_speed * delta_x * dt;
        }
        
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
        float pitch = game_object.transform.rotation.x;

        glm::vec3 forward_dir = {
            glm::cos(pitch) * glm::sin(yaw),
            -glm::sin(pitch),
            glm::cos(pitch) * glm::cos(yaw)
        };

        glm::vec3 right_dir = glm::cross(forward_dir, glm::vec3(0.0f, -1.0f, 0.0f));
        glm::vec3 up_dir = glm::cross(right_dir, forward_dir);

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
