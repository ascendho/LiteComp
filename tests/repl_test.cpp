#include <gtest/gtest.h>

#include "litecomp/repl.hpp"

TEST(ReplModule, PromptConstantsAreDefined) {
    EXPECT_FALSE(PROMPT.empty());
    EXPECT_FALSE(ERRORSIGN.empty());
    EXPECT_EQ(PROMPT, ">> ");
}
