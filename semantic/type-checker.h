#pragma once
#include <ast/ast.h>
#include <ast/node-visitor.h>

namespace semantic {

class TypeChecker : public ast::NodeVisitor {
  public:
    void visit(ast::Program &) override {}

    void visit(ast::VariableDeclaration &) override {}
    void visit(ast::FunctionDeclaration &) override {}

    void visit(ast::CompoundStatement &) override {}
    void visit(ast::DeclarationStatement &) override {}
    void visit(ast::ExpressionStatement &) override {}
    void visit(ast::ReturnStatement &) override {}
    void visit(ast::IfStatement &) override {}

    void visit(ast::IdentifierExpr &) override {}
    void visit(ast::UnaryExpression &e) override
    {
        e.type();
    }
    void visit(ast::BinaryExpression &) override {}
    void visit(ast::CallExpression &) override {}

    void visit(ast::IntegerLiteralExpr &) override {}
    void visit(ast::FloatLiteralExpr &) override {}
    void visit(ast::StringLiteralExpr &) override {}
};

} // namespace semantic
