#pragma once

// Project includes
#include "system/i_system.h"

namespace dae
{
    class point_light_system final : public i_system
    {
    public:
        point_light_system(device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~point_light_system() override = default;

        point_light_system(point_light_system const &other)            = delete;
        point_light_system(point_light_system &&other)                 = delete;
        point_light_system &operator=(point_light_system const &other) = delete;
        point_light_system &operator=(point_light_system &&other)      = delete;

        void update(frame_info &frame_info, global_ubo &ubo);
        void render(frame_info &frame_info);

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
