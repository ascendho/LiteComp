#include <gtest/gtest.h>

#include "tests/test_helpers.hpp"

TEST(VmModule, ExecutesArithmetic) {
    auto [result, err] = test_helpers::eval_input("1 + 2 * 3");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "7");
}

TEST(VmModule, ArrayOutOfRangeReturnsNull) {
    auto [result, err] = test_helpers::eval_input("[1, 2, 3][99]");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "null");
}

TEST(VmModule, HashMissingKeyReturnsNull) {
    auto [result, err] = test_helpers::eval_input("{1: 2}[3]");
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(result, "null");
}
