#pragma once
#include "nondeterminstic-finite-automaton.h"

#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Node = int;
using Node_set = std::set<int>;

class DeterminsticFiniteAutomaton {
  public:
    using State = int;
    using Symbol = int;

    static DeterminsticFiniteAutomaton from_nfa(NFA const &nfa)
    {
        DeterminsticFiniteAutomaton ret;

        std::map<Node_set, Node> map;
        auto mapped_node = [&map](Node_set const &set) {
            static Node t{};
            if (!map.contains(set)) {
                map.insert({set, t++});
            }
            return map.at(set);
        };

        std::queue<Node_set> q;
        std::set<Node_set> vis;
        auto begin_set = nfa.epsilon_closure({std::from_range, nfa.begins()});
        ret.begin_ = mapped_node(begin_set); // Generate begin for begin set
        q.push(begin_set);
        while (!q.empty()) {
            auto us = q.front();
            q.pop();

            if (vis.contains(us)) {
                continue;
            }
            vis.insert(us);

            auto node_id = mapped_node(us);
            auto in_ends = [&nfa](auto const &u) {
                return nfa.ends().contains(u);
            };
            if (std::ranges::find_if(us, in_ends) != us.end()) {
                ret.ends_.insert(node_id);
            }

            // edge to next vertices; Get vertices by edge
            std::unordered_map<int, Node_set> vs_from_e;
            for (auto u : us) {
                nfa.check_in_g(u);
                for (auto [w, v] : nfa.edges_of(u)) {
                    vs_from_e[w].insert(v);
                }
            }
            for (auto const &[w, vs] : vs_from_e) {
                if (w == NFA::epsilon) { // Ignores epsilon edges
                    continue;
                }
                auto ec = nfa.epsilon_closure(vs);
                ret.g_[node_id].insert({w, mapped_node(ec)});
                q.push(ec);
            }
        }

        // For DEBUG purposes. Contains node mappings in conversion.
        // std::cout << "Node set mappings:\n";
        // for (auto const &[k, v] : map) {
        //     std::cout << "keys: ";
        //     for (auto e : k) {
        //         std::cout << e << ' ';
        //     }
        //     std::cout << "-> " << v << '\n';
        // }

        return ret;
    }

    DeterminsticFiniteAutomaton(State begin, std::vector<State> const &ends)
        : begin_{begin}, ends_{ends.begin(), ends.end()}
    {
        g_[begin_];
        for (auto end : ends_) {
            g_[end];
        }
    }

    void dump(std::ostream &os = std::cout) const
    {
        std::println(os, "In this DFA,");
        std::print(os, "Number of vertices: {}", g_.size());
        std::print(os, "\nBegin: {}", begin_);
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

    void add(State u, State v, Symbol w)
    {
        if (g_[u].contains(w)) {
            throw std::logic_error("Node already has the edge");
        }
        g_[u].insert({w, v});
        g_[v];
    }

    bool match(std::string_view s) const
    {
        auto state = begin_;
        for (auto c : s) {
            check_in_g(state);
            if (!g_.at(state).contains(c)) {
                return false;
            }
            state = g_.at(state).at(c);
        }
        return ends_.contains(state);
    }

    void check_in_g(Node node) const
    {
        if (!g_.contains(node)) {
            throw std::runtime_error{
                std::format("NFA doesn't contains node={}", node)};
        }
    }

  private:
    DeterminsticFiniteAutomaton() = default;

    std::unordered_map<int, std::unordered_map<Node, int>> g_;
    State begin_{};
    std::unordered_set<State> ends_;
};

using DFA = DeterminsticFiniteAutomaton;
