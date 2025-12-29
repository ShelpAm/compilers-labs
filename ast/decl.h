#pragma once
#include <ast/expr.h>
#include <ast/node.h>
#include <ast/type.h>
#include <lex/token.h>
#include <memory>
#include <vector>

namespace semantic {

struct Symbol;

class SemanticAnalyzer;

} // namespace semantic

class Parser;

namespace ast {

class Expression;
class CompoundStatement;

class Declaration : public Node {
  public:
    void set_symbol(semantic::Symbol *sym)
    {
        symbol_ = sym;
    }

    [[nodiscard]] semantic::Symbol const &symbol() const
    {
        return *symbol_;
    }

  private:
    semantic::Symbol *symbol_{};
};

class VariableDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&name() const
    {
        return name_;
    }

    [[nodiscard]] auto &&type() const
    {
        return type_;
    }

    [[nodiscard]] auto &&init() const
    {
        return init_;
    }

  private:
    std::string name_;
    TypePtr type_;
    ExpressionPtr init_;
};

class FunctionDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&name() const
    {
        return name_;
    }

    [[nodiscard]] auto &&return_type() const
    {
        return return_type_;
    }

    [[nodiscard]] auto &&body() const
    {
        return body_;
    }

    [[nodiscard]] auto &&parameters() const
    {
        return parameters_;
    }

  private:
    struct Parameter {
        TypePtr type;
        std::string name;
    };

    TypePtr return_type_;
    std::string name_;
    std::vector<Parameter> parameters_;
    std::unique_ptr<CompoundStatement> body_;
};

} // namespace ast
