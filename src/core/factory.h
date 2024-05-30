#pragma once

// Project includes
#include "model.h"

// Standard includes
#include <memory>

namespace dae
{
    struct factory final
    {
        static std::unique_ptr<model> create_oval(device *device_ptr, glm::vec3 offset, float radiusX, float radiusY, int segments);
        static std::unique_ptr<model> create_n_gon(device *device_ptr, glm::vec3 offset, float radius, int sides);
    };
}
