#include <ast/recursive-node-visitor.h>

#include <ast/ast.h>

void ast::RecursiveNodeVisitor::visit(ast::Program &p)
{
    for (auto const &d : p.declaration_statements()) {
        d->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::VariableDeclaration &vd)
{
    if (auto const &init = vd.init()) {
        init->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::FunctionDeclaration &fd)
{
    if (auto const &body = fd.body()) {
        body->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::CompoundStatement &cs)
{
    for (auto const &s : cs.statments()) {
        s->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::DeclarationStatement &ds)
{
    ds.declaration()->accept(*this);
}
void ast::RecursiveNodeVisitor::visit(ast::ExpressionStatement &es)
{
    es.expr()->accept(*this);
}
void ast::RecursiveNodeVisitor::visit(ast::ReturnStatement &rs)
{
    if (auto const &ret = rs.returned_value()) {
        ret->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::IfStatement &is)
{
    is.condition()->accept(*this);
    if (auto const &tb = is.true_branch()) {
        tb->accept(*this);
    }
    if (auto const &fb = is.false_branch()) {
        fb->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::WhileStatement &ws)
{
    ws.condition()->accept(*this);
    ws.body()->accept(*this);
}
void ast::RecursiveNodeVisitor::visit(ast::CallExpression &ce)
{
    ce.callee()->accept(*this);
    for (auto const &arg : ce.arguments()) {
        arg->accept(*this);
    }
}
void ast::RecursiveNodeVisitor::visit(ast::UnaryExpression &e)
{
    e.expr()->accept(*this);
}
void ast::RecursiveNodeVisitor::visit(ast::BinaryExpression &e)
{
    e.lhs()->accept(*this);
    e.rhs()->accept(*this);
}
void ast::RecursiveNodeVisitor::visit(ast::IdentifierExpression & /*unused*/)
{
    // No child nodes
}
void ast::RecursiveNodeVisitor::visit(ast::IntegerLiteralExpr & /*unused*/)
{
    // No child nodes
}
void ast::RecursiveNodeVisitor::visit(ast::FloatLiteralExpr & /*unused*/)
{
    // No child nodes
}
void ast::RecursiveNodeVisitor::visit(ast::StringLiteralExpr & /*unused*/)
{
    // No child nodes
}
void ast::RecursiveNodeVisitor::visit(ast::IndexExpression &ie)
{
    ie.base()->accept(*this);
    for (auto const &index : ie.indices()) {
        index->accept(*this);
    }
}
