#pragma once
#include <ast.h>
#include <diagnostics.h>
#include <lexer.h>

class Parser {
  public:
    Parser(Lexer &lexer, Diagnostics &diags) : lexer_(lexer), diags_(diags) {}

    std::unique_ptr<ast::Program> parse_program();

  private:
    std::unique_ptr<ast::Declaration> parse_declaration();
    std::unique_ptr<ast::Statement> parse_statement();
    std::unique_ptr<ast::CompoundStatement> parse_compound_statement();
    std::unique_ptr<ast::Expression> parse_expression();

    [[nodiscard]] Token const &peek() const;
    Token consume();
    bool expect(TokenKind);
    bool expect_and_consume(TokenKind);

    Lexer &lexer_;
    Diagnostics &diags_;
};
