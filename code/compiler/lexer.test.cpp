#define CATCH_CONFIG_MAIN

#include "../include/catch.hpp"

#include "lexer.hpp"

TEST_CASE("Test Next Token") {
    // 测试代码，尽可能将每一个词法单元都测试一遍
    std::string input = R"(declare five = 5;
declare ten = 10;

declare add = func(x, y) {
  x + y;
};

    declare result = add(five, ten);
    !-/*5;
5 < 10 > 5;

if (5 < 10) {
    return true;
} else {
    return false;
}

10 == 10;
10 != 9;
"foobar"
"foo bar"
[1, 2];
{"foo": "bar"}

)";


    auto testInput = Lexer(input);

    // 测试断言
    Token tests[] = {
            Token{TokenType::DECLARE, "declare"},
            Token{TokenType::IDENTIFIER, "five"},
            Token{TokenType::ASSIGN, "="},
            Token{TokenType::INTEGER, "5"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::DECLARE, "declare"},
            Token{TokenType::IDENTIFIER, "ten"},
            Token{TokenType::ASSIGN, "="},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::DECLARE, "declare"},
            Token{TokenType::IDENTIFIER, "add"},
            Token{TokenType::ASSIGN, "="},
            Token{TokenType::FUNCTION, "func"},
            Token{TokenType::LBRACKET, "("},
            Token{TokenType::IDENTIFIER, "x"},
            Token{TokenType::COMMA, ","},
            Token{TokenType::IDENTIFIER, "y"},
            Token{TokenType::RBRACKET, ")"},
            Token{TokenType::LCBRACKET, "{"},
            Token{TokenType::IDENTIFIER, "x"},
            Token{TokenType::PLUS, "+"},
            Token{TokenType::IDENTIFIER, "y"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::RCBRACKET, "}"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::DECLARE, "declare"},
            Token{TokenType::IDENTIFIER, "result"},
            Token{TokenType::ASSIGN, "="},
            Token{TokenType::IDENTIFIER, "add"},
            Token{TokenType::LBRACKET, "("},
            Token{TokenType::IDENTIFIER, "five"},
            Token{TokenType::COMMA, ","},
            Token{TokenType::IDENTIFIER, "ten"},
            Token{TokenType::RBRACKET, ")"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::BANG, "!"},
            Token{TokenType::MINUS, "-"},
            Token{TokenType::SLASH, "/"},
            Token{TokenType::ASTERISK, "*"},
            Token{TokenType::INTEGER, "5"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::INTEGER, "5"},
            Token{TokenType::LESS, "<"},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::GREAT, ">"},
            Token{TokenType::INTEGER, "5"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::IF, "if"},
            Token{TokenType::LBRACKET, "("},
            Token{TokenType::INTEGER, "5"},
            Token{TokenType::LESS, "<"},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::RBRACKET, ")"},
            Token{TokenType::LCBRACKET, "{"},
            Token{TokenType::RETURN, "return"},
            Token{TokenType::TRUE, "true"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::RCBRACKET, "}"},
            Token{TokenType::ELSE, "else"},
            Token{TokenType::LCBRACKET, "{"},
            Token{TokenType::RETURN, "return"},
            Token{TokenType::FALSE, "false"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::RCBRACKET, "}"},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::EQUAL, "=="},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::INTEGER, "10"},
            Token{TokenType::NOTEQUAL, "!="},
            Token{TokenType::INTEGER, "9"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::STRING, "foobar"},
            Token{TokenType::STRING, "foo bar"},
            Token{TokenType::LSBRACKET, "["},
            Token{TokenType::INTEGER, "1"},
            Token{TokenType::COMMA, ","},
            Token{TokenType::INTEGER, "2"},
            Token{TokenType::RSBRACKET, "]"},
            Token{TokenType::SEMICOLON, ";"},
            Token{TokenType::LCBRACKET, "{"},
            Token{TokenType::STRING, "foo"},
            Token{TokenType::COLON, ":"},
            Token{TokenType::STRING, "bar"},
            Token{TokenType::RCBRACKET, "}"},
            Token{TokenType::ENDOFFILE, ""},
    };

    // 循环遍历测试
    for (const Token &t: tests) {
        auto tok = testInput.scan();

        if (!(tok == t)) {
            std::cerr << "The type of token is wrong. expected='" << searchToken(t.type) << "(" << t.literal
                      << ")', got='"
                      << searchToken(tok.type) << "(" << tok.literal << ")'" << std::endl;
        }

        REQUIRE(tok == t);
    }
}