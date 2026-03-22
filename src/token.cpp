#include "litecomp/token.hpp"

#include <map>

namespace
{

    // 关键字哈希表
    std::map<std::string, TokenType> keywords = {
        {"func", TokenType::FUNCTION},
        {"declare", TokenType::DECLARE},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"return", TokenType::RETURN}};

    // 词法单元哈希表
    std::map<TokenType, std::string> tokenLiterals = {
        {TokenType::ILLEGAL, "ILLEGAL"},
        {TokenType::ENDOFFILE, "EOF"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::INTEGER, "INTEGER"},
        {TokenType::STRING, "STRING"},
        {TokenType::ASSIGN, "="},
        {TokenType::PLUS, "+"},
        {TokenType::MINUS, "-"},
        {TokenType::BANG, "!"},
        {TokenType::ASTERISK, "*"},
        {TokenType::SLASH, "/"},
        {TokenType::LESS, "<"},
        {TokenType::GREAT, ">"},
        {TokenType::EQUAL, "=="},
        {TokenType::NOTEQUAL, "!="},
        {TokenType::SEMICOLON, ";"},
        {TokenType::COLON, ":"},
        {TokenType::COMMA, ","},
        {TokenType::LBRACKET, "("},
        {TokenType::RBRACKET, ")"},
        {TokenType::LCBRACKET, "{"},
        {TokenType::RCBRACKET, "}"},
        {TokenType::LSBRACKET, "["},
        {TokenType::RSBRACKET, "]"},
        {TokenType::FUNCTION, "FUNCTION"},
        {TokenType::DECLARE, "DECLARE"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::IF, "IF"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::RETURN, "RETURN"},
    };

} // namespace

TokenType searchIdentifier(const std::string &ident)
{
    auto contains = keywords.find(ident);

    // 用户自定义标识符
    if (contains == keywords.end())
    {
        return TokenType::IDENTIFIER;
    }

    return keywords[ident];
}

Token::Token(const TokenType &t, std::string l) : type{t}, literal{std::move(l)} {}

Token::Token(const Token &other) : type{other.type}, literal{std::string{other.literal}} {}

Token::Token(Token &&other) noexcept : type(other.type), literal(std::move(other.literal))
{
    other.type = TokenType::ILLEGAL;
    other.literal = "";
}

Token &Token::operator=(const Token &other)
{
    if (this == &other)
        return *this;

    type = other.type;
    literal = other.literal;

    return *this;
}

Token &Token::operator=(Token &&other) noexcept
{
    if (this == &other)
        return *this;

    type = other.type;
    literal = std::move(other.literal);

    other.type = TokenType::ILLEGAL;
    other.literal = "";

    return *this;
}

bool Token::operator==(const Token &t) const
{
    return type == t.type && literal == t.literal;
}

bool Token::operator!=(const Token &t) const
{
    return !(type == t.type && literal == t.literal);
}

std::string searchToken(TokenType t)
{
    auto contains = tokenLiterals.find(t);

    // 找不到对应的token
    if (contains == tokenLiterals.end())
    {
        return "TOKEN_NOT_DEFINED";
    }

    return tokenLiterals[t];
}