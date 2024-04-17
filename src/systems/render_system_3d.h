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
    class render_system_3d
    {
    public:
        render_system_3d(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~render_system_3d();

        render_system_3d(render_system_3d const &)            = delete;
        render_system_3d &operator=(render_system_3d const &) = delete;
        
        void render_game_objects(frame_info &frame_info);

    private:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
        void create_pipeline(VkRenderPass render_pass);
        
    private:
        device                     &device_;
        
        std::unique_ptr<pipeline>  pipeline_;
        VkPipelineLayout               pipeline_layout_;
        
    };
}
