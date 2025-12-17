#pragma once
#include <ast/decl.h>
#include <vector>

namespace semantic {

class SemanticAnalyzer;

}

namespace ast {

class Program : public Node {
    friend class ::Parser;
    friend class semantic::SemanticAnalyzer;

  public:
    Program();

    void dump(std::ostream &os, int indent = 0) const override;

  private:
    std::vector<std::unique_ptr<Declaration>> decls_;
};

} // namespace ast
