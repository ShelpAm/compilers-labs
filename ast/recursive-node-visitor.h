#pragma once
#include <ast/node-visitor.h>

namespace ast {

class RecursiveNodeVisitor : public NodeVisitor {
  public:
    RecursiveNodeVisitor() = default;
    RecursiveNodeVisitor(RecursiveNodeVisitor const &) = default;
    RecursiveNodeVisitor(RecursiveNodeVisitor &&) = delete;
    RecursiveNodeVisitor &operator=(RecursiveNodeVisitor const &) = default;
    RecursiveNodeVisitor &operator=(RecursiveNodeVisitor &&) = delete;
    ~RecursiveNodeVisitor() override = default;

    void visit(Program &p) override;

    void visit(VariableDeclaration &vd) override;
    void visit(FunctionDeclaration &fd) override;

    void visit(CompoundStatement &cs) override;
    void visit(DeclarationStatement &ds) override;
    void visit(ExpressionStatement &es) override;
    void visit(ReturnStatement &rs) override;
    void visit(IfStatement &is) override;
    void visit(WhileStatement &ws) override;

    void visit(CallExpression &ce) override;
    void visit(UnaryExpression &ue) override;
    void visit(BinaryExpression &be) override;
    void visit(IdentifierExpression &ie) override;
    void visit(IntegerLiteralExpr &ie) override;
    void visit(FloatLiteralExpr &fe) override;
    void visit(StringLiteralExpr &se) override;
    void visit(IndexExpression &ie) override;

    void visit(BasicType &bt) override;
    void visit(ArrayType &at) override;
    void visit(PointerType &pt) override;
};

} // namespace ast
