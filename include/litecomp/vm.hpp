#pragma once

#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "litecomp/code.hpp"

// 前向声明
struct Bytecode;
struct Frame;
struct Object;
struct Error;
struct Boolean;

/**
 * 虚拟机硬编码限制
 */
constexpr int MAXFRAMES = 1024;     // 最大递归/嵌套调用深度（调用栈大小）
constexpr int STACKSIZE = 2048;     // 数据栈大小（用于存放中间计算结果和局部变量）
constexpr int GLOBALSSIZE = 65536;  // 全局变量池大小

/**
 * @brief 虚拟机 (VM) 结构体
 * 负责解析并运行字节码，管理程序状态。
 */
struct VM {
    VM() = delete;
    VM(const VM &other) = delete;
    VM(VM &&other) noexcept = delete;
    VM &operator=(const VM &other) = delete;
    VM &operator=(VM &&other) noexcept = delete;

    /**
     * @brief 构造函数：初始化虚拟机
     * @param bytecode 编译器生成的字节码（包含指令流和常量池）
     */
    explicit VM(std::shared_ptr<Bytecode> &&bytecode);

    /**
     * @brief 构造函数：带初始全局变量状态
     * 用于在多次执行或 REPL 环境中保留全局变量。
     */
    explicit VM(std::shared_ptr<Bytecode> &&bytecode, std::array<std::shared_ptr<Object>, GLOBALSSIZE> s);

    // --- 调用栈 (Call Stack) ---
    std::array<std::shared_ptr<Frame>, MAXFRAMES> frames; // 存放所有的栈帧
    int frames_index;                                     // 指向当前正在执行的栈帧（调用深度计数）

    // --- 常量与数据栈 (Operand Stack) ---
    std::vector<std::shared_ptr<Object>> constants;       // 常量池（只读）
    std::array<std::shared_ptr<Object>, STACKSIZE> stack; // 操作数栈，用于计算和传递参数

    // 栈指针 (Stack Pointer)，指向栈顶空闲位置
    int sp; 

    // --- 全局变量池 ---
    std::array<std::shared_ptr<Object>, GLOBALSSIZE> globals;

    // --- 栈帧管理方法 ---
    std::shared_ptr<Frame> current_frame();                      // 获取当前正在运行的帧
    std::shared_ptr<Error> push_frame(std::shared_ptr<Frame> f); // 压入新帧（进入函数）
    std::shared_ptr<Frame> pop_frame();                          // 弹出当前帧（函数返回）

    // --- 数据栈操作方法 ---
    std::shared_ptr<Error> push(std::shared_ptr<Object> o); // 压入数据
    std::shared_ptr<Object> pop();                          // 弹出数据
    std::shared_ptr<Object> last_popped_stack_elem();       // 获取刚刚弹出的最后一个元素（用于测试/调试）

    // --- 函数调用相关 ---
    std::shared_ptr<Error> execute_call(int num_args);                                  // 执行调用逻辑（分发给闭包或内置函数）
    std::shared_ptr<Error> call_closure(std::shared_ptr<Object> cl, int num_args);      // 调用闭包（自定义函数）
    std::shared_ptr<Error> push_closure(int const_index, int num_free);                 // 实例化闭包，捕获自由变量
    std::shared_ptr<Error> call_builtin(std::shared_ptr<Object> builtin, int num_args); // 调用 C++ 实现的内置函数

    // --- 复合对象构建 ---
    std::shared_ptr<Object> build_array(int start_index, int end_index);                                    // 从栈中提取元素构建数组
    std::tuple<std::shared_ptr<Object>, std::shared_ptr<Error>> build_hash(int start_index, int end_index); // 构建哈希表

    // --- 指令执行分支 (Operations) ---
    std::shared_ptr<Error> execute_binary_operation(OpType op); // 执行二元算术运算 (+ - * /)
    std::shared_ptr<Error> execute_comparison(OpType op);       // 执行比较运算 (== != > <)

    // 具体的整数/字符串操作实现
    std::shared_ptr<Error> execute_integer_comparison(OpType op, std::shared_ptr<Object> left, std::shared_ptr<Object> right);
    std::shared_ptr<Error> execute_binary_integer_operation(OpType op, std::shared_ptr<Object> left, std::shared_ptr<Object> right);
    std::shared_ptr<Error> execute_binary_string_operation(OpType op, std::shared_ptr<Object> left, std::shared_ptr<Object> right);

    // 前缀运算实现
    std::shared_ptr<Error> execute_bang_operator();  // 执行逻辑非 (!)
    std::shared_ptr<Error> execute_minus_operator(); // 执行数值取负 (-)

    // 索引访问相关 (Expression[index])
    std::shared_ptr<Error> execute_index_expression(std::shared_ptr<Object> left, std::shared_ptr<Object> index);
    std::shared_ptr<Error> execute_array_index(std::shared_ptr<Object> array, std::shared_ptr<Object> index);
    std::shared_ptr<Error> execute_hash_index(std::shared_ptr<Object> hash, std::shared_ptr<Object> index);

    /**
     * @brief 虚拟机核心循环
     * 逐条读取字节码指令、解码并执行，直到代码结束或报错。
     * @return 运行过程中产生的错误，运行成功返回 nullptr
     */
    std::shared_ptr<Error> run();
};

/**
 * @brief 工具函数：将 C++ bool 转换为运行时的 Boolean 对象
 */
std::shared_ptr<Boolean> native_bool_to_boolean_object(bool input);

/**
 * @brief 工具函数：判断一个运行时的对象是否为“真”值
 * 遵循语言的真值规则（如：0、null、false 为假，其余为真）
 */
bool is_truthy(std::shared_ptr<Object> obj);