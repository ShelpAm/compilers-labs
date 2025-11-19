#pragma once
#include <deque>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Symbol = std::string;
using SymbolString = std::deque<Symbol>;
using SymbolSet = std::unordered_set<Symbol>;

constexpr Symbol eol{"$"};

struct Production {
    Symbol from; // Will be recognized as nonterminal
    std::vector<SymbolString> tos;
};

struct SingleProduction {
    Symbol from;
    SymbolString to;
};
template <>
struct std::formatter<SingleProduction> : std::formatter<std::string> {
    auto format(SingleProduction const &sp, std::format_context &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("{}->{}", sp.from, sp.to), ctx);
    }
};

struct SingleProductionHandle {
    auto operator<=>(SingleProductionHandle const &) const = default;

    Symbol from;
    std::size_t index;
};
template <> struct std::hash<SingleProductionHandle> {
    std::size_t operator()(SingleProductionHandle const &handle) const noexcept
    {
        return std::hash<decltype(handle.from)>{}(handle.from) ^
               (std::hash<decltype(handle.index)>{}(handle.index) << 1);
    }
};
template <>
struct std::formatter<SingleProductionHandle> : std::formatter<std::string> {
    auto format(SingleProductionHandle const &handle,
                std::format_context &ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("Single_production_handle{{from={}, index={}}}",
                        handle.from, handle.index),
            ctx);
    }
};

class Grammar;

class SelectSetIterator {
    friend class SelectSetView;
    using iterator =
        std::unordered_map<SingleProductionHandle, SymbolSet>::const_iterator;

  public:
    bool operator!=(SelectSetIterator const &other) const
    {
        if (owner_ != other.owner_) {
            throw std::runtime_error{"owner of the two iterator differs"};
        }
        return it_ != other.it_;
    }

    SelectSetIterator &operator++()
    {
        ++it_;
        return *this;
    }

    SelectSetIterator operator++(int)
    {
        auto before = SelectSetIterator{it_++, owner_};
        return before;
    }

    std::pair<SingleProduction, std::unordered_set<Symbol>> operator*() const;

  private:
    SelectSetIterator(iterator it, Grammar const *owner)
        : it_{it}, owner_{owner}
    {
    }

    iterator it_;
    Grammar const *owner_;
};

class SelectSetView {
    friend class Grammar;

  public:
    SelectSetIterator begin();
    SelectSetIterator end();

  private:
    SelectSetView(Grammar const *owner) : owner_{owner} {}

    Grammar const *owner_;
};

using LL1ParseTable =
    std::unordered_map<Symbol, std::unordered_map<Symbol, SingleProduction>>;

class Grammar {
    friend class SelectSetView;
    using NullableSet = SymbolSet;
    using FirstSet = std::unordered_map<Symbol, SymbolSet>;
    using FollowSet = std::unordered_map<Symbol, SymbolSet>;
    using SelectSet = std::unordered_map<SingleProductionHandle, SymbolSet>;

  public:
    static Grammar from_file(std::filesystem::path const &p);

    Grammar(std::vector<Production> prods, Symbol epsilon);

    void build();
    void summary() const;

    [[nodiscard]] bool is_nullable(Symbol const &s) const;
    [[nodiscard]] SymbolSet const &first(Symbol const &s) const;
    [[nodiscard]] SymbolSet first(SymbolString const &str) const;
    [[nodiscard]] SymbolSet const &follow(Symbol const &s) const;
    [[nodiscard]] SymbolSet const &
    select(SingleProductionHandle const &handle) const;

    [[nodiscard]] LL1ParseTable compute_ll1_parse_table() const;

    [[nodiscard]] SingleProduction
    get_single_production(SingleProductionHandle const &handle) const;

    [[nodiscard]] bool match(SymbolString s) const;
    [[nodiscard]] bool match_with_recursive_descending(SymbolString s) const;

  private:
    bool is_epsilon(SymbolString const &str) const;

    SymbolSet extract_epsilon(SymbolSet s) const;

    [[nodiscard]] SelectSetView make_select_set_view() const;

    void add_production(Production r);

    void build_terminals();

    void build_nullable_set();
    void build_first_set();
    void build_follow_set();
    void build_select_set();

    Symbol begin_;
    Symbol epsilon_;

    std::unordered_map<Symbol, std::vector<SymbolString>> productions_;
    SymbolSet nonterminals_;
    SymbolSet terminals_;

    // After being built, followings will have valid values.
    // FIRST SET of the Grammar
    std::unordered_set<Symbol> nullables_; // Symbols that can derive to epsilon
    std::unordered_map<Symbol, SymbolSet> first_set_;
    std::unordered_map<Symbol, SymbolSet> follow_set_;
    SelectSet select_set_;

    std::unordered_set<Symbol> invalid_symbols_; // Contains invalid symbols
};
