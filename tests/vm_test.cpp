// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include "litecomp/bytecode.hpp"
#include "litecomp/frame.hpp"
#include "litecomp/object.hpp"
#include "litecomp/vm.hpp"
#include "tests/test_helpers.hpp"

// [测试用例] 验证 VmModule::ExecutesArithmetic
TEST(VmModule, ExecutesArithmetic) {
    auto [result, err] = test_helpers::eval_input("1 + 2 * 3");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "7");
}

// [测试用例] 验证 VmModule::ArrayOutOfRangeReturnsNull
TEST(VmModule, ArrayOutOfRangeReturnsNull) {
    auto [result, err] = test_helpers::eval_input("[1, 2, 3][99]");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "null");
}

// [测试用例] 验证 VmModule::HashMissingKeyReturnsNull
TEST(VmModule, HashMissingKeyReturnsNull) {
    auto [result, err] = test_helpers::eval_input("{1: 2}[3]");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "null");
}

// [测试用例] 验证 VmModule::UnsupportedStringComparisonReturnsError
TEST(VmModule, UnsupportedStringComparisonReturnsError) {
    auto [result, err] = test_helpers::eval_input("\"a\" == \"a\"");
    EXPECT_TRUE(result.empty());
    EXPECT_NE(err.find("unsupported types for comparison"), std::string::npos);
}

// [测试用例] 验证 VmModule::PushFrameGuardPreventsOverflow
TEST(VmModule, PushFrameGuardPreventsOverflow) {
    auto bytecode = std::make_shared<Bytecode>();
    bytecode->instructions = make(OpType::OpNull);

    VM vm(std::move(bytecode));
    auto fn = std::make_shared<CompiledFunction>(CompiledFunction(make(OpType::OpNull)));
    auto cl = std::make_shared<Closure>(Closure(fn));

    std::shared_ptr<Error> err = nullptr;
    for (int i = vm.frames_index; i < MAXFRAMES + 1; ++i) {
        err = vm.push_frame(new_frame(cl, 0));
        if (err) {
            break;
        }
    }

    ASSERT_NE(err, nullptr);
    EXPECT_NE(err->message.find("frame overflow"), std::string::npos);
}
