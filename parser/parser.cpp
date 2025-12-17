#include <parser/parser.h>

using namespace ast;

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
                     to_string(kind), peek().value);
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

    if (!expect_true(is_type, "is_type")) {
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
        fn->return_type_ = type_tok.kind;
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
    case TokenKind::semicolon:
        consume();
        return std::make_unique<ast::EmptyStatement>();

    default: {
        auto stmt = std::make_unique<ast::ExpressionStatement>();
        stmt->expr_ = parse_expression();
        if (!expect_and_consume(TokenKind::semicolon))
            return nullptr;
        return stmt;
    }
    }
}

// lhs op rhs
// 左结合
ast::ExpressionPtr Parser::parse_expression()
{
    return try_parse_add_minus();
}

ast::ExpressionPtr Parser::try_parse_add_minus()
{
    auto expr = std::make_unique<ast::BinaryOperationExpr>();
    expr->lhs_ = try_parse_mult_div_mod();
    if (!expr->lhs_)
        return nullptr;

    while (true) {
        switch (Token tok = peek(); tok.kind) {
        case TokenKind::plus:
        case TokenKind::minus: {
            expr->op_ = consume().value;
            expr->rhs_ = try_parse_mult_div_mod();
            if (!expr->rhs_)
                return nullptr;

            auto old = std::exchange(
                expr, std::make_unique<ast::BinaryOperationExpr>());
            expr->lhs_ = std::move(old);
            break;
        }
        case TokenKind::star:
        case TokenKind::slash:
        case TokenKind::percent:
        default:
            return std::move(expr->lhs_);
        }
    }
}

ast::ExpressionPtr Parser::try_parse_mult_div_mod()
{
    auto bin = std::make_unique<ast::BinaryOperationExpr>();
    bin->lhs_ = try_parse_unary_expr();
    if (!bin->lhs_)
        return nullptr;

    while (true) {
        switch (Token tok = peek(); tok.kind) {
        case TokenKind::star:
        case TokenKind::slash:
        case TokenKind::percent: {
            bin->op_ = consume().value;
            bin->rhs_ = try_parse_unary_expr();
            if (!bin->rhs_)
                return nullptr;

            auto old = std::exchange(
                bin, std::make_unique<ast::BinaryOperationExpr>());
            bin->lhs_ = std::move(old);
            break;
        }
        default:
            return std::move(bin->lhs_);
        }
    }
}

ast::ExpressionPtr Parser::try_parse_unary_expr()
{
    // Recursively consumes unary operators
    if (auto tok = peek();
        tok.is(TokenKind::plus) || tok.is(TokenKind::minus)) {
        auto unary = std::make_unique<ast::UnaryOperationExpr>();
        unary->op_ = consume().value;
        unary->expr_ = try_parse_unary_expr();
        return unary;
    }

    return try_parse_postfix_expr();
}

ast::ExpressionPtr Parser::try_parse_postfix_expr()
{
    auto call = std::make_unique<CallExpression>();
    call->callee_ = parse_primary_expression();
    if (!call->callee_)
        return nullptr;

    while (true) {
        if (peek().is(TokenKind::l_paren)) {
            consume();
            if (!expect_and_consume(TokenKind::r_paren))
                return nullptr;

            auto old = std::exchange(call, std::make_unique<CallExpression>());
            call->callee_ = std::move(old);
            continue;
        }
        break;
    }

    return std::move(call->callee_);
}

ast::ExpressionPtr Parser::parse_primary_expression()
{
    using enum TokenKind;

    switch (Token tok = peek(); tok.kind) {
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
    case l_paren: {
        consume();
        auto expr = parse_expression();
        expect_and_consume(r_paren);
        return expr;
    }
    default:
        expect_true([](auto) { return false; }, "primary expression");
        // diags_.error("{}: expected 'expression', got '{}'",
        // tok.source_location, tok.value);
        consume();
        return nullptr;
    }
}
