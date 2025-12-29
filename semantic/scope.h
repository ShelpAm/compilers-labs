#pragma once
#include <helper.h>
#include <map>
#include <memory>
#include <ranges>
#include <semantic/symbol-table.h>
#include <semantic/symbol.h>
#include <semantic/type.h>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_set>
#include <utility>

namespace semantic {

class Scope {
    friend class Context;

  public:
    Scope(Scope *parent) : parent_(parent)
    {
        if (parent_ != nullptr)
            parent_->add_child(this);
    }

    // Since Scope doesn't owns any resources, default copy/move is ok
    Scope(Scope const &) = default;
    Scope(Scope &&) noexcept = default;
    Scope &operator=(Scope const &) = default;
    Scope &operator=(Scope &&) = default;

    ~Scope() = default;

    Scope *parent() const
    {
        return parent_;
    }

    void add_child(Scope *child)
    {
        children_.push_back(child);
    }

    Symbol *lookup_symbol(std::string const &name)
    {
        auto *symbol = lookup_local_symbol(name);
        if (symbol != nullptr) {
            return symbol;
        }
        if (parent_ != nullptr) {
            return parent_->lookup_symbol(name);
        }
        return nullptr;
    }

    Symbol *lookup_local_symbol(std::string const &name)
    {
        auto *symbol = symbol_table_.lookup(name);
        if (symbol != nullptr) {
            return symbol;
        }
        return nullptr;
    }

    void define_symbol(std::string const &name, Symbol symbol)
    {
        spdlog::debug("Defining symbol '{}'", symbol.name);
        symbol_table_.define(name, std::move(symbol));
    }

    Type *lookup_type(std::string const &name)
    {
        if (auto it = named_types_.find(name); it != named_types_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    ArrayType *define_array_type(ArrayType array_type)
    {
        auto [it, inserted] = array_types_.emplace(
            array_type, std::make_unique<ArrayType>(array_type));
        return it->second.get();
    }

    PointerType *define_pointer_type(PointerType pointer_type)
    {
        auto [it, inserted] = pointer_types_.emplace(
            pointer_type, std::make_unique<PointerType>(pointer_type));
        return it->second.get();
    }

    FunctionType *define_function_type(FunctionType function_type)
    {
        auto [it, inserted] = function_types_.emplace(
            function_type, std::make_unique<FunctionType>(function_type));
        return it->second.get();
    }

    ArrayType *get_array_type(ArrayType const &atype)
    {
        auto it = array_types_.find(atype);
        if (it != array_types_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    PointerType *get_pointer_type(PointerType const &ptype)
    {
        auto it = pointer_types_.find(ptype);
        if (it != pointer_types_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    FunctionType *get_function_type(FunctionType const &ftype)
    {
        auto it = function_types_.find(ftype);
        if (it != function_types_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void dump(std::size_t indent)
    {
        spdlog::debug("{}Scope at {}:", indent_string(indent),
                      static_cast<void *>(this));
        // All things in the scope
        symbol_table_.dump(indent + 1);

        spdlog::debug("{}Array Types:", indent_string(indent + 1));
        for (auto const &[atype, type] : array_types_) {
            type->dump(indent + 2);
        }

        spdlog::debug("{}Pointer Types:", indent_string(indent + 1));
        for (auto const &[ptype, type] : pointer_types_) {
            type->dump(indent + 2);
        }

        spdlog::debug("{}Function Types:", indent_string(indent + 1));
        for (auto const &[ftype, type] : function_types_) {
            type->dump(indent + 2);
        }

        for (auto const &[name, type] : named_types_) {
            spdlog::debug("{}Named Type {}: Size={}, TypeKind={}",
                          indent_string(indent + 2), name, type->size,
                          to_string(type->typekind));
        }
        spdlog::debug("{}Child Scopes:", indent_string(indent + 1));
        for (auto const &child : children_) {
            // child->dump(indent + 1);
            spdlog::debug("{}Child Scope at {:p}", indent_string(indent + 2),
                          static_cast<void *>(child));
        }
    }

  private:
    Scope *parent_{nullptr};
    std::vector<Scope *> children_;

    SymbolTable symbol_table_;

    // Functions, lambda
    // std::vector<std::unique_ptr<Type>> unnamed_types_;

    std::map<ArrayType, std::unique_ptr<ArrayType>> array_types_;
    std::map<PointerType, std::unique_ptr<PointerType>> pointer_types_;
    std::map<FunctionType, std::unique_ptr<FunctionType>> function_types_;

    // User-defined types
    std::unordered_map<std::string, std::unique_ptr<Type>> named_types_;
};

} // namespace semantic
