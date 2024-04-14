#pragma once

// Project includes
#include "../lve_pipeline.h"
#include "../lve_device.h"
#include "../lve_game_object.h"
#include "../lve_camera.h"
#include "../lve_frame_info.h"

// Standard includes
#include <memory>
#include <vector>

namespace lve
{
    class point_light_system
    {
    public:
        point_light_system(lve_device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~point_light_system();

        point_light_system(point_light_system const &)            = delete;
        point_light_system &operator=(point_light_system const &) = delete;
        
        void render(frame_info &frame_info);

    private:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
        void create_pipeline(VkRenderPass render_pass);
        
    private:
        lve_device                     &device_;
        
        std::unique_ptr<lve_pipeline>  pipeline_;
        VkPipelineLayout               pipeline_layout_;
        
    };
}
