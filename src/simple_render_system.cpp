#include "simple_render_system.h"

// Standard includes
#include <stdexcept>
#include <array>

// GLM includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve
{
    struct simple_push_constant_data
    {
        glm::mat4 transform{1.0f};
        glm::mat4 normal_matrix{1.0f};
    };
    
    simple_render_system::simple_render_system(lve_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
        : device_{device}
    {
        create_pipeline_layout(global_set_layout);
        create_pipeline(render_pass);
    }

    simple_render_system::~simple_render_system()
    {
        vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
    }

    void simple_render_system::render_game_objects(frame_info &frame_info, std::vector<lve_game_object>& game_objects)
    {
        pipeline_->bind(frame_info.command_buffer);

        auto projection_view = frame_info.camera.get_projection() * frame_info.camera.get_view();

        for (auto &obj : game_objects)
        {
            simple_push_constant_data push{};
            auto model_matrix = obj.transform.mat4();
            push.transform = projection_view * model_matrix;
            push.normal_matrix = obj.transform.normal_matrix();

            vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(simple_push_constant_data),
                &push);
            
            obj.model->bind(frame_info.command_buffer);
            obj.model->draw(frame_info.command_buffer);
        }
    }

    void simple_render_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
    {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset     = 0;
        push_constant_range.size       = sizeof(simple_push_constant_data);

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};
        
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount         = 0;
        pipeline_layout_info.pSetLayouts            = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges    = &push_constant_range;

        if (vkCreatePipelineLayout(device_.device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create pipeline layout!"};
        }
    }

    void simple_render_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr and "Cannot create pipeline before pipeline layout");
        
        pipeline_config_info pipeline_config{};
        lve_pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<lve_pipeline>(
            device_,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipeline_config);
    }
}
