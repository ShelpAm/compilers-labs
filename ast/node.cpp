#include <ast/node.h>

#include <semantic/semantic-analyzer.h>

void ast::Node::accept([[maybe_unused]] semantic::SemanticAnalyzer &sa) const
{
    throw std::runtime_error{
        "Falling back to the base implementation of Node::accept"};
}
