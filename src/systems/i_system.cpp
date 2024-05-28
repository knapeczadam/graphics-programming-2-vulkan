#include "i_system.h"

namespace dae
{
    i_system::i_system(device &device)
        : device_(device)
    {
    }

    i_system::~i_system()
    {
        vkDestroyPipelineLayout(device_.get_logical_device(), pipeline_layout_, nullptr);
    }
}
