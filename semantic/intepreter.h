#pragma once
#include <ast/node-visitor.h>
#include <optional>
#include <semantic/frame.h>
#include <semantic/semantic-analyzer.h>
#include <string>
#include <unordered_map>

namespace semantic {

class Intepreter : public ast::NodeVisitor {
  public:
    Intepreter(Context *ctx, Diagnostics *diags);

    void dump(std::ostream &os);

    void visit(ast::Program &p) override;
    void visit(ast::VariableDeclaration &vd) override;
    void visit(ast::FunctionDeclaration &fd) override;

    void visit(ast::CompoundStatement &cs) override;
    void visit(ast::DeclarationStatement &ds) override;
    void visit(ast::ReturnStatement &rs) override; // Temporary solution for
    // jump out from function in advance: using exception
    void visit(ast::IfStatement &is) override;
    void visit(ast::WhileStatement &ws) override;

    void visit(ast::CallExpression &ce) override;
    void visit(ast::UnaryExpression &uoe) override;
    void visit(ast::BinaryExpression &boe) override;
    void visit(ast::IdentifierExpr &ie) override;
    void visit(ast::IntegerLiteralExpr &ie) override;
    void visit(ast::FloatLiteralExpr &fe) override;
    void visit(ast::StringLiteralExpr &se) override;

    /// Should be private, but put here for test
    void enter_subframe();

    void leave_frame();

  private:
    std::string eval(ast::ExpressionPtr const &expr)
    {
        expr->accept(*this);
        return last_visited_.value();
    }

    std::string eval(ast::Expression *expr)
    {
        expr->accept(*this);
        spdlog::debug("Evaluated result: {}", last_visited_.value());
        return last_visited_.value();
    }

    Context *ctx_;
    Diagnostics *diags_;
    std::unique_ptr<Frame> curr_frame_;
    std::optional<std::string> last_visited_;
    std::optional<std::string> last_returned_;
};

} // namespace semantic
