#pragma once
#include <cassert>
#include <iostream>
// #include <ostream>

namespace semantic {

class SemanticAnalyzer;

}

namespace ast {

class Node {
  public:
    Node() = default;
    Node(Node const &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node const &) = delete;
    Node &operator=(Node &&) = delete;
    virtual ~Node() = default;

    virtual void dump(std::ostream &os, int indent = 0) const = 0;

    virtual void accept([[maybe_unused]] semantic::SemanticAnalyzer &sa) const;

  protected:
    static void make_indent(std::ostream &os, int n)
    {
        // std::println(std::cerr, "Printing indent: {}", n);
        for (int i = 0; i < n; ++i)
            os << "    ";
    }
};

} // namespace ast
