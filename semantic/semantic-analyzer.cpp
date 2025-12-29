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
    ctx_->define_builtin_type("int",
                              BuiltinType(4, BuiltinType::Kind::integer_type));
    ctx_->define_builtin_type("float",
                              BuiltinType(4, BuiltinType::Kind::float_type));
    ctx_->define_builtin_type("string",
                              BuiltinType(8, BuiltinType::Kind::string_type));
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
    vd.type()->accept(*this);
    Type *type = vd.type()->type();
    if (type == nullptr) {
        // Error should be reported in type resolution. Not here.
        // diags_->error("Unknown type '{}'", vd.type());
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

    if (auto const &init = vd.init()) {
        init->accept(*this);
    }
}

void semantic::SemanticAnalyzer::visit(ast::FunctionDeclaration &fd)
{
    auto *out = ctx_->current_scope();
    ctx_->push_scope();
    auto *in = ctx_->current_scope();

    if (out->lookup_local_symbol(fd.name()) != nullptr) {
        diags_->error("Function re-declaration error: {}", fd.name());
        return;
    }

    // Constructs a function type instance
    std::vector<Type *> param_types;
    for (auto &[paramtype, name, psymbol] : fd.parameters()) {
        paramtype->accept(*this);
        param_types.push_back(paramtype->type());
        assert(!name.empty()); // Name won't be empty, as we changed the syntax.
        in->define_symbol(name, Symbol{.name = name,
                                       .type_ptr = param_types.back(),
                                       .symbolkind = SymbolKind::variable});
        psymbol = in->lookup_local_symbol(name);
    }
    fd.return_type()->accept(*this);
    auto ft = FunctionType(fd.return_type()->type(), param_types, &fd);

    auto *ptr = out->define_function_type(ft);
    out->define_symbol(fd.name(), Symbol{.name = fd.name(),
                                         .type_ptr = ptr,
                                         .symbolkind = SymbolKind::variable});

    fd.set_symbol(out->lookup_local_symbol(fd.name()));

    // Bypasses compound statement scope for function parameters
    for (auto const &stmt : fd.body()->statements()) {
        stmt->accept(*this);
    }

    ctx_->pop_scope();
}

void semantic::SemanticAnalyzer::visit(ast::IfStatement &is)
{
    is.condition()->accept(*this);
    if (is.condition()->type() != resolve_type("int")) {
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
    if (ws.condition()->type() != resolve_type("int")) {
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
    for (auto const &stmt : cs.statements()) {
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
        if (index->type()->typekind != TypeKind::builtin_type) {
            diags_->error("{}: Invalid index type: expected integer, got {}",
                          ie.source_range(),
                          to_string(index->type()->typekind));
            return;
        }
        auto *bt = static_cast<BuiltinType *>(index->type());
        if (bt->builintypekind != BuiltinType::Kind::integer_type) {
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

    auto *s = callee_p->symbol();
    if (s == nullptr) {
        throw std::runtime_error("Symbol unbound");
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
    auto *p = resolve_type("int");
    assert(p && "int should be found");
    ie.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::FloatLiteralExpr &fe)
{
    auto *p = resolve_type("float");
    assert(p && "float should be found");
    fe.set_type(p);
}
void semantic::SemanticAnalyzer::visit(ast::StringLiteralExpr &se)
{
    auto *p = resolve_type("string");
    assert(p && "string should be found");
    se.set_type(p);
}

void semantic::SemanticAnalyzer::visit(ast::BasicType &bt)
{
    auto *p = resolve_type(bt.name());
    if (p == nullptr) {
        diags_->error("{}: Unknown type '{}'", bt.source_range(), bt.name());
        return;
    }
    bt.set_type(p);
}

void semantic::SemanticAnalyzer::visit(ast::ArrayType &at)
{
    at.element_type()->accept(*this);
    auto *elem_type = at.element_type()->type();
    if (elem_type == nullptr) {
        diags_->error("{}: Unknown element type in array type",
                      at.source_range());
        return;
    }

    ArrayType array_type(elem_type, at.size());
    at.set_type(ctx_->current_scope()->define_array_type(array_type));
}

void semantic::SemanticAnalyzer::visit(ast::PointerType &pt)
{
    pt.pointee_type()->accept(*this);
    auto *pointee_type = pt.pointee_type()->type();
    if (pointee_type == nullptr) {
        diags_->error("{}: Unknown pointee type in pointer type",
                      pt.source_range());
        return;
    }

    PointerType pointer_type(pointee_type);
    pt.set_type(ctx_->current_scope()->define_pointer_type(pointer_type));
}

semantic::Type *semantic::SemanticAnalyzer::resolve_type(std::string_view name)
{
    if (auto *p = ctx_->find_builtin_type(std::string{name}))
        return p;

    if (auto *p = ctx_->current_scope()->lookup_type(std::string{name}))
        return p;

    return nullptr;
}
