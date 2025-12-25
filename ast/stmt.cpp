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

void ast::IfStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "IfStatement\n";

    make_indent(os, indent + 1);
    os << "Condition:\n";

    if (condition_) {
        condition_->dump(os, indent + 2);
    }
    else {
        make_indent(os, indent + 2);
        os << "NO CONDITION PROVIDED\n";
    }

    make_indent(os, indent + 1);
    os << "TrueBranch:\n";
    if (true_branch_) {
        true_branch_->dump(os, indent + 2);
    }

    make_indent(os, indent + 1);
    os << "FalseBranch:\n";
    if (false_branch_) {
        false_branch_->dump(os, indent + 2);
    }
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

void ast::IfStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}

void ast::WhileStatement::dump(std::ostream &os, int indent) const
{
    make_indent(os, indent);
    os << "WhileStatement\n";

    make_indent(os, indent + 1);
    os << "Condition:\n";

    if (condition_) {
        condition_->dump(os, indent + 2);
    }
    else {
        make_indent(os, indent + 2);
        os << "NO CONDITION PROVIDED\n";
    }

    make_indent(os, indent + 1);
    os << "Body:\n";
    if (body_) {
        body_->dump(os, indent + 2);
    }
}

void ast::WhileStatement::accept(NodeVisitor &v)
{
    v.visit(*this);
}
