#include "litecomp/token.hpp"
#include <map>

// 匿名空间：这些映射表仅在当前文件内可见，用于辅助查找
namespace
{
    /**
     * @brief 关键字映射表
     * 将编程语言中的保留字（字符串）映射为对应的 TokenType。
     * 当词法分析器（Lexer）读取到一个单词时，会通过此表判断它是否为关键字。
     */
    std::map<std::string, TokenType> keywords = {
        {"func", TokenType::FUNCTION},     // 函数定义关键字
        {"declare", TokenType::DECLARE},   // 变量声明关键字
        {"true", TokenType::TRUE},         // 布尔真
        {"false", TokenType::FALSE},       // 布尔假
        {"if", TokenType::IF},             // 条件判断
        {"else", TokenType::ELSE},         // 条件分支
        {"return", TokenType::RETURN}      // 返回语句
    };

    /**
     * @brief 词法单元字符串映射表
     * 用于调试和错误输出。将枚举值转换为人类可读的字符串。
     */
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

}

/**
 * @brief 根据字符串查找对应的 TokenType
 * @param ident 被检测的单词
 * @return 如果在关键字表中找到，则返回关键字类型；否则返回 IDENTIFIER（标识符）。
 */
TokenType searchIdentifier(const std::string &ident)
{
    auto contains = keywords.find(ident);

    // 如果在关键字哈希表中没找到，说明这是一个用户自定义的标识符（如变量名 x）
    if (contains == keywords.end())
    {
        return TokenType::IDENTIFIER;
    }

    // 否则返回对应的关键字类型（如 TokenType::IF）
    return keywords[ident];
}

/**
 * @brief 构造函数：初始化 Token 类型和内容
 */
Token::Token(const TokenType &t, std::string l) : type{t}, literal{std::move(l)} {}

/**
 * @brief 拷贝构造函数
 */
Token::Token(const Token &other) : type{other.type}, literal{std::string{other.literal}} {}

/**
 * @brief 移动构造函数
 * 将资源从 other 转移到当前对象，并重置 other 的状态。
 */
Token::Token(Token &&other) noexcept : type(other.type), literal(std::move(other.literal))
{
    other.type = TokenType::ILLEGAL; // 标记原对象为非法
    other.literal = "";              // 清空原对象字面量
}

/**
 * @brief 拷贝赋值运算符
 */
Token &Token::operator=(const Token &other)
{
    if (this == &other)
        return *this;

    type = other.type;
    literal = other.literal;

    return *this;
}

/**
 * @brief 移动赋值运算符
 */
Token &Token::operator=(Token &&other) noexcept
{
    if (this == &other)
        return *this;

    type = other.type;
    literal = std::move(other.literal);

    // 重置被移动的对象
    other.type = TokenType::ILLEGAL;
    other.literal = "";

    return *this;
}

/**
 * @brief 相等判定：当类型和字面量都相同时认为 Token 相等
 */
bool Token::operator==(const Token &t) const
{
    return type == t.type && literal == t.literal;
}

/**
 * @brief 不等判定
 */
bool Token::operator!=(const Token &t) const
{
    return !(type == t.type && literal == t.literal);
}

/**
 * @brief 根据枚举类型查找其对应的易读名称
 * 主要用于打印解析错误，例如：“Expected '(', got 'INTEGER'”
 */
std::string searchToken(TokenType t)
{
    auto contains = tokenLiterals.find(t);

    // 如果枚举值未在表中定义，返回占位符
    if (contains == tokenLiterals.end())
    {
        return "TOKEN_NOT_DEFINED";
    }

    return tokenLiterals[t];
}