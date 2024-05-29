#pragma once

// Project includes
#include "src/vulkan/pipeline.h"

// Standard includes
#include <memory>

namespace dae
{
    // Forward declarations
    class device;
    
    class i_system
    {
    public:
        explicit i_system(device *device_ptr);
        virtual ~i_system();

        i_system(i_system const &other)            = delete;
        i_system(i_system &&other)                 = delete;
        i_system &operator=(i_system const &other) = delete;
        i_system &operator=(i_system &&other)      = delete;

    protected:
        virtual void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) = 0;
        virtual void create_pipeline(VkRenderPass render_pass) = 0;

    protected:
        device                    *device_ptr_;

        std::unique_ptr<pipeline> pipeline_;
        VkPipelineLayout          pipeline_layout_ = VK_NULL_HANDLE;
    };
}
