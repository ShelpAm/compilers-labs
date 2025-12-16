#pragma once
#include <filesystem>
#include <format>
#include <fstream>
#include <string_view>
#include <unordered_map>

struct SourceLocation {
    std::size_t row;
    std::size_t column;
};

template <>
struct std::formatter<SourceLocation> : std::formatter<std::string_view> {
    static auto format(SourceLocation const &loc, format_context &ctx)
    {
        return std::format_to(ctx.out(), "{}:{}", loc.row, loc.column);
    }
};

enum class TokenKind : unsigned char {
    unknown,

    keyword_int,
    keyword_float,
    keyword_string,
    keyword_return,

    identifier,
    integer_val,
    string_val,
    float_val,
    comment,
    semicolon,

    // Operators
    plus,
    minus,
    star,
    slash,
    percent,
    equal,

    l_brace,
    r_brace,

    l_paren,
    r_paren,

    eof
};

constexpr std::string_view to_string(TokenKind kind) noexcept
{
    using enum TokenKind;

    switch (kind) {
    case unknown:
        return "unknown";

    case keyword_int:
        return "keyword_int";
    case keyword_float:
        return "keyword_float";
    case keyword_string:
        return "keyword_string";
    case keyword_return:
        return "keyword_return";

    case identifier:
        return "identifier";
    case integer_val:
        return "integer_val";
    case float_val:
        return "float_val";
    case string_val:
        return "string_val";
    case comment:
        return "comment";
    case semicolon:
        return "semicolon";

    case plus:
        return "plus";
    case minus:
        return "minus";
    case star:
        return "star";
    case slash:
        return "slash";
    case percent:
        return "percent";
    case equal:
        return "equal";

    case l_brace:
        return "l_brace";
    case r_brace:
        return "r_brace";
    case l_paren:
        return "l_paren";
    case r_paren:
        return "r_paren";

    case eof:
        return "eof";
    }

    // 理论上不可达，但防御性兜底
    return "unknown";
}

TokenKind identifier_to_token_kind(std::string_view s) noexcept;

struct Token {
    [[nodiscard]] bool is(TokenKind k) const
    {
        return kind == k;
    }

    TokenKind kind{TokenKind::unknown};
    std::string value;
    SourceLocation source_location{};
};

class Lexer {
  public:
    Lexer(std::filesystem::path const &path);

    Token const &peek() const;
    Token lex();

  private:
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
    SourceLocation source_location_;
    mutable std::optional<Token> peeked_token_;
};
