#include <lex/lexer.h>

TokenKind identifier_to_token_kind(std::string_view s) noexcept
{
    using enum TokenKind;

    static std::unordered_map<std::string_view, TokenKind> const map{
        {"int", keyword_int},       {"float", keyword_float},
        {"string", keyword_string}, {"return", keyword_return},
        {"if", keyword_if},         {"else", keyword_else},
        {"while", keyword_while},
    };

    if (map.contains(s))
        return map.at(s);
    return identifier;
}

Lexer::Lexer(std::filesystem::path const &path)
    : ifs_(path), source_location_{.row = 1, .column = 1}
{
    if (!ifs_.is_open()) {
        throw std::runtime_error{
            std::format("Cannot open '{}'", path.string())};
    }
}

Token Lexer::lex()
{
    Token tok;
    while ((tok = lex_one()).is(TokenKind::comment)) {
        // Skips comments
    }
    return tok;
}

void Lexer::lex_comment(Token &result)
{
    result.kind = TokenKind::comment;
    char ch;
    while (true) {
        ch = read_char();
        if (ch == '\n' || ch == EOF) {
            break;
        }
        result.value += ch;
    }
}

void Lexer::lex_numeric(Token &result)
{
    result.kind = TokenKind::integer_val;
    while (true) {
        char ch = peek_char();
        if (std::isdigit(ch) != 0) {
            result.value += ch;
        }
        else if (ch == '.') {
            if (result.kind == TokenKind::integer_val) {
                result.kind = TokenKind::float_val;
                result.value += ch;
            }
            else if (result.kind == TokenKind::float_val) {
                break;
            }
        }
        else {
            break;
        }
        read_char();
    }
}

void Lexer::lex_identifier(Token &result)
{
    while (true) {
        char ch = peek_char();
        if (std::isalnum(ch) != 0 || ch == '_') {
            result.value += ch;
        }
        else {
            break;
        }
        read_char();
    }

    result.kind = identifier_to_token_kind(result.value);
}

void Lexer::lex_string(Token &result)
{
    result.kind = TokenKind::string_val;

    if (read_char() != '\"') {
        throw std::logic_error{"Impossible. Must be an error in hlvm"};
    }
    result.value += '\"';

    while (true) {
        char ch = read_char();
        if (ch == EOF) {
            result.kind = TokenKind::unknown;
            break;
        }
        result.value += ch;
        if (ch == '\"') {
            break;
        }
    }
}

void Lexer::lex_semicolon(Token &result)
{
    result.kind = TokenKind::semicolon;
    result.value = read_char();
}

void Lexer::lex_operator(Token &result)
{
    auto ch = read_char();
    result.value += ch;
    switch (ch) {
    case '+':
        result.kind = TokenKind::plus;
        break;
    case '-':
        result.kind = TokenKind::minus;
        break;
    case '*':
        result.kind = TokenKind::star;
        break;
    case '/':
        result.kind = TokenKind::slash;
        break;
    case '%':
        result.kind = TokenKind::percent;
        break;
    case '(':
        result.kind = TokenKind::l_paren;
        break;
    case ')':
        result.kind = TokenKind::r_paren;
        break;
    case '[':
        result.kind = TokenKind::l_bracket;
        break;
    case ']':
        result.kind = TokenKind::r_bracket;
        break;
    case '{':
        result.kind = TokenKind::l_brace;
        break;
    case '}':
        result.kind = TokenKind::r_brace;
        break;
    case '=':
        if (peek_char() == '=') { // equalequal ==
            result.value += read_char();
            result.kind = TokenKind::equalequal;
        }
        else {
            result.kind = TokenKind::equal;
        }
        break;
    case '<':
        if (peek_char() == '=') {
            result.value += read_char();
            result.kind = TokenKind::lessthan;
        }
        else {
            result.kind = TokenKind::less;
        }
        break;
    case '>':
        if (peek_char() == '=') {
            result.value += read_char();
            result.kind = TokenKind::morethan;
        }
        else {
            result.kind = TokenKind::more;
        }
        break;
    case ',':
        result.kind = TokenKind::comma;
        break;
    default:
        result.kind = TokenKind::unknown;
    }
}

char Lexer::read_char()
{
    char ch;
    ifs_.read(&ch, 1);

    if (ifs_.eof()) {
        return EOF;
    }

    last_source_location_ = source_location_;
    if (ch == '\n') {
        source_location_.column = 1;
        source_location_.row += 1;
    }
    else {
        source_location_.column += 1;
    }
    return ch;
}

char Lexer::peek_char()
{
    auto ch = ifs_.peek();
    if (ch == std::ifstream::traits_type::eof()) {
        return EOF;
    }
    return static_cast<char>(ch);
}

Token Lexer::lex_one()
{
    // Fetch peeked value.
    if (peeked_token_.has_value()) {
        auto ret = std::move(*peeked_token_);
        peeked_token_.reset();
        return ret;
    }

    char ch; // Current char

    // Reads out spaces and newlines, as they are not any part of token.
    while (true) {
        ch = peek_char();
        if (std::isspace(ch) != 0) {
            read_char();
        }
        else {
            break;
        }
    }

    Token result;
    result.source_location = source_location_;
    result.source_range.begin = source_location_;

    if (ch == EOF) {
        result.kind = TokenKind::eof;
        return result;
    }

    // clang-format off
    switch (ch) {
    case '#':
        lex_comment(result);
        break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        lex_numeric(result);
        break;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    case '_':
        lex_identifier(result);
        break;
    case '\"':
        lex_string(result);
        break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ',':
    case '=':
    case '<':
    case '>':
        lex_operator(result);
        break;
    case ';':
        lex_semicolon(result);
        break;
    default:
        result.kind = TokenKind::unknown;
        result.value = read_char();
    }
    // clang-format on

    result.source_range.end = last_source_location_;
    return result;
}
