// module;
#include <grammar.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <ranges>
#include <utility>

// module grammar;

Grammar::Grammar(std::vector<Production> productions, Symbol epsilon)
    : begin_{productions.front().from}, epsilon_{std::move(epsilon)}
{
    for (auto &production : productions)
        add_production(std::move(production));

    build();
}

void Grammar::add_production(Production r)
{
    nonterminals_.insert(r.from); // All r.from are considered as nonterminals,
                                  // and other symbols are considered terminal.
    productions_[r.from].append_range(std::move(r.tos));
}

void Grammar::build()
{
    // Nonterminals have been built in `add`ing production phase.
    build_terminals();

    build_nullable_set();
    build_first_set();
    build_follow_set();
    build_select_set();
}

bool Grammar::is_nullable(Symbol const &s) const
{
    return nullables_.contains(s);
}

Symbol_set const &Grammar::first(Symbol const &s) const
{
    return first_set_.at(s);
}

Symbol_set Grammar::first(Symbol_string const &str) const
{
    if (is_epsilon(str)) {
        return {epsilon_};
    }

    Symbol_set res;

    bool nullable{true};
    for (auto const &symbol : str) {
        if (terminals_.contains(symbol)) {
            res.insert(symbol);
        }
        else {
            assert(nonterminals_.contains(symbol));
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
        res.insert(epsilon_);

    return res;
}

Symbol_set const &Grammar::follow(Symbol const &s) const
{
    return follow_set_.at(s);
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
            if (to.size() == 1 && to[0] == epsilon_)
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
}

void Grammar::build_first_set()
{
    for (auto const &nonterminal : nonterminals_)
        if (nullables_.contains(nonterminal))
            first_set_[nonterminal].insert(epsilon_);

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
                        terminals_.contains(symbol)
                            ? std::unordered_set{symbol}
                            : extract_epsilon(first_set_[symbol]));
                    auto cur_size = first_set_[from].size();
                    changed |= prev_size != cur_size;
                    if (!nullables_.contains(symbol))
                        break;
                }
            }
    } while (changed);
}

void Grammar::build_follow_set()
{
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
                    if (nonterminals_.contains(e)) {
                        auto fs = first(beta);
                        auto node = fs.extract(epsilon_); // Step 3
                        auto prev_size = follow_set_[e].size();
                        follow_set_[e].insert_range(fs); // Step 2
                        if (!node.empty()) // FIRST(beta) contains epsilon
                            follow_set_[e].insert_range(follow_set_[from]);
                        auto cur_size = follow_set_[e].size();
                        changed |= prev_size != cur_size;
                    }
                    beta.push_front(e);
                }
            }
    } while (changed);
}

void Grammar::build_select_set()
{
    for (auto const &[from, tos] : productions_)
        for (auto const &[i, to] : tos | std::views::enumerate) {
            auto idx = static_cast<std::size_t>(i);
            auto cur = Single_production_handle{.from = from, .index = idx};
            auto fs = first(to);
            auto node = fs.extract(epsilon_);
            select_set_[cur].insert_range(fs);
            if (!node.empty()) { // first contains epsilon
                select_set_[cur].insert_range(follow_set_[from]);
            }
        }
}

Symbol_set Grammar::extract_epsilon(Symbol_set s) const
{
    s.extract(epsilon_);
    return s;
}

Single_production
Grammar::get_single_production(Single_production_handle const &handle) const
{
    return {.from{handle.from},
            .to{productions_.at(handle.from)[handle.index]}};
}

std::pair<Single_production, Symbol_set> Select_set_iterator::operator*() const
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

void Grammar::build_terminals()
{
    for (auto const &[_, tos] : productions_)
        for (auto const &to : tos)
            for (auto const &s : to)
                if (!nonterminals_.contains(s))
                    terminals_.insert(s);
}

void Grammar::summary() const
{
    std::println("\033[36mNullable set\033[0m: {}", nullables_);
    std::println("\033[36mFIRST set\033[0m: {}", first_set_);
    std::println("\033[36mFOLLOW set\033[0m: {}", follow_set_);
    std::println("\033[36mSELECT set:\033[0m");
    for (auto const &[prod, set] : select_set_view()) {
        std::println("    {}: {},", prod, set);
    }

    auto map = ll1_parse_table();
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
}

