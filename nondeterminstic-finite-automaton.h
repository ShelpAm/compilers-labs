#pragma once
#include "regular-expression.h"

#include <iostream>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Node = int;
using Node_set = std::set<int>;

class NondeterminsticFiniteAutomaton {
    friend class DeterminsticFiniteAutomaton;

  public:
    // static NondeterminsticFiniteAutomaton
    // from_regular_expression(RegularExpression const &re)
    // {
    //     NondeterminsticFiniteAutomaton ret;
    //     auto state = 0;
    //     ret.begins_ = {state};
    //     auto const &expr = re.re_;
    //
    //     std::stack<char> operator_stack;
    //     std::stack<char> operand_stack;
    //
    //     for (auto i = 0UZ; i != expr.size();) { // 不考虑非法情况
    //         auto sym = expr[i];
    //         if (i + 1 != expr.size()) {
    //             auto const next = expr[i + 1];
    //             if (next == '+') {
    //                 ret.add_aliased(state, state + 1, sym);
    //                 ret.add_aliased(state + 1, state + 1, sym);
    //                 ret.add_aliased(state + 1, state + 2, epsilon);
    //                 state += 2;
    //                 i += 2;
    //             }
    //             else if (next == '*') {
    //                 ret.add_aliased(state, state, sym);
    //                 ret.add_aliased(state, state + 1, epsilon);
    //                 state += 1;
    //                 i += 2;
    //             }
    //             else {
    //                 ret.add_aliased(state, state + 1, sym);
    //                 state += 1;
    //                 i += 1;
    //             }
    //         }
    //         else {
    //             ret.add_aliased(state, state + 1, sym);
    //             state += 1;
    //             i += 1;
    //         }
    //     }
    //     ret.ends_ = {state};
    //     return ret;
    // }

    NondeterminsticFiniteAutomaton(std::vector<int> const &begin,
                                   std::vector<int> const &ends)
        : begins_(begin.begin(), begin.end()), ends_(ends.begin(), ends.end())
    {
    }

    void add(int u, int v, int w)
    {
        g_[u].insert({w, v});
        g_[v];
    }

    void dump(std::ostream &os = std::cout) const
    {
        std::println(os, "In this NFA,");
        std::print(os, "Number of vertices: {}", g_.size());
        std::print(os, "\nBegins: ");
        for (auto e : begins_) {
            std::print(os, "{} ", e);
        }
        std::print(os, "\nEnds: ");
        for (auto e : ends_) {
            std::print(os, "{} ", e);
        }
        std::println(os, "\nAll edges:");
        for (auto const &[u, es] : g_) {
            for (auto [w, v] : es) {
                std::println(os, "{} --> {} weight={}", u, v, w);
            }
        }
    }

    void check_in_g(Node node) const
    {
        if (!g_.contains(node)) {
            throw std::runtime_error{
                std::format("NFA doesn't contains node={}", node)};
        }
    }

    std::unordered_multimap<int, int> const &edges_of(Node node) const
    {
        return g_.at(node);
    }

    Node_set epsilon_closure(Node_set set) const
    {
        std::queue<Node> q;
        for (auto node : set) {
            q.push(node);
        }

        while (!q.empty()) {
            auto node = q.front();
            q.pop();

            check_in_g(node);
            auto [begin, end] = edges_of(node).equal_range(
                NondeterminsticFiniteAutomaton::epsilon);
            for (auto it = begin; it != end; ++it) {
                auto v = it->second;
                if (!set.contains(v)) {
                    set.insert(v);
                    q.push(v);
                }
            }
        }

        return set;
    }

    static constexpr int epsilon{-1}; // empty edge

    std::unordered_set<int> const &begins() const
    {
        return begins_;
    }

    std::unordered_set<int> const &ends() const
    {
        return ends_;
    }

  private:
    NondeterminsticFiniteAutomaton() = default;

    void add_aliased(int u, int v, int w)
    {
        if (w == '.') {
            add_all_symbols(u, v);
        }
        else {
            add(u, v, w);
        }
    }

    void add_all_symbols(int u, int v)
    {
        for (auto const &[sym, _] : g_) {
            add(u, v, sym);
        }
    }

    std::unordered_map<int, std::unordered_multimap<int, int>> g_;
    std::unordered_set<int> begins_;
    std::unordered_set<int> ends_;
};

using NFA = NondeterminsticFiniteAutomaton;
