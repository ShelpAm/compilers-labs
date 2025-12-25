#pragma once

namespace ast {

class Program;

class VariableDeclaration;
class FunctionDeclaration;

class CompoundStatement;
class DeclarationStatement;
class ExpressionStatement;
class ReturnStatement;
class IfStatement;
class WhileStatement;

class IdentifierExpr;
class UnaryExpression;
class BinaryExpression;
class CallExpression;
class IndexExpression;
class IntegerLiteralExpr;
class FloatLiteralExpr;
class StringLiteralExpr;

class NodeVisitor {
  public:
    NodeVisitor() = default;
    NodeVisitor(NodeVisitor const &) = default;
    NodeVisitor(NodeVisitor &&) = delete;
    NodeVisitor &operator=(NodeVisitor const &) = default;
    NodeVisitor &operator=(NodeVisitor &&) = delete;
    virtual ~NodeVisitor() = default;

    // Evaluates

    virtual void visit(Program &) = 0;

    virtual void visit(VariableDeclaration &) = 0;
    virtual void visit(FunctionDeclaration &) = 0;

    virtual void visit(CompoundStatement &) = 0;
    virtual void visit(DeclarationStatement &) = 0;
    virtual void visit(ExpressionStatement &);
    virtual void visit(ReturnStatement &);
    virtual void visit(IfStatement &);
    virtual void visit(WhileStatement &);

    virtual void visit(IdentifierExpr &);
    virtual void visit(UnaryExpression &);
    virtual void visit(BinaryExpression &);
    virtual void visit(CallExpression &);
    virtual void visit(IndexExpression &);
    virtual void visit(IntegerLiteralExpr &) = 0;
    virtual void visit(FloatLiteralExpr &) = 0;
    virtual void visit(StringLiteralExpr &) = 0;
};

} // namespace ast