bool Grammar::is_epsilon(Symbol_string const &str) const
{
    return str.size() == 1 && str[0] == epsilon_;
}

bool Grammar::match(Symbol_string s) const
{
    assert(std::ranges::all_of(
        s, [this](auto const &s) { return terminals_.contains(s); }));

    s.push_back(eol);
    Symbol_string t{eol, begin_}; // Acts as a stack
    auto ll1t = ll1_parse_table();
    while (!t.empty()) {
        // std::println("t={}  {}=s", t, s);
        if (terminals_.contains(t.back()) || t.back() == eol) {
            if (t.back() != s.front()) {
                // std::println("Terminal doesn't match, failed to match");
                return false;
            }
            t.pop_back();
            s.pop_front();
            continue;
        }
        auto jt = ll1t[t.back()].find(s.front());
        if (jt == ll1t[t.back()].end()) {
            // std::println("No item to match, failed to match");
            return false;
        }
        auto const &prod = jt->second;
        // std::println("Using production {}", prod);
        assert(prod.from == t.back());
        t.pop_back();
        if (!is_epsilon(prod.to))
            t.insert_range(t.end(), prod.to | std::views::reverse);
    }
    // No need to check eol because t won't contain eol. TODO: maybe this should
    // be checked?
    // std::println("All symbols in t matched, matching ok");
    return true;
}

LL1_parse_table Grammar::ll1_parse_table() const
{
    LL1_parse_table table;
    for (auto const &[prod, set] : select_set_view()) {
        for (auto const &terminal : set) {
            if (table[prod.from].contains(terminal))
                throw std::runtime_error{"Not a LL(1) grammar"};
            table[prod.from][terminal] = prod;
        }
    }
    return table;
}

Grammar Grammar::from_file(std::filesystem::path const &p)
{
    std::ifstream ifs{p};

    std::string ep;
    ifs >> ep;
    int n;
    ifs >> n;
    ifs.get();
    // LINE FORMAT: A -> B d | a | <epsilon>
    std::vector<Production> prods;
    for (int i{}; i != n; ++i) {
        std::string line;
        std::getline(ifs, line);
        Production r;

        auto to_sv =
            std::views::transform([](auto t) { return std::string_view{t}; });
        auto tokens = line | std::views::split(' ') | to_sv |
                      std::ranges::to<std::vector>();

        auto from = (tokens | std::views::take(1)).front();
        auto tos = tokens | std::views::drop(2) | std::views::split("|") |
                   std::ranges::to<std::vector<Symbol_string>>();
        r.from = from;
        r.tos = tos;

        prods.push_back(r);
    }

    return {prods, ep};
}

Symbol_set const &Grammar::select(Single_production_handle const &handle) const
{
    return select_set_.at(handle);
}

bool Grammar::match_with_recursive_descending(Symbol_string str) const
{
    auto const &tbl = ll1_parse_table();
    auto it = str.begin();
    auto token = [this, &it, &str](Symbol const &symbol) {
        // std::println("Go into {}", symbol);
        if (symbol == epsilon_) {
            return true;
        }
        bool res{it != str.end() && symbol == *it};
        ++it;
        return res;
    };
    std::unordered_map<Symbol, std::function<bool()>> functions;
    for (auto const &nonterminal : nonterminals_) {
        auto fn = [this, &nonterminal, token, &it, &functions, &tbl]() -> bool {
            // std::println("Go into {}", nonterminal);
            return tbl.at(nonterminal).contains(*it) &&
                   std::ranges::all_of(
                       tbl.at(nonterminal).at(*it).to,
                       [this, token, &functions](auto const &s) {
                           if (terminals_.contains(s) || s == epsilon_)
                               return token(s);
                           return functions.at(s)();
                       });
            // Another implemetation not using std::ranges::allof.
            // if (!tbl.at(nonterminal).contains(*it)) {
            //     return false;
            // }
            // for (auto const &s : tbl.at(nonterminal).at(*it).to) {
            //     if (terminals_.contains(s) || s == epsilon_)
            //         if (!token(s))
            //             return false;
            //     if (!functions.at(s)())
            //         return false;
            // }
            // return true;
        };
        functions.insert({nonterminal, fn});
    };

    return functions.at(begin_)();
}
