#pragma once
#include <ast/node.h>
#include <memory>
#include <semantic/type.h>
#include <vector>

class Parser;

namespace ast {

class NodeVisitor;

class Expression : public Node {
  public:
    void set_type(semantic::BasicType *type)
    {
        type_ = type;
    }

    semantic::BasicType const &type()
    {
        return *type_;
    }

  private:
    semantic::BasicType *type_;
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

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto value() const
    {
        return std::stoll(value_);
    }

    [[nodiscard]] std::string const &strvalue() const
    {
        return value_;
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

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::string const &value() const
    {
        return value_;
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

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::string const &value() const
    {
        return value_;
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

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::string const &name() const
    {
        return name_;
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
        os << "CallExpression\n";

        make_indent(os, indent + 1);
        os << "Callee: " << '\n';
        callee_->dump(os, indent + 2);

        make_indent(os, indent + 1);
        os << "Arguments:\n";
        for (auto const &arg : arguments_) {
            arg->dump(os, indent + 2);
        }
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] ExpressionPtr const &callee() const
    {
        return callee_;
    }

    [[nodiscard]] std::vector<ExpressionPtr> const &arguments() const
    {
        return arguments_;
    }

  private:
    ExpressionPtr callee_;
    std::vector<ExpressionPtr> arguments_;
};

class IndexExpression : public PostfixExpression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "IndexExpression\n";

        make_indent(os, indent + 1);
        os << "Base:" << '\n';
        base_->dump(os, indent + 2);

        make_indent(os, indent + 1);
        os << "Indices:\n";
        for (auto const &index : indices_) {
            index->dump(os, indent + 2);
        }
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] ExpressionPtr const &base() const
    {
        return base_;
    }

    [[nodiscard]] std::vector<ExpressionPtr> const &indices() const
    {
        return indices_;
    }

  private:
    ExpressionPtr base_;
    std::vector<ExpressionPtr> indices_;
};

class UnaryExpression : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "UnaryExpression\n";
        make_indent(os, indent + 1);
        os << std::format("Operator: {}", op_) << '\n';
        expr_->dump(os, indent + 1);
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::string const &op() const
    {
        return op_;
    }

    [[nodiscard]] ExpressionPtr const &expr() const
    {
        return expr_;
    }

  private:
    std::string op_;
    ExpressionPtr expr_;
};

class BinaryExpression : public Expression {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "BinaryExpression\n";
        make_indent(os, indent + 1);
        os << std::format("Operator: {}", op_.value) << '\n';
        lhs_->dump(os, indent + 1);
        rhs_->dump(os, indent + 1);
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] Token const &op() const
    {
        return op_;
    }

    [[nodiscard]] ExpressionPtr const &lhs() const
    {
        return lhs_;
    }

    [[nodiscard]] ExpressionPtr const &rhs() const
    {
        return rhs_;
    }

  private:
    Token op_;
    ExpressionPtr lhs_;
    ExpressionPtr rhs_;
};

} // namespace ast
