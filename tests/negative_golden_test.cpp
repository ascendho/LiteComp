// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "litecomp/compiler.hpp"
#include "litecomp/parser.hpp"
#include "tests/test_helpers.hpp"

namespace {

struct NegativeCase {
    std::string name;
    std::string input;
    std::string expected_error;
};

}  // namespace

// [测试用例] 验证 NegativeGolden::ParserErrorsAreStable
TEST(NegativeGolden, ParserErrorsAreStable) {
    const std::vector<NegativeCase> cases = {
        {"missing_prefix_parse_fn", ";", "No Bounded Parse Function Found for ; token"},
        {"declare_missing_identifier", "declare = 1;", "Expected token should be IDENTIFIER, while got = instead"},
    };

    for (const auto& c : cases) {
        std::vector<std::string> parser_errors;
        (void)test_helpers::parse_program(c.input, &parser_errors);

        ASSERT_FALSE(parser_errors.empty()) << c.name;
        EXPECT_EQ(parser_errors.front(), c.expected_error) << c.name;
    }
}

// [测试用例] 验证 NegativeGolden::CompileErrorsAreStable
TEST(NegativeGolden, CompileErrorsAreStable) {
    const std::vector<NegativeCase> cases = {
        {"undefined_identifier", "x", "x is Not Defined"},
    };

    for (const auto& c : cases) {
        std::vector<std::string> parser_errors;
        std::shared_ptr<Compiler> compiler;
        auto err = test_helpers::compile_program(c.input, &compiler, &parser_errors);

        ASSERT_TRUE(parser_errors.empty()) << c.name;
        ASSERT_NE(err, nullptr) << c.name;
        EXPECT_EQ(err->message, c.expected_error) << c.name;
    }
}

// [测试用例] 验证 NegativeGolden::RuntimeErrorsAreStable
TEST(NegativeGolden, RuntimeErrorsAreStable) {
    const std::vector<NegativeCase> cases = {
        {"wrong_number_of_args", "func(a) { a }(1, 2)", "Wrong Number of Arguments: want=1, got=2"},
        {"unsupported_string_comparison", "\"a\" == \"a\"", "unsupported types for comparison: STRING STRING"},
    };

    for (const auto& c : cases) {
        auto [result, err] = test_helpers::eval_input(c.input);

        EXPECT_TRUE(result.empty()) << c.name;
        EXPECT_EQ(err, c.expected_error) << c.name;
    }
}
