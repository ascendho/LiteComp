// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include <vector>

#include "litecomp/bytecode.hpp"
#include "litecomp/compiler.hpp"
#include "litecomp/parser.hpp"
#include "tests/test_helpers.hpp"

namespace {
Instructions concat(const std::vector<Instructions>& list) {
    Instructions out;
    for (const auto& ins : list) {
        out.insert(out.end(), ins.begin(), ins.end());
    }
    return out;
}
}  // namespace

// [测试用例] 验证 CompilerModule::CompileSimpleArithmeticProgram
TEST(CompilerModule, CompileSimpleArithmeticProgram) {
    std::vector<std::string> parser_errors;
    std::shared_ptr<Compiler> compiler;

    auto err = test_helpers::compile_program("1 + 2", &compiler, &parser_errors);
    ASSERT_TRUE(parser_errors.empty());
    ASSERT_EQ(err, nullptr);

    auto bytecode = compiler->bytecode();
    auto expected = concat({
        make(OpType::OpConstant, 0),
        make(OpType::OpConstant, 1),
        make(OpType::OpAdd),
        make(OpType::OpPop),
    });

    EXPECT_EQ(bytecode->instructions, expected);
    ASSERT_EQ(bytecode->constants.size(), 2u);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(bytecode->constants[0])->value, 1);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(bytecode->constants[1])->value, 2);
}

// [测试用例] 验证 CompilerModule::CompileUndefinedIdentifierReturnsError
TEST(CompilerModule, CompileUndefinedIdentifierReturnsError) {
    std::vector<std::string> parser_errors;
    std::shared_ptr<Compiler> compiler;

    auto err = test_helpers::compile_program("x", &compiler, &parser_errors);
    ASSERT_TRUE(parser_errors.empty());
    ASSERT_NE(err, nullptr);
    EXPECT_NE(err->message.find("Not Defined"), std::string::npos);
}
