#include <parser/parser.h>

#include <stacktrace>

using namespace ast;

Token const &Parser::peek() const
{
    return lexer_.peek();
}

Token Parser::consume()
{
    auto ret = lexer_.lex();
    previous_token_.emplace(ret);
    return ret;
}

bool Parser::expect(TokenKind kind)
{
    if (!peek().is(kind)) {
        diags_.error("{}: expected '{}', got '{}'", peek().source_location,
                     to_string(kind), peek().value);
        diags_.error("Stacktrace: {}", std::stacktrace::current());
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

    program->set_source_begin(program->decls_.front()->source_range().begin);
    program->set_source_begin(program->decls_.back()->source_range().end);
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
        auto fn = std::make_unique<ast::FunctionDeclaration>();

        consume(); // (

        // Parses parameters list
        bool first_time{true};
        while (peek().is_not(TokenKind::r_paren)) {
            if (!first_time && !expect_and_consume(TokenKind::comma))
                return nullptr;

            // Type
            auto can_be_type = [](auto k) {
                return is_type(k) || k == identifier;
            };
            if (!expect_true(can_be_type, "type"))
                return nullptr;

            auto param_type_tok = consume();
            std::string param_name;
            if (!peek().is(r_paren) && !peek().is(comma)) { // param name
                param_name = consume().value;
            }
            fn->parameters_.push_back({param_type_tok.value, param_name});

            if (first_time)
                first_time = false;
        }

        if (!expect_and_consume(r_paren))
            return nullptr;

        auto body = parse_compound_statement();
        if (!body)
            return nullptr;

        fn->set_source_begin(type_tok.source_location);
        fn->set_source_end(body->source_range().end);
        fn->return_type_ = type_tok;
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

    if (!expect(semicolon))
        return nullptr;

    auto semi_tok = consume();

    auto var = std::make_unique<ast::VariableDeclaration>();
    var->set_source_begin(type_tok.source_location);
    var->set_source_end(semi_tok.source_location);
    var->type_ = type_tok;
    var->name_ = name_tok.value;
    var->init_ = std::move(init);
    return var;
}

std::unique_ptr<CompoundStatement> Parser::parse_compound_statement()
{
    auto block = std::make_unique<CompoundStatement>();
    block->set_source_begin(peek().source_location);

    if (!expect_and_consume(TokenKind::l_brace))
        return nullptr;

    while (!peek().is(TokenKind::r_brace)) {
        auto stmt = parse_statement();
        if (!stmt)
            return nullptr;
        block->stmts_.push_back(std::move(stmt));
    }

    auto rbrace_tok = consume(); // }
    block->set_source_end(rbrace_tok.source_location);

    return block;
}

std::unique_ptr<ast::Statement> Parser::parse_statement()
{
    switch (peek().kind) {
    case TokenKind::keyword_return: {
        auto stmt = std::make_unique<ast::ReturnStatement>();
        stmt->set_source_begin(peek().source_location);

        consume(); // consume 'return'

        if (!peek().is(TokenKind::semicolon)) {
            stmt->returned_value_ = parse_expression();
            if (!stmt->returned_value_)
                return nullptr;
        }

        stmt->set_source_end(peek().source_location);
        if (!expect_and_consume(TokenKind::semicolon))
            return nullptr;

        return stmt;
    }
    case TokenKind::keyword_if:
        return parse_if_statement();
    case TokenKind::l_brace:
        return parse_compound_statement();

    case TokenKind::keyword_int:
    case TokenKind::keyword_float:
    case TokenKind::keyword_string: {
        auto stmt = std::make_unique<ast::DeclarationStatement>();
        stmt->decl_ = parse_declaration();
        stmt->set_source_begin(stmt->decl_->source_range().begin);
        stmt->set_source_begin(stmt->decl_->source_range().end);
        return stmt;
    }
    case TokenKind::semicolon: {
        auto es = std::make_unique<ast::EmptyStatement>();
        auto semi_tok = consume();
        es->set_source_begin(semi_tok.source_location);
        es->set_source_end(semi_tok.source_location);
        return es;
    }
    default: {
        auto stmt = std::make_unique<ast::ExpressionStatement>();
        stmt->expr_ = parse_expression();
        if (!stmt->expr_)
            return nullptr;

        stmt->set_source_begin(stmt->expr_->source_range().begin);
        stmt->set_source_end(peek().source_location);
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
    return try_parse_equalequal();
}

// x [== y]
// ast::ExpressionPtr Parser::try_parse_equalequal()
// {
//     auto boe = std::make_unique<BinaryOperationExpr>();
//     boe->lhs_ = try_parse_add_minus();
//     if (!boe->lhs_)
//         return nullptr;
//
//     switch (peek().kind) {
//     case TokenKind::equalequal: {
//         boe->op_ = consume().value;
//         boe->rhs_ = try_parse_add_minus();
//         if (!boe->rhs_)
//             return nullptr;
//
//         auto old =
//             std::exchange(boe, std::make_unique<ast::BinaryOperationExpr>());
//         boe->lhs_ = std::move(old);
//         break;
//     }
//     default:
//         break;
//     }
//
//     boe->set_source_begin(boe->lhs_->source_range().begin);
//     boe->set_source_end(boe->lhs_->source_range().end);
//     return std::move(boe->lhs_);
// }
ast::ExpressionPtr Parser::try_parse_equalequal()
{
    auto lhs = try_parse_add_minus();
    if (!lhs)
        return nullptr;

    while (peek().kind == TokenKind::equalequal) {
        auto op_token = consume(); // ==
        auto rhs = try_parse_add_minus();
        if (!rhs)
            return nullptr;

        auto expr = std::make_unique<ast::BinaryOperationExpr>();
        expr->lhs_ = std::move(lhs);
        expr->op_ = op_token.value;
        expr->rhs_ = std::move(rhs);

        expr->set_source_begin(expr->lhs_->source_range().begin);
        expr->set_source_end(expr->rhs_->source_range().end);

        lhs = std::move(expr);
    }

    return lhs;
}

// ast::ExpressionPtr Parser::try_parse_add_minus()
// {
//     auto expr = std::make_unique<ast::BinaryOperationExpr>();
//     expr->lhs_ = try_parse_mult_div_mod();
//     if (!expr->lhs_)
//         return nullptr;
//
//     while (true) {
//         switch (Token tok = peek(); tok.kind) {
//         case TokenKind::plus:
//         case TokenKind::minus: {
//             expr->op_ = consume().value;
//             expr->rhs_ = try_parse_mult_div_mod();
//             if (!expr->rhs_)
//                 return nullptr;
//
//             auto old = std::exchange(
//                 expr, std::make_unique<ast::BinaryOperationExpr>());
//             expr->lhs_ = std::move(old);
//             break;
//         }
//         case TokenKind::star:
//         case TokenKind::slash:
//         case TokenKind::percent:
//         default:
//             return std::move(expr->lhs_);
//         }
//     }
// }
ast::ExpressionPtr Parser::try_parse_add_minus()
{
    auto lhs = try_parse_mult_div_mod();
    if (!lhs)
        return nullptr;

    while (true) {
        Token tok = peek();
        if (tok.kind != TokenKind::plus && tok.kind != TokenKind::minus)
            break;

        auto op_token = consume();
        auto rhs = try_parse_mult_div_mod();
        if (!rhs)
            return nullptr;

        auto expr = std::make_unique<ast::BinaryOperationExpr>();
        expr->lhs_ = std::move(lhs);
        expr->op_ = op_token.value;
        expr->rhs_ = std::move(rhs);

        expr->set_source_begin(expr->lhs_->source_range().begin);
        expr->set_source_end(expr->rhs_->source_range().end);

        lhs = std::move(expr);
    }

    return lhs;
}

// ast::ExpressionPtr Parser::try_parse_mult_div_mod()
// {
//     auto bin = std::make_unique<ast::BinaryOperationExpr>();
//     bin->lhs_ = try_parse_unary_expr();
//     if (!bin->lhs_)
//         return nullptr;
//
//     while (true) {
//         switch (Token tok = peek(); tok.kind) {
//         case TokenKind::star:
//         case TokenKind::slash:
//         case TokenKind::percent: {
//             bin->op_ = consume().value;
//             bin->rhs_ = try_parse_unary_expr();
//             if (!bin->rhs_)
//                 return nullptr;
//
//             auto old = std::exchange(
//                 bin, std::make_unique<ast::BinaryOperationExpr>());
//             bin->lhs_ = std::move(old);
//             break;
//         }
//         default:
//             return std::move(bin->lhs_);
//         }
//     }
// }
ast::ExpressionPtr Parser::try_parse_mult_div_mod()
{
    auto lhs = try_parse_unary_expr();
    if (!lhs)
        return nullptr;

    while (true) {
        Token tok = peek();
        switch (tok.kind) {
        case TokenKind::star:
        case TokenKind::slash:
        case TokenKind::percent: {
            auto op = consume().value;
            auto rhs = try_parse_unary_expr();
            if (!rhs)
                return nullptr;

            auto bin = std::make_unique<ast::BinaryOperationExpr>();
            bin->lhs_ = std::move(lhs);
            bin->op_ = op;
            bin->rhs_ = std::move(rhs);

            bin->set_source_begin(bin->lhs_->source_range().begin);
            bin->set_source_end(bin->rhs_->source_range().end);

            lhs = std::move(bin);
            break;
        }
        default:
            return lhs;
        }
    }
}

ast::ExpressionPtr Parser::try_parse_unary_expr()
{
    // Recursively consumes unary operators
    if (auto tok = peek();
        tok.is(TokenKind::plus) || tok.is(TokenKind::minus)) {
        auto op_tok = consume();

        auto unary = std::make_unique<ast::UnaryOperationExpr>();
        unary->op_ = op_tok.value;
        unary->expr_ = try_parse_unary_expr();

        unary->set_source_begin(op_tok.source_location);
        unary->set_source_end(unary->expr_->source_range().end);

        return unary;
    }

    return try_parse_postfix_expr();
}

// xyz()
// ast::ExpressionPtr Parser::try_parse_postfix_expr()
// {
//     auto call = std::make_unique<CallExpression>();
//     call->callee_ = parse_primary_expression();
//     if (!call->callee_)
//         return nullptr;
//
//     while (true) {
//         switch (peek().kind) {
//         case TokenKind::l_paren: {
//             consume();
//
//             // Parses arguments list
//             bool first_time{true};
//             while (peek().is_not(TokenKind::r_paren)) {
//                 if (!first_time && !expect_and_consume(TokenKind::comma))
//                     return nullptr;
//
//                 auto arg = parse_expression();
//                 if (!arg)
//                     return nullptr;
//                 call->arguments_.push_back(std::move(arg));
//
//                 if (first_time)
//                     first_time = false;
//             }
//
//             if (!expect_and_consume(TokenKind::r_paren))
//                 return nullptr;
//
//             auto old = std::exchange(call,
//             std::make_unique<CallExpression>()); call->callee_ =
//             std::move(old); break;
//         }
//         default:
//             return std::move(call->callee_);
//         }
//     }
// }
ast::ExpressionPtr Parser::try_parse_postfix_expr()
{
    auto expr = parse_primary_expression();
    if (!expr)
        return nullptr;

    while (true) {
        switch (peek().kind) {
        case TokenKind::l_paren: {
            consume(); // (

            auto call = std::make_unique<CallExpression>();
            call->callee_ = std::move(expr);

            bool first = true;
            while (peek().is_not(TokenKind::r_paren)) {
                if (!first && !expect_and_consume(TokenKind::comma))
                    return nullptr;

                auto arg = parse_expression();
                if (!arg)
                    return nullptr;

                call->arguments_.push_back(std::move(arg));
                first = false;
            }

            if (!expect_and_consume(TokenKind::r_paren))
                return nullptr;

            // source range：callee.begin -> ')'
            call->set_source_begin(call->callee_->source_range().begin);
            call->set_source_end(previous_token().source_location);

            expr = std::move(call);
            break;
        }
        default:
            return expr;
        }
    }
}

// ast::ExpressionPtr Parser::parse_primary_expression()
// {
//     using enum TokenKind;
//
//     switch (Token tok = peek(); tok.kind) {
//     case identifier: {
//         auto expr = std::make_unique<ast::IdentifierExpr>();
//         expr->name_ = consume().value;
//         return expr;
//     }
//     case integer_val: {
//         auto expr = std::make_unique<ast::IntegerLiteralExpr>();
//         expr->value_ = consume().value;
//         return expr;
//     }
//     case float_val: {
//         auto expr = std::make_unique<ast::FloatLiteralExpr>();
//         expr->value_ = consume().value;
//         return expr;
//     }
//     case string_val: {
//         auto expr = std::make_unique<ast::StringLiteralExpr>();
//         expr->value_ = consume().value;
//         return expr;
//     }
//     case l_paren: {
//         consume();
//         auto expr = parse_expression();
//         expect_and_consume(r_paren);
//         return expr;
//     }
//     default:
//         expect_true([](auto) { return false; }, "primary expression");
//         return nullptr;
//     }
// }
ast::ExpressionPtr Parser::parse_primary_expression()
{
    using enum TokenKind;

    switch (Token tok = peek(); tok.kind) {
    case identifier: {
        auto t = consume();
        auto expr = std::make_unique<ast::IdentifierExpr>();
        expr->name_ = t.value;
        expr->set_source_begin(t.source_location);
        expr->set_source_end(t.source_location);
        return expr;
    }
    case integer_val: {
        auto t = consume();
        auto expr = std::make_unique<ast::IntegerLiteralExpr>();
        expr->value_ = t.value;
        expr->set_source_begin(t.source_location);
        expr->set_source_end(t.source_location);
        return expr;
    }
    case float_val: {
        auto t = consume();
        auto expr = std::make_unique<ast::FloatLiteralExpr>();
        expr->value_ = t.value;
        expr->set_source_begin(t.source_location);
        expr->set_source_end(t.source_location);
        return expr;
    }
    case string_val: {
        auto t = consume();
        auto expr = std::make_unique<ast::StringLiteralExpr>();
        expr->value_ = t.value;
        expr->set_source_begin(t.source_location);
        expr->set_source_end(t.source_location);
        return expr;
    }
    case l_paren: {
        consume(); // '('
        auto expr = parse_expression();
        if (!expr)
            return nullptr;
        expect_and_consume(r_paren);
        // 不改 expr 的 range：括号不属于语义
        return expr;
    }
    default:
        expect_true([](auto) { return false; }, "primary expression");
        return nullptr;
    }
}

// Grammar: if () ... [else ...]
std::unique_ptr<ast::Statement> Parser::parse_if_statement()
{
    auto if_statement = std::make_unique<IfStatement>();
    if_statement->set_source_begin(peek().source_location);

    if (!expect_and_consume(TokenKind::keyword_if))
        return nullptr;
    if (!expect_and_consume(TokenKind::l_paren))
        return nullptr;
    if_statement->condition_ = parse_expression();
    if (!if_statement->condition_)
        return nullptr;
    if (!expect_and_consume(TokenKind::r_paren))
        return nullptr;
    if_statement->true_branch_ = parse_statement();
    if (!if_statement->true_branch_)
        return nullptr;

    if (peek().is(TokenKind::keyword_else)) {
        consume(); // else
        if_statement->false_branch_ = parse_statement();
        if (!if_statement->false_branch_)
            return nullptr;
    }

    if_statement->set_source_end(previous_token().source_location);
    return if_statement;
}
