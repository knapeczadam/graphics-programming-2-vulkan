#include "scene_loader.h"


#include "scene.h"
#include "scene_config_manager.h"
#include "scene_manager.h"
#include "src/core/factory.h"

namespace dae
{
    void scene_loader::load_scenes()
    {
        load_2d_scene();
        load_3d_scene();
        load_light_scene();
        load_pbr_scene();
    }

    void scene_loader::load_2d_scene()
    {
        auto scene_ptr = scene_manager::instance().find("2d");
        auto go_ptr = scene_ptr->create_game_object("oval");
        go_ptr->model = factory::create_oval({}, 0.5f, 0.5f, 50);
        go_ptr->transform.translation = {2.0f, -1.0f, 0.0f};

        go_ptr = scene_ptr->create_game_object("ngon");
        go_ptr->model = factory::create_n_gon({}, 0.5f, 3);
        go_ptr->transform.translation = {-2.0f, -1.0f, 0.0f};
    }

    void scene_loader::load_3d_scene()
    {
        auto scene_ptr = scene_manager::instance().find("3d");
        auto go_ptr = scene_ptr->create_game_object("suzanne");
        go_ptr->model = model::create_model("data/assets/models/suzanne.obj");
        go_ptr->transform.translation = {-1.2f, 0.0f, 2.5f};
        go_ptr->transform.scale = glm::vec3{-0.5f};
        
        go_ptr = scene_ptr->create_game_object("beetle");
        go_ptr->model = model::create_model("data/assets/models/beetle.obj");
        go_ptr->transform.translation = {-0.2f, 2.0f, 1.5f};
        go_ptr->transform.scale = glm::vec3{-5.8f};

        go_ptr = scene_ptr->create_game_object("quad");
        go_ptr->model = model::create_model("data/assets/models/quad.obj");
        go_ptr->transform.translation = {0.0f, 0.0f, 0.0f};
        go_ptr->transform.scale = glm::vec3{3};
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

    void scene_loader::load_pbr_scene()
    {
        auto const &scene_config = scene_config_manager::instance().scene_config();
        for (auto const &object : scene_config["objects"])
        {
            
        }
        
        auto scene_ptr = scene_manager::instance().find("pbr");
        auto go_ptr = scene_ptr->create_game_object("vehicle");
        go_ptr->model = model::create_model("data/assets/models/vehicle.obj");
        go_ptr->transform.translation = {0.0f, -2.2f, 0.0f};
        go_ptr->transform.scale = glm::vec3{0.1f};
        go_ptr->set_material(0.4f, 0.96f, 0.915f, 1.0f, 0.0f, 0.6f);
    }
}
