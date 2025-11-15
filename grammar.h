#pragma once
#include <deque>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Symbol = std::string;
using Symbol_string = std::deque<Symbol>;
using Symbol_set = std::unordered_set<Symbol>;

constexpr Symbol eol{"$"};

struct Production {
    Symbol from; // Will be recognized as nonterminal
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
    using Iterator = std::unordered_map<Single_production_handle,
                                        Symbol_set>::const_iterator;

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

using LL1_parse_table =
    std::unordered_map<Symbol, std::unordered_map<Symbol, Single_production>>;

class Grammar {
    friend class Select_set_view;
    using Nullable_set = Symbol_set;
    using First_set = std::unordered_map<Symbol, Symbol_set>;
    using Follow_set = std::unordered_map<Symbol, Symbol_set>;
    using Select_set = std::unordered_map<Single_production_handle, Symbol_set>;

  public:
    static Grammar from_file(std::filesystem::path const &p);

    Grammar(std::vector<Production> productions, Symbol epsilon);

    void build();
    void summary() const;

    [[nodiscard]] bool is_nullable(Symbol const &s) const;
    [[nodiscard]] Symbol_set const &first(Symbol const &s) const;
    [[nodiscard]] Symbol_set first(Symbol_string const &str) const;
    [[nodiscard]] Symbol_set const &follow(Symbol const &s) const;
    [[nodiscard]] Symbol_set const &
    select(Single_production_handle const &handle) const;

    [[nodiscard]] LL1_parse_table ll1_parse_table() const;

    [[nodiscard]] Single_production
    get_single_production(Single_production_handle const &handle) const;

    [[nodiscard]] bool match(Symbol_string s) const;
    [[nodiscard]] bool match_with_recursive_descending(Symbol_string s) const;

  private:
    bool is_epsilon(Symbol_string const &str) const;

    Symbol_set extract_epsilon(Symbol_set s) const;

    [[nodiscard]] Select_set_view select_set_view() const;

    void add_production(Production r);

    void build_terminals();

    void build_nullable_set();
    void build_first_set();
    void build_follow_set();
    void build_select_set();

    Symbol begin_;
    Symbol epsilon_;

    std::unordered_map<Symbol, std::vector<Symbol_string>> productions_;
    Symbol_set nonterminals_;
    Symbol_set terminals_;

    // After being built, followings will have valid values.
    // FIRST SET of the Grammar
    std::unordered_set<Symbol> nullables_; // Symbols that can derive to epsilon
    std::unordered_map<Symbol, Symbol_set> first_set_;
    std::unordered_map<Symbol, Symbol_set> follow_set_;
    Select_set select_set_;

    std::unordered_set<Symbol> invalid_symbols_; // Contains invalid symbols
};
