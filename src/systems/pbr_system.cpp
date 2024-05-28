#include "pbr_system.h"

// Standard includes
#include <stdexcept>

namespace dae
{
    pbr_system::pbr_system(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
        : i_system{device}
    {
        create_pipeline_layout(global_set_layout);
        create_pipeline(render_pass);
    }

    void pbr_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
    {
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};
        
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts            = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges    = VK_NULL_HANDLE;

        if (vkCreatePipelineLayout(device_.get_logical_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create pipeline layout!"};
        }
    }

    void pbr_system::create_pipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr and "Cannot create pipeline before pipeline layout");
        
        pipeline_config_info pipeline_config{};
        pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<pipeline>(
            device_,
            "shaders/pbr.vert.spv",
            "shaders/pbr.frag.spv",
            pipeline_config);
    }
}
