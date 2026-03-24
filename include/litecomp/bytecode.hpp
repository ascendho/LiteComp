#pragma once

#include <memory>
#include <vector>

#include "litecomp/code.hpp"
#include "litecomp/object.hpp"

/**
 * @brief 字节码结构体
 * 该结构体封装了编译器输出的最终结果，供虚拟机（VM）直接运行。
 */
struct Bytecode {
    // 默认构造函数、拷贝构造、移动构造及其赋值运算符
    Bytecode() = default;
    Bytecode(const Bytecode &other) = default;
    Bytecode(Bytecode &&other) noexcept = default;
    Bytecode &operator=(const Bytecode &other) = default;
    Bytecode &operator=(Bytecode &&other) noexcept = default;

    /**
     * @brief 指令序列 (Instructions)
     * 存储编译后的二进制操作码及其操作数。
     * 通常是一个 vector<uint8_t>，虚拟机通过遍历这个序列来执行程序逻辑。
     */
    Instructions instructions;

    /**
     * @brief 常量池 (Constant Pool)
     * 存储程序中使用的字面量（如整数、字符串、已编译的函数等）。
     * 在指令序列中，类似 OpConstant 的指令会通过索引来引用这个 vector 中的对象。
     * 这种设计可以避免在指令流中嵌入变长数据，提高执行效率。
     */
    std::vector<std::shared_ptr<Object>> constants;
};