#pragma once
#include <ast/program.h>
#include <ast/stmt.h>
#include <diagnostics.h>
#include <lex/lexer.h>
#include <source_location>
#include <stacktrace>

/// @brief Does grammar analysis
class Parser {
  public:
    Parser(Lexer &lexer, Diagnostics &diags) : lexer_(lexer), diags_(diags) {}

    std::unique_ptr<ast::Program> parse_program();

  private:
    std::unique_ptr<ast::Declaration> parse_declaration();
    std::unique_ptr<ast::Statement> parse_statement();
    std::unique_ptr<ast::Statement> parse_if_statement();
    std::unique_ptr<ast::Statement> parse_while_statement();
    std::unique_ptr<ast::CompoundStatement> parse_compound_statement();
    ast::ExpressionPtr parse_expression();
    ast::ExpressionPtr try_parse_assignment_expr();
    ast::ExpressionPtr try_parse_equality_expr();
    ast::ExpressionPtr try_parse_add_minus();
    ast::ExpressionPtr try_parse_mult_div_mod();
    ast::ExpressionPtr try_parse_unary_expr();
    ast::ExpressionPtr try_parse_postfix_expr();
    ast::ExpressionPtr parse_primary_expression();

    [[nodiscard]] Token const &peek() const;
    Token consume();
    bool expect(TokenKind);
    bool expect_true(std::invocable<TokenKind> auto &&pred,
                     std::string_view what)
    {
        if (!pred(peek().kind)) {
            diags_.error("{}: expected '{}', got '{}'", peek().source_location,
                         what, peek().value);
            diags_.error("Stacktrace: {}", std::stacktrace::current());
            return false;
        }
        return true;
    }
    bool expect_and_consume(TokenKind);
    [[nodiscard]] Token const &previous_token() const
    {
        if (!previous_token_.has_value()) {
            throw std::runtime_error{"previous token unavailble before the "
                                     "first call to `consume()`"};
        }
        return *previous_token_;
    }

    // Unavailble before the first call to `consume()`
    std::optional<Token> previous_token_;
    Lexer &lexer_;
    Diagnostics &diags_;
};
