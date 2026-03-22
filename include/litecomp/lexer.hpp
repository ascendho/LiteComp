#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include "litecomp/token.hpp"

struct Lexer {
    Lexer() = delete;

    explicit Lexer(const std::string &input_in);

    Lexer(const Lexer &other) = delete;

    Lexer(Lexer &&other) noexcept;

    Lexer &operator=(const Lexer &other) = delete;

    Lexer &operator=(Lexer &&other) noexcept = delete;

    // 移动扫描指针位置
    void move();

    // 读取标识符
    std::string readIdentifier();

    // 读取整个数字
    std::string readWholeNumber();

    // 忽略空白字符
    void ignoreWhitespace();

    // 窥探下一个字符
    char eyeNextChar();

    // 读取字符串
    std::string readString();

    // 核心函数：扫描
    Token scan();

private:
    std::string src;      // 源代码
    int curPos;           // 当前token位置
    int nextPos;          // 下一个token位置
    char ch;              // 当前扫描到的字符
};

#endif