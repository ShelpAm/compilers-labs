// a-zA-Z

#include <cassert>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class NFA {
    friend class DFA;

  public:
    using Node = int;
    using Node_set = std::set<int>;

    NFA(std::vector<int> const &begin, std::vector<int> const &ends)
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
            auto [begin, end] = edges_of(node).equal_range(NFA::epsilon);
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

  private:
    std::unordered_map<int, std::unordered_multimap<int, int>> g_;
    std::unordered_set<int> begins_;
    std::unordered_set<int> ends_;
};

class DFA {
  public:
    using Node = int;
    using Node_set = std::set<int>;

    DFA(int begin, std::vector<int> const &ends)
        : begin_{begin}, ends_{ends.begin(), ends.end()}
    {
    }

    explicit DFA(NFA const &nfa)
    {
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
        auto begin_set = nfa.epsilon_closure({std::from_range, nfa.begins_});
        begin_ = mapped_node(begin_set); // Generate begin for begin set
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
                return nfa.ends_.contains(u);
            };
            if (std::ranges::find_if(us, in_ends) != us.end()) {
                ends_.insert(node_id);
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
                g_[node_id].insert({w, mapped_node(ec)});
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

    void add(int u, int v, int w)
    {
        if (g_[u].contains(w)) {
            throw std::logic_error("Node already has the edge");
        }
        g_[u].insert({w, v});
        g_[v];
    }

    bool match(std::string_view s) const
    {
        auto i = begin_;
        for (auto c : s) {
            check_in_g(i);
            if (!g_.at(i).contains(c)) {
                return false;
            }
            i = g_.at(i).at(c);
        }
        return ends_.contains(i);
    }

    void check_in_g(Node node) const
    {
        if (!g_.contains(node)) {
            throw std::runtime_error{
                std::format("NFA doesn't contains node={}", node)};
        }
    }

  private:
    std::unordered_map<int, std::unordered_map<Node, int>> g_;
    int begin_;
    std::unordered_set<int> ends_;
};

// Fails, those two FA should be the same.
int main()
{
    auto nfa = NFA({0}, {7});
    nfa.add(0, 1, 'b');
    nfa.add(1, 2, NFA::epsilon);
    nfa.add(2, 3, 'a');
    nfa.add(3, 2, 'b');
    nfa.add(2, 5, NFA::epsilon);
    nfa.add(5, 6, 'a');
    nfa.add(6, 7, 'b');
    nfa.add(1, 4, 'b');
    nfa.add(4, 5, 'b');
    nfa.add(5, 1, NFA::epsilon);
    auto converted = DFA(nfa);
    auto hand = DFA(0, {4});
    hand.add(0, 1, 'b');
    hand.add(1, 2, 'a');
    hand.add(1, 4, 'b');
    hand.add(2, 4, 'b');
    hand.add(4, 2, 'a');
    hand.add(4, 0, 'b');

    std::string text{"babab"};
    std::print("Input test text: ");
    std::cin >> text;
    auto converted_match = converted.match(text);
    auto hand_match = hand.match(text);
    if (converted_match == hand_match) {
        std::println(
            "The matching result for \"{}\" is the same for two dfas: {}.",
            text, converted_match ? "matched" : "unmatched");
    }
    else {
        std::println("The matching result for \"{}\" is different for two "
                     "dfas. Converted "
                     "{}, but hand {}.",
                     text, converted_match ? "matched" : "unmatched",
                     hand_match ? "matched" : "unmatched");
    }

    // Simple test from zzs
    {
        auto fa = DFA(0, {3});
        fa.add(0, 1, 'a');
        fa.add(1, 3, 'c');
        fa.add(0, 2, 'b');
        fa.add(2, 3, 'a');
        fa.add(3, 2, 'b');
        assert(fa.match("acbabababa"));
        assert(!fa.match("acbababab"));
    }
}
