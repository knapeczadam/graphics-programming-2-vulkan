#pragma once

// Standard includes
#include <functional>

// GLM includes
#include <glm/glm.hpp>

namespace dae
{
#define ONE_TAB "\t"
#define TWO_TABS "\t\t"

#define RED_TEXT(text) "\033[1;31m" text "\033[0m"
#define GREEN_TEXT(text) "\033[1;32m" text "\033[0m"
#define MAGENTA_TEXT(text) "\033[1;35m" text "\033[0m"
#define YELLOW_TEXT(text) "\033[1;33m" text "\033[0m"
#define LEFT_PAR "\033[1;33m(\033[0m"
#define RIGHT_PAR "\033[1;33m)\033[0m"
#define SLASH "\033[1;33m/\033[0m"
    
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
