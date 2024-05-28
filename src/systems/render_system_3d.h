#pragma once

// Project includes
#include "i_system.h"

namespace dae
{
    class render_system_3d final : public i_system
    {
    public:
        render_system_3d(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~render_system_3d() override = default;

        render_system_3d(render_system_3d const &other)            = delete;
        render_system_3d(render_system_3d &&other)                 = delete;
        render_system_3d &operator=(render_system_3d const &other) = delete;
        render_system_3d &operator=(render_system_3d &&other)      = delete;
        
        void render_game_objects(frame_info &frame_info);

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
