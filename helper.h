#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>

inline std::string indent_string(std::size_t n) noexcept
{
    static std::unordered_map<std::size_t, std::string> cache;
    if (!cache.contains(n)) {
        cache.insert({n, std::string(n * 4, ' ')});
    }
    return cache.at(n);
}
