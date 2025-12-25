#pragma once
#include <print>
#include <utility>

class Diagnostics {
  public:
    template <typename... Ts>
    void error(std::format_string<Ts...> fmt, Ts &&...ts)
    {
        std::print("error: ");
        std::println(fmt, std::forward<Ts>(ts)...);
    }
};
