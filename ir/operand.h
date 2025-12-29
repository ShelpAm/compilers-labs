#pragma once
#include <cstdint>
#include <string>

namespace ir {

struct Operand {
    enum class Kind : std::uint8_t {
        op_register,
        op_immediate,
        op_label,
    };

    explicit Operand(Kind kind) : kind(kind) {}
    Operand(Operand const &) = default;
    Operand(Operand &&) = delete;
    Operand &operator=(Operand const &) = default;
    Operand &operator=(Operand &&) = delete;
    virtual ~Operand() = default;

    virtual std::string to_string() = 0;

    Kind kind;
};

struct Register : public Operand {
    explicit Register(std::size_t id) : Operand(Kind::op_register), id(id) {}

    std::string to_string() override
    {
        return "%" + std::to_string(id);
    }

    std::size_t id;
};

struct Immediate : public Operand {
    explicit Immediate(std::int64_t value)
        : Operand(Kind::op_immediate), value(value)
    {
    }

    std::string to_string() override
    {
        return std::to_string(value);
    }

    std::int64_t value;
};

struct Label : public Operand {
    Label(std::size_t id) : Operand(Kind::op_label), id(id) {}

    std::string to_string() override
    {
        return "l" + std::to_string(id);
    }

    std::size_t id;
};

} // namespace ir
