#pragma once

// Project includes
#include "src/core/model.h"
#include "src/utility/material.h"

// Standard includes
#include <memory>
#include <string>

// GLM includes
#include <glm/gtc/matrix_transform.hpp>

namespace dae
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
    
    class game_object final
    {
    public:
        // Type aliases
        using id_t = unsigned int;

    public:
        explicit game_object(std::string name) : id_{next_id_++}, name_{std::move(name)} {}
        ~game_object() = default;
        
        game_object(game_object const &)            = delete;
        game_object &operator=(game_object const &) = delete;
        game_object(game_object &&)                 = default;
        game_object &operator=(game_object &&)      = default;

        [[nodiscard]] auto id() const -> id_t { return id_; }
        [[nodiscard]] auto name() const -> std::string { return name_; }
        [[nodiscard]] auto material() const -> material const & { return material_; }
        void set_material(float r, float g, float b, float metallic, float roughness)
        {
            material_ = dae::material{glm::vec3{r, g, b}, metallic, roughness};
        }

    public:
        std::unique_ptr<model> model     = {};
        glm::vec3              color     = {};
        transform_component    transform = {};

        std::unique_ptr<point_light_component> point_light = nullptr;

    private:
        id_t          id_;
        std::string   name_;
        dae::material material_ = {};

        static id_t next_id_;
    };
}
