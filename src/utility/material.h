#pragma once

// GLM includes
#include <glm/glm.hpp>

namespace dae
{
    struct material
    {
        glm::vec3 base_color = {};
        float     metallic   = 0.0f;
        float     roughness  = 0.0f;
    };
}