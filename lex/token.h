#pragma once
#include <format>
#include <string>
#include <string_view>
#include <utility>

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

struct SourceRange {
    SourceLocation begin;
    SourceLocation end;
};

template <>
struct std::formatter<SourceRange> : std::formatter<std::string_view> {
    static auto format(SourceRange const &range, format_context &ctx)
    {
        return std::format_to(ctx.out(), "{}-{}", range.begin, range.end);
    }
};

enum class TokenKind : unsigned char {
    unknown,

    keyword_int,
    keyword_float,
    keyword_char,
    keyword_string, // Should be removed.
    keyword_return,
    keyword_if,
    keyword_else,
    keyword_while,
    keyword_var,
    keyword_func,

    identifier,
    integer_literal,
    string_literal,
    float_literal,
    comment,   // //
    colon,     // :
    semicolon, // ;
    comma,     // ,

    // Operators
    plus,
    minus,
    star,
    slash,
    percent,
    equal,

    // Comparator operators
    equalequal,
    less,
    more,
    lessthan,
    morethan,

    l_brace,
    r_brace,

    l_bracket,
    r_bracket,

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
    case keyword_char:
        return "keyword_char";
    case keyword_string:
        return "keyword_string";
    case keyword_return:
        return "keyword_return";
    case keyword_if:
        return "keyword_if";
    case keyword_else:
        return "keyword_else";
    case keyword_while:
        return "keyword_while";
    case keyword_var:
        return "keyword_var";
    case keyword_func:
        return "keyword_func";

    case identifier:
        return "identifier";
    case integer_literal:
        return "integer_val";
    case float_literal:
        return "float_val";
    case string_literal:
        return "string_val";
    case comment:
        return "comment";
    case colon:
        return "colon";
    case semicolon:
        return "semicolon";
    case comma:
        return "comma";

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

    case equalequal:
        return "equalequal";
    case less:
        return "less";
    case lessthan:
        return "lessthan";
    case more:
        return "more";
    case morethan:
        return "morethan";

    case l_brace:
        return "l_brace";
    case r_brace:
        return "r_brace";
    case l_bracket:
        return "l_bracket";
    case r_bracket:
        return "r_bracket";
    case l_paren:
        return "l_paren";
    case r_paren:
        return "r_paren";

    case eof:
        return "eof";
    }

    return "unknown";
}

inline bool is_type_keyword(TokenKind kind)
{
    using enum TokenKind;
    return kind == keyword_int || kind == keyword_float ||
           kind == keyword_char || kind == keyword_string;
}

inline bool is_likely_type(TokenKind k)
{
    switch (k) {
    case TokenKind::keyword_int:
    case TokenKind::keyword_float:
    case TokenKind::keyword_char:
    case TokenKind::keyword_string:
    case TokenKind::identifier:
        return true;
    default:
        return false;
    }
    std::unreachable();
}

inline bool is_keyword(TokenKind kind)
{
    switch (kind) {
    case TokenKind::keyword_int:
    case TokenKind::keyword_float:
    case TokenKind::keyword_string:
    case TokenKind::keyword_return:
    case TokenKind::keyword_if:
    case TokenKind::keyword_else:
    case TokenKind::keyword_while:
        return true;
    default:
        return false;
    }
}

struct Token {
    [[nodiscard]] bool is(TokenKind k) const
    {
        return kind == k;
    }

    [[nodiscard]] bool is_not(TokenKind k) const
    {
        return !is(k);
    }

    TokenKind kind{TokenKind::unknown};
    std::string value;
    SourceRange source_range{};
    // FIXME: add this [[deprecated("Use source range please")]]
    SourceLocation source_location{};
};
