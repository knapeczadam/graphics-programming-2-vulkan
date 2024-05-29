#include "point_light_system.h"

// Project includes
#include "src/vulkan/device.h"

// Standard includes
#include <array>
#include <map>
#include <ranges>
#include <stdexcept>

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace dae
{
    struct point_light_push_constants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };
    
    point_light_system::point_light_system(device *device_ptr, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
        : i_system{device_ptr}
    {
        create_pipeline_layout(global_set_layout);
        create_pipeline(render_pass);
    }

    void point_light_system::update(frame_info &frame_info, global_ubo &ubo)
    {
        auto rotate_light = glm::rotate(
            glm::mat4{1.0f},
            frame_info.frame_time,
            {0.0f, -1.0f, 0.0f}
        );
        
        int light_index = 0;
        for (auto &obj : frame_info.game_objects | std::views::values)
        {
            if (obj.point_light == nullptr) continue;

            assert(light_index < MAX_LIGHTS and "Point lights exceed maximum specified");

            // update light position
            obj.transform.translation = glm::vec3{rotate_light * glm::vec4{obj.transform.translation, 1.0f}};

            // copy light to ubo
            ubo.point_lights[light_index].position = glm::vec4{obj.transform.translation, 1.0f};
            ubo.point_lights[light_index].color    = glm::vec4{obj.color, obj.point_light->light_intensity};

            ++light_index;
        }
        ubo.num_lights = light_index;
    }

void point_light_system::render(frame_info &frame_info)
    {
        std::map<float, game_object::id_t> sorted;
        for (auto &go : frame_info.game_objects | std::views::values)
        {
            if (go.get_name() != "point_light") continue;

            auto offset = frame_info.camera.get_position() - go.transform.translation;
            float dis_squared = glm::dot(offset, offset);
            sorted[dis_squared] = go.get_id();
        }
        
        pipeline_->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            0,
            1,
            &frame_info.global_descriptor_set,
            0,
            nullptr
        );
        
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            auto &go = frame_info.game_objects.at(it->second);

            point_light_push_constants push{};
            push.position = glm::vec4{go.transform.translation, 1.0f};
            push.color    = glm::vec4{go.color, go.point_light->light_intensity};
            push.radius   = go.transform.scale.x;

            vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(point_light_push_constants),
                &push
            );
            vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
        }
    }

    void point_light_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
    {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset     = 0;
        push_constant_range.size       = sizeof(point_light_push_constants);

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};
        
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts            = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges    = &push_constant_range;

        if (vkCreatePipelineLayout(device_ptr_->logical_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create pipeline layout!"};
        }
    }

    void point_light_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr and "Cannot create pipeline before pipeline layout");
        
        pipeline_config_info pipeline_config{};
        pipeline::default_pipeline_config_info(pipeline_config);
        pipeline::enable_alpha_blending(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<pipeline>(
            device_ptr_,
            "data/shaders/point_light.vert.spv",
            "data/shaders/point_light.frag.spv",
            pipeline_config);
    }
}
