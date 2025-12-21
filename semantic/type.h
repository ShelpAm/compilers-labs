#pragma once
#include <string>

namespace semantic {

enum class TypeKind : unsigned char {
    integer_type,
    float_type,
    string_type,
    // class_type
};

struct Type {
  public:
    std::size_t size;
    TypeKind typekind;
};

} // namespace semantic
