#include "lve_app.h"

// Project includes
#include "lve_camera.h"
#include "systems/render_system_2d.h"
#include "systems/render_system_3d.h"
#include "systems/point_light_system.h"
#include "movement_controller.h"
#include "lve_buffer.h"

// Standard includes
#include <array>
#include <chrono>
#include <stdexcept>

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace dae
{
    lve_app::lve_app()
    {
        global_pool_ = lve_descriptor_pool::builder(device_)
                       .set_max_sets(lve_swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lve_swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .build();
        
        load_game_objects();
    }

    lve_app::~lve_app()
    {
    }

    void lve_app::run()
    {
        std::vector<std::unique_ptr<lve_buffer>> ubo_buffers(lve_swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); ++i)
        {
            ubo_buffers[i] = std::make_unique<lve_buffer>(
                device_,
                sizeof(global_ubo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            ubo_buffers[i]->map();
        }

        auto global_set_layout = lve_descriptor_set_layout::builder(device_)
                                 .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                 .build();

        std::vector<VkDescriptorSet> global_descriptor_sets(lve_swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < global_descriptor_sets.size(); ++i)
        {
            auto buffer_info = ubo_buffers[i]->descriptor_info();
            lve_descriptor_writer(*global_set_layout, *global_pool_)
                .write_buffer(0, &buffer_info)
                .build(global_descriptor_sets[i]);
        }
        
        render_system_3d render_system_3d {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        render_system_2d render_system_2d {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        point_light_system point_light_system {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        lve_camera camera{};
        // camera.set_view_direction(glm::vec3{0.0f}, glm::vec3{0.5f, 0.0f, 1.0f});
        camera.set_view_target(glm::vec3{-1.0f, -2.0f, -20.0f}, glm::vec3{2.0f, -2.0f, 2.5f});

        auto viewer_object = lve_game_object::create_game_object("viewer");
        viewer_object.transform.translation.z = -2.5f;
        movement_controller camera_controller = {};

        auto current_time = std::chrono::high_resolution_clock::now();

        //---------------------------------------------------------
        // GAME LOOP
        //---------------------------------------------------------
        while (not window_.should_close())
        {
            glfwPollEvents();

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            // frame_time = glm::min(frame_time, max_frame_time);

            camera_controller.move(window_.get_glfw_window(), frame_time, viewer_object);
            camera.set_view_yxz(viewer_object.transform.translation, viewer_object.transform.rotation);
            
            float aspect = renderer_.get_aspect_ratio();
            camera.set_orthographic_projection(-aspect, aspect, -1, 1, -1, 1);
            camera.set_perspective_projection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

            if (auto command_buffer = renderer_.begin_frame())
            {
                int frame_index = renderer_.get_frame_index();
                
                frame_info frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera,
                    global_descriptor_sets[frame_index],
                    game_objects_
                };
                
                // update
                global_ubo ubo{};
                ubo.projection = camera.get_projection();
                ubo.view = camera.get_view();
                point_light_system.update(frame_info, ubo);
                ubo_buffers[frame_index]->write_to_buffer(&ubo);
                ubo_buffers[frame_index]->flush();
                
                // render
                renderer_.begin_swap_chain_render_pass(command_buffer);
                render_system_2d.render(frame_info);
                render_system_3d.render_game_objects(frame_info);
                point_light_system.render(frame_info);
                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }
        vkDeviceWaitIdle(device_.device());
    }
    std::unique_ptr<lve_model> createCubeModel(lve_device& device, glm::vec3 offset) {
      lve_model::lve_builder modelBuilder{};
      modelBuilder.vertices = {
          // left face (white)
          {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
     
          // right face (yellow)
          {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
     
          // top face (orange, remember y axis points down)
          {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
     
          // bottom face (red)
          {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
     
          // nose face (blue)
          {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
     
          // tail face (green)
          {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      };
      for (auto& v : modelBuilder.vertices) {
        v.position += offset;
      }

      modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                              12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
     
      return std::make_unique<lve_model>(device, modelBuilder);
    }

    std::unique_ptr<lve_model> create_oval_model(lve_device &device, glm::vec3 offset, float radiusX, float radiusY,
                                                 int segments)
    {
        lve_model::lve_builder modelBuilder{};

        // Generate vertices for the oval shape
        for (int i = 0; i <= segments; ++i)
        {
            float angle = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            float x = radiusX * std::cos(angle);
            float y = radiusY * std::sin(angle);
            float r = std::rand() % 101 / 100.0f;
            float g = std::rand() % 101 / 100.0f;
            float b = std::rand() % 101 / 100.0f;
            modelBuilder.vertices.push_back({{x + offset.x, y + offset.y, offset.z}, {r, g, b}});
        }

        // Generate indices for the triangles forming the oval shape
        for (int i = 1; i < segments; ++i)
        {
            modelBuilder.indices.push_back(0);
            modelBuilder.indices.push_back(i);
            modelBuilder.indices.push_back(i + 1);
        }

        // Close the oval by connecting the last and first vertices
        modelBuilder.indices.push_back(0);
        modelBuilder.indices.push_back(segments);
        modelBuilder.indices.push_back(1);

        return std::make_unique<lve_model>(device, modelBuilder);
    }

    std::unique_ptr<lve_model> create_n_gon_model(lve_device &device, glm::vec3 offset, float radius, int sides)
    {
        lve_model::lve_builder modelBuilder{};

        // Calculate the angle between each vertex based on the number of sides
        float angleIncrement = glm::two_pi<float>() / static_cast<float>(sides);

        // Generate vertices for the n-gon shape
        for (int i = 0; i < sides; ++i)
        {
            float angle = angleIncrement * static_cast<float>(i);
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle);
            float r = std::rand() % 101 / 100.0f;
            float g = std::rand() % 101 / 100.0f;
            float b = std::rand() % 101 / 100.0f;
            modelBuilder.vertices.push_back({{x + offset.x, y + offset.y, offset.z}, {r, g, b}});
        }

        // Generate indices for the triangles forming the n-gon shape
        for (int i = 1; i < sides - 1; ++i)
        {
            modelBuilder.indices.push_back(0);
            modelBuilder.indices.push_back(i);
            modelBuilder.indices.push_back(i + 1);
        }

        // Close the n-gon by connecting the last and first vertices
        modelBuilder.indices.push_back(0);
        modelBuilder.indices.push_back(sides - 1);
        modelBuilder.indices.push_back(1);

        return std::make_unique<lve_model>(device, modelBuilder);
    }
    
    void lve_app::load_game_objects()
    {
        std::shared_ptr<lve_model> model = lve_model::create_model_from_file(device_, "models/smooth_vase.obj");
        auto go1 = lve_game_object::create_game_object("3d");
        go1.model = model;
        go1.transform.translation = {-0.5f, 0.5f, 0.0f};
        go1.transform.scale = glm::vec3{3.0f, 1.5f, 2.0f};
        game_objects_.emplace(go1.get_id(), std::move(go1));
        
        model = lve_model::create_model_from_file(device_, "models/flat_vase.obj");
        auto go2 = lve_game_object::create_game_object("3d");
        go2.model = model;
        go2.transform.translation = {0.5f, 0.5f, 0.0f};
        go2.transform.scale = glm::vec3{3};
        game_objects_.emplace(go2.get_id(), std::move(go2));
        
        model = lve_model::create_model_from_file(device_, "models/quad.obj");
        auto go3 = lve_game_object::create_game_object("3d");
        go3.model = model;
        go3.transform.translation = {0.0f, 0.5f, 0.0f};
        go3.transform.scale = glm::vec3{3};
        game_objects_.emplace(go3.get_id(), std::move(go3));

        auto point_light = lve_game_object::make_point_light(0.2f);
        game_objects_.emplace(point_light.get_id(), std::move(point_light));
        
        std::vector<glm::vec3> light_colors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f} //
        };

        for (int i = 0; i < light_colors.size(); ++i)
        {
            auto point_light = lve_game_object::make_point_light(0.2f);
            point_light.color = light_colors[i];
            auto rotate_light = glm::rotate(
                glm::mat4{1.0f},
                (i * glm::two_pi<float>()) / light_colors.size(),
                {0.0f, -1.0f, 0.0f}
            );
            point_light.transform.translation = glm::vec3{rotate_light * glm::vec4{-1.0f, -1.0f, -1.0f, 1.0f}};
            game_objects_.emplace(point_light.get_id(), std::move(point_light));
        }


        // model = lve_model::create_model_from_vertices(device_, vertices);;
        model = create_oval_model(device_, {}, 0.5f, 0.5f, 50);
        auto go4  = lve_game_object::create_game_object("2d");
        go4.model = model;
        go4.transform.translation = {0.0f, 0.0f, -2.0f};
        // go4.color = {1.0f, 0.0f, 1.0f};
        // go4.transform.rotation = glm::vec3{.25f * glm::two_pi<float>()};
        game_objects_.emplace(go4.get_id(), std::move(go4));
        
        // model = lve_model::create_model_from_vertices(device_, vertices);;
        model = create_n_gon_model(device_, {}, 0.5f, 3);
        auto go5  = lve_game_object::create_game_object("2d");
        go5.model = model;
        go5.transform.translation = {0.0f, 0.0f, +2.0f};
        // go4.color = {1.0f, 0.0f, 1.0f};
        // go4.transform.rotation = glm::vec3{.25f * glm::two_pi<float>()};
        game_objects_.emplace(go5.get_id(), std::move(go5));
    }
}
