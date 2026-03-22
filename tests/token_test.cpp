#include <gtest/gtest.h>

#include "litecomp/token.hpp"

TEST(TokenModule, SearchIdentifierAndKeyword) {
    EXPECT_EQ(searchIdentifier("func"), TokenType::FUNCTION);
    EXPECT_EQ(searchIdentifier("declare"), TokenType::DECLARE);
    EXPECT_EQ(searchIdentifier("my_var"), TokenType::IDENTIFIER);
}

TEST(TokenModule, SearchToken) {
    EXPECT_EQ(searchToken(TokenType::PLUS), "+");
    EXPECT_EQ(searchToken(TokenType::ENDOFFILE), "EOF");
    EXPECT_EQ(searchToken(static_cast<TokenType>(255)), "TOKEN_NOT_DEFINED");
}

TEST(TokenModule, TokenCopyMoveAndEquality) {
    Token a{TokenType::INTEGER, "123"};
    Token b{a};
    EXPECT_EQ(a, b);

    Token c{TokenType::ILLEGAL, ""};
    c = std::move(b);
    EXPECT_EQ(c.type, TokenType::INTEGER);
    EXPECT_EQ(c.literal, "123");
}
