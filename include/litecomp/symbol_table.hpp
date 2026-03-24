#pragma once

#include <iomanip>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief 符号作用域枚举
 * 标识一个变量是在哪里定义的，这直接决定了虚拟机使用哪条指令（如 OpGetGlobal vs OpGetLocal）
 */
enum class SymbolScope {
    LocalScope,     // 局部作用域（函数内部）
    GlobalScope,    // 全局作用域（最外层）
    BuiltinScope,   // 内置作用域（预定义的系统函数）
    FreeScope,      // 自由变量作用域（闭包捕获的外部局部变量）
    FunctionScope   // 函数名作用域（用于函数内部引用自身，实现递归）
};

/**
 * @brief 符号结构体
 * 记录一个标识符的元数据
 */
struct Symbol {
    std::string name;   // 变量名
    SymbolScope scope;  // 作用域类型
    int index;          // 在该作用域下的索引偏移量（例如第几个局部变量）

    // 比较运算符，用于单元测试或逻辑判定
    bool operator==(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;
};

// 重载输出流运算符，方便打印符号信息（调试用）
std::ostream& operator<<(std::ostream& out, const Symbol& sym);

/**
 * @brief 符号表类
 * 编译器在遍历 AST 时，通过符号表来管理标识符的声明和查找。
 * 支持嵌套（链式结构），实现词法作用域。
 */
struct SymbolTable {
    SymbolTable() = default;

    // 符号表通常随编译深度创建，禁止随意拷贝
    SymbolTable(const SymbolTable& other) = delete;
    SymbolTable(SymbolTable&& other) noexcept = default;
    SymbolTable& operator=(const SymbolTable& other) = delete;
    SymbolTable& operator=(SymbolTable&& other) noexcept = delete;

    std::shared_ptr<SymbolTable> outer;  // 指向父级符号表（外部作用域）

    std::map<std::string, Symbol> store; // 当前作用域内的符号映射表

    int num_definitions;                 // 当前符号表已定义的符号数量（用于分配下一个索引）

    std::vector<Symbol> free_symbols;    // 当前作用域捕获的自由变量列表（闭包使用）

    /**
     * @brief 定义一个新符号
     * 根据当前是否有父表，自动判定为 Global 或 Local 作用域。
     */
    Symbol define(std::string name);

    /**
     * @brief 定义一个自由变量
     * 用于闭包：将外部作用域的符号“提升”到当前作用域的自由变量列表中。
     */
    Symbol define_free(Symbol original);

    /**
     * @brief 定义内置函数符号
     */
    Symbol define_builtin(int index, std::string name);

    /**
     * @brief 定义函数自身的名称
     * 用于支持函数内部通过名称调用自己（递归），而不会污染外部作用域。
     */
    Symbol define_function_name(std::string name);

    /**
     * @brief 解析/查找符号
     * 从当前作用域开始查找，如果没有找到则递归向父级作用域查找。
     * @return tuple<符号, 是否找到>
     */
    std::tuple<Symbol, bool> resolve(const std::string& name);
};

// 创建一个新的顶层符号表（全局作用域）
std::shared_ptr<SymbolTable> new_symbol_table();

// 创建一个嵌套的符号表（如进入一个函数时创建）
std::shared_ptr<SymbolTable> new_enclosed_symbol_table(std::shared_ptr<SymbolTable> outer);