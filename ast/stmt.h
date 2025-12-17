#pragma once
#include <ast/ast.h>
#include <ast/decl.h>
#include <ast/expr.h>
#include <memory>
#include <vector>

namespace ast {

class Statement : public Node {};

class EmptyStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;
};

class CompoundStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::vector<std::unique_ptr<Statement>> stmts_;
};

class ReturnStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Expression> value_;
};

class DeclarationStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Declaration> decl_;
};

class ExpressionStatement : public Statement {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::unique_ptr<Expression> expr_;
};

} // namespace ast
