#include <iostream>
#include <semantic/semantic-analyzer.h>

#include <algorithm>
#include <diagnostics.h>
#include <ranges>
#include <semantic/context.h>
#include <semantic/scope.h>

semantic::SemanticAnalyzer::SemanticAnalyzer(Context *ctx, Diagnostics *diags)
    : ctx_(ctx), diags_(diags)
{
    using enum TypeKind;
    ctx_->define_builtin_type("int", BasicType(4, integer_type));
    ctx_->define_builtin_type("float", BasicType(4, float_type));
    ctx_->define_builtin_type("string", BasicType(4, string_type));
}

void semantic::SemanticAnalyzer::visit(ast::Program &prog)
{
    ctx_->push_scope();
    prog.global_scope_ = ctx_->current_scope();
    for (auto const &decl : prog.decls_) {
        decl->accept(*this);
    }
    ctx_->pop_scope();
}

void semantic::SemanticAnalyzer::visit(ast::VariableDeclaration &vd)
{
    BasicType *type = find_type_by_name(vd.type().value);
    if (type == nullptr) {
        diags_->error("Unknown type '{}'", vd.type().value);
        return;
    }

    if (ctx_->current_scope()->lookup_local_symbol(vd.name()) != nullptr) {
        diags_->error("Variable re-declaration error: {}", vd.name());
        return;
    }

    ctx_->current_scope()->define_symbol(
        vd.name(), Symbol{.name = vd.name(),
                          .type_ptr = type,
                          .symbolkind = SymbolKind::variable});

    vd.set_symbol(ctx_->current_scope()->lookup_local_symbol(vd.name()));
}

void semantic::SemanticAnalyzer::visit(ast::FunctionDeclaration &fd)
{
    if (ctx_->current_scope()->lookup_local_symbol(fd.name()) != nullptr) {
        diags_->error("Function re-declaration error: {}", fd.name());
        return;
    }

    auto *out = ctx_->current_scope();

    ctx_->push_scope();

    // Constructs a function type instance
    std::vector<BasicType *> param_types;
    for (auto const &[paramtype, name] : fd.parameters()) {
        param_types.push_back(find_type_by_name(paramtype));
        ctx_->current_scope()->define_symbol(
            name, Symbol{.name = name,
                         .type_ptr = param_types.back(),
                         .symbolkind = SymbolKind::variable});
    }
    auto ft = FunctionType(
        BasicType{.size = 4, .typekind = TypeKind::function_type},
        find_type_by_name(fd.return_type().value), param_types, &fd);

    auto *ptr = out->define_unnamed_type(ft);

    out->define_symbol(fd.name(), Symbol{.name = fd.name(),
                                         .type_ptr = ptr,
                                         .symbolkind = SymbolKind::variable});

    fd.set_symbol(out->lookup_local_symbol(fd.name()));

    // Bypasses compound statement scope for function parameters
    for (auto const &stmt : fd.body()->statments()) {
        stmt->accept(*this);
    }

    ctx_->pop_scope();
}

void semantic::SemanticAnalyzer::visit(ast::IfStatement &is)
{
    is.condition()->accept(*this);
    if (is.condition()->type() != find_type_by_name("int")) {
        diags_->error("{}: Invalid condition type: expected integer, got {}",
                      is.source_range(),
                      to_string(is.condition()->type()->typekind));
        return;
    }

    is.true_branch()->accept(*this);
    if (is.false_branch()) {
        is.false_branch()->accept(*this);
    }
}

void semantic::SemanticAnalyzer::visit(ast::WhileStatement &ws)
{
    ws.condition()->accept(*this);
    if (ws.condition()->type() != find_type_by_name("int")) {
        diags_->error("{}: Invalid condition type: expected integer, got {}",
                      ws.source_range(),
                      to_string(ws.condition()->type()->typekind));
        return;
    }

    ws.body()->accept(*this);
}

void semantic::SemanticAnalyzer::visit(ast::CompoundStatement &cs)
{
    ctx_->push_scope();
    for (auto const &stmt : cs.statments()) {
        stmt->accept(*this);
    }
    ctx_->pop_scope();
}

void semantic::SemanticAnalyzer::visit(ast::DeclarationStatement &ds)
{
    ds.declaration()->accept(*this);
}

void semantic::SemanticAnalyzer::visit(ast::IndexExpression &ie)
{
    ie.base()->accept(*this);
    for (auto const &index : ie.indices()) {
        index->accept(*this);
        if (index->type()->typekind != TypeKind::integer_type) {
            diags_->error("{}: Invalid index type: expected integer, got {}",
                          ie.source_range(),
                          to_string(index->type()->typekind));
            return;
        }
    }

    ie.set_type(ie.base()->type());
}

