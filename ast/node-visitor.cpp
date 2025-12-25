#include <ast/node-visitor.h>

#include <ast/expr.h>
#include <ast/stmt.h>

void ast::NodeVisitor::visit(ExpressionStatement &es)
{
    es.expr()->accept(*this);
}

void ast::NodeVisitor::visit(ReturnStatement & /*unused*/) {}

void ast::NodeVisitor::visit(IfStatement & /*unused*/) {}

void ast::NodeVisitor::visit(WhileStatement & /*unused*/) {}

void ast::NodeVisitor::visit(CallExpression & /*unused*/) {}

void ast::NodeVisitor::visit(IndexExpression & /*unused*/) {}

void ast::NodeVisitor::visit(IdentifierExpr & /*unused*/) {}

void ast::NodeVisitor::visit(UnaryExpression & /*unused*/) {}

void ast::NodeVisitor::visit(BinaryExpression & /*unused*/) {}
