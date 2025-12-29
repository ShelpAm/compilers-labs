#pragma once
// #include <ast/ast.h>
#include <helper.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace ast {

class FunctionDeclaration;

}

namespace semantic {

enum class TypeKind : unsigned char {
    function_type,
    builtin_type,
    class_type,
    array_type,
    pointer_type,
};

inline std::string to_string(TypeKind k)
{
    switch (k) {
    case TypeKind::function_type:
        return "function_type";
    case TypeKind::builtin_type:
        return "builtin_type";
    case TypeKind::class_type:
        return "class_type";
    case TypeKind::array_type:
        return "array_type";
    case TypeKind::pointer_type:
        return "pointer_type";
    default:
        return "unknown";
    }
}

struct Type {
    Type(Type const &) = default;
    Type(Type &&) = delete;
    Type &operator=(Type const &) = default;
    Type &operator=(Type &&) = delete;
    Type(std::size_t size, TypeKind typekind) : size(size), typekind(typekind)
    {
    }
    virtual ~Type() = default;

    virtual void dump(std::size_t indent = 0)
    {
        spdlog::debug("{}Type:", indent_string(indent));
    }

    auto operator<=>(Type const &other) const noexcept = default;

    std::size_t size;
    TypeKind typekind; // Only behave as a tag
};

struct BuiltinType : public Type {
    enum class Kind : unsigned char {
        integer_type,
        float_type,
        char_type,
        string_type,
    };

    BuiltinType(std::size_t size, Kind builtintypekind)
        : Type(size, TypeKind::builtin_type), builintypekind(builtintypekind)
    {
    }

    void dump(std::size_t indent = 0) override
    {
        spdlog::debug("{}BuiltinType:", indent_string(indent));
        spdlog::debug("{}Kind: {}", indent_string(indent + 1),
                      static_cast<unsigned>(builintypekind));
    }

    Kind builintypekind;

  private:
    static std::string to_string(Kind k)
    {
        switch (k) {
        case Kind::integer_type:
            return "integer_type";
        case Kind::float_type:
            return "float_type";
        case Kind::char_type:
            return "char_type";
        case Kind::string_type:
            return "string_type";
        default:
            return "unknown";
        }
    }
};

// Size must be the same as PointerType
struct FunctionType : public Type {
    FunctionType(Type *return_type, std::vector<Type *> const &parameter_types,
                 ast::FunctionDeclaration *decl)
        : Type(sizeof(void *), TypeKind::function_type),
          return_type(return_type), parameter_types(parameter_types), decl(decl)
    {
    }

    void dump(std::size_t indent = 0) override
    {
        spdlog::debug("{}FunctionType:", indent_string(indent));
        spdlog::debug("{}Return Type: {}", indent_string(indent + 1),
                      to_string(return_type->typekind));
        spdlog::debug("{}Parameter Types:", indent_string(indent + 1));
        for (auto const &p : parameter_types) {
            spdlog::debug("{}{}", indent_string(indent + 2),
                          to_string(p->typekind));
        }
    }

    auto operator<=>(FunctionType const &other) const noexcept = default;

    Type *return_type;
    std::vector<Type *> parameter_types;
    ast::FunctionDeclaration *decl;
};

struct ArrayType : public Type {
    ArrayType(Type *element_type, std::size_t length)
        : Type(element_type->size * length, TypeKind::array_type),
          element_type(element_type), length(length)
    {
    }

    auto operator<=>(ArrayType const &other) const noexcept = default;

    void dump(std::size_t indent = 0) override
    {
        spdlog::debug("{}ArrayType:", indent_string(indent));
        spdlog::debug("{}Element Type: {}", indent_string(indent + 1),
                      to_string(element_type->typekind));
        spdlog::debug("{}Length: {}", indent_string(indent + 1), length);
    }

    Type *element_type;
    std::size_t length;
};

struct PointerType : public Type {
    PointerType(Type *pointee_type)
        : Type(sizeof(void *), TypeKind::pointer_type),
          pointee_type(pointee_type)
    {
    }

    void dump(std::size_t indent = 0) override
    {
        spdlog::debug("{}PointerType:", indent_string(indent));
        spdlog::debug("{}Pointee Type: {}", indent_string(indent + 1),
                      to_string(pointee_type->typekind));
    }

    auto operator<=>(PointerType const &other) const noexcept = default;

    Type *pointee_type;
};

// Temporarily unsupported
// struct UserDefinedType : public BasicType {
//      std::string name;
//      std::vector<BasicType *,,, fieldname> fields;
// };

} // namespace semantic
