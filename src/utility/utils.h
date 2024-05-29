#pragma once

// Standard includes
#include <functional>

// GLM includes
#include <glm/glm.hpp>

namespace dae
{
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hash_combine(std::size_t &seed, T const &v, Rest const &... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hash_combine(seed, rest), ...);
    };

    namespace math
    {
        inline auto reject(glm::vec3 const &v1, glm::vec3 const &v2) -> glm::vec3
        {
            return v1 - v2 * (glm::dot(v1, v2) / glm::dot(v2, v2));
        }
    }
}
