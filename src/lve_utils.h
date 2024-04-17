#pragma once

// Standard includes
#include <functional>

namespace dae
{
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hash_combine(std::size_t &seed, T const &v, Rest const &... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hash_combine(seed, rest), ...);
    };
}
