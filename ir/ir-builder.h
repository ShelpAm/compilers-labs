#pragma once
#include <ast/ast.h>
#include <cassert>
#include <diagnostics.h>
#include <ir/instruction.h>
#include <semantic/context.h>

namespace semantic {

class Context;

}

namespace ir {

class IRBuilder : public ast::RecursiveNodeVisitor {
  public:
    IRBuilder() = default;

    void visit(ast::VariableDeclaration &vd) override
    {
        assert(vd.symbol());
        auto *id = symbol_reg_[vd.symbol()] = reg();
        if (vd.init()) {
            auto *val = emit_expr(vd.init());
            emit({.op = Operator::add, .operands{id, imm(0), val}});
        }
    }

    void visit(ast::FunctionDeclaration &fd) override
    {
        auto *func_label = symbol_label_[fd.symbol()] = label();
        emit({.op = Operator::label, .operands = {func_label}});
        for (auto const &param : fd.parameters()) {
            // FIXME: this should be linked to arguments
            symbol_reg_[param.symbol] = reg();
        }
        for (auto const &stmt : fd.body()->statements())
            stmt->accept(*this);
    }

    void visit(ast::ReturnStatement &rs) override
    {
        emit({.op = Operator::ret,
              .operands = {emit_expr(rs.returned_value())}});
    }

    void visit(ast::IfStatement &is) override
    {
        auto *cond = emit_expr(is.condition());

        auto *l_true = label();
        auto *l_false = label();
        auto *l_end = label();

        emit({.op = Operator::beq, .operands = {cond, imm(0), l_false}});

        // True block
        emit({.op = Operator::label, .operands = {l_true}});
        is.true_branch()->accept(*this);
        emit({.op = Operator::j, .operands = {l_end}});

        // False block
        emit({.op = Operator::label, .operands = {l_false}});
        if (is.false_branch())
            is.false_branch()->accept(*this);

        // End
        emit({.op = Operator::label, .operands = {l_end}});
    }

    void visit(ast::WhileStatement &ws) override
    {
        auto *l_begin = label();
        auto *l_end = label();

        // Condition
        emit({.op = Operator::label, .operands = {l_begin}});
        auto *cond = emit_expr(ws.condition());
        emit({.op = Operator::beq, .operands = {l_end, cond, imm(0)}});

        ws.body()->accept(*this);

        // J
        emit({.op = Operator::j, .operands = {l_begin}});

        // End
        emit({.op = Operator::label, .operands = {l_end}});
    }

    void visit(ast::UnaryExpression &e) override
    {
        auto *expr = emit_expr(e.expr());
        auto *dst = reg();
        if (e.op() == "-") {
            emit({.op = Operator::sub, .operands = {dst, imm(0), expr}});
        }
        else if (e.op() == "+") {
            emit({.op = Operator::add, .operands = {dst, imm(0), expr}});
        }
        last_register_ = dst;
    }

    void visit(ast::BinaryExpression &e) override
    {
        auto *lhs = emit_expr(e.lhs());
        auto *rhs = emit_expr(e.rhs());
        auto *dst = reg();
        if (e.op().is(TokenKind::plus)) {
            emit({.op = Operator::add, .operands = {dst, lhs, rhs}});
        }
        else if (e.op().is(TokenKind::minus)) {
            emit({.op = Operator::sub, .operands = {dst, lhs, rhs}});
        }
        else if (e.op().is(TokenKind::star)) {
            emit({.op = Operator::mul, .operands = {dst, lhs, rhs}});
        }
        else if (e.op().is(TokenKind::slash)) {
            emit({.op = Operator::div, .operands = {dst, lhs, rhs}});
        }
        else if (e.op().is(TokenKind::percent)) {
            emit({.op = Operator::mod, .operands = {dst, lhs, rhs}});
        }
        else if (e.op().is(TokenKind::equalequal)) {
            auto *tmp = reg();
            emit({.op = Operator::sub, .operands = {tmp, lhs, rhs}});
            emit({.op = Operator::sub, .operands = {tmp, imm(1), tmp}});
        }
        last_register_ = dst;
    }

    void visit(ast::IdentifierExpression &e) override
    {
        if (!symbol_reg_.contains(e.symbol())) {
            throw std::runtime_error{
                std::format("{}: symbol_reg_ doesn't contain e.symbol(): {}",
                            e.source_range(), e.name())};
        }
        last_register_ = symbol_reg_.at(e.symbol());
    }

    void visit(ast::IntegerLiteralExpr &e) override
    {
        auto *dst = reg();
        emit({.op = Operator::add, .operands = {dst, imm(0), imm(e.value())}});
        last_register_ = dst;
    }

    void visit(ast::FloatLiteralExpr &e) override
    {
        auto *dst = reg();
        emit({.op = Operator::add,
              .operands = {dst, imm(0), imm(std::stoll(e.value()))}});
        last_register_ = dst;
    }

    void visit(ast::StringLiteralExpr &e) override
    {
        // ?
        (void)e;
        throw std::runtime_error{"Doesn't support string literal"};
    }

    void visit(ast::IndexExpression &e) override
    {
        // +e.base()->type()->size;
        (void)e;
        throw std::runtime_error{"Doesn't support index expression"};
    }

    void visit(ast::CallExpression &e) override
    {
        auto *l = symbol_label_.at(e.callee()->symbol());
        std::vector<Operand *> operands;
        operands.push_back(l);
        for (auto const &arg : e.arguments()) {
            operands.push_back(emit_expr(arg));
        }
        emit({.op = Operator::call, .operands = std::move(operands)});
    }

    void dump()
    {
        for (auto const &ins : instructions_) {
            std::string buf;
            if (ins.op != Operator::label)
                buf += "  ";
            buf += to_string(ins.op);
            buf += ' ';
            for (bool first{true}; auto const &operand : ins.operands) {
                if (!first) {
                    buf += ", ";
                }
                first = false;
                buf += operand->to_string();
            }
            spdlog::debug("{}", buf);
        }
    }

  private:
    void emit(Instruction const &ins)
    {
        instructions_.push_back(ins);
    }

    Register *emit_expr(ast::ExpressionPtr const &e)
    {
        e->accept(*this);
        return last_register_;
    }

    Register *reg()
    {
        registers_.push_back(std::make_unique<Register>(next_reg_id_++));
        return registers_.back().get();
    }

    Immediate *imm(std::int64_t v)
    {
        immediates_.push_back(std::make_unique<Immediate>(v));
        return immediates_.back().get();
    }

    Label *label()
    {
        labels_.push_back(std::make_unique<Label>(next_label_id_++));
        return labels_.back().get();
    }

    std::size_t next_reg_id_{};
    std::size_t next_label_id_{};
    std::vector<std::unique_ptr<Register>> registers_;
    std::vector<std::unique_ptr<Immediate>> immediates_;
    std::vector<std::unique_ptr<Label>> labels_;
    Register *last_register_{};
    std::vector<Instruction> instructions_;
    std::unordered_map<semantic::Symbol *, Register *> symbol_reg_;
    std::unordered_map<semantic::Symbol *, Label *> symbol_label_;
};

} // namespace ir
