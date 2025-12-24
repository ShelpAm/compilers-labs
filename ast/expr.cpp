#include <ast/expr.h>
#include <semantic/semantic-analyzer.h>

void ast::IntegerLiteralExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}
void ast::FloatLiteralExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::StringLiteralExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::IdentifierExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::CallExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::UnaryOperationExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::BinaryOperationExpr::accept(NodeVisitor &v)
{
    v.visit(*this);
}
