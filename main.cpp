#include "grammar.h"
#include <iostream>
#include <print>
#include <ranges>

int main()
{
    std::print("Enter the epsilon: ");
    char ep;
    std::cin >> ep;
    std::print("Enter the number of rules: ");
    int n;
    std::cin >> n;
    std::cin.get();
    // LINE FORMAT: A->Bd|a|<epsilon>
    std::vector<Production> prods;
    for (int i{}; i != n; ++i) {
        std::println("Enter the {}-th rule:", i + 1);
        Production r;
        std::string line;
        std::getline(std::cin, line);
        auto p = line.find("->");
        r.from = line.substr(0, p);
        for (auto part : line.substr(p + 2) | std::views::split('|')) {
            auto trans_epsilon =
                part | std::views::transform([&ep](auto const &ch) {
                    return ch == ep ? epsilon : std::string{ch};
                }) |
                std::ranges::to<Symbol_string>();
            r.tos.push_back(trans_epsilon);
        }
        std::println("from: {}, tos: {}", r.from, r.tos);
        prods.push_back(r);
    }

    Grammar g(prods);
    for (auto [p, q] : g.select_set_view()) {
    }

    // std::println("{}",
    //              g.first_set({"L", "J", "F", " ", "I", "S", " ", "S", "B"}));
}
