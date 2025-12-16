#include <parser.h>

using namespace ast;

namespace {

bool is_type(TokenKind kind)
{
    using enum TokenKind;
    return kind == keyword_int || kind == keyword_float ||
           kind == keyword_string;
}

} // namespace

Token const &Parser::peek() const
{
    return lexer_.peek();
}

Token Parser::consume()
{
    return lexer_.lex();
}

bool Parser::expect(TokenKind kind)
{
    if (!peek().is(kind)) {
        diags_.error("{}: expected '{}', got '{}'", peek().source_location,
                     to_string(kind), to_string(peek().kind));
        return false;
    }
    return true;
}

bool Parser::expect_and_consume(TokenKind kind)
{
    if (!expect(kind))
        return false;
    consume();
    return true;
}

std::unique_ptr<Program> Parser::parse_program()
{
    auto program = std::make_unique<Program>();

    while (!peek().is(TokenKind::eof)) {
        auto decl = parse_declaration();
        if (!decl)
            return nullptr;
        program->decls_.push_back(std::move(decl));
    }

    return program;
}

std::unique_ptr<ast::Declaration> Parser::parse_declaration()
{
    using enum TokenKind;

    if (!is_type(peek().kind)) {
        diags_.error("{}: expected type, got '{}'", peek().source_location,
                     to_string(peek().kind));
        return nullptr;
    }
    auto type_tok = consume();

    if (!expect(identifier))
        return nullptr;

    Token name_tok = consume();

    // -------- function declaration --------
    if (peek().is(l_paren)) {
        consume(); // (

        if (!expect_and_consume(r_paren))
            return nullptr;

        auto body = parse_compound_statement();
        if (!body)
            return nullptr;

        auto fn = std::make_unique<ast::FunctionDeclaration>();
        fn->name_ = name_tok.value;
        fn->body_ = std::move(body);
        return fn;
    }

    // -------- variable declaration --------
    std::unique_ptr<ast::Expression> init;

    if (peek().is(equal)) {
        consume(); // =
        init = parse_expression();
        if (!init)
            return nullptr;
    }

    if (!expect_and_consume(semicolon))
        return nullptr;

    auto var = std::make_unique<ast::VariableDeclaration>();
    var->type_ = type_tok.kind;
    var->name_ = name_tok.value;
    var->init_ = std::move(init);
    return var;
}

std::unique_ptr<CompoundStatement> Parser::parse_compound_statement()
{
    if (!expect_and_consume(TokenKind::l_brace))
        return nullptr;

    auto block = std::make_unique<CompoundStatement>();

    while (!peek().is(TokenKind::r_brace)) {
        auto stmt = parse_statement();
        if (!stmt)
            return nullptr;
        block->stmts_.push_back(std::move(stmt));
    }

    consume(); // }

    return block;
}

std::unique_ptr<ast::Statement> Parser::parse_statement()
{
    switch (peek().kind) {
    case TokenKind::keyword_return: {
        auto stmt = std::make_unique<ast::ReturnStatement>();

        consume(); // consume 'return'

        if (!peek().is(TokenKind::semicolon)) {
            stmt->value_ = parse_expression();
            if (!stmt->value_)
                return nullptr;
        }

        if (!expect_and_consume(TokenKind::semicolon))
            return nullptr;

        return stmt;
    }
    case TokenKind::l_brace:
        return parse_compound_statement();

    case TokenKind::keyword_int:
    case TokenKind::keyword_float:
    case TokenKind::keyword_string: {
        auto stmt = std::make_unique<ast::DeclarationStatement>();
        stmt->decl_ = parse_declaration();
        return stmt;
    }

    default: {
        auto stmt = std::make_unique<ast::ExpressionStatement>();
        stmt->expr_ = parse_expression();
        expect_and_consume(TokenKind::semicolon);
        return stmt;
    }
    }
}

std::unique_ptr<ast::Expression> Parser::parse_expression()
{
    using enum TokenKind;

    Token tok = peek();

    switch (tok.kind) {
    case identifier: {
        auto expr = std::make_unique<ast::IdentifierExpr>();
        expr->name_ = consume().value;
        return expr;
    }
    case integer_val: {
        auto expr = std::make_unique<ast::IntegerLiteralExpr>();
        expr->value_ = consume().value;
        return expr;
    }
    case float_val: {
        auto expr = std::make_unique<ast::FloatLiteralExpr>();
        expr->value_ = consume().value;
        return expr;
    }
    case string_val: {
        auto expr = std::make_unique<ast::StringLiteralExpr>();
        expr->value_ = consume().value;
        return expr;
    }
    default:
        diags_.error("{}: expected expression, got '{}'", tok.source_location,
                     to_string(tok.kind));
        consume();
        return nullptr;
    }
}
