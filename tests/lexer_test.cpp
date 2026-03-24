// =============================================================================
// Test module
// This file validates behavior and guards against regressions for LiteComp.
// =============================================================================
#include <gtest/gtest.h>

#include <vector>

#include "litecomp/lexer.hpp"

// [测试用例] 验证 LexerModule::ScanBasicProgram
TEST(LexerModule, ScanBasicProgram) {
    const std::string input = R"(declare x = 5; if (x < 10) { x } else { 0 })";
    Lexer lexer(input);

    std::vector<TokenType> expected = {
        TokenType::DECLARE, TokenType::IDENTIFIER, TokenType::ASSIGN, TokenType::INTEGER,
        TokenType::SEMICOLON, TokenType::IF,         TokenType::LBRACKET, TokenType::IDENTIFIER,
        TokenType::LESS,    TokenType::INTEGER,      TokenType::RBRACKET, TokenType::LCBRACKET,
        TokenType::IDENTIFIER, TokenType::RCBRACKET, TokenType::ELSE, TokenType::LCBRACKET,
        TokenType::INTEGER, TokenType::RCBRACKET, TokenType::ENDOFFILE};

    for (TokenType t : expected) {
        Token tok = lexer.scan();
        EXPECT_EQ(tok.type, t);
    }
}

// [测试用例] 验证 LexerModule::ScanStringLiteral
TEST(LexerModule, ScanStringLiteral) {
    Lexer lexer("\"hello\"");
    Token tok = lexer.scan();
    EXPECT_EQ(tok.type, TokenType::STRING);
    EXPECT_EQ(tok.literal, "hello");
}
