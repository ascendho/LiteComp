#pragma once

#include <memory>
#include <vector>

#include "litecomp/code.hpp"
#include "litecomp/symbol_table.hpp"

// 前向声明，减少头文件依赖
struct Node;
struct Object;
struct Error;
struct Bytecode;

/**
 * @brief 已发出的指令记录
 * 用于记录最近一次写入字节码序列的指令及其位置，便于进行指令优化（如移除多余的 Pop）。
 */
struct EmittedInstruction {
    OpType opcode; // 操作码类型
    int position;  // 该指令在当前字节码数组中的起始偏移位置
};

/**
 * @brief 编译作用域 (Compilation Scope)
 * 编译器为每个函数（包括全局作用域）维护一个独立的 Scope。
 * 这样在编译函数内部代码时，生成的指令会存储在当前 Scope 中，而不会污染全局指令流。
 */
struct CompilationScope {
    Instructions instructions;               // 当前作用域生成的字节码序列
    EmittedInstruction last_instruction;     // 最后一条指令记录
    EmittedInstruction previous_instruction; // 倒数第二条指令记录
};

/**
 * @brief 编译器结构体
 * 负责将 AST 转换为虚拟机可执行的字节码。
 */
struct Compiler {
    Compiler() = default;
    Compiler(const Compiler &other) = default;
    Compiler(Compiler &&other) noexcept = default;
    Compiler &operator=(const Compiler &other) = default;
    Compiler &operator=(Compiler &&other) noexcept = default;

    /**
     * @brief 全局常量池
     * 存储程序中所有的字面量（如数字、字符串、已编译函数对象）。
     */
    std::vector<std::shared_ptr<Object>> constants;

    /**
     * @brief 符号表
     * 维护变量名到索引和作用域的映射。
     */
    std::shared_ptr<SymbolTable> symbol_table;

    /**
     * @brief 作用域栈
     * 每当编译一个函数时，会 push 一个新 Scope；编译结束时 pop。
     */
    std::vector<CompilationScope> scopes;

    /**
     * @brief 当前作用域索引
     * 指向 scopes 向量中当前正在操作的范围（通常是末尾）。
     */
    int scope_index;

    /**
     * @brief 核心编译函数
     * 根据 AST 节点的类型（如 IfExpression, InfixExpression 等），递归生成字节码。
     * @return 如果编译过程出错，返回 Error 对象，否则返回 nullptr。
     */
    std::shared_ptr<Error> compile(std::shared_ptr<Node> node);

    /**
     * @brief 向常量池添加对象
     * @return 返回该对象在常量池中的索引位置。
     */
    int add_constant(std::shared_ptr<Object> obj);

    // =========================================================================
    // 指令发射 (Emitting)
    // =========================================================================

    // 发射无操作数的指令（如 OpAdd, OpPop）
    int emit(OpType op);

    // 发射带一个操作数的指令（如 OpConstant, OpJump）
    int emit(OpType op, int operand);

    // 发射带两个操作数的指令（如 OpClosure）
    int emit(OpType op, int first_operand, int second_operand);

    // 将原始字节码追加到当前作用域的指令流中
    int add_instruction(Instructions ins);

    // =========================================================================
    // 指令优化与回填 (Optimization & Back-patching)
    // =========================================================================

    // 更新当前作用域的“最后指令”记录
    void set_last_instruction(OpType op, int pos);

    // 检查最后一条指令是否为指定类型
    bool last_instruction_is(OpType op) const;

    // 移除最后一条 Pop 指令
    // 场景：在表达式语句末尾通常会加 Pop，但如果该表达式是函数的最后一行，则需要把值留给 Return。
    void remove_last_pop();

    // 将最后一条 Pop 替换为 Return 指令
    void replace_last_pop_with_return();

    // 在指定位置替换指令（常用于回填跳转地址）
    void replace_instruction(int pos, Instructions new_instruction);

    // 修改已发射指令的操作数（用于回填跳转偏移量）
    void change_operand(int op_pos, int operand);

    // =========================================================================
    // 作用域管理
    // =========================================================================

    // 进入新作用域（如开始编译函数体）
    void enter_scope();

    // 离开当前作用域并返回其中的所有指令
    Instructions leave_scope();

    /**
     * @brief 加载符号
     * 根据符号的作用域（Global, Local, Builtin, Free），生成对应的获取指令。
     */
    void load_symbol(Symbol s);

    /**
     * @brief 打包编译结果
     * 将生成的指令流和常量池封装成虚拟机可读取的 Bytecode 对象。
     */
    std::shared_ptr<Bytecode> bytecode();
};

// 创建一个全新的编译器实例
std::shared_ptr<Compiler> newCompiler();

// 使用已有状态（符号表和常量池）创建编译器（通常用于 REPL 或增量编译）
std::shared_ptr<Compiler> new_compiler_with_state(
        std::shared_ptr<SymbolTable> s, std::vector<std::shared_ptr<Object>> constants);