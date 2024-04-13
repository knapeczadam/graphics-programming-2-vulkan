#pragma once

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve
{
    class lve_camera
    {
    public:
        void set_orthographic_projection(float left, float right, float top, float bottom, float near, float far);
        void set_perspective_projection(float fovy, float aspect, float near, float far);

        void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void set_view_yxz(glm::vec3 position, glm::vec3 rotation);

        auto get_projection() const -> glm::mat4 { return projection_matrix_; }
        auto get_view() const -> glm::mat4 { return view_matrix_; }

    private:
        glm::mat4 projection_matrix_ {1.0f};
        glm::mat4 view_matrix_{1.0f};
    };
}
