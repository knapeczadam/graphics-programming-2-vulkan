#pragma once

// Standard includes
#include <memory>
#include <unordered_map>

// Project includes
#include "lve_model.h"

// GLM includes
#include <glm/gtc/matrix_transform.hpp>

namespace lve
{

    struct transform_component
    {
        glm::vec3 translation = {};
        glm::vec3 scale       = {1.0f, 1.0f, 1.0f};
        glm::vec3 rotation    = {};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        // Intrinsic rotations: R = Y(1), X(2), Z(3)
        // Extrinsic rotations: R = Z(3), X(2), Y(1)
        glm::mat4 mat4();
        glm::mat4 normal_matrix();
    };

    struct point_light_component
    {
        float light_intensity = 1.0f;
    };
    
    class lve_game_object
    {
    public:
        // Type aliases
        using id_t = unsigned int;
        using map = std::unordered_map<id_t, lve_game_object>;
        
    public:
        lve_game_object(lve_game_object const &)            = delete;
        lve_game_object &operator=(lve_game_object const &) = delete;
        lve_game_object(lve_game_object &&)                 = default;
        lve_game_object &operator=(lve_game_object &&)      = default;

        
        static auto create_game_object() -> lve_game_object
        {
            static id_t current_id = 0;
            return lve_game_object{current_id++};
        }

        static auto make_point_light(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3{1.0f}) -> lve_game_object;

        [[nodiscard]] auto get_id() const -> id_t { return id_; }

    private:
        lve_game_object(id_t id) : id_{id} {}

    public:
        std::shared_ptr<lve_model> model     = {};
        glm::vec3                  color     = {};
        transform_component        transform = {};

        std::unique_ptr<point_light_component> point_light = nullptr;

    private:
        id_t id_;
    };
}
