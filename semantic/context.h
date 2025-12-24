#include <semantic/symbol-table.h>
#include <semantic/symbol.h>
#include <semantic/type.h>

namespace semantic {

struct Context {
    SymbolTable symbol_table_;

    // Functions, lambda
    std::vector<std::unique_ptr<FunctionType>> unnamed_types_;

    // User-defined types, built-in types
    std::unordered_map<std::string, std::unique_ptr<BasicType>> named_types_;
};

} // namespace semantic
