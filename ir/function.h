#pragma once
#include <ir/instruction.h>
#include <vector>

namespace ir {

struct Function {
    std::vector<Instruction> instructions_;
};

} // namespace ir
