#include <ast/node-visitor.h>

#include <ast/expr.h>
#include <ast/stmt.h>

void ast::NodeVisitor::visit(ExpressionStatement &es)
{
    es.expr()->accept(*this);
}

void ast::NodeVisitor::visit(ReturnStatement &rs) {}

void ast::NodeVisitor::visit(CallExpression &ce) {}

void ast::NodeVisitor::visit(IdentifierExpr &)
{
    // default: do nothing
}

void ast::NodeVisitor::visit(UnaryOperationExpr &)
{
    // default: do nothing
}

void ast::NodeVisitor::visit(BinaryOperationExpr &)
{
    // default: do nothing
}
