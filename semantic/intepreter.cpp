#include <ast/ast.h>
#include <diagnostics.h>
#include <functional>
#include <iostream>
#include <ranges>
#include <semantic/context.h>
#include <semantic/frame.h>
#include <semantic/intepreter.h>
#include <semantic/symbol.h>
#include <spdlog/spdlog.h>
#include <utility>

// struct Finally {
//     Finally(Finally const &) = default;
//     Finally(Finally &&) = delete;
//     Finally &operator=(Finally const &) = default;
//     Finally &operator=(Finally &&) = delete;
//     Finally(std::function<void()> f) : func_(std::move(f)) {}
//     ~Finally()
//     {
//         func_();
//     }
//
//   private:
//     std::function<void()> func_;
// };

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
    for (auto const &d : p.declarations()) {
        std::println("Visiting declaration");
        d->accept(*this);
    }

    auto *main = ctx_->symbol_table_.lookup("main");
    if (main == nullptr) {
        diags_->error("Cannot find 'main'. Did you forget to define it?");
        return;
    }
    auto *ft = static_cast<FunctionType *>(main->type_ptr);
    assert(ft);
    assert(ft->decl);
    assert(ft->decl->body());
    enter_subframe();
    ft->decl->body()->accept(*this);
    dump(std::cout); // TODO: Only for test
    leave_frame();
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

void semantic::Intepreter::visit(ast::FunctionDeclaration &fd) {}

void semantic::Intepreter::visit(ast::CompoundStatement &cs)
{
    spdlog::debug("Executing body of compound statement");
    for (auto const &s : cs.statments()) {
        s->accept(*this);
    }
}

void semantic::Intepreter::visit(ast::DeclarationStatement &ds)
{
    ds.declaration()->accept(*this);
}

void semantic::Intepreter::visit(ast::CallExpression &ce)
{
    // Check if is a callable function
    auto *callee_p = dynamic_cast<ast::IdentifierExpr *>(ce.callee().get());
    if (callee_p == nullptr) {
        diags_->error(
            "Callee not an identifier, as we only support identifiers");
        last_visited_.reset();
        return;
    }
    auto *sym = ctx_->symbol_table_.lookup(callee_p->name());
    if (sym == nullptr || sym->type_ptr->typekind != TypeKind::function_type) {
        diags_->error("Callee '{}' not a function", callee_p->name());
        return;
    }

    enter_subframe();
    auto *ft = static_cast<FunctionType *>(sym->type_ptr);
    auto *decl = ft->decl;
    // Sets parameters
    spdlog::debug("Setting parameters");
    for (auto const &[arg, param] :
         std::views::zip(ce.arguments(), decl->parameters())) {
        spdlog::debug("Setting parameter '{}'", param.name);
        curr_frame_->new_variable(param.name, eval(arg));
    }
    spdlog::debug("Parameters set");
    dump(std::cout);
    // Invokes the function
    decl->body()->accept(*this);
    leave_frame();
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

void semantic::Intepreter::visit(ast::UnaryOperationExpr &uoe)
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

void semantic::Intepreter::visit(ast::BinaryOperationExpr &boe)
{
    auto lhs = std::stoll(eval(boe.lhs().get()));
    auto rhs = std::stoll(eval(boe.rhs().get()));

    auto const &op = boe.op();
    auto execute = [&] {
        if (op == "+") {
            return lhs + rhs;
        }
        if (op == "-") {
            return lhs - rhs;
        }
        if (op == "*") {
            return lhs * rhs;
        }
        if (op == "/") {
            return lhs / rhs;
        }
        if (op == "%") {
            return lhs % rhs;
        }
        throw std::runtime_error("Only support 四则运算和取模");
    };

    // 只支持 int + int
    last_visited_ = std::to_string(execute());
}

void semantic::Intepreter::visit(ast::IdentifierExpr &ie)
{
    auto p = curr_frame_->lookup_rvalue(ie.name());
    if (!p.has_value()) {
        diags_->error("Undefined variable '{}'", ie.name());
        curr_frame_->dump(std::cout);
        last_visited_.reset();
        return;
    }
    last_visited_ = *p;
}

void semantic::Intepreter::visit(ast::ReturnStatement &rs)
{
    rs.returned_value()->accept(*this);
    last_returned_ = last_visited_;
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
