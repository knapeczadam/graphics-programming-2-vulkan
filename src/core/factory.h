#pragma once

// Project includes
#include "model.h"

// Standard includes
#include <memory>

namespace dae
{
    struct factory final
    {
        static std::unique_ptr<model> create_oval(glm::vec3 offset, float radiusX, float radiusY, int segments);
        static std::unique_ptr<model> create_n_gon(glm::vec3 offset, float radius, int sides);
    };
}
