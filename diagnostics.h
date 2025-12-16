#pragma once
#include <print>
#include <utility>

class Diagnostics {
  public:
    template <typename... Ts>
    void error(std::format_string<Ts...> fmt, Ts &&...ts)
    {
        std::println(fmt, std::forward<Ts>(ts)...);
    }
};
