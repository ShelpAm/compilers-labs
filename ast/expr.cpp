#include <ast/expr.h>
#include <semantic/semantic-analyzer.h>

void ast::IntegerLiteralExpr::accept(semantic::SemanticAnalyzer &sa)
{
    sa.visit(*this);
}
