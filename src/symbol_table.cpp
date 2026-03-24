#include "litecomp/symbol_table.hpp"

// =============================================================================
// Symbol 结构体运算符重载
// =============================================================================

bool Symbol::operator==(const Symbol& other) const {
    return name == other.name && scope == other.scope && index == other.index;
}

bool Symbol::operator!=(const Symbol& other) const {
    return !(name == other.name && scope == other.scope && index == other.index);
}

/**
 * @brief 重载输出流运算符
 * 用于调试，将符号信息以易读的方式打印出来（例如：Symbol{"x", SymbolScope::GlobalScope, 0}）
 */
std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
    out << "Symbol{\"" << sym.name << "\", SymbolScope::";

    switch (sym.scope) {
        case SymbolScope::LocalScope:    out << "LocalScope";    break;
        case SymbolScope::GlobalScope:   out << "GlobalScope";   break;
        case SymbolScope::BuiltinScope:  out << "BuiltinScope";  break;
        case SymbolScope::FreeScope:     out << "FreeScope";     break;
        case SymbolScope::FunctionScope: out << "FunctionScope"; break;
        default:                         out << "UNDEFINED";     break;
    }

    out << ", " << sym.index << "}";
    return out;
}

// =============================================================================
// 符号表工厂函数
// =============================================================================

// 创建一个顶层符号表（全局）
std::shared_ptr<SymbolTable> new_symbol_table() {
    return std::make_shared<SymbolTable>(SymbolTable{});
}

// 创建一个嵌套符号表（局部），并关联到外部作用域
std::shared_ptr<SymbolTable> new_enclosed_symbol_table(std::shared_ptr<SymbolTable> outer) {
    auto s = new_symbol_table();
    s->outer = outer;
    return s;
}

// =============================================================================
// SymbolTable 成员函数实现
// =============================================================================

/**
 * @brief 在当前作用域定义一个新变量
 * @param name 变量名
 * @return 创建好的 Symbol 对象
 */
Symbol SymbolTable::define(std::string name) {
    auto symbol = Symbol{};
    symbol.name = name;

    // 分配当前索引位置
    symbol.index = num_definitions; 

    // 如果没有外部符号表，则该变量属于全局作用域
    if (!outer) {
        symbol.scope = SymbolScope::GlobalScope;
    } else {
        // 如果有父级表，说明当前处于函数或代码块内部，属于局部作用域
        symbol.scope = SymbolScope::LocalScope;
    }

    store[name] = symbol; // 存入当前作用域映射表
    num_definitions++;    // 增加定义计数，为下一个变量分配新索引
    return symbol;
}

/**
 * @brief 定义一个自由变量（用于实现闭包）
 * 自由变量是指在当前函数内使用，但定义在外部函数作用域中的非全局变量。
 * @param original 来自外部作用域的原始符号
 */
Symbol SymbolTable::define_free(Symbol original) {
    // 将原始符号存入“捕获列表”
    free_symbols.push_back(original);

    // 在当前作用域创建一个 FreeScope 类型的符号，索引对应 free_symbols 向量的位置
    auto symbol = Symbol{
        original.name,
        SymbolScope::FreeScope,
        static_cast<int>(free_symbols.size()) - 1
    };

    store[original.name] = symbol;
    return symbol;
}

/**
 * @brief 定义内置函数（如 len, puts）
 */
Symbol SymbolTable::define_builtin(int index, std::string name) {
    auto symbol = Symbol{name, SymbolScope::BuiltinScope, index};
    store[name] = symbol;
    return symbol;
}

/**
 * @brief 定义函数自身的名称（用于支持递归）
 * 将函数名绑定到 FunctionScope，索引通常为 0。
 */
Symbol SymbolTable::define_function_name(std::string name) {
    auto symbol = Symbol{name, SymbolScope::FunctionScope, 0};
    store[name] = symbol;
    return symbol;
}

/**
 * @brief 查找/解析符号名称
 * 这是符号表最核心的递归逻辑。
 * @param name 要查找的变量名
 * @return tuple<符号对象, 是否成功>
 */
std::tuple<Symbol, bool> SymbolTable::resolve(const std::string& name) {
    // 1. 尝试在当前符号表中查找
    auto it = store.find(name);
    if (it != store.end()) {
        return std::make_tuple(it->second, true);
    }

    // 2. 如果当前没找到，且存在外部作用域，则递归向上查找
    if (outer) {
        auto [o_obj, o_ok] = outer->resolve(name);
        
        // 如果父级作用域也没找到，直接返回失败
        if (!o_ok) {
            return std::make_tuple(o_obj, o_ok);
        }

        // 3. 跨作用域规则处理：
        // 如果在外部找到的是“全局变量”或“内置函数”，直接返回原始符号。
        // 因为这些符号在任何地方都可以通过全局或内置指令访问，不需要被闭包捕获。
        if (o_obj.scope == SymbolScope::GlobalScope || o_obj.scope == SymbolScope::BuiltinScope) {
            return std::make_tuple(o_obj, o_ok);
        }

        // 4. 关键：闭包捕获逻辑
        // 如果在外部找到的是“局部变量”（来自外层函数），
        // 那么对于当前的内层函数来说，它就是一个“自由变量”（Free Variable）。
        // 我们需要调用 define_free 将其“拉”进当前作用域的捕获列表中。
        auto free = define_free(o_obj);
        return std::make_tuple(free, true);
    }

    // 找不到符号
    return std::make_tuple(Symbol{}, false);
}