#include "engine.h"

// Project includes
#include "src/core/factory.h"
#include "src/engine/camera.h"
#include "src/engine/game_time.h"
#include "src/input/movement_controller.h"
#include "src/input/shading_mode_controller.h"
#include "src/system/pbr_system.h"
#include "src/system/point_light_system.h"
#include "src/system/render_system_2d.h"
#include "src/system/render_system_3d.h"
#include "src/utility/texture.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/device.h"
#include "src/vulkan/renderer.h"

// Standard includes
#include <chrono>
#include <thread>

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace dae
{
    engine::engine()
    {
        window_ptr_= &window::instance();
        window_ptr_->init(width, height, "Hello Vulkan!");

        device_ptr_ = &device::instance();
        device_ptr_->init(window_ptr_);

        renderer_ptr_ = &renderer::instance();
        renderer_ptr_->init(window_ptr_, device_ptr_);
        
        global_pool_ = descriptor_pool::builder(device_ptr_)
                       .set_max_sets(swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .build();
        
        load_game_objects();
    }

    void engine::run(std::function<void()> const &load)
    {
        load();
        
        std::vector<std::unique_ptr<buffer>> ubo_buffers(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); ++i)
        {
            ubo_buffers[i] = std::make_unique<buffer>(
                device_ptr_,
                sizeof(global_ubo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            ubo_buffers[i]->map();
        }

        auto global_set_layout = descriptor_set_layout::builder(device_ptr_)
                                 .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                 .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .build();


        texture diffuse_texture{device_ptr_, "data/assets/textures/vehicle_diffuse.png", VK_FORMAT_R8G8B8A8_SRGB};
        texture normal_texture{device_ptr_, "data/assets/textures/vehicle_normal.png", VK_FORMAT_R8G8B8A8_UNORM};
        texture specular_texture{device_ptr_, "data/assets/textures/vehicle_specular.png", VK_FORMAT_R8G8B8A8_SRGB};
        texture gloss_texture{device_ptr_, "data/assets/textures/vehicle_gloss.png", VK_FORMAT_R8G8B8A8_SRGB};

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
            descriptor_writer(global_set_layout.get(), global_pool_.get())
                .write_buffer(0, &buffer_info)
                .write_image(1, &diffuse_image_info)
                .write_image(2, &normal_image_info)
                .write_image(3, &specular_image_info)
                .write_image(4, &emission_image_info)
                .build(global_descriptor_sets[i]);
        }
        
        render_system_3d render_system_3d {device_ptr_, renderer_ptr_->swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        render_system_2d render_system_2d {device_ptr_, renderer_ptr_->swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        point_light_system point_light_system {device_ptr_, renderer_ptr_->swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        pbr_system pbr_system {device_ptr_, renderer_ptr_->swap_chain_render_pass(), global_set_layout->get_descriptor_set_layout()};
        camera camera{};
        // camera.set_view_direction(glm::vec3{0.0f}, glm::vec3{2.5f, 0.0f, 1.0f});
        // camera.set_view_target(glm::vec3{0.0f, -1.5f, -5.0f}, glm::vec3{0.0f, 0.0f, 0.0f});

        auto viewer_object = game_object::create_game_object("viewer");
        viewer_object.transform.translation = {0.0f, -1.5f, -5.0f};
        viewer_object.transform.rotation = {-0.2f, 0.0f, 0.0f};
        movement_controller camera_controller = {};
        glfwSetKeyCallback(window_ptr_->get_glfw_window(), shading_mode_controller::key_callback);
        
        global_ubo ubo{};
        auto &frame_info = frame_info::instance();

        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto last_time = high_resolution_clock::now();
        float lag         = 0.0f;

        //---------------------------------------------------------
        // GAME LOOP
        //---------------------------------------------------------
        while (not window_ptr_->should_close())
        {
            glfwPollEvents();

            auto current_time = high_resolution_clock::now();
            game_time::instance().set_delta_time(duration<float>(current_time - last_time).count()); // dt always has a 1 frame delay
            
            last_time = current_time;
            lag += game_time::instance().delta_time();

            camera_controller.move(window_ptr_->get_glfw_window(), viewer_object);
            camera.set_view_yxz(viewer_object.transform.translation, viewer_object.transform.rotation);
            
            float aspect = renderer_ptr_->aspect_ratio();
            camera.set_orthographic_projection(-aspect, aspect, -1, 1, -1, 1);
            camera.set_perspective_projection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

            if (auto command_buffer = renderer_ptr_->begin_frame())
            {
                int frame_index = renderer_ptr_->frame_index();
                
                frame_info.frame_index = frame_index;
                frame_info.command_buffer = command_buffer;
                frame_info.camera_ptr = &camera;
                frame_info.global_descriptor_set = global_descriptor_sets[frame_index];
                frame_info.game_objects_ptr = &game_objects_;
                frame_info.ubo_ptr = &ubo;
                
                // update
                ubo.projection = camera.get_projection();
                ubo.view = camera.get_view();
                ubo.inverse_view = camera.get_inverse_view();
                
                point_light_system.update();
                
                ubo_buffers[frame_index]->write_to_buffer(&ubo);
                ubo_buffers[frame_index]->flush();
                
                // render
                renderer_ptr_->begin_swap_chain_render_pass(command_buffer);
                render_system_2d.render();
                render_system_3d.render();
                pbr_system.render();
                point_light_system.render();
                
                renderer_ptr_->end_swap_chain_render_pass(command_buffer);
                renderer_ptr_->end_frame();
                
                auto const sleep_time = current_time + milliseconds(static_cast<long long>(game_time::instance().ms_per_frame())) - high_resolution_clock::now();
                std::this_thread::sleep_for(sleep_time);
            }
        }
        vkDeviceWaitIdle(device_ptr_->logical_device());
    }

    void engine::load_game_objects()
    {
        std::shared_ptr<model> model = model::create_model_from_file(device_ptr_, "data/assets/models/suzanne.obj");
        auto go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {-1.2f, 0.0f, 2.5f};
        go.transform.scale = glm::vec3{-0.5f};
        game_objects_.emplace(go.id(), std::move(go));
        
        model = model::create_model_from_file(device_ptr_, "data/assets/models/beetle.obj");
        go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {-0.2f, 2.0f, 1.5f};
        go.transform.scale = glm::vec3{-5.8f};
        game_objects_.emplace(go.id(), std::move(go));
        
        model = model::create_model_from_file(device_ptr_, "data/assets/models/quad.obj");
        go = game_object::create_game_object("3d");
        go.model = model;
        go.transform.translation = {0.0f, 0.0f, 0.0f};
        go.transform.scale = glm::vec3{3};
        game_objects_.emplace(go.id(), std::move(go));
        
        model = model::create_model_from_file(device_ptr_, "data/assets/models/vehicle.obj");
        go = game_object::create_game_object("pbr");
        go.model = model;
        go.transform.translation = {0.0f, -2.2f, 0.0f};
        go.transform.scale = glm::vec3{0.1f};
        go.set_material(0.4f, 0.96f, 0.915f, 1.0f, 0.0f, 0.6f);
        game_objects_.emplace(go.id(), std::move(go));

        go = game_object::make_point_light(0.2f);
        game_objects_.emplace(go.id(), std::move(go));
        
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
            game_objects_.emplace(point_light.id(), std::move(point_light));
        }


        go  = game_object::create_game_object("2d");
        model = factory::create_oval(device_ptr_, {}, 0.5f, 0.5f, 50);
        go.model = model;
        go.transform.translation = {2.0f, -1.0f, 0.0f};
        game_objects_.emplace(go.id(), std::move(go));
        
        go  = game_object::create_game_object("2d");
        model = factory::create_n_gon(device_ptr_, {}, 0.5f, 3);
        go.model = model;
        go.transform.translation = {-2.0f, -1.0f, 0.0f};
        game_objects_.emplace(go.id(), std::move(go));
    }
}
