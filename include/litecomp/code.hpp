#pragma once

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

/**
 * @brief 指令序列
 * 字节码实际上就是一个由 uint8_t（字节）组成的向量。
 */
typedef std::vector<uint8_t> Instructions;

/**
 * @brief 重载输出流运算符
 * 用于格式化打印指令序列，方便调试（反汇编）。
 */
std::ostream& operator<<(std::ostream& out, const Instructions& ins);

// 操作码类型定义为单字节
typedef uint8_t Opcode;

/**
 * @brief 操作码枚举类
 * 定义了虚拟机支持的所有指令类型。
 */
enum class OpType : Opcode {
    OpConstant,        // 加载常量（从常量池）
    OpAdd,             // 加法
    OpSub,             // 减法
    OpMul,             // 乘法
    OpDiv,             // 除法
    OpPop,             // 弹出栈顶元素
    OpTrue,            // 推入布尔值 True
    OpFalse,           // 推入布尔值 False
    OpEqual,           // 等于判定 (==)
    OpNotEqual,        // 不等于判定 (!=)
    OpGreaterThan,     // 大于判定 (>)
    OpMinus,           // 负号运算符 (-)
    OpBang,            // 取反运算符 (!)
    OpJumpNotTruthy,   // 如果栈顶为假则跳转
    OpJump,            // 无条件跳转
    OpNull,            // 推入 Null 值
    OpSetGlobal,       // 设置全局变量
    OpGetGlobal,       // 获取全局变量
    OpArray,           // 构建数组
    OpHash,            // 构建哈希表
    OpIndex,           // 索引访问 (obj[index])
    OpCall,            // 调用函数
    OpReturnValue,     // 返回值
    OpReturn,          // 空返回
    OpSetLocal,        // 设置局部变量
    OpGetLocal,        // 获取局部变量
    OpGetBuiltin,      // 获取内置函数
    OpClosure,         // 构建闭包
    OpGetFree,         // 获取自由变量
    OpCurrentClosure   // 获取当前闭包（支持递归）
};

/**
 * @brief 指令定义结构体
 * 记录每一条指令的元数据：名称以及每个操作数的宽度（字节数）。
 */
struct Definition {
    Definition() = delete;

    Definition(std::string name);

    Definition(const Definition& other) = default;

    Definition(Definition&& other) noexcept = default;

    Definition& operator=(const Definition& other) = default;

    Definition& operator=(Definition&& other) noexcept = default;

    std::string name;                // 指令名称，如 "OpConstant"
    std::vector<int> operand_widths; // 每个操作数占用的字节数列表
};

/**
 * @brief 全局操作码定义字典
 * 记录了 OpType 到其详细定义（Definition）的映射。
 */
extern std::map<OpType, std::shared_ptr<Definition>> definitions;

// 辅助函数：创建新的指令定义
std::shared_ptr<Definition> new_definition(std::string name);
std::shared_ptr<Definition> new_definition(std::string name, int operand_width);
std::shared_ptr<Definition> new_definition(std::string name, int first_operand_width, int second_operand_width);

/**
 * @brief 查找指令定义
 * @return 返回定义指针以及是否找到的布尔值
 */
std::tuple<std::shared_ptr<Definition>, bool> lookup(const OpType& op);

/**
 * @brief 模板工具函数：将枚举类型的 OpType 安全转换为底层的 uint8_t 操作码
 */
template<typename Enumeration>
constexpr std::enable_if_t<std::is_enum<Enumeration>::value,
        std::underlying_type_t<Enumeration>> as_opcode(const Enumeration value) {
    return static_cast<std::underlying_type_t<Enumeration>>(value);
}

/**
 * @brief 反汇编格式化
 * 将指令和操作数转换成易读的字符串。
 */
std::string fmt_instruction(std::shared_ptr<Definition> def, std::vector<int> operands);

/**
 * @brief 编码指令 (Make)
 * 将操作码和操作数打包成字节流（Instructions）。
 * 这是编译器生成字节码时使用的核心函数。
 */
Instructions make(OpType op);                                          // 无参数指令
Instructions make(OpType op, int operand);                             // 单参数指令
Instructions make(OpType op, int first_operand, int second_operand);   // 双参数指令

/**
 * @brief 读取字节码中的 8 位整数
 */
int read_uint_8(Instructions ins, int offset);

/**
 * @brief 读取字节码中的 16 位整数（通常用于常量池索引或跳转地址）
 */
int read_uint_16(Instructions ins, int offset);

/**
 * @brief 解码操作数
 * 根据指令定义，从字节流中读取该指令的所有操作数。
 * @return 返回操作数数组以及读取的总长度
 */
std::tuple<std::vector<int>, int> read_operands(std::shared_ptr<Definition> def, Instructions ins);