#include <ast/ast.h>
#include <diagnostics.h>
#include <functional>
#include <iostream>
#include <ranges>
#include <semantic/context.h>
#include <semantic/frame.h>
#include <semantic/intepreter.h>
#include <semantic/return-signal.h>
#include <semantic/symbol.h>
#include <spdlog/spdlog.h>
#include <utility>

void semantic::Intepreter::dump(std::ostream &os)
{
    if (curr_frame_) {
        std::vector<Frame *> frames;
        frames.push_back(curr_frame_.get());
        while (frames.back() != nullptr) {
            frames.push_back(frames.back()->parent().get());
        }
        frames.pop_back(); // nullptr
        for (auto i = frames.size() - 1; i != -1UZ; --i) {
            frames[i]->dump(os, static_cast<int>(frames.size() - 1 - i));
        }
    }
    else {
        spdlog::debug("Currently doesn't in any frame.");
    }

    if (last_returned_.has_value()) {
        spdlog::debug("Last returned value: {}", *last_returned_);
    }
    else {
        spdlog::debug("Didn't return from any function");
    }
}

void semantic::Intepreter::visit(ast::Program &p)
{
    for (auto const &d : p.declaration_statements()) {
        spdlog::debug("Visiting declaration statements");
        d->accept(*this);
    }

    auto *scope = p.global_scope();
    assert(scope != nullptr);
    auto *symbol = scope->lookup_symbol("main");
    if (symbol == nullptr) {
        diags_->error("Cannot find 'main'. Did you forget to define it?");
        return;
    }
    enter_subframe();
    try {
        if (symbol->symbolkind != SymbolKind::variable ||
            symbol->type_ptr->typekind != TypeKind::function_type) {
            diags_->error("'main' is not a function, its type: {}",
                          to_string(symbol->type_ptr->typekind));
            return;
        }
        auto *ft = static_cast<FunctionType *>(symbol->type_ptr);
        assert(ft && ft->decl && ft->decl->body());
        ft->decl->body()->accept(*this);
    }
    catch (ReturnSignal const &e) {
        ;
    }
}

void semantic::Intepreter::visit(ast::VariableDeclaration &vd)
{
    if (auto const &init = vd.init()) {
        init->accept(*this);
        curr_frame_->new_variable(vd.name(), last_visited_.value());
    }
    // else {
    //     curr_frame->new_variable(vd.name(), "");
    // }
    // std::println("Defining variable {}", vd.name());
}

void semantic::Intepreter::visit(ast::FunctionDeclaration & /*unused*/) {}

void semantic::Intepreter::visit(ast::CompoundStatement &cs)
{
    spdlog::debug("Executing body of compound statement");
    for (auto const &s : cs.statements()) {
        s->accept(*this);
    }
}

void semantic::Intepreter::visit(ast::DeclarationStatement &ds)
{
    spdlog::debug("{}: Executing declaration statement", ds.source_range());
    ds.declaration()->accept(*this);
}

// Leaving frame is responsibility of internal function body, not call itself.
void semantic::Intepreter::visit(ast::CallExpression &ce)
{
    // Check if is a callable function
    auto *callee_p =
        dynamic_cast<ast::IdentifierExpression *>(ce.callee().get());
    if (callee_p == nullptr) {
        diags_->error(
            "{}: Callee not an identifier, but we only support identifiers",
            callee_p->source_range());
        last_visited_.reset();
        return;
    }

    if (callee_p->name() == "print") {
        spdlog::info("Program printing: {}", eval(ce.arguments().front()));
        return;
    }

    auto *sym = callee_p->symbol();
    if (sym == nullptr) {
        throw std::logic_error("Symbol unbounded");
        // diags_->error("{}: Undefined identifier '{}'",
        // callee_p->source_range(),
        //               callee_p->name());
        // spdlog::error("Can find local symbol {}? {}", callee_p->name(),
        //               ctx_->current_scope()->lookup_local_symbol(
        //                   callee_p->name()) != nullptr);
        // return;
    }
    if (sym->type_ptr->typekind != TypeKind::function_type) {
        diags_->error("Callee '{}' not a function", callee_p->name());
        return;
    }

    spdlog::debug("{}: Executing function '{}'", ce.source_range(),
                  callee_p->name());
    enter_subframe();
    auto *ft = static_cast<FunctionType *>(sym->type_ptr);
    auto *decl = ft->decl;
    // Sets parameters
    spdlog::debug("Setting parameters");
    for (auto const &[arg, param] :
         std::views::zip(ce.arguments(), decl->parameters())) {
        curr_frame_->new_variable(param.name, eval(arg));
        spdlog::debug("Setting parameter {}={}", param.name,
                      *curr_frame_->lookup_rvalue(param.name));
    }
    spdlog::debug("Parameters set");
    // dump(std::cout);
    // Invokes the function
    try {
        decl->body()->accept(*this);
    }
    catch (ReturnSignal const &e) {
        // Doing nothing, because it's a signal to leave function
        ;
    }
    // leave_frame();
}

