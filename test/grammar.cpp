#include <grammar.h>
#include <gtest/gtest.h>
#include <print>
#include <ranges>

TEST(Grammar, SimpleTest)
{
    auto g = Grammar::from_file("./test/test.g");

    auto const ns = std::unordered_set<Symbol>{{"S"}, {"M"}, {"H"}, {"K"}};
    for (auto const &s : ns)
        ASSERT_TRUE(g.is_nullable(s));

    using set = std::unordered_map<Symbol, SymbolSet>;
    auto const first_set = set{{"S", {"d", "a", "e", "b", "#"}},
                               {"H", {"e", "#"}},
                               {"K", {"d", "#"}},
                               {"L", {"e"}},
                               {"M", {"d", "b", "#"}}};
    for (auto const &[k, v] : first_set)
        ASSERT_TRUE(g.first(k) == v);

    auto const follow_set = set{{"H", {"$", "0", "f"}},
                                {"L", {"$", "e", "0", "a", "d", "b"}},
                                {"M", {"$", "0", "e"}},
                                {"K", {"e", "0", "$"}},
                                {"S", {"0", "$"}}};
    for (auto const &[k, v] : follow_set)
        ASSERT_TRUE(g.follow(k) == v);

    /*
    #
    5
    S -> M H | a
    H -> L S 0 | #
    K -> d M L | #
    L -> e H f
    M -> K | b L M
    */
    auto const select_set =
        std::unordered_map<SingleProductionHandle, SymbolSet>{
            {{.from{"S"}, .index = 0}, {"$", "0", "d", "b", "e"}},
            {{.from{"S"}, .index = 1}, {"a"}},

            {{.from{"H"}, .index = 0}, {"e"}},
            {{.from{"H"}, .index = 1}, {"f", "0", "$"}},

            {{.from{"K"}, .index = 0}, {"d"}},
            {{.from{"K"}, .index = 1}, {"$", "0", "e"}},

            {{.from{"L"}, .index = 0}, {"e"}},

            {{.from{"M"}, .index = 0}, {"e", "0", "$", "d"}},
            {{.from{"M"}, .index = 1}, {"b"}},
        };

    for (auto const &[handle, s] : select_set)
        ASSERT_TRUE(g.select(handle) == s);

    auto const a = Symbol{"a"};
    auto const b = Symbol{"b"};
    auto const e = Symbol{"e"};
    auto const f = Symbol{"f"};
    auto const zero = Symbol{"0"};

    ASSERT_TRUE(g.match({a}));
    ASSERT_TRUE(g.match({b, e, f, e, f, a, zero}));
    ASSERT_FALSE(g.match({b}));

    ASSERT_TRUE(g.match_with_recursive_descending({a}));
    ASSERT_TRUE(g.match_with_recursive_descending({b, e, f, e, f, a, zero}));
    ASSERT_FALSE(g.match_with_recursive_descending({b}));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
