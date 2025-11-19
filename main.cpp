#include <grammar.h>

#include <iostream>
#include <print>
#include <ranges>

int main()
{
    std::print("Enter the epsilon: ");
    std::string ep;
    std::cin >> ep;
    std::print("Enter the number of rules: ");
    int n;
    std::cin >> n;
    std::cin.get();

    auto to_sv =
        std::views::transform([](auto t) { return std::string_view{t}; });
    // LINE FORMAT: A -> B d | a | <epsilon>
    std::vector<Production> prods;
    for (int i{}; i != n; ++i) {
        std::println("Enter the {}-th rule:", i + 1);
        std::string line;
        std::getline(std::cin, line);
        Production r;

        auto tokens = line | std::views::split(' ') | to_sv |
                      std::ranges::to<std::vector>();

        auto from = (tokens | std::views::take(1)).front();
        auto tos = tokens | std::views::drop(2) | std::views::split("|") |
                   std::ranges::to<std::vector<SymbolString>>();
        r.from = from;
        r.tos = tos;

        prods.push_back(r);
    }

    Grammar g(prods, ep);
    g.summary();

    // std::println("{}",
    //              g.first_set({"L", "J", "F", " ", "I", "S", " ", "S", "B"}));
}
