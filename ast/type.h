#pragma once
#include <ast/expr.h>
#include <ast/node.h>
#include <memory>
#include <semantic/type.h>

class Parser;

namespace ast {

class Type : public Node {
  public:
    void set_type(semantic::Type *type)
    {
        type_ = type;
    }

    [[nodiscard]] semantic::Type *type() const
    {
        return type_;
    }

  private:
    semantic::Type *type_{};
};
using TypePtr = std::unique_ptr<Type>;

class BasicType : public Type {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "BasicType(" << name_ << ")\n";
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] std::string const &name() const
    {
        return name_;
    }

  private:
    std::string name_;
};

class ArrayType : public Type {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "ArrayType\n";
        element_type_->dump(os, indent + 1);
        make_indent(os, indent + 1);
        os << "Size: " << size_ << '\n';
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&element_type() const
    {
        return element_type_;
    }

    [[nodiscard]] auto &&size() const
    {
        return size_;
    }

  private:
    std::unique_ptr<Type> element_type_;
    std::size_t size_;
};

class PointerType : public Type {
    friend class ::Parser;

  public:
    void dump(std::ostream &os, int indent) const override
    {
        make_indent(os, indent);
        os << "PointerType\n";
        pointee_type_->dump(os, indent + 1);
    }

    void accept(NodeVisitor &v) override;

    [[nodiscard]] auto &&pointee_type() const
    {
        return pointee_type_;
    }

  private:
    std::unique_ptr<Type> pointee_type_;
};

} // namespace ast
