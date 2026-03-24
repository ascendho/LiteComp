#include "litecomp/code.hpp"

/**
 * @brief 全局指令定义表
 * 每一项包含：操作码类型 -> {指令名称, 操作数占用字节数列表}
 * 例如：OpConstant 后面跟着 2 个字节（16位索引），OpCall 后面跟着 1 个字节（参数个数）
 */
std::map<OpType, std::shared_ptr<Definition>> definitions = {
    {OpType::OpConstant, new_definition("OpConstant", 2)},       // 常量池索引 (2字节)
    {OpType::OpAdd, new_definition("OpAdd")},                    // 加法 (无参数)
    {OpType::OpSub, new_definition("OpSub")},                    // 减法 (无参数)
    {OpType::OpMul, new_definition("OpMul")},
    {OpType::OpDiv, new_definition("OpDiv")},
    {OpType::OpPop, new_definition("OpPop")},                    // 弹出栈顶
    {OpType::OpTrue, new_definition("OpTrue")},
    {OpType::OpFalse, new_definition("OpFalse")},
    {OpType::OpEqual, new_definition("OpEqual")},
    {OpType::OpNotEqual, new_definition("OpNotEqual")},
    {OpType::OpGreaterThan, new_definition("OpGreaterThan")},
    {OpType::OpMinus, new_definition("OpMinus")},                // 前缀负号
    {OpType::OpBang, new_definition("OpBang")},                  // 前缀逻辑非
    {OpType::OpJumpNotTruthy, new_definition("OpJumpNotTruthy", 2)}, // 条件跳转 (2字节地址)
    {OpType::OpJump, new_definition("OpJump", 2)},               // 无条件跳转 (2字节地址)
    {OpType::OpNull, new_definition("OpNull")},
    {OpType::OpSetGlobal, new_definition("OpSetGlobal", 2)},     // 设置全局变量索引 (2字节)
    {OpType::OpGetGlobal, new_definition("OpGetGlobal", 2)},     // 获取全局变量索引 (2字节)
    {OpType::OpArray, new_definition("OpArray", 2)},             // 数组长度 (2字节)
    {OpType::OpHash, new_definition("OpHash", 2)},               // 哈希表键值对数量 (2字节)
    {OpType::OpIndex, new_definition("OpIndex")},                // 索引操作 (无参数，从栈中取)
    {OpType::OpCall, new_definition("OpCall", 1)},               // 函数调用，1字节代表参数个数
    {OpType::OpReturnValue, new_definition("OpReturnValue")},    // 返回带值
    {OpType::OpReturn, new_definition("OpReturn")},              // 空返回
    {OpType::OpSetLocal, new_definition("OpSetLocal", 1)},       // 设置局部变量索引 (1字节)
    {OpType::OpGetLocal, new_definition("OpGetLocal", 1)},       // 获取局部变量索引 (1字节)
    {OpType::OpGetBuiltin, new_definition("OpGetBuiltin", 1)},   // 内置函数索引 (1字节)
    {OpType::OpClosure, new_definition("OpClosure", 2, 1)},      // 闭包：2字节常量索引 + 1字节自由变量个数
    {OpType::OpGetFree, new_definition("OpGetFree", 1)},         // 获取自由变量索引 (1字节)
    {OpType::OpCurrentClosure, new_definition("OpCurrentClosure")}, // 获取当前闭包（用于递归）
};

// =============================================================================
// Definition 构造工厂
// =============================================================================

Definition::Definition(std::string name) : name{name}, operand_widths{} {}

// 创建不带操作数的指令定义
std::shared_ptr<Definition> new_definition(std::string name) {
    return std::make_shared<Definition>(Definition(name));
}

// 创建带 1 个操作数的指令定义
std::shared_ptr<Definition> new_definition(std::string name, int operand_width) {
    auto definition = std::make_shared<Definition>(Definition{name});
    definition->operand_widths.push_back(operand_width);
    return definition;
}

// 创建带 2 个操作数的指令定义
std::shared_ptr<Definition> new_definition(std::string name, int first_operand_width, int second_operand_width) {
    auto definition = std::make_shared<Definition>(Definition{name});
    definition->operand_widths.push_back(first_operand_width);
    definition->operand_widths.push_back(second_operand_width);
    return definition;
}

/**
 * @brief 在全局定义表中查找操作码的元数据
 * @return std::tuple<定义指针, 是否成功>
 */
std::tuple<std::shared_ptr<Definition>, bool> lookup(const OpType &op)
{
    auto contains = definitions.find(op);
    if (contains == definitions.end()) {
        return std::make_tuple(nullptr, false);
    }
    return std::make_tuple(definitions[op], true);
}

/**
 * @brief 将指令及其操作数格式化为字符串，用于反汇编显示
 * 例如："OpConstant 65534"
 */
