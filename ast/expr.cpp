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

void ast::IdentifierExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::CallExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::UnaryExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::BinaryExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::IndexExpression::accept(NodeVisitor &v)
{
    v.visit(*this);
}
