#pragma once
#include <ast/ast.h>
#include <unordered_map>

class Diagnostics;

namespace semantic {

struct Context;

class SemanticAnalyzer : public ast::NodeVisitor {
  public:
    SemanticAnalyzer(Context *ctx, Diagnostics *diags);

    /// @brief Analyzes a program
    void visit(ast::Program &prog) override;

    void summary() const;

    void visit(ast::VariableDeclaration &vd) override;
    void visit(ast::FunctionDeclaration &fd) override;
    void visit(ast::CompoundStatement &cs) override;
    void visit(ast::DeclarationStatement &ds) override;
    void visit(ast::IntegerLiteralExpr &ie) override;
    void visit(ast::FloatLiteralExpr &fe) override;
    void visit(ast::StringLiteralExpr &se) override;

  private:
    BasicType *find_type_by_name(std::string_view name);
    Context *ctx_;
    Diagnostics *diags_;
};

} // namespace semantic
