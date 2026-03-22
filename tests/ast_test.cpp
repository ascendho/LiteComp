#include <gtest/gtest.h>

#include <memory>

#include "litecomp/parser.hpp"

namespace {
std::shared_ptr<Program> parse(const std::string& input) {
    Lexer lexer(input);
    Parser parser(std::move(lexer));
    auto program = parser.parseProgram();
    if (!parser.errors().empty()) {
        return nullptr;
    }
    return program;
}
}  // namespace

TEST(AstModule, ProgramCopyAssignmentClearsOldStatements) {
    auto one_stmt = parse("1;");
    auto two_stmts = parse("1; 2;");
    ASSERT_NE(one_stmt, nullptr);
    ASSERT_NE(two_stmts, nullptr);

    Program target;
    target = *one_stmt;
    ASSERT_EQ(target.statements.size(), 1u);

    target = *two_stmts;
    EXPECT_EQ(target.statements.size(), 2u);
    EXPECT_EQ(target.string(), "12");
}

TEST(AstModule, IdentifierStringAndClone) {
    Token token{TokenType::IDENTIFIER, "abc"};
    Identifier id(token, "abc");

    EXPECT_EQ(id.string(), "abc");
    auto clone = std::dynamic_pointer_cast<Identifier>(id.clone());
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->value, "abc");
}
