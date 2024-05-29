#pragma once

// Project includes
#include "system/i_system.h"

namespace dae
{
    class pbr_system final : public i_system
    {
    public:
        pbr_system(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~pbr_system() override = default;

        pbr_system(pbr_system const &other)            = delete;
        pbr_system(pbr_system &&other)                 = delete;
        pbr_system &operator=(pbr_system const &other) = delete;
        pbr_system &operator=(pbr_system &&other)      = delete;
        
        void render_game_objects(frame_info &frame_info);

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
