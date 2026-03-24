#pragma once

#include <string>
#include "litecomp/token.hpp"

/**
 * @brief 词法分析器结构体
 * 负责将输入的源代码字符串转换为 Token 流。
 */
struct Lexer {
    // 禁止默认构造，必须提供输入源代码
    Lexer() = delete;

    /**
     * @brief 构造函数
     * @param input_in 待解析的源代码字符串
     */
    explicit Lexer(const std::string &input_in);

    // 禁止拷贝构造（词法分析器通常拥有唯一的扫描状态）
    Lexer(const Lexer &other) = delete;

    // 允许移动构造
    Lexer(Lexer &&other) noexcept;

    // 禁止拷贝赋值
    Lexer &operator=(const Lexer &other) = delete;

    // 禁止移动赋值
    Lexer &operator=(Lexer &&other) noexcept = delete;

    /**
     * @brief 移动扫描指针位置
     * 读取源代码中的下一个字符，并更新当前的 `ch` 以及位置索引 `curPos` 和 `nextPos`。
     */
    void move();

    /**
     * @brief 读取标识符
     * 当遇到字母时调用，持续读取直到遇到非字母/非数字字符。
     * 用于识别变量名（如 `x`）或关键字（如 `let`, `fn`）。
     * @return 识别到的标识符字符串
     */
    std::string readIdentifier();

    /**
     * @brief 读取整个数字
     * 持续读取数字字符直到遇到非数字。
     * @return 识别到的数字字符串（如 "123"）
     */
    std::string readWholeNumber();

    /**
     * @brief 忽略空白字符
     * 跳过源代码中的空格、制表符、换行符等不需要处理的字符。
     */
    void ignoreWhitespace();

    /**
     * @brief 窥探下一个字符（Lookahead）
     * 查看 `nextPos` 处的字符但不移动当前的扫描指针。
     * 常用于处理多字符操作符（例如判断是 `=` 还是 `==`）。
     * @return 下一个位置的字符
     */
    char eyeNextChar();

    /**
     * @brief 读取字符串字面量
     * 处理双引号包裹的内容（如 "hello world"）。
     * @return 引号内部的字符串内容
     */
    std::string readString();

    /**
     * @brief 核心函数：扫描并获取下一个 Token
     * 识别当前指针处的字符，生成对应的 Token 对象，并移动指针。
     * @return 解析出的 Token
     */
    Token scan();

private:
    std::string src;      // 源代码全文
    int curPos;           // 当前正在处理的字符位置（索引）
    int nextPos;          // 下一个将要读取的字符位置（用于窥探）
    char ch;              // 当前正在处理的字符（curPos 指向的字符）
};