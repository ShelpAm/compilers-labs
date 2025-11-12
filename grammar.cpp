#include "grammar.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <ranges>
#include <utility>

Grammar::Grammar(std::vector<Production> productions)
    : begin_{productions.front().from}
{
    for (auto &production : productions)
        add_production(std::move(production));

    build();
}

void Grammar::add_production(Production r)
{
    if (is_terminal(r.from))
        throw std::invalid_argument{
            std::format("r.from({}) is terminal", r.from)};

    nonterminals_.insert(r.from);
    for (auto const &to : r.tos)
        for (auto const &symbol : to)
            if (!is_terminal(symbol))
                nonterminals_.insert(symbol);
            else
                terminals_.insert(symbol);
    productions_[r.from].append_range(std::move(r.tos));
}

void Grammar::build()
{
    build_nullable_set();
    build_first_set();
    build_follow_set();
    build_select_set();

    std::println("\033[36mNullable set\033[0m: {}", nullables_);
    std::println("\033[36mFIRST set\033[0m: {}", first_set_);
    std::println("\033[36mFOLLOW set\033[0m: {}", follow_set_);
    std::println("\033[36mSELECT set:\033[0m");
    for (auto const &[prod, set] : select_set_view()) {
        std::println("    {}: {}", prod, set);
    }

    std::unordered_map<Symbol, std::unordered_map<Symbol, Single_production>>
        map;
    for (auto const &[prod, set] : select_set_view()) {
        for (auto const &terminal : set) {
            if (map[prod.from].contains(terminal))
                throw std::runtime_error{"Not a LL(1) grammar"};
            map[prod.from][terminal] = prod;
        }
    }

    std::println("\033[36mLL(1) parsing table:\033[0m");
    std::print("    ");
    for (auto const &terminal : terminals_) {
        std::print("{:20}", terminal);
    }
    std::println("");
    for (auto const &nonterminal : nonterminals_) {
        std::print("{:3}", nonterminal);
        for (auto const &terminal : terminals_) {
            std::print("{:20}",
                       map[nonterminal].contains(terminal)
                           ? std::format("{}", map[nonterminal][terminal])
                           : "");
        }
        std::println();
    }

    assert(nullables_built && first_set_built && follow_set_built &&
           select_set_built);
    built = true;
}

std::unordered_set<Symbol> const &Grammar::empty_set() const
{
    return nullables_;
}

std::unordered_set<Symbol> Grammar::first_set(Symbol_string const &str) const
{
    if (!first_set_built)
        throw std::runtime_error{"should first build the Grammar"};

    if (is_epsilon(str)) {
        return {epsilon};
    }

    std::unordered_set<Symbol> res;

    bool nullable{true};
    for (auto const &symbol : str) {
        if (is_terminal(symbol)) {
            res.insert(symbol);
        }
        else {
            if (!first_set_.contains(symbol))
                throw std::invalid_argument{
                    std::format("Invalid symbol: {}", symbol)};
            res.insert_range(extract_epsilon(first_set_.at(symbol)));
        }
        if (!nullables_.contains(symbol)) {
            nullable = false;
            break;
        }
    }
    if (nullable)
        res.insert(epsilon);

    return res;
}

void Grammar::build_nullable_set()
{
    // Map of dependency count
    std::unordered_map<Symbol, std::vector<std::size_t>> nullable_cnt;
    std::unordered_map<
        Symbol, std::unordered_map<Single_production_handle, std::size_t>>
        dep_map;

    std::queue<Symbol> que; // For nullable propagation.

    for (auto const &[from, tos] : productions_) {
        nullable_cnt[from].assign(tos.size(), 0);
        for (auto const &[i, to] : tos | std::views::enumerate) {
            auto idx = static_cast<std::size_t>(i);
            if (to.size() == 1 && to[0] == epsilon)
                que.push(from);
            nullable_cnt[from][idx] = to.size();
            for (auto const &symbol : to) {
                ++dep_map[symbol][{.from{from}, .index = idx}];
            }
        }
    }

    while (!que.empty()) {
        auto s = que.front();
        que.pop();

        if (nullables_.contains(s)) {
            continue;
        }
        nullables_.insert(s);

        for (auto const &[edge, count] : dep_map[s]) {
            auto const &[from, index] = edge;
            auto left = nullable_cnt[from][index] -= count;
            if (left == 0)
                que.push(from);
        }
    }

    nullables_built = true;
}

