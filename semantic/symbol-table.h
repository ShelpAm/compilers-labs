#pragma once
#include <cstddef>
#include <helper.h>
#include <print>
#include <semantic/symbol.h>
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace semantic {

class SymbolTable {
  public:
    bool define(std::string const &name, Symbol s)
    {
        if (symbol_table_.contains(name)) {
            return false;
        }
        symbol_table_.insert({name, std::make_unique<Symbol>(std::move(s))});
        return true;
    }

    Symbol *lookup(std::string const &name)
    {
        if (auto it = symbol_table_.find(name); it != symbol_table_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void dump(std::size_t indent) const
    {
        spdlog::debug("{}Symbol Table:", indent_string(indent));
        for (auto const &[name, symbol] : symbol_table_) {
            spdlog::debug("{}    {} -> {}", indent_string(indent), name,
                          symbol->type_ptr->canonical_name());
        }
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbol_table_;
};

} // namespace semantic
