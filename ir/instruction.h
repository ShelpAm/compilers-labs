#pragma once
#include <ir/operand.h>
#include <memory>
#include <vector>

namespace ir {

enum class Operator : unsigned char {
    unknown,

    label,

    load,
    store,

    add,
    sub,
    // addi,
    mul,
    div,
    mod,

    call,

    ret,

    beq,
    bne,
    blt,
    bge,
    j,
};

inline std::string_view to_string(Operator op)
{
    switch (op) {
    case Operator::unknown:
        return "unknown";

    case Operator::load:
        return "load";
    case Operator::store:
        return "store";

    case Operator::add:
        return "add";
    case Operator::sub:
        return "sub";
    case Operator::mul:
        return "mul";
    case Operator::div:
        return "div";
    case Operator::mod:
        return "mod";

    case Operator::call:
        return "call";
    case Operator::ret:
        return "ret";

    case Operator::beq:
        return "beq";
    case Operator::bne:
        return "bne";
    case Operator::blt:
        return "blt";
    case Operator::bge:
        return "bge";
    case Operator::j:
        return "j";

    case Operator::label:
        return "label";
    }

    return "unknown";
}

struct Instruction {
    Operator op;
    std::vector<Operand *> operands;
};

} // namespace ir
