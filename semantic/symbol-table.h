#pragma once
#include <print>
#include <semantic/symbol.h>
#include <unordered_map>

namespace semantic {

class SymbolTable {
  public:
    bool define(std::string const &name, Symbol symbol)
    {
        if (symbol_table_.contains(name)) {
            return false;
        }
        symbol_table_.insert({name, std::move(symbol)});
        return true;
    }

    Symbol *lookup(std::string const &name)
    {
        if (auto it = symbol_table_.find(name); it != symbol_table_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void summary() const
    {
        std::println("Symbols:");
        for (auto const &[name, symbol] : symbol_table_) {
            std::println("    {} -> {}", name,
                         symbol.symbolkind == SymbolKind::variable
                             ? to_string(symbol.type_ptr->typekind)
                             : "some function");
        }
    }

  private:
    std::unordered_map<std::string, Symbol> symbol_table_;
};

} // namespace semantic
