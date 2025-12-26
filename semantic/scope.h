#pragma once
#include <helper.h>
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

    FunctionType *define_unnamed_type(FunctionType t)
    {
        unnamed_types_.push_back(std::make_unique<FunctionType>(std::move(t)));
        return unnamed_types_.back().get();
    }

    BasicType *lookup_type(std::string const &name)
    {
        if (auto it = named_types_.find(name); it != named_types_.end()) {
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
        spdlog::debug("{}Unnamed Types:", indent_string(indent + 1));
        for (auto const &[j, type] : std::views::enumerate(unnamed_types_)) {
            auto const ind = indent_string(indent + 2);
            spdlog::debug("{}Unnamed Type {}:", ind, j);
            spdlog::debug("{}    Size: {}", ind, type->size);
            spdlog::debug("{}    TypeKind: {}", ind, to_string(type->typekind));
            spdlog::debug("{}    Return Type: {}", ind,
                          to_string(type->return_type->typekind));
            spdlog::debug("{}    Parameter Types:", ind);
            for (auto const &p : type->parameter_types) {
                spdlog::debug("{}        {}", indent_string(indent + 2),
                              to_string(p->typekind));
            }
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
    std::vector<std::unique_ptr<FunctionType>> unnamed_types_;

    // User-defined types
    std::unordered_map<std::string, std::unique_ptr<BasicType>> named_types_;
};

} // namespace semantic
