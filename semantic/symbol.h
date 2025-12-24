#pragma once
#include <semantic/type.h>

namespace semantic {

enum class SymbolKind : unsigned char {
    type,
    variable,
};

struct Symbol {
  public:
    std::string name;
    BasicType *type_ptr;
    SymbolKind symbolkind;
};

} // namespace semantic
