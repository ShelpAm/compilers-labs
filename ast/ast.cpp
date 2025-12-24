#include <ast/ast.h>
#include <cassert>
#include <lex/lexer.h>
#include <semantic/semantic-analyzer.h>

namespace ast {

Program::Program() = default;

void Program::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "Program\n";
    for (auto const &d : decls_)
        d->dump(os, indent + 1);
}

void Program::accept(NodeVisitor &v)
{
    v.visit(*this);
}

} // namespace ast
