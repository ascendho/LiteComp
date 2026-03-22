#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum class TokenType {
    // 非法字符
    ILLEGAL,

    // 文件末尾
    ENDOFFILE,

    // 标识符
    IDENTIFIER,

    // 整型和字符串
    INTEGER,
    STRING,

    // 运算符
    ASSIGN,
    PLUS,
    MINUS,
    BANG,
    ASTERISK,
    SLASH,

    LESS,
    GREAT,
    EQUAL,
    NOTEQUAL,

    // 分隔符
    SEMICOLON,
    COLON,
    COMMA,

    LBRACKET,
    RBRACKET,
    LCBRACKET,
    RCBRACKET,
    LSBRACKET,
    RSBRACKET,

    // 关键字
    FUNCTION,
    DECLARE,
    TRUE,
    FALSE,
    IF,
    ELSE,
    RETURN
};

// 搜索词法单元
std::string searchToken(TokenType t);

// 搜索标识符
TokenType searchIdentifier(const std::string &ident);

struct Token {
    Token() = default;

    Token(const TokenType &t, std::string l);

    Token(const Token &other);

    Token(Token &&other) noexcept;

    Token &operator=(const Token &other);

    Token &operator=(Token &&other) noexcept;

    bool operator==(const Token &) const;

    bool operator!=(const Token &) const;

    TokenType type;

    std::string literal;
};

#endif