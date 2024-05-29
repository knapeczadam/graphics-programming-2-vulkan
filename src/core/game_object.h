#pragma once

// Project includes
#include "src/core/material.h"
#include "src/core/model.h"

// Standard includes
#include <memory>
#include <string>
#include <unordered_map>

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

    class game_object
    {
    public:
        // Type aliases
        using id_t = unsigned int;
        using map = std::unordered_map<id_t, game_object>;
        
    public:
        game_object(game_object const &)            = delete;
        game_object &operator=(game_object const &) = delete;
        game_object(game_object &&)                 = default;
        game_object &operator=(game_object &&)      = default;

        
        static auto create_game_object(std::string const &name) -> game_object
        {
            static id_t current_id = 0;
            return game_object{current_id++, name};
        }

        static auto make_point_light(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3{1.0f}) -> game_object;

        [[nodiscard]] auto get_id() const -> id_t { return id_; }
        [[nodiscard]] auto get_name() const -> std::string { return name_; }
        [[nodiscard]] auto get_material() const -> material const & { return material_; }
        void set_material(float r, float g, float b, float a, float metallic, float roughness)
        {
            material_ = material{glm::vec4{r, g, b, a}, metallic, roughness};
        }

    private:
        game_object(id_t id, std::string name) : id_{id}, name_{std::move(name)} {}

    public:
        std::shared_ptr<model> model     = {};
        glm::vec3                  color     = {};
        transform_component        transform = {};

        std::unique_ptr<point_light_component> point_light = nullptr;

    private:
        id_t        id_;
        std::string name_;
        material    material_ = {};
    };
}
