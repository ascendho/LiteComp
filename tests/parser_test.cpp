#include <gtest/gtest.h>

#include <vector>

#include "litecomp/parser.hpp"

TEST(ParserModule, ParseExpressionPrecedence) {
    Lexer lexer("1 + 2 * 3");
    Parser parser(std::move(lexer));
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.errors().empty());
    ASSERT_EQ(program->statements.size(), 1u);
    EXPECT_EQ(program->string(), "(1 + (2 * 3))");
}

TEST(ParserModule, ReportsMissingPrefixParseFunction) {
    Lexer lexer(";");
    Parser parser(std::move(lexer));
    (void)parser.parseProgram();

    auto errs = parser.errors();
    ASSERT_FALSE(errs.empty());
    EXPECT_NE(errs.front().find("No Bounded Parse Function Found"), std::string::npos);
}