void Grammar::build_first_set()
{
    assert(nullables_built);

    for (auto const &nonterminal : nonterminals_)
        if (nullables_.contains(nonterminal))
            first_set_[nonterminal].insert(epsilon);

    bool changed{true};
    do {
        changed = false;
        for (auto const &[from, tos] : productions_)
            for (auto const &to : tos) {
                if (is_epsilon(to)) { // Ignore epsilon
                    continue;
                }
                for (auto const &symbol : to) {
                    auto prev_size = first_set_[from].size();
                    first_set_[from].insert_range(
                        is_terminal(symbol)
                            ? std::unordered_set{symbol}
                            : extract_epsilon(first_set_[symbol]));
                    auto cur_size = first_set_[from].size();
                    changed |= prev_size != cur_size;
                    if (!nullables_.contains(symbol))
                        break;
                }
            }
    } while (changed);

    first_set_built = true;
}

void Grammar::build_follow_set()
{
    assert(first_set_built);

    follow_set_[begin_].insert(eol);

    bool changed{true};
    do {
        changed = false;
        for (auto const &[from, tos] : productions_)
            for (auto const &to : tos) {
                if (is_epsilon(to)) { // Ignore epsilon
                    continue;
                }

                // alpha e beta
                Symbol_string beta;
                for (auto i = to.size() - 1; i != -1UZ; --i) {
                    auto const &e = to[i];
                    if (!is_terminal(e)) {
                        auto first = first_set(beta);
                        auto node = first.extract(epsilon); // Step 3
                        auto prev_size = follow_set_[e].size();
                        follow_set_[e].insert_range(first); // Step 2
                        if (!node.empty()) // FIRST(beta) contains epsilon
                            follow_set_[e].insert_range(follow_set_[from]);
                        auto cur_size = follow_set_[e].size();
                        changed |= prev_size != cur_size;
                    }
                    beta.push_front(e);
                }
            }
    } while (changed);

    follow_set_built = true;
}

void Grammar::build_select_set()
{
    assert(first_set_built && follow_set_built);

    for (auto const &[from, tos] : productions_)
        for (auto const &[i, to] : tos | std::views::enumerate) {
            auto idx = static_cast<std::size_t>(i);
            auto cur = Single_production_handle{.from = from, .index = idx};
            auto first = first_set(to);
            auto node = first.extract(epsilon);
            select_set_[cur].insert_range(first);
            if (!node.empty()) { // first contains epsilon
                select_set_[cur].insert_range(follow_set_[from]);
            }
        }

    select_set_built = true;
}

std::unordered_set<Symbol>
Grammar::extract_epsilon(std::unordered_set<Symbol> s)
{
    s.extract(epsilon);
    return s;
}

Single_production
Grammar::get_single_production(Single_production_handle const &handle) const
{
    return {.from{handle.from},
            .to{productions_.at(handle.from)[handle.index]}};
}

std::pair<Single_production, std::unordered_set<Symbol>>
Select_set_iterator::operator*() const
{
    auto const &[handle, set] = *it_;
    return {owner_->get_single_production(handle), set};
}

Select_set_iterator Select_set_view::begin()
{
    return Select_set_iterator{owner_->select_set_.begin(), owner_};
}

Select_set_iterator Select_set_view::end()
{
    return Select_set_iterator{owner_->select_set_.end(), owner_};
}

Select_set_view Grammar::select_set_view() const
{
    return Select_set_view{this};
}
