#pragma once
#include <filesystem>
#include <format>
#include <fstream>
#include <lex/token.h>
#include <string_view>
#include <unordered_map>

TokenKind identifier_to_token_kind(std::string_view s) noexcept;

class Lexer {
  public:
    Lexer(std::filesystem::path const &path);

    Token const &peek() const;
    Token lex();

  private:
    Token lex_one();

    void lex_comment(Token &result);
    void lex_numeric(Token &result);

    // This also filters out keywords.
    void lex_identifier(Token &result);

    void lex_string(Token &result);
    void lex_semicolon(Token &result);
    void lex_operator(Token &result);

    char read_char();
    char peek_char();

    std::ifstream ifs_;
    SourceLocation last_source_location_;
    SourceLocation source_location_;
    mutable std::optional<Token> peeked_token_;
};
