#include <gtest/gtest.h>

#include "litecomp/bytecode.hpp"

TEST(BytecodeModule, StoresInstructionsAndConstants) {
    Bytecode code;
    code.instructions = make(OpType::OpTrue);
    code.constants.push_back(std::make_shared<Integer>(Integer(42)));

    ASSERT_EQ(code.instructions.size(), 1u);
    EXPECT_EQ(code.instructions[0], as_opcode(OpType::OpTrue));
    ASSERT_EQ(code.constants.size(), 1u);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(code.constants[0])->value, 42);
}
