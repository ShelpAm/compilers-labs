#pragma once
#include <ast/node.h>
#include <lex/token.h>
#include <memory>

class Parser;

namespace ast {

class Expression;
class CompoundStatement;

class Declaration : public Node {};

class VariableDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

  private:
    TokenKind type_;
    std::string name_;
    std::unique_ptr<Expression> init_;
};

class FunctionDeclaration : public Declaration {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override;

  private:
    TokenKind return_type_;
    std::string name_;
    std::unique_ptr<CompoundStatement> body_;
};

} // namespace ast