void semantic::SemanticAnalyzer::visit(ast::CallExpression &ce)
{
    ce.callee()->accept(*this);
    for (auto const &arg : ce.arguments()) {
        arg->accept(*this);
    }

    // Type checking
    auto *callee_p =
        dynamic_cast<ast::IdentifierExpression *>(ce.callee().get());
    if (callee_p == nullptr) {
        diags_->error("{}: Invalid function call: callee is not an identifier",
                      ce.source_range());
        return;
    }

    auto *s = ctx_->current_scope()->lookup_symbol(callee_p->name());
    if (s == nullptr) {
        diags_->error("{}: Undefined function '{}'", ce.source_range(),
                      callee_p->name());
        return;
    }
    if (s->type_ptr->typekind != TypeKind::function_type) {
        diags_->error("{}: Callee '{}' is not a function", ce.source_range(),
                      callee_p->name());
        return;
    }

    auto *ft = static_cast<FunctionType *>(s->type_ptr);
    if (ft->parameter_types.size() != ce.arguments().size()) {
        diags_->error("{}: Argument count mismatch in function call to '{}': "
                      "expected {}, got {}",
                      ce.source_range(), callee_p->name(),
                      ft->parameter_types.size(), ce.arguments().size());
        return;
    }

    for (std::size_t i = 0; i < ce.arguments().size(); ++i) {
        if (ce.arguments()[i]->type() != ft->parameter_types[i]) {
            diags_->error(
                "{}: Argument type mismatch in function call to '{}': "
                "expected {}, got {}",
                ce.source_range(), callee_p->name(),
                to_string(ft->parameter_types[i]->typekind),
                to_string(ce.arguments()[i]->type()->typekind));
            return;
        }
    }

    ce.set_type(ft->return_type);
}

void semantic::SemanticAnalyzer::visit(ast::IdentifierExpression &ie)
{
    auto *s = ctx_->current_scope()->lookup_symbol(ie.name());
    if (s == nullptr) {
        diags_->error("{}: Undefined identifier '{}'", ie.source_range(),
                      ie.name());
        return;
    }

    ie.set_symbol(s);
    ie.set_type(s->type_ptr);
}

void semantic::SemanticAnalyzer::visit(ast::UnaryExpression &ue)
{
    ue.expr()->accept(*this);
    ue.set_type(ue.expr()->type());
}

void semantic::SemanticAnalyzer::visit(ast::BinaryExpression &be)
{
    be.lhs()->accept(*this);
    be.rhs()->accept(*this);

    if (be.lhs()->type() != be.rhs()->type()) {
        diags_->error("{}: Type mismatch in binary expression: left is {}, "
                      "right is {}",
                      be.source_range(), to_string(be.lhs()->type()->typekind),
                      to_string(be.rhs()->type()->typekind));
        return;
    }

    switch (be.op().kind) {
    case TokenKind::plus:
    case TokenKind::minus:
    case TokenKind::star:
    case TokenKind::slash:
    case TokenKind::percent:
        if (be.lhs()->type() != be.lhs()->type()) {
            diags_->error("{}: Invalid operand types for arithmetic "
                          "operation: {}",
                          be.source_range(),
                          to_string(be.lhs()->type()->typekind));
            return;
        }
        be.set_type(be.lhs()->type());
        break;
    case TokenKind::equal:
        if (be.lhs()->type() != be.rhs()->type()) {
            diags_->error("{}: Invalid operand types for assignment: {}",
                          be.source_range(),
                          to_string(be.lhs()->type()->typekind));
            return;
        }
        be.set_type(be.lhs()->type());
        break;
    case TokenKind::less:
    case TokenKind::lessthan:
    case TokenKind::more:
    case TokenKind::morethan:
    case TokenKind::equalequal:
        if (be.lhs()->type() != be.rhs()->type()) {
            diags_->error("{}: Invalid operand types for assignment: {}",
                          be.source_range(),
                          to_string(be.lhs()->type()->typekind));
            return;
        }
        be.set_type(ctx_->get_builtin_type("int")); // Actually boolean
        break;
    default:
        throw std::runtime_error(
            std::format("Unknown binary operator {}", be.op().value));
    }
}

void semantic::SemanticAnalyzer::visit(ast::IntegerLiteralExpr &ie)
{
    auto *p = find_type_by_name("int");
    assert(p && "int should be found");
    ie.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::FloatLiteralExpr &fe)
{
    auto *p = find_type_by_name("float");
    assert(p && "float should be found");
    fe.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::StringLiteralExpr &se)
{
    auto *p = find_type_by_name("string");
    assert(p && "string should be found");
    se.set_type(p);
}

semantic::BasicType *
semantic::SemanticAnalyzer::find_type_by_name(std::string_view name)
{
    if (auto *p = ctx_->get_builtin_type(std::string{name}))
        return p;

    if (auto *p = ctx_->current_scope()->lookup_type(std::string{name}))
        return p;

    return nullptr;
}
