// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include <vector>

#include "litecomp/builtins.hpp"

// [测试用例] 验证 BuiltinsModule::LenSupportsArrayAndString
TEST(BuiltinsModule, LenSupportsArrayAndString) {
    auto array = std::make_shared<Array>(Array{});
    array->elements.push_back(std::make_shared<Integer>(Integer(1)));
    array->elements.push_back(std::make_shared<Integer>(Integer(2)));

    auto arr_len = __len({array});
    auto str_len = __len({std::make_shared<String>(String("hello"))});

    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(arr_len)->value, 2);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(str_len)->value, 5);
}

// [测试用例] 验证 BuiltinsModule::PushReturnsNewArray
TEST(BuiltinsModule, PushReturnsNewArray) {
    auto array = std::make_shared<Array>(Array{});
    array->elements.push_back(std::make_shared<Integer>(Integer(1)));

    auto pushed = __push({array, std::make_shared<Integer>(Integer(2))});
    auto pushed_arr = std::dynamic_pointer_cast<Array>(pushed);

    ASSERT_NE(pushed_arr, nullptr);
    EXPECT_EQ(array->elements.size(), 1u);
    EXPECT_EQ(pushed_arr->elements.size(), 2u);
}

// [测试用例] 验证 BuiltinsModule::UnknownBuiltinByNameReturnsNullptr
TEST(BuiltinsModule, UnknownBuiltinByNameReturnsNullptr) {
    EXPECT_EQ(getBuiltinByName("unknown_builtin"), nullptr);
}
