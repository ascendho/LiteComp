#pragma once

#include <memory>
#include "litecomp/code.hpp"

// 前向声明
struct Closure;

/**
 * @brief 栈帧结构体 (Stack Frame)
 * 栈帧代表了当前正在执行的一个函数调用上下文。
 */
struct Frame {
    // 禁止默认构造，因为每个栈帧必须关联具体的闭包和指针
    Frame() = delete;

    // 默认的拷贝与移动语义
    Frame(const Frame& other) = default;
    Frame(Frame&& other) noexcept = default;
    Frame& operator=(const Frame& other) = default;
    Frame& operator=(Frame&& other) noexcept = default;

    /**
     * @brief 构造函数
     * @param cl 指向当前正在执行的闭包 (Closure)
     * @param ip 初始指令指针 (Instruction Pointer)
     * @param base_pointer 栈基址指针
     */
    Frame(std::shared_ptr<Closure> cl, int ip, int base_pointer);

    /**
     * @brief 当前执行的闭包
     * 闭包中包含了编译后的函数字节码以及该函数捕获的自由变量。
     */
    std::shared_ptr<Closure> cl;

    /**
     * @brief 指令指针 (Instruction Pointer)
     * 记录该函数当前执行到了字节码的哪一个位置（偏移量）。
     */
    int ip;

    /**
     * @brief 栈基址指针 (Base Pointer)
     * 指向该函数在虚拟机全局栈（Stack）中的起始位置。
     * 局部变量的索引（如 OpGetLocal 0）是相对于这个 base_pointer 来计算的。
     */
    int base_pointer;

    /**
     * @brief 获取当前帧对应的指令集
     * 这是一个辅助方法，直接从闭包关联的已编译函数中提取字节码序列。
     * @return 字节码指令引用
     */
    const Instructions& instructions() const;
};

/**
 * @brief 创建新栈帧的辅助函数
 * @param cl 要执行的闭包
 * @param base_pointer 该闭包在虚拟机栈中的起始位置
 * @return 栈帧的智能指针
 */
std::shared_ptr<Frame> new_frame(std::shared_ptr<Closure> cl, int base_pointer);