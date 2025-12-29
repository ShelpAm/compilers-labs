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

class IdentifierExpression;
class UnaryExpression;
class BinaryExpression;
class CallExpression;
class IndexExpression;
class IntegerLiteralExpr;
class FloatLiteralExpr;
class StringLiteralExpr;

class BasicType;
class ArrayType;
class PointerType;

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
    virtual void visit(ExpressionStatement &) = 0;
    virtual void visit(ReturnStatement &) = 0;
    virtual void visit(IfStatement &) = 0;
    virtual void visit(WhileStatement &) = 0;

    virtual void visit(IdentifierExpression &) = 0;
    virtual void visit(UnaryExpression &) = 0;
    virtual void visit(BinaryExpression &) = 0;
    virtual void visit(CallExpression &) = 0;
    virtual void visit(IndexExpression &) = 0;
    virtual void visit(IntegerLiteralExpr &) = 0;
    virtual void visit(FloatLiteralExpr &) = 0;
    virtual void visit(StringLiteralExpr &) = 0;

    virtual void visit(BasicType &) = 0;
    virtual void visit(ArrayType &) = 0;
    virtual void visit(PointerType &) = 0;
};

} // namespace ast
