#include <ast.h>
#include <cassert>
#include <lexer.h>

namespace ast {

Program::Program() = default;

void Program::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "Program\n";
    for (auto const &d : decls_)
        d->dump(os, indent + 1);
}

void CompoundStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "StatementSequence\n";

    for (auto const &stmt : stmts_)
        stmt->dump(os, indent + 1);
}

void ExpressionStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "ExpressionStatement\n";

    // TODO: Implement this
    // expr_->dump(os, indent + 1);
}

void ReturnStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "ReturnStatement\n";
    if (value_)
        value_->dump(os, indent + 1);
}

void DeclarationStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "DeclarationStatement\n";
    if (decl_)
        decl_->dump(os, indent + 1);
}

} // namespace ast
