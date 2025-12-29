#pragma once
#include <cassert>
#include <lex/token.h>
#include <ostream>

inline void make_indent(std::ostream &os, int n)
{
    // std::println(std::cerr, "Printing indent: {}", n);
    for (int i = 0; i < n; ++i)
        os << "    ";
}

namespace ast {

class NodeVisitor;

class Node {
  public:
    Node() = default;
    Node(Node const &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node const &) = delete;
    Node &operator=(Node &&) = delete;
    virtual ~Node() = default;

    virtual void dump(std::ostream &os, int indent = 0) const = 0;

    virtual void accept(NodeVisitor &v) = 0;

    template <typename T> T *as()
    {
        return static_cast<T *>(this);
    }

    [[nodiscard]] SourceRange source_range() const
    {
        return source_range_;
    }

  protected:
    void set_source_begin(SourceLocation sb)
    {
        source_range_.begin = sb;
    }

    void set_source_end(SourceLocation se)
    {
        source_range_.end = se;
    }

  private:
    SourceRange source_range_{};
};

} // namespace ast
