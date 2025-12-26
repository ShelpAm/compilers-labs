#pragma once
// #include <ast/ast.h>
#include <memory>
#include <string>
#include <vector>

namespace ast {

class FunctionDeclaration;

}

namespace semantic {

enum class TypeKind : unsigned char {
    function_type,

    integer_type,
    float_type,
    string_type,
    // pointer_type, // Temporarily unsupported
    // class_type // User defined // Temporarily unsupported
};

inline std::string to_string(TypeKind k)
{
    switch (k) {
    case TypeKind::function_type:
        return "function_type";
    case TypeKind::integer_type:
        return "integer_type";
    case TypeKind::float_type:
        return "float_type";
    case TypeKind::string_type:
        return "string_type";
    default:
        return "unknown";
    }
}

struct BasicType {
    std::size_t size;
    TypeKind typekind;
};

// Size must be the same as PointerType
struct FunctionType : public BasicType {
    BasicType *return_type;
    std::vector<BasicType *> parameter_types;
    ast::FunctionDeclaration *decl;
};

// Temporarily unsupported
// struct UserDefinedType : public BasicType {
//      std::string name;
//      std::vector<BasicType *,,, fieldname> fields;
// };

} // namespace semantic
