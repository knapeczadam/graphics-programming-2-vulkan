#pragma once

// Project includes
#include "src/system/i_system.h"

namespace dae
{
    class render_3d_system final : public i_system
    {
    public:
        explicit render_3d_system(VkDescriptorSetLayout global_set_layout);
        ~render_3d_system() override = default;

        render_3d_system(render_3d_system const &other)            = delete;
        render_3d_system(render_3d_system &&other)                 = delete;
        render_3d_system &operator=(render_3d_system const &other) = delete;
        render_3d_system &operator=(render_3d_system &&other)      = delete;
        
        void render() override;

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
