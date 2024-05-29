#include "app.h"

// Project includes
#include "engine/buffer.h"
#include "engine/camera.h"
#include "input/movement_controller.h"
#include "system/pbr_system.h"
#include "system/point_light_system.h"
#include "system/render_system_2d.h"
#include "system/render_system_3d.h"
#include "util/texture.h"

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
    app::app()
    {
        global_pool_ = descriptor_pool::builder(device_)
                       .set_max_sets(swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .build();
        
        load_game_objects();
    }

    app::~app()
    {
    }

    void app::run()
    {
        std::vector<std::unique_ptr<buffer>> ubo_buffers(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); ++i)
        {
            ubo_buffers[i] = std::make_unique<buffer>(
                device_,
                sizeof(global_ubo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            ubo_buffers[i]->map();
        }

        auto global_set_layout = descriptor_set_layout::builder(device_)
                                 .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                 .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .build();

        texture diffuse_texture{device_, "textures/vehicle_diffuse.png", VK_FORMAT_B8G8R8A8_UNORM};
        texture normal_texture{device_, "textures/vehicle_normal.png", VK_FORMAT_R8G8B8A8_SRGB};
        texture specular_texture{device_, "textures/vehicle_specular.png", VK_FORMAT_R8G8B8A8_UNORM};
        texture gloss_texture{device_, "textures/vehicle_gloss.png", VK_FORMAT_R8G8B8A8_UNORM};

        VkDescriptorImageInfo diffuse_image_info{};
        diffuse_image_info.sampler     = diffuse_texture.sampler();
        diffuse_image_info.imageView   = diffuse_texture.image_view();
        diffuse_image_info.imageLayout = diffuse_texture.image_layout();

        VkDescriptorImageInfo normal_image_info{};
        normal_image_info.sampler     = normal_texture.sampler();
        normal_image_info.imageView   = normal_texture.image_view();
        normal_image_info.imageLayout = normal_texture.image_layout();

        VkDescriptorImageInfo specular_image_info{};
        specular_image_info.sampler     = specular_texture.sampler();
        specular_image_info.imageView   = specular_texture.image_view();
        specular_image_info.imageLayout = specular_texture.image_layout();

        VkDescriptorImageInfo emission_image_info{};
        emission_image_info.sampler     = gloss_texture.sampler();
        emission_image_info.imageView   = gloss_texture.image_view();
        emission_image_info.imageLayout = gloss_texture.image_layout();
        

        std::vector<VkDescriptorSet> global_descriptor_sets(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < global_descriptor_sets.size(); ++i)
        {
            auto buffer_info = ubo_buffers[i]->descriptor_info();
            descriptor_writer(*global_set_layout, *global_pool_)
                .write_buffer(0, &buffer_info)
                .write_image(1, &diffuse_image_info)
                .write_image(2, &normal_image_info)
                .write_image(3, &specular_image_info)
                .write_image(4, &emission_image_info)
                .build(global_descriptor_sets[i]);
        }
        
        render_system_3d render_system_3d {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        render_system_2d render_system_2d {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        point_light_system point_light_system {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        pbr_system pbr_system {device_, renderer_.get_swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        camera camera{};
        // camera.set_view_direction(glm::vec3{0.0f}, glm::vec3{2.5f, 0.0f, 1.0f});
        // camera.set_view_target(glm::vec3{0.0f, -1.5f, -5.0f}, glm::vec3{0.0f, 0.0f, 0.0f});

        auto viewer_object = game_object::create_game_object("viewer");
        viewer_object.transform.translation = {0.0f, -1.5f, -5.0f};
        viewer_object.transform.rotation = {-0.2f, 0.0f, 0.0f};
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
                ubo.inverse_view = camera.get_inverse_view();
                point_light_system.update(frame_info, ubo);
                ubo_buffers[frame_index]->write_to_buffer(&ubo);
                ubo_buffers[frame_index]->flush();
                
                // render
                renderer_.begin_swap_chain_render_pass(command_buffer);
                render_system_2d.render(frame_info);
                render_system_3d.render_game_objects(frame_info);
                pbr_system.render_game_objects(frame_info);
                point_light_system.render(frame_info);
                renderer_.end_swap_chain_render_pass(command_buffer);
                renderer_.end_frame();
            }
        }
        vkDeviceWaitIdle(device_.get_logical_device());
    }
    
    std::unique_ptr<model> create_oval_model(device &device, glm::vec3 offset, float radiusX, float radiusY,
                                                 int segments)
    {
        model::builder modelBuilder{};

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

        return std::make_unique<model>(device, modelBuilder);
    }

    std::unique_ptr<model> create_n_gon_model(device &device, glm::vec3 offset, float radius, int sides)
    {
        model::builder modelBuilder{};

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

        return std::make_unique<model>(device, modelBuilder);
    }
    
    void app::load_game_objects()
    {
        std::shared_ptr<model> model = model::create_model_from_file(device_, "models/suzanne.obj");
        auto go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {-1.2f, 0.0f, 2.5f};
        go.transform.scale = glm::vec3{-0.5f};
        game_objects_.emplace(go.get_id(), std::move(go));
        
        model = model::create_model_from_file(device_, "models/beetle.obj");
        go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {-0.2f, 2.0f, 1.5f};
        go.transform.scale = glm::vec3{-5.8f};
        game_objects_.emplace(go.get_id(), std::move(go));
        
        model = model::create_model_from_file(device_, "models/quad.obj");
        go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {0.0f, 0.0f, 0.0f};
        go.transform.scale = glm::vec3{3};
        game_objects_.emplace(go.get_id(), std::move(go));
        
        model = model::create_model_from_file(device_, "models/sphere.obj");
        go = game_object::create_game_object("pbr");
        go.model = model;
        go.transform.translation = {0.0f, -2.2f, 0.0f};
        go.transform.scale = glm::vec3{0.1f};
        go.set_material(0.4f, 0.96f, 0.915f, 1.0f, 0.0f, 0.6f);
        game_objects_.emplace(go.get_id(), std::move(go));

        go = game_object::make_point_light(0.2f);
        game_objects_.emplace(go.get_id(), std::move(go));
        
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
            auto point_light = game_object::make_point_light(0.2f);
            point_light.color = light_colors[i];
            auto rotate_light = glm::rotate(
                glm::mat4{1.0f},
                (i * glm::two_pi<float>()) / light_colors.size(),
                {0.0f, -1.0f, 0.0f}
            );
            point_light.transform.translation = glm::vec3{rotate_light * glm::vec4{-1.0f, -1.0f, 0.0f, 1.0f}};
            game_objects_.emplace(point_light.get_id(), std::move(point_light));
        }


        go  = game_object::create_game_object("2d");
        model = create_oval_model(device_, {}, 0.5f, 0.5f, 50);
        go.model = model;
        go.transform.translation = {2.0f, -1.0f, 0.0f};
        game_objects_.emplace(go.get_id(), std::move(go));
        
        go  = game_object::create_game_object("2d");
        model = create_n_gon_model(device_, {}, 0.5f, 3);
        go.model = model;
        go.transform.translation = {-2.0f, -1.0f, 0.0f};
        game_objects_.emplace(go.get_id(), std::move(go));
    }
}
