#include "pbr_system.h"

// Project includes
#include "src/vulkan/device.h"

// Standard includes
#include <ranges>
#include <stdexcept>

namespace dae
{
    struct pbr_push_constant
    {
        glm::mat4 model_matrix{1.0f};
        glm::mat4 normal_matrix{1.0f};
        float r;
        float g;
        float b;
        float metallic;
        float roughness;
    };
    
    pbr_system::pbr_system(device *device_ptr, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
        : i_system{device_ptr}
    {
        create_pipeline_layout(global_set_layout);
        create_pipeline(render_pass);
    }

    void pbr_system::render()
    {
        auto &frame_info = frame_info::instance();
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

        for (auto &obj : *frame_info.game_objects_ptr | std::views::values)
        {
            if (obj.model == nullptr) continue;
            if (obj.name() != "pbr") continue;
            
            pbr_push_constant push{};
            push.model_matrix = obj.transform.mat4();
            push.normal_matrix = obj.transform.normal_matrix();
            push.r = obj.material().base_color.r;
            push.g = obj.material().base_color.g;
            push.b = obj.material().base_color.b;
            push.metallic = obj.material().metallic;
            push.roughness = obj.material().roughness;

            vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(pbr_push_constant),
                &push);
            
            obj.model->bind(frame_info.command_buffer);
            obj.model->draw(frame_info.command_buffer);
        }
    }

    void pbr_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
    {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset     = 0;
        push_constant_range.size       = sizeof(pbr_push_constant);
        
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

    void pbr_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr and "Cannot create pipeline before pipeline layout");
        
        pipeline_config_info pipeline_config{};
        pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<pipeline>(
            device_ptr_,
            "data/shaders/pbr.vert.spv",
            "data/shaders/pbr.frag.spv",
            pipeline_config);
    }
}
