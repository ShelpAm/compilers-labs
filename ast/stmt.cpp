#include <ast/stmt.h>

#include <ast/node-visitor.h>

void ast::EmptyStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "EmptyStatement\n";
}

void ast::CompoundStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "CompoundStatement\n";

    for (auto const &stmt : stmts_)
        stmt->dump(os, indent + 1);
}

void ast::ExpressionStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "ExpressionStatement\n";
    if (expr_)
        expr_->dump(os, indent + 1);
}

void ast::ReturnStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "ReturnStatement\n";
    if (returned_value_)
        returned_value_->dump(os, indent + 1);
}

void ast::DeclarationStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "DeclarationStatement\n";
    if (decl_)
        decl_->dump(os, indent + 1);
}

void ast::EmptyStatement::accept(NodeVisitor & /*unused*/) {}

void ast::CompoundStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::DeclarationStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::ReturnStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::ExpressionStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}
