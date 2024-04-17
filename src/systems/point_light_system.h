#pragma once

// Project includes
#include "../pipeline.h"
#include "../device.h"
#include "../game_object.h"
#include "../camera.h"
#include "../frame_info.h"

// Standard includes
#include <memory>
#include <vector>

namespace dae
{
    class point_light_system
    {
    public:
        point_light_system(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~point_light_system();

        point_light_system(point_light_system const &)            = delete;
        point_light_system &operator=(point_light_system const &) = delete;

        void update(frame_info &frame_info, global_ubo &ubo);
        void render(frame_info &frame_info);

    private:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
        void create_pipeline(VkRenderPass render_pass);
        
    private:
        device                     &device_;
        
        std::unique_ptr<pipeline>  pipeline_;
        VkPipelineLayout               pipeline_layout_;
        
    };
}
