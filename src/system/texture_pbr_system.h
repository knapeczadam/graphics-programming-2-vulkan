#pragma once

// Project includes
#include "src/system/i_system.h"

namespace dae
{
    class texture_pbr_system final : public i_system
    {
    public:
        explicit texture_pbr_system(VkDescriptorSetLayout global_set_layout);
        ~texture_pbr_system() override = default;

        texture_pbr_system(texture_pbr_system const &other)            = delete;
        texture_pbr_system(texture_pbr_system &&other)                 = delete;
        texture_pbr_system &operator=(texture_pbr_system const &other) = delete;
        texture_pbr_system &operator=(texture_pbr_system &&other)      = delete;
        
        void render() override;

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
