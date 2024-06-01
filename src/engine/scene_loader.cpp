#include "scene_loader.h"

// Project includes
#include "src/core/factory.h"
#include "src/engine/scene.h"
#include "src/engine/scene_config_manager.h"
#include "src/engine/scene_manager.h"

namespace dae
{
    void scene_loader::load_scenes()
    {
        load_2d_scene();
        load_3d_scene();
        load_light_scene();
        load_material_pbr_scene();
        load_texture_pbr_scene();
    }

    void scene_loader::load_2d_scene()
    {
        auto scene_ptr = scene_manager::instance().find("2d");
        auto go_ptr = scene_ptr->create_game_object("oval");
        go_ptr->model = factory::create_oval({}, 0.5f, 0.5f, 50);
        go_ptr->transform.translation = {2.0f, -1.0f, 0.0f};
        
        scene_ptr = scene_manager::instance().find("2d");
        auto const &scene_config = scene_config_manager::instance().scene_config();
        
        go_ptr = scene_ptr->create_game_object("oval");
        go_ptr->model = factory::create_oval({}, 0.5f, 0.5f, 30);
        go_ptr->transform.translation = {2.0f, -1.0f, 2.0f};
        go_ptr->transform.rotation = {0.0f, 0.0f, glm::pi<float>() / 2.0f};

        go_ptr = scene_ptr->create_game_object("ngon");
        go_ptr->model = factory::create_n_gon({}, 0.5f, 3);
        go_ptr->transform.translation = {-2.0f, -1.0f, 0.0f};
        
        
        for (auto const &object : scene_config["2d"])
        {
            std::string name = object.contains("name") ? object["name"] : "game_object";
            auto go_ptr = scene_ptr->create_game_object(name);
            if (object.contains("transform"))
            {
                auto transform = object["transform"];
                glm::vec3 position = transform.contains("position") ? glm::vec3{transform["position"][0], transform["position"][1], transform["position"][2]} : glm::vec3{0.0f};
                glm::vec3 rotation = transform.contains("rotation") ? glm::vec3{transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]} : glm::vec3{0.0f};
                glm::vec3 scale = glm::vec3{1.0f};
                if (transform.contains("scale"))
                {
                    if (transform["scale"].is_number())
                    {
                        scale = glm::vec3{transform["scale"]};
                    }
                    else if (transform["scale"].is_array())
                    {
                        scale = glm::vec3{transform["scale"][0], transform["scale"][1], transform["scale"][2]};
                    }
                }
                go_ptr->transform.translation = position;
                go_ptr->transform.rotation = rotation;
                go_ptr->transform.scale = scale;
            }
            if (object.contains("model"))
            {
                go_ptr->model = model::create_model(object["model"]);
            }
            if (object.contains("texture"))
            {
                texture_path_ = object["texture"];
                go_ptr->use_texture = true;
            }
        }
    }

    void scene_loader::load_3d_scene()
    {
        auto scene_ptr = scene_manager::instance().find("3d");
        auto const &scene_config = scene_config_manager::instance().scene_config();
        
        for (auto const &object : scene_config["3d"])
        {
            std::string name = object.contains("name") ? object["name"] : "game_object";
            auto go_ptr = scene_ptr->create_game_object(name);
            if (object.contains("transform"))
            {
                auto transform = object["transform"];
                glm::vec3 position = transform.contains("position") ? glm::vec3{transform["position"][0], transform["position"][1], transform["position"][2]} : glm::vec3{0.0f};
                glm::vec3 rotation = transform.contains("rotation") ? glm::vec3{transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]} : glm::vec3{0.0f};
                glm::vec3 scale = glm::vec3{1.0f};
                if (transform.contains("scale"))
                {
                    if (transform["scale"].is_number())
                    {
                        scale = glm::vec3{transform["scale"]};
                    }
                    else if (transform["scale"].is_array())
                    {
                        scale = glm::vec3{transform["scale"][0], transform["scale"][1], transform["scale"][2]};
                    }
                }
                go_ptr->transform.translation = position;
                go_ptr->transform.rotation = rotation;
                go_ptr->transform.scale = scale;
            }
            if (object.contains("model"))
            {
                go_ptr->model = model::create_model(object["model"]);
            }
        }
    }

    void scene_loader::load_light_scene()
    {
        auto scene_ptr = scene_manager::instance().find("light");
        
        std::vector<glm::vec3> light_colors{
                {1.f, .1f, .1f},
                {.1f, .1f, 1.f},
                {.1f, 1.f, .1f},
                {1.f, 1.f, .1f},
                {.1f, 1.f, 1.f},
                {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < light_colors.size(); ++i)
        {
            auto go_ptr = scene_ptr->create_game_object("point_light");
            go_ptr->color = light_colors[i];
            go_ptr->point_light = std::make_unique<point_light_component>();
            go_ptr->point_light->light_intensity = 0.2f;
            auto rotate_light = glm::rotate(
                glm::mat4{1.0f},
                (i * glm::two_pi<float>()) / light_colors.size(),
                {0.0f, -1.0f, 0.0f}
            );
            go_ptr->transform.translation = glm::vec3{rotate_light * glm::vec4{-1.0f, -1.0f, 0.0f, 1.0f}};
            go_ptr->transform.scale = glm::vec3{0.1f};
        }
    }

    void scene_loader::load_material_pbr_scene()
    {
        auto scene_ptr = scene_manager::instance().find("material_pbr");
        auto const &scene_config = scene_config_manager::instance().scene_config();
        
        for (auto const &object : scene_config["material_pbr"])
        {
            std::string name = object.contains("name") ? object["name"] : "game_object";
            auto go_ptr = scene_ptr->create_game_object(name);
            if (object.contains("transform"))
            {
                auto transform = object["transform"];
                glm::vec3 position = transform.contains("position") ? glm::vec3{transform["position"][0], transform["position"][1], transform["position"][2]} : glm::vec3{0.0f};
                glm::vec3 rotation = transform.contains("rotation") ? glm::vec3{transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]} : glm::vec3{0.0f};
                glm::vec3 scale = glm::vec3{1.0f};
                if (transform.contains("scale"))
                {
                    if (transform["scale"].is_number())
                    {
                        scale = glm::vec3{transform["scale"]};
                    }
                    else if (transform["scale"].is_array())
                    {
                        scale = glm::vec3{transform["scale"][0], transform["scale"][1], transform["scale"][2]};
                    }
                }
                go_ptr->transform.translation = position;
                go_ptr->transform.rotation = rotation;
                go_ptr->transform.scale = scale;
            }
            if (object.contains("model"))
            {
                go_ptr->model = model::create_model(object["model"]);
            }

            float r, g, b;
            r = g = b = 1.0f;
            if (object.contains("base_color"))
            {
                if (object["base_color"].is_number())
                {
                    r = g = b = object["base_color"];
                }
                else if (object["base_color"].is_array())
                {
                    r = object["base_color"][0];
                    g = object["base_color"][1];
                    b = object["base_color"][2];
                }
            }
            float metallic = object.contains("metallic") ? static_cast<float>(object["metallic"]) : 0.0f;
            float roughness = object.contains("roughness") ? static_cast<float>(object["roughness"]) : 0.0f;
            go_ptr->set_material(r, g, b, metallic, roughness);
        }
    }

    void scene_loader::load_texture_pbr_scene()
    {
        auto scene_ptr = scene_manager::instance().find("texture_pbr");
        auto const &scene_config = scene_config_manager::instance().scene_config();
        
        for (auto const &object : scene_config["texture_pbr"])
        {
            std::string name = object.contains("name") ? object["name"] : "game_object";
            auto go_ptr = scene_ptr->create_game_object(name);
            if (object.contains("transform"))
            {
                auto transform = object["transform"];
                glm::vec3 position = transform.contains("position") ? glm::vec3{transform["position"][0], transform["position"][1], transform["position"][2]} : glm::vec3{0.0f};
                glm::vec3 rotation = transform.contains("rotation") ? glm::vec3{transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]} : glm::vec3{0.0f};
                glm::vec3 scale = glm::vec3{1.0f};
                if (transform.contains("scale"))
                {
                    if (transform["scale"].is_number())
                    {
                        scale = glm::vec3{transform["scale"]};
                    }
                    else if (transform["scale"].is_array())
                    {
                        scale = glm::vec3{transform["scale"][0], transform["scale"][1], transform["scale"][2]};
                    }
                }
                go_ptr->transform.translation = position;
                go_ptr->transform.rotation = rotation;
                go_ptr->transform.scale = scale;
            }
            if (object.contains("model"))
            {
                go_ptr->model = model::create_model(object["model"]);
            }
            if (object.contains("textures"))
            {
                auto textures = object["textures"];
                if (textures.contains("diffuse"))
                {
                    diffuse_texture_path_ = textures["diffuse"];
                }
                if (textures.contains("normal"))
                {
                    normal_texture_path_ = textures["normal"];
                }
                if (textures.contains("specular"))
                {
                    specular_texture_path_ = textures["specular"];
                }
                if (textures.contains("glossiness"))
                {
                    glossiness_texture_path_ = textures["glossiness"];
                }
            }
        }
    }
}