void semantic::Intepreter::visit(ast::IntegerLiteralExpr &ie)
{
    last_visited_ = ie.strvalue();
}

void semantic::Intepreter::visit(ast::FloatLiteralExpr &fe)
{
    last_visited_ = fe.value();
}

void semantic::Intepreter::visit(ast::StringLiteralExpr &se)
{
    last_visited_ = se.value();
}

void semantic::Intepreter::visit(ast::UnaryExpression &uoe)
{
    uoe.expr()->accept(*this);
    auto value = std::stoll(*last_visited_);

    auto const &op = uoe.op();
    if (op == "+") {
    }
    else if (op == "-") {
        value = -value;
    }

    last_visited_ = std::to_string(value);
}

void semantic::Intepreter::visit(ast::BinaryExpression &boe)
{
    auto rhs = std::stoll(eval(boe.rhs().get()));

    auto const &op = boe.op();
    auto execute = [&] {
        switch (op.kind) {
        case TokenKind::plus: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs + rhs;
        }
        case TokenKind::minus: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs - rhs;
        }
        case TokenKind::star: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs * rhs;
        }
        case TokenKind::slash: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs / rhs;
        }
        case TokenKind::percent: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs % rhs;
        }
        case TokenKind::equalequal: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs == rhs ? 1LL : 0LL;
        }
        case TokenKind::less: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs < rhs ? 1LL : 0LL;
        }
        case TokenKind::lessthan: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs <= rhs ? 1LL : 0LL;
        }
        case TokenKind::more: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs > rhs ? 1LL : 0LL;
        }
        case TokenKind::morethan: {
            auto lhs = std::stoll(eval(boe.lhs().get()));
            return lhs >= rhs ? 1LL : 0LL;
        }
        case TokenKind::equal: {
            auto *pidentifier =
                dynamic_cast<ast::IdentifierExpression *>(boe.lhs().get());
            if (pidentifier == nullptr) {
                diags_->error(
                    "{}: Left-hand side of assignment expression not an "
                    "identifier",
                    boe.source_range());
                last_visited_.reset();
            }
            auto *pvar = curr_frame_->lookup_lvalue(pidentifier->name());
            *pvar = std::to_string(rhs);
            return std::stoll(*pvar);
        }
        default:
            throw std::runtime_error(
                std::format("Unknown binary operator {}", op.value));
        }
    };

    last_visited_ = std::to_string(execute());
}

void semantic::Intepreter::visit(ast::IdentifierExpression &ie)
{
    auto p = curr_frame_->lookup_rvalue(ie.name());
    if (!p.has_value()) {
        diags_->error("{}: Undefined variable '{}'", ie.source_range(),
                      ie.name());
        curr_frame_->dump(std::cout);
        last_visited_.reset();
        return;
    }
    last_visited_ = *p;
}

void semantic::Intepreter::visit(ast::ReturnStatement &rs)
{
    last_returned_ = eval(rs.returned_value());
    leave_frame();
    throw ReturnSignal{};
}

void semantic::Intepreter::visit(ast::IfStatement &is)
{
    spdlog::debug("Executing if statement");
    if (eval(is.condition()) != "0") {
        spdlog::debug("True hit");
        if (is.true_branch()) {
            is.true_branch()->accept(*this);
        }
    }
    else {
        spdlog::debug("False hit");
        if (is.false_branch()) {
            is.false_branch()->accept(*this);
        }
    }
}

void semantic::Intepreter::visit(ast::WhileStatement &ws)
{
    spdlog::debug("Executing while statement");
    while (eval(ws.condition()) != "0") {
        spdlog::debug("While loop iteration");
        ws.body()->accept(*this);
    }
}

semantic::Intepreter::Intepreter(Context *ctx, Diagnostics *diags)
    : ctx_(ctx), diags_(diags)
{
}

void semantic::Intepreter::enter_subframe()
{
    // return;
    auto neo = std::make_unique<Frame>(std::move(curr_frame_));
    curr_frame_ = std::move(neo);
}

void semantic::Intepreter::leave_frame()
{
    // return;
    auto parent = std::move(curr_frame_->parent());
    curr_frame_ = std::move(parent);
}
