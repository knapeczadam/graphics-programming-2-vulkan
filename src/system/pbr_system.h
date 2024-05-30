#pragma once

// Project includes
#include "src/engine/frame_info.h"
#include "src/system/i_system.h"

namespace dae
{
    class pbr_system final : public i_system
    {
    public:
        pbr_system(device *device_ptr, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~pbr_system() override = default;

        pbr_system(pbr_system const &other)            = delete;
        pbr_system(pbr_system &&other)                 = delete;
        pbr_system &operator=(pbr_system const &other) = delete;
        pbr_system &operator=(pbr_system &&other)      = delete;
        
        void render();

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
    };
}
