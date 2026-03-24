// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include "litecomp/repl.hpp"

// [测试用例] 验证 ReplModule::PromptConstantsAreDefined
TEST(ReplModule, PromptConstantsAreDefined) {
    EXPECT_FALSE(PROMPT.empty());
    EXPECT_FALSE(ERRORSIGN.empty());
    EXPECT_EQ(PROMPT, ">> ");
}
