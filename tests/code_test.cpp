// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include <vector>

#include "litecomp/code.hpp"

// [测试用例] 验证 CodeModule::MakeAndReadSingleOperandInstruction
TEST(CodeModule, MakeAndReadSingleOperandInstruction) {
    Instructions ins = make(OpType::OpConstant, 65534);
    ASSERT_EQ(ins.size(), 3u);
    EXPECT_EQ(read_uint_16(ins, 1), 65534);
}

// [测试用例] 验证 CodeModule::MakeAndReadTwoOperandInstruction
TEST(CodeModule, MakeAndReadTwoOperandInstruction) {
    Instructions ins = make(OpType::OpClosure, 513, 7);
    ASSERT_EQ(ins.size(), 4u);
    EXPECT_EQ(read_uint_16(ins, 1), 513);
    EXPECT_EQ(read_uint_8(ins, 3), 7);
}

// [测试用例] 验证 CodeModule::LookupUnknownOpcode
TEST(CodeModule, LookupUnknownOpcode) {
    auto [def, ok] = lookup(static_cast<OpType>(255));
    EXPECT_FALSE(ok);
    EXPECT_EQ(def, nullptr);
}

// [测试用例] 验证 CodeModule::FormatInstruction
TEST(CodeModule, FormatInstruction) {
    auto [def, ok] = lookup(OpType::OpGetLocal);
    ASSERT_TRUE(ok);
    EXPECT_EQ(fmt_instruction(def, {3}), "OpGetLocal 3");
}
