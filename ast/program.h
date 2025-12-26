#pragma once
#include <ast/stmt.h>
#include <vector>

namespace semantic {

class Scope;
class SemanticAnalyzer;

} // namespace semantic

namespace ast {

class Program : public Node {
    friend class ::Parser;
    friend class semantic::SemanticAnalyzer;

  public:
    Program();

    void dump(std::ostream &os, int indent = 0) const override;

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::vector<std::unique_ptr<DeclarationStatement>> const &
    declaration_statements() const
    {
        return decls_;
    }

    [[nodiscard]] semantic::Scope *global_scope() const
    {
        return global_scope_;
    }

  private:
    std::vector<std::unique_ptr<DeclarationStatement>> decls_;
    semantic::Scope *global_scope_{};
};

} // namespace ast
