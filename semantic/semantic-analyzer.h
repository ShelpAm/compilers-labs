#pragma once
#include <ast/ast.h>
#include <unordered_map>

class Diagnostics;

namespace semantic {

struct Context;

/// brief A symbol collector and name resolver and type checker.
class SemanticAnalyzer : public ast::RecursiveNodeVisitor {
  public:
    SemanticAnalyzer(Context *ctx, Diagnostics *diags);

    /// @brief Analyzes a program
    void visit(ast::Program &prog) override;

    void visit(ast::VariableDeclaration &vd) override;
    void visit(ast::FunctionDeclaration &fd) override;

    void visit(ast::IfStatement &is) override;
    void visit(ast::WhileStatement &ws) override;
    void visit(ast::CompoundStatement &cs) override;
    void visit(ast::DeclarationStatement &ds) override;

    void visit(ast::IndexExpression &ie) override;
    void visit(ast::CallExpression &ce) override;
    void visit(ast::IdentifierExpression &ie) override;
    void visit(ast::UnaryExpression &ue) override;
    void visit(ast::BinaryExpression &be) override;
    void visit(ast::IntegerLiteralExpr &ie) override;
    void visit(ast::FloatLiteralExpr &fe) override;
    void visit(ast::StringLiteralExpr &se) override;

    void visit(ast::BasicType &bt) override;
    void visit(ast::ArrayType &at) override;
    void visit(ast::PointerType &pt) override;

  private:
    Type *resolve_type(std::string_view name);

    Context *ctx_;
    Diagnostics *diags_;
};

} // namespace semantic