std::string fmt_instruction(std::shared_ptr<Definition> def, std::vector<int> operands) {
    auto operand_count = def->operand_widths.size();

    if (operands.size() != operand_count) {
        return "ERROR: operand length " + std::to_string(operands.size()) + " does not match defined " + std::to_string(operand_count) + "\n";
    }

    switch (operand_count) {
        case 0:
            return def->name;
        case 1:
            return def->name + " " + std::to_string(operands.at(0));
        case 2:
            return def->name + " " + std::to_string(operands.at(0)) + " " + std::to_string(operands.at(1));
        default:
            return "ERROR: unhandled operand_count for " + def->name;
    }
}

/**
 * @brief 指令序列的输出重载：实现反汇编功能
 * 遍历字节数组，将其转换成带地址偏移的可读文本。
 */
std::ostream& operator<<(std::ostream& out, const Instructions& ins) {
    int i = 0;

    // 格式化地址前导 0
    out.fill('0'); 

    while (i < static_cast<int>(ins.size())) {
        auto[def, ok] = lookup(static_cast<OpType>(ins.at(i)));
        if (!ok) {
            out << "ERROR: opcode undefined at pos " << i << std::endl;
            i++;
            continue;
        }

        // 截取当前操作码之后的字节流用于读取操作数
        Instructions slice = Instructions(ins.begin() + i + 1, ins.end());
        auto[operands, read] = read_operands(def, slice);

        // 输出格式： "0003 OpConstant 2" (地址 指令 参数)
        out << std::setw(4) << i << " ";
        out << fmt_instruction(def, operands) << std::endl;

        // 步进：1字节操作码 + 已读取的操作数总宽度
        i += (1 + read);
    }
    return out;
}

// =============================================================================
// make 函数：将指令编码为二进制字节 (Encoding)
// =============================================================================

// 处理无参数指令
Instructions make(OpType op) {
    auto [def, ok] = lookup(op);
    if (!ok) throw std::exception();

    // 长度必须只有 1 个字节（即操作码本身）
    std::vector<uint8_t> instruction = {as_opcode(op)};
    return instruction;
}

// 处理带 1 个参数的指令
Instructions make(OpType op, int operand) {
    auto [def, ok] = lookup(op);
    if (!ok) throw std::exception();

    // 计算总长度：1字节操作码 + 操作数宽度
    int instruction_len = 1 + def->operand_widths.at(0);
    std::vector<uint8_t> instruction(instruction_len);
    // 第 0 位存操作码
    instruction[0] = as_opcode(op); 

    int offset = 1;
    auto width = def->operand_widths.at(0);

    switch (width) {
        // 16位整数，存入 2 个字节
        case 2: 
            // 大端序处理：高位在前
            instruction[offset] = static_cast<uint16_t>(operand) >> 8;
            instruction[offset + 1] = static_cast<uint16_t>(operand) & 0xFF;
            break;
        // 8位整数，存入 1 个字节
        case 1: 
            instruction[offset] = static_cast<uint8_t>(operand);
            break;
    }
    return instruction;
}

// 处理带 2 个参数的指令 (如 OpClosure)
Instructions make(OpType op, int first_operand, int second_operand) {
    auto [def, ok] = lookup(op);
    if (!ok || def->operand_widths.size() != 2) throw std::exception();

    int instruction_len = 1 + def->operand_widths[0] + def->operand_widths[1];
    std::vector<uint8_t> instruction(instruction_len);
    instruction[0] = as_opcode(op);

    int offset = 1;
    // 循环处理两个操作数
    std::vector<int> ops = {first_operand, second_operand};
    for (int i = 0; i < 2; ++i) {
        int width = def->operand_widths[i];
        if (width == 2) {
            instruction[offset] = static_cast<uint16_t>(ops[i]) >> 8;
            instruction[offset + 1] = static_cast<uint16_t>(ops[i]) & 0xFF;
            offset += 2;
        } else if (width == 1) {
            instruction[offset] = static_cast<uint8_t>(ops[i]);
            offset += 1;
        }
    }
    return instruction;
}

// =============================================================================
// 读取函数：从字节流中提取数值 (Decoding)
// =============================================================================

// 读取 8 位单字节值
int read_uint_8(Instructions ins, int offset) {
    return static_cast<int>(ins[offset]);
}

// 读取 16 位双字节值（大端序拼装）
int read_uint_16(Instructions ins, int offset) {
    auto hi = static_cast<uint16_t>(ins[offset]) << 8;    // 高位左移 8 位
    auto lo = static_cast<uint16_t>(ins[offset+1]);       // 低位
    return static_cast<int>(lo | hi);                     // 按位或组合
}

/**
 * @brief 根据指令定义，从指令序列中提取所有操作数
 * @return tuple<操作数列表, 总计读取的字节数>
 */
std::tuple<std::vector<int>, int> read_operands(std::shared_ptr<Definition> def, Instructions ins) {
    std::vector<int> operands(def->operand_widths.size());
    int offset = 0;

    for (int i = 0; i < static_cast<int>(def->operand_widths.size()); i++) {
        auto width = def->operand_widths.at(i);

        switch (width) {
            case 2:
                operands[i] = read_uint_16(ins, offset);
                break;
            case 1:
                operands[i] = read_uint_8(ins, offset);
                break;
        }
        offset += width;
    }

    return std::make_tuple(operands, offset);
}