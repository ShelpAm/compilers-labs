#pragma once

#include <deque>
#include <format>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Symbol = std::string;
using Symbol_string = std::deque<Symbol>;

constexpr Symbol epsilon{"#"};
constexpr Symbol eol{"$"};

struct Production {
    Symbol from;
    std::vector<Symbol_string> tos;
};

struct Single_production {
    Symbol from;
    Symbol_string to;
};
template <>
struct std::formatter<Single_production> : std::formatter<std::string> {
    auto format(Single_production const &sp, std::format_context &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("{}->{}", sp.from, sp.to), ctx);
    }
};

struct Single_production_handle {
    auto operator<=>(Single_production_handle const &) const = default;

    Symbol from;
    std::size_t index;
};
template <> struct std::hash<Single_production_handle> {
    std::size_t
    operator()(Single_production_handle const &handle) const noexcept
    {
        return std::hash<decltype(handle.from)>{}(handle.from) ^
               (std::hash<decltype(handle.index)>{}(handle.index) << 1);
    }
};
template <>
struct std::formatter<Single_production_handle> : std::formatter<std::string> {
    auto format(Single_production_handle const &handle,
                std::format_context &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("Single_production_handle{{from={}, index={}}}",
                        handle.from, handle.index),
            ctx);
    }
};

class Grammar;

class Select_set_iterator {
    friend class Select_set_view;
    using Iterator =
        std::unordered_map<Single_production_handle,
                           std::unordered_set<Symbol>>::const_iterator;

  public:
    bool operator!=(Select_set_iterator const &other) const
    {
        if (owner_ != other.owner_) {
            throw std::runtime_error{"owner of the two iterator differs"};
        }
        return it_ != other.it_;
    }

    Select_set_iterator &operator++()
    {
        ++it_;
        return *this;
    }

    Select_set_iterator operator++(int)
    {
        auto before = Select_set_iterator{it_++, owner_};
        return before;
    }

    std::pair<Single_production, std::unordered_set<Symbol>> operator*() const;

  private:
    Select_set_iterator(Iterator it, Grammar const *owner)
        : it_{it}, owner_{owner}
    {
    }

    Iterator it_;
    Grammar const *owner_;
};

class Select_set_view {
    friend class Grammar;

  public:
    Select_set_iterator begin();
    Select_set_iterator end();

  private:
    Select_set_view(Grammar const *owner) : owner_{owner} {}

    Grammar const *owner_;
};

class Grammar {
    friend class Select_set_view;

  public:
    Grammar(std::vector<Production> productions);

    void build();

    std::unordered_set<Symbol> const &empty_set() const;

    std::unordered_set<Symbol> first_set(Symbol_string const &str) const;

    Single_production
    get_single_production(Single_production_handle const &handle) const;

    [[nodiscard]] Select_set_view select_set_view() const;

  private:
    static bool is_terminal(Symbol const &s)
    {
        return s.size() == 1 &&
               (std::islower(s[0]) != 0 || (std::isdigit(s[0]) != 0));
    }

    static bool is_epsilon(Symbol_string const &str)
    {
        return str.size() == 1 && str[0] == epsilon;
    }

    void add_production(Production r);

    void build_nullable_set();
    void build_first_set();
    void build_follow_set();
    void build_select_set();

    static std::unordered_set<Symbol>
    extract_epsilon(std::unordered_set<Symbol> s);

    Symbol begin_;

    std::unordered_map<Symbol, std::vector<Symbol_string>> productions_;
    std::unordered_set<Symbol> nonterminals_;
    std::unordered_set<Symbol> terminals_;

    // After being built, followings will have valid values.
    bool built{};
    bool nullables_built{};
    bool first_set_built{};
    bool follow_set_built{};
    bool select_set_built{};

    // FIRST SET of the Grammar
    std::unordered_set<Symbol> nullables_; // Symbols that can derive to epsilon
    std::unordered_map<Symbol, std::unordered_set<Symbol>> first_set_;
    std::unordered_map<Symbol, std::unordered_set<Symbol>> follow_set_;
    std::unordered_map<Single_production_handle, std::unordered_set<Symbol>>
        select_set_;

    std::unordered_set<Symbol> invalid_symbols_; // Contains invalid symbols
};
