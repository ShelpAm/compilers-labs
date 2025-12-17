#pragma once
#include <ast/node.h>
#include <memory>
#include <semantic/type.h>
#include <vector>

class Parser;

namespace ast {

class Expression : public Node {
  public:
    // TODO: Does nothing, implement after.
    void accept(semantic::SemanticAnalyzer &sa) const override {}

  protected:
    semantic::Type *type;
};
using ExpressionPtr = std::unique_ptr<Expression>;

class PrimaryExpression : public Expression {};
using PrimaryExpressionPtr = std::unique_ptr<PrimaryExpression>;

class IntegerLiteralExpr : public PrimaryExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "IntegerLiteral(" << value_ << ")\n";
    }

    [[nodiscard]] std::int_least64_t value() const
    {
        return std::stoll(value_);
    }

  private:
    std::string value_;
};

class FloatLiteralExpr : public PrimaryExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "FloatLiteral(" << value_ << ")\n";
    }

  private:
    std::string value_;
};

class StringLiteralExpr : public PrimaryExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "StringLiteral(" << value_ << ")\n";
    }

  private:
    std::string value_;
};

class IdentifierExpr : public PrimaryExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "IdentifierExpr(" << name_ << ")\n";
    }

  private:
    std::string name_;
};

class PostfixExpression : public Expression {};

class CallExpression : public PostfixExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "CallExpr\n";

        make_indent(os, indent + 1);
        os << "Callee: " << '\n';
        callee_->dump(os, indent + 2);

        make_indent(os, indent + 1);
        os << "Arguments:\n";
        for (auto const &arg : arguments_) {
            arg->dump(os, indent + 2);
        }
    }

  private:
    ExpressionPtr callee_;
    std::vector<ExpressionPtr> arguments_;
};

class UnaryOperationExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "UnaryOperationExpr\n";
        make_indent(os, indent + 1);
        os << std::format("Operator: {}", op_) << '\n';
        expr_->dump(os, indent + 1);
    }

  private:
    std::string op_;
    ExpressionPtr expr_;
};

class BinaryOperationExpr : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "BinaryOperationExpr\n";
        make_indent(os, indent + 1);
        os << std::format("Operator: {}", op_) << '\n';
        lhs_->dump(os, indent + 1);
        rhs_->dump(os, indent + 1);
        // make_indent(os, indent + 1);
        // os << "Deduced Value: " << value() << '\n';
    }

    [[nodiscard]] std::int_least64_t value() const
    {
        std::int_least64_t lhs;
        if (auto *pl = dynamic_cast<BinaryOperationExpr *>(lhs_.get())) {
            lhs = pl->value();
        }
        else if (auto *pl2 = dynamic_cast<IntegerLiteralExpr *>(lhs_.get())) {
            lhs = pl2->value();
        }
        else {
            throw std::exception();
        }
        std::int_least64_t rhs;
        if (auto *p = dynamic_cast<BinaryOperationExpr *>(rhs_.get())) {
            rhs = p->value();
        }
        else if (auto *p1 = dynamic_cast<IntegerLiteralExpr *>(rhs_.get())) {
            rhs = p1->value();
        }
        else {
            throw std::exception();
        }
        if (op_ == "+") {
            return lhs + rhs;
        }
        if (op_ == "-") {
            return lhs - rhs;
        }
        if (op_ == "*") {
            return lhs * rhs;
        }
        if (op_ == "/") {
            return lhs / rhs;
        }
        if (op_ == "%") {
            return lhs % rhs;
        }
        throw std::exception();
    }

  private:
    std::string op_;
    ExpressionPtr lhs_;
    ExpressionPtr rhs_;
};

} // namespace ast
