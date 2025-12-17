#include <ast/ast.h>
#include <determinstic-finite-automaton.h>
#include <grammar.h>
#include <gtest/gtest.h>
#include <lex/lexer.h>
#include <nondeterminstic-finite-automaton.h>
#include <parser/parser.h>
#include <print>
#include <semantic/semantic-analyzer.h>

TEST(Automaton, Basic)
{
    RegularExpression re("");

    NFA nfa({0}, {2});
    nfa.add(0, 1, 'a');
    nfa.add(0, 2, 'c');
    nfa.add(1, 2, 'b');
    nfa.add(2, 2, 'd');

    auto dfa = DFA::from_nfa(nfa);

    EXPECT_TRUE(dfa.match("ab"));
    EXPECT_TRUE(dfa.match("c"));
    EXPECT_TRUE(dfa.match("abdd"));
    EXPECT_TRUE(dfa.match("cdd"));
}

TEST(Lexer, Basic)
{
    Lexer lexer("system64.hlvm");
    while (true) {
        auto token = lexer.lex();
        if (token.is(TokenKind::eof)) {
            break;
        }
        std::println(R"(token: type: {:12} in {}: {})", to_string(token.kind),
                     token.source_location, token.value);
    }
}

TEST(AST, Basic)
{
    Lexer lexer("system64.hlvm");
    Diagnostics diags;

    Parser parser(lexer, diags);

    auto prog = parser.parse_program();
    ASSERT_TRUE(prog);
    prog->dump(std::cout);
}

TEST(Sematic, Basic)
{
    Lexer lexer("system64.hlvm");
    Diagnostics diags;

    Parser parser(lexer, diags);

    auto prog = parser.parse_program();
    ASSERT_TRUE(prog);

    semantic::SemanticAnalyzer analyzer(&diags);
    analyzer.analyze(*prog);
    analyzer.summary();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
