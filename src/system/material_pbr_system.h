#pragma once

// Project includes
#include "src/system/i_system.h"

namespace dae
{
    class material_pbr_system final : public i_system
    {
    public:
        explicit material_pbr_system(VkDescriptorSetLayout global_set_layout);
        ~material_pbr_system() override = default;

        material_pbr_system(material_pbr_system const &other)            = delete;
        material_pbr_system(material_pbr_system &&other)                 = delete;
        material_pbr_system &operator=(material_pbr_system const &other) = delete;
        material_pbr_system &operator=(material_pbr_system &&other)      = delete;

        void render() override;

    protected:
        void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
        void create_pipeline(VkRenderPass render_pass) override;
        
    };
}
