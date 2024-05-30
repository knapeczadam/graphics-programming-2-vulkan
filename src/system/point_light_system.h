#pragma once

// Project includes
#include "src/engine/frame_info.h"
#include "src/system/i_system.h"

namespace dae
{
    class point_light_system final : public i_system
    {
    public:
        explicit point_light_system(VkDescriptorSetLayout global_set_layout);
        ~point_light_system() override = default;

        point_light_system(point_light_system const &other)            = delete;
        point_light_system(point_light_system &&other)                 = delete;
        point_light_system &operator=(point_light_system const &other) = delete;
        point_light_system &operator=(point_light_system &&other)      = delete;

        void update() override;
        void render() override;

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
