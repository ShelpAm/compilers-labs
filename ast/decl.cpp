#include <ast/decl.h>

#include <ast/node-visitor.h>
#include <ast/stmt.h>
#include <semantic/semantic-analyzer.h>

void ast::VariableDeclaration::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "VariableDeclaration\n";

    make_indent(os, indent + 1);
    os << "Type: " << type_.value << "\n";

    make_indent(os, indent + 1);
    os << "Name: " << name_ << "\n";

    if (init_) {
        make_indent(os, indent + 1);
        os << "Init:\n";
        init_->dump(os, indent + 2);
    }
}

void ast::VariableDeclaration::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::FunctionDeclaration::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "FunctionDeclaration\n";

    make_indent(os, indent + 1);
    os << "ReturnType: " << return_type_.value << "\n";

    make_indent(os, indent + 1);
    os << "Name: " << name_ << "\n";

    make_indent(os, indent + 1);
    os << "Parameters:\n";
    for (auto const &[type, name] : parameters_) {
        make_indent(os, indent + 2);
        os << type << ' ' << name << '\n';
    }

    if (body_) {
        make_indent(os, indent + 1);
        os << "Body:\n";
        body_->dump(os, indent + 2);
    }
}

void ast::FunctionDeclaration::accept(NodeVisitor &v)
{
    v.visit(*this);
}
