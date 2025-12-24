#pragma once
#include <ast/decl.h>
#include <ast/expr.h>
#include <ast/node.h>
#include <memory>
#include <vector>

class Parser;

namespace ast {

class Declaration;
class Expression;
class NodeVisitor;

class Statement : public Node {};

class EmptyStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;
};

class CompoundStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&statments() const
    {
        return stmts_;
    }

  private:
    std::vector<std::unique_ptr<Statement>> stmts_;
};

class ReturnStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] ExpressionPtr const &returned_value() const
    {
        return returned_value_;
    }

  private:
    std::unique_ptr<Expression> returned_value_;
};

class IfStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] ExpressionPtr const &condition() const
    {
        return condition_;
    }

    [[nodiscard]] std::unique_ptr<Statement> const &true_branch() const
    {
        return true_branch_;
    }

    [[nodiscard]] std::unique_ptr<Statement> const &false_branch() const
    {
        return false_branch_;
    }

  private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Statement> true_branch_;
    std::unique_ptr<Statement> false_branch_;
};

class DeclarationStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&declaration() const
    {
        return decl_;
    }

  private:
    std::unique_ptr<Declaration> decl_;
};

class ExpressionStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::unique_ptr<Expression> const &expr() const
    {
        return expr_;
    }

  private:
    std::unique_ptr<Expression> expr_;
};

} // namespace ast
