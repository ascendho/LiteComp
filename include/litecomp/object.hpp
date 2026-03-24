#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "litecomp/code.hpp"

// 前向声明
struct Identifier;
struct BlockStatement;

/**
 * @brief 对象类型枚举
 * 定义了语言中所有支持的数据类型
 */
enum class ObjectType {
    INTEGER_OBJ,           // 整数
    BOOLEAN_OBJ,           // 布尔值
    NULL_OBJ,              // 空值
    RETURN_VALUE_OBJ,      // 返回值包装器（用于内部控制流）
    ERROR_OBJ,             // 错误对象
    FUNCTION_OBJ,          // AST 形式的函数（用于解释器）
    STRING_OBJ,            // 字符串
    BUILTIN_OBJ,           // 内置函数（由 C++ 实现）
    ARRAY_OBJ,             // 数组
    HASH_OBJ,              // 哈希表（字典）
    COMPILED_FUNCTION_OBJ, // 已编译的函数（用于虚拟机）
    CLOSURE_OBJ            // 闭包对象
};

// 获取对象类型的字符串字面量（用于调试）
std::string objecttype_literal(ObjectType);

/**
 * @brief 运行时的基础对象接口
 * 所有在语言执行过程中产生的值（Value）都必须继承自该接口。
 */
struct Object {
    virtual ~Object() = default;

    // 返回对象的类型
    virtual ObjectType type() const = 0;

    // 返回对象的字符串表示（用于打印输出）
    virtual std::string inspect() const = 0;

    // 实现对象的深拷贝
    virtual std::shared_ptr<Object> clone() const = 0;
};

/**
 * @brief 环境/作用域 (Environment)
 * 记录变量名与对象之间的映射关系，支持嵌套作用域（外部作用域 outer）。
 */
struct Environment {
    Environment() = default;
    Environment(const Environment& other);
    Environment(Environment&& other) noexcept;
    Environment& operator=(const Environment& other);
    Environment& operator=(Environment&& other) noexcept;

    std::map<std::string, std::shared_ptr<Object>> store; // 存储当前作用域的变量
    std::shared_ptr<Environment> outer;                   // 指向父级作用域（实现闭包的关键）

    // 获取变量
    std::tuple<std::shared_ptr<Object>, bool> get(const std::string &name);
    // 设置变量
    std::shared_ptr<Object> set(std::string name, std::shared_ptr<Object> val);
    // 克隆环境
    std::shared_ptr<Environment> clone() const;
};

// 创建新环境的辅助函数
std::shared_ptr<Environment> new_environment();
// 创建一个嵌套在 outer 之内的子环境
std::shared_ptr<Environment> new_enclosed_environment(const Environment &outer);

// =============================================================================
// 哈希支持 (用于将对象作为哈希表的键)
// =============================================================================

/**
 * @brief 哈希键
 * 只有支持哈希的对象（如整型、布尔、字符串）才能生成 HashKey。
 */
struct HashKey {
    HashKey(ObjectType t, uint64_t v);
    ObjectType type;

    // 实际的哈希值
    uint64_t value; 

    bool operator==(const HashKey &other) const;
    bool operator!=(const HashKey &other) const;

    // 用于 std::map 的排序
    bool operator<(const HashKey &other) const; 
};

// 哈希表中的键值对存储结构
struct HashPair {
    std::shared_ptr<Object> key;
    std::shared_ptr<Object> value;
};

// 可哈希接口：只有继承自此接口的对象才能作为哈希表的 Key
struct Hashable {
    virtual ~Hashable() = default;
    virtual HashKey hash_key() const = 0;
};

// 哈希对象实现
struct Hash : public Object {
    Hash(std::map<HashKey, HashPair> p);
    // ... 构造函数与赋值运算符
    std::map<HashKey, HashPair> pairs;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

// =============================================================================
// 具体对象类型实现
// =============================================================================

// 内置函数类型定义（接收参数列表，返回一个对象）
typedef std::function<std::shared_ptr<Object>(std::vector<std::shared_ptr<Object>>)> builtin_fn;

/**
 * @brief 解释器模式下的函数对象
 */
struct Function : public Object {
    Function(std::vector<std::shared_ptr<Identifier>> p, std::shared_ptr<BlockStatement> b, const std::shared_ptr<Environment> &e);
    // ...
    std::vector<std::shared_ptr<Identifier>> parameters; // 参数名列表
    std::shared_ptr<BlockStatement> body;                // 函数体代码块
    std::shared_ptr<Environment> env;                    // 函数定义时的环境（闭包环境）

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 内置函数对象 (如 len(), print())
 */
struct Builtin : public Object {
    explicit Builtin(builtin_fn v);
    builtin_fn builtin_function;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 数组对象
 */
struct Array : public Object {
    explicit Array();
    std::vector<std::shared_ptr<Object>> elements;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 返回值对象
 * 用于在解释执行 AST 时，信号化 return 语句，确保能跳出多层嵌套调用。
 */
struct ReturnValue : public Object {
    explicit ReturnValue(std::shared_ptr<Object> v);
    std::shared_ptr<Object> value;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 错误对象
 */
struct Error : public Object {
    explicit Error(std::string v);
    std::string message;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 整数对象 (支持哈希)
 */
struct Integer : public Object, Hashable {
    explicit Integer(int v);
    int value;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
    HashKey hash_key() const override;
};

/**
 * @brief 布尔对象 (支持哈希)
 */
struct Boolean : public Object, Hashable {
    explicit Boolean(bool v);
    bool value;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
    HashKey hash_key() const override;
};

// 获取全局唯一的 True/False 对象引用
std::shared_ptr<Boolean> get_true_ref();
std::shared_ptr<Boolean> get_false_ref();

/**
 * @brief 字符串对象 (支持哈希)
 */
struct String : public Object, Hashable {
    explicit String(std::string v);
    std::string value;

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
    HashKey hash_key() const override;
};

/**
 * @brief 空值对象 (Null)
 */
struct Null : public Object {
    Null() = default;
    ObjectType type() const override;
    std::shared_ptr<Object> clone() const override;
    std::string inspect() const override;
};

/**
 * @brief 已编译的函数对象
 * 包含虚拟机执行所需的指令序列和元数据（局部变量数、参数个数）。
 */
struct CompiledFunction : public Object {
    CompiledFunction(const Instructions& instructions);
    // ...
    Instructions instructions; // 指令字节码
    int num_locals;            // 局部变量数量
    int num_parameters;        // 参数个数

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

/**
 * @brief 闭包对象
 * 将已编译的函数与其所捕获的自由变量（Free Variables）绑定在一起。
 */
struct Closure : public Object {
    Closure() = default;
    Closure(std::shared_ptr<CompiledFunction> fn);
    // ...
    std::shared_ptr<CompiledFunction> fn;         // 对应的函数代码
    std::vector<std::shared_ptr<Object>> free;    // 捕获的自由变量列表

    ObjectType type() const override;
    std::string inspect() const override;
    std::shared_ptr<Object> clone() const override;
};

// 获取全局唯一的 Null 对象引用
std::shared_ptr<Null> get_null_ref();

// 创建错误对象的辅助函数
std::shared_ptr<Error> new_error(std::string message);

// 检查一个对象是否为错误类型
bool is_error(const std::shared_ptr<Object>& obj);