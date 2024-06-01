#include "engine.h"

// Project includes
#include "src/core/factory.h"
#include "src/engine/camera.h"
#include "src/engine/frame_info.h"
#include "src/engine/game_time.h"
#include "src/engine/scene_manager.h"
#include "src/input/movement_controller.h"
#include "src/input/shading_mode_controller.h"
#include "src/system/point_light_system.h"
#include "src/system/render_2d_system.h"
#include "src/system/render_3d_system.h"
#include "src/system/texture_pbr_system.h"
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

#include "scene_loader.h"
#include "src/system/material_pbr_system.h"

namespace dae
{
    std::string engine::data_path;
    
    engine::engine(std::string const &path)
    {
        data_path = path;
        
        window_ptr_= &window::instance();
        window_ptr_->init(width, height, "Graphics Programming 2 | Adam Knapecz");

        device_ptr_   = &device::instance();
        renderer_ptr_ = &renderer::instance();
        
        global_pool_ = descriptor_pool::builder()
                       .set_max_sets(swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swap_chain::MAX_FRAMES_IN_FLIGHT)
                       .build();
    }

    void engine::run(std::function<void()> const &load)
    {
        std::vector<std::unique_ptr<buffer>> ubo_buffers(swap_chain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); ++i)
        {
            ubo_buffers[i] = std::make_unique<buffer>(
                sizeof(global_ubo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            ubo_buffers[i]->map();
        }

        auto global_set_layout = descriptor_set_layout::builder()
                                 .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                 .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .add_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                 .build();

        // scenes
        auto &scene_manager = scene_manager::instance();
        scene_manager.create_scene("2d", std::make_unique<render_2d_system>(global_set_layout->get_descriptor_set_layout()));
        scene_manager.create_scene("3d", std::make_unique<render_3d_system>(global_set_layout->get_descriptor_set_layout()));
        scene_manager.create_scene("material_pbr", std::make_unique<material_pbr_system>(global_set_layout->get_descriptor_set_layout()));
        scene_manager.create_scene("texture_pbr", std::make_unique<texture_pbr_system>(global_set_layout->get_descriptor_set_layout()));
        scene_manager.create_scene("light", std::make_unique<point_light_system>(global_set_layout->get_descriptor_set_layout()));
        load();

        // textures
        texture diffuse_texture{scene_loader::instance().diffuse_texture_path(), VK_FORMAT_R8G8B8A8_SRGB};
        texture normal_texture{scene_loader::instance().normal_texture_path(), VK_FORMAT_R8G8B8A8_UNORM};
        texture specular_texture{scene_loader::instance().specular_texture_path(), VK_FORMAT_R8G8B8A8_SRGB};
        texture gloss_texture{scene_loader::instance().glossiness_texture_path(), VK_FORMAT_R8G8B8A8_SRGB};

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
        
        camera camera{};
        // camera.set_view_direction(glm::vec3{0.0f}, glm::vec3{2.5f, 0.0f, 1.0f});
        // camera.set_view_target(glm::vec3{0.0f, -1.5f, -5.0f}, glm::vec3{0.0f, 0.0f, 0.0f});

        auto viewer_object = game_object("viewer");
        viewer_object.transform.translation = {0.0f, -1.5f, -5.0f};
        viewer_object.transform.rotation = {-0.2f, 0.0f, 0.0f};
        movement_controller camera_controller = {};

        // register input callbacks
        glfwSetKeyCallback(window_ptr_->get_glfw_window(), shading_mode_controller::key_callback);

        // ubo and frame info
        global_ubo ubo{};
        auto &frame_info = frame_info::instance();

        // time
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto last_time = high_resolution_clock::now();
        float lag         = 0.0f;

        //---------------------------------------------------------
        // Game Loop
        //---------------------------------------------------------
        while (not window_ptr_->should_close())
        {
            // input
            glfwPollEvents();

            // time
            auto current_time = high_resolution_clock::now();
            game_time::instance().set_delta_time(duration<float>(current_time - last_time).count()); // dt always has a 1 frame delay
            
            last_time = current_time;
            lag += game_time::instance().delta_time();

            // camera
            camera_controller.move(window_ptr_->get_glfw_window(), viewer_object);
            camera.set_view_yxz(viewer_object.transform.translation, viewer_object.transform.rotation);
            
            float aspect = renderer_ptr_->aspect_ratio();
            camera.set_orthographic_projection(-aspect, aspect, -1, 1, -1, 1);
            camera.set_perspective_projection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

            if (auto command_buffer = renderer_ptr_->begin_frame())
            {
                int frame_index = renderer_ptr_->frame_index();

                // frame info
                frame_info.frame_index = frame_index;
                frame_info.command_buffer = command_buffer;
                frame_info.camera_ptr = &camera;
                frame_info.global_descriptor_set = global_descriptor_sets[frame_index];
                frame_info.ubo_ptr = &ubo;

                // ubo
                ubo.projection = camera.get_projection();
                ubo.view = camera.get_view();
                ubo.inverse_view = camera.get_inverse_view();
                
                // update
                scene_manager.update();
                ubo_buffers[frame_index]->write_to_buffer(&ubo);
                ubo_buffers[frame_index]->flush();
                
                // render
                renderer_ptr_->begin_swap_chain_render_pass(command_buffer);
                scene_manager.render();
                renderer_ptr_->end_swap_chain_render_pass(command_buffer);
                renderer_ptr_->end_frame();
                
                auto const sleep_time = current_time + milliseconds(static_cast<long long>(game_time::instance().ms_per_frame())) - high_resolution_clock::now();
                std::this_thread::sleep_for(sleep_time);
            }
        }
        vkDeviceWaitIdle(device_ptr_->logical_device());
    }
}
