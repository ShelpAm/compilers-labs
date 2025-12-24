#pragma once
#include <ast/node.h>
#include <lex/token.h>
#include <memory>
#include <vector>

namespace semantic {

class Symbol;

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
    semantic::Symbol *symbol_;
};

class VariableDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

    void accept(NodeVisitor &v) override;

    auto &&name() const
    {
        return name_;
    }

    auto &&type() const
    {
        return type_;
    }

    auto &&init() const
    {
        return init_;
    }

  private:
    Token type_;
    std::string name_;
    std::unique_ptr<Expression> init_;
};

class FunctionDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

    void accept(NodeVisitor &v) override;

    auto &&name() const
    {
        return name_;
    }

    auto &&return_type() const
    {
        return return_type_;
    }

    auto &&body() const
    {
        return body_;
    }

    auto &&parameters() const
    {
        return parameters_;
    }

  private:
    struct Parameter {
        std::string type;
        std::string name;
    };

    Token return_type_;
    std::string name_;
    std::vector<Parameter> parameters_;
    std::unique_ptr<CompoundStatement> body_;
};

} // namespace ast
