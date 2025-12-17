#pragma once
#include <ast/ast.h>
#include <diagnostics.h>
#include <semantic/type.h>
#include <unordered_map>

namespace semantic {

class SemanticAnalyzer {
  public:
    SemanticAnalyzer(Diagnostics *diags) : diags_(diags) {}

    void analyze(ast::Program &prog)
    {
        for (auto const &decl : prog.decls_) {
            visit(*decl);
        }
    }

    void summary() const
    {
        for (auto const &[name, decl] : typetable_) {
            std::println("{} ->", name);
            decl->dump(std::cout, 1);
        }
    }

    void visit(ast::Node &node)
    {
        node.accept(*this);
    }

    void visit(ast::VariableDeclaration const &vd)
    {
        if (typetable_.contains(vd.name())) {
            diags_->error("Variable re-declaration error: {}", vd.name());
            return;
        }
        typetable_.insert({vd.name(), &vd});
    }

    void visit(ast::FunctionDeclaration const &fd)
    {
        if (typetable_.contains(fd.name())) {
            diags_->error("Function re-declaration error: {}", fd.name());
            return;
        }
        typetable_.insert({fd.name(), &fd});

        visit(*fd.body());
    }

    void visit(ast::CompoundStatement const &cs)
    {
        for (auto const &stmt : cs.statments()) {
            visit(*stmt);
        }
    }

    void visit(ast::DeclarationStatement const &ds)
    {
        visit(*ds.declaration());
    }

  private:
    std::unordered_map<std::string, ast::Declaration const *> typetable_;
    Diagnostics *diags_;
};

} // namespace semantic
