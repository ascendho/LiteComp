#pragma once

#include <string>

/**
 * @brief 词法单元类型枚举
 * 涵盖了语言支持的所有符号、关键字和字面量类型
 */
enum class TokenType {
    // --- 特殊状态 ---
    ILLEGAL,    // 非法字符（识别失败）
    ENDOFFILE,  // 文件末尾 (EOF)

    // --- 标识符与字面量 ---
    IDENTIFIER, // 标识符 (变量名, 函数名，如: x, add)
    INTEGER,    // 整型字面量 (如: 5, 123)
    STRING,     // 字符串字面量 (如: "hello")

    // --- 运算符 ---
    ASSIGN,     // =
    PLUS,       // +
    MINUS,      // -
    BANG,       // !
    ASTERISK,   // *
    SLASH,      // /

    // --- 比较运算符 ---
    LESS,       // <
    GREAT,      // >
    EQUAL,      // ==
    NOTEQUAL,   // !=

    // --- 分隔符与标点 ---
    SEMICOLON,  // ; (语句结束)
    COLON,      // : (哈希键值对或类型标注)
    COMMA,      // , (参数分隔)

    // --- 括号 ---
    LBRACKET,   // (  (左圆括号)
    RBRACKET,   // )  (右圆括号)
    LCBRACKET,  // {  (左大括号 - 用于代码块或哈希)
    RCBRACKET,  // }  (右大括号)
    LSBRACKET,  // [  (左中括号 - 用于数组或索引)
    RSBRACKET,  // ]  (右中括号)

    // --- 关键字 (Keywords) ---
    FUNCTION,   // fn 或 function
    DECLARE,    // let 或 var (变量声明)
    TRUE,       // true
    FALSE,      // false
    IF,         // if
    ELSE,       // else
    RETURN      // return
};

/**
 * @brief 将 TokenType 枚举值转换为易读的字符串名称
 * 主要用于调试打印和错误信息输出 (例如: 将 TokenType::PLUS 转为 "PLUS")
 */
std::string searchToken(TokenType t);

/**
 * @brief 根据字符串判断其是否为关键字
 * 如果字符串是 "fn"，返回 TokenType::FUNCTION；
 * 如果是普通单词 "apple"，则返回 TokenType::IDENTIFIER。
 */
TokenType searchIdentifier(const std::string &ident);

/**
 * @brief 词法单元结构体
 * 封装了一个 Token 的类型及其在源代码中的原始字面量
 */
struct Token {
    Token() = default;

    /**
     * @brief 构造函数
     * @param t Token 类型
     * @param l 原始字符串内容
     */
    Token(const TokenType &t, std::string l);

    // --- 拷贝与移动语义 (保证资源管理正确) ---
    Token(const Token &other);
    Token(Token &&other) noexcept;
    Token &operator=(const Token &other);
    Token &operator=(Token &&other) noexcept;

    // --- 比较运算符 ---
    bool operator==(const Token &) const;
    bool operator!=(const Token &) const;

    TokenType type;      // 词法单元的类别
    std::string literal; // 词法单元在源代码中的原始文本内容 (例如 "5", "==")
};