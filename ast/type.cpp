#include <ast/node-visitor.h>
#include <ast/type.h>

void ast::BasicType::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::ArrayType::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::PointerType::accept(NodeVisitor &v)
{
    v.visit(*this);
}
