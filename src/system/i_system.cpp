#include "i_system.h"

// Project includes
#include "src/vulkan/device.h"

namespace dae
{
    i_system::i_system()
        : device_ptr_(&device::instance())
    {
    }

    i_system::~i_system()
    {
        vkDestroyPipelineLayout(device_ptr_->logical_device(), pipeline_layout_, nullptr);
    }
}
