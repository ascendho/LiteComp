#include "litecomp/object.hpp"
#include "litecomp/ast.hpp"
#include <sstream>
#include <tuple>

// =============================================================================
// 全局单例优化
// =============================================================================
// 初始化全局唯一的 Null 和 Boolean 对象。
// 这样做可以避免在程序运行期间反复创建 true/false/null 实例，节省内存并提高比较效率。
auto GLOBAL_NULL = std::make_shared<Null>(Null{});
auto GLOBAL_TRUE = std::make_shared<Boolean>(Boolean{true});
auto GLOBAL_FALSE = std::make_shared<Boolean>(Boolean{false});

std::shared_ptr<Null> get_null_ref() { return GLOBAL_NULL; }
std::shared_ptr<Boolean> get_true_ref() { return GLOBAL_TRUE; }
std::shared_ptr<Boolean> get_false_ref() { return GLOBAL_FALSE; }

// =============================================================================
// Environment (环境/作用域实现)
// =============================================================================

// 拷贝构造：实现深拷贝，确保子环境拥有独立的变量存储空间
Environment::Environment(const Environment& other) : store{} {
    for (const auto &s: other.store) {
        store[s.first] = s.second->clone();
    }
    if (other.outer) {
        outer = other.outer->clone();
    }
}

// 移动构造：高效转移变量映射表
Environment::Environment(Environment&& other) noexcept {
    store.merge(other.store);
    outer = std::move(other.outer);
    other.outer = nullptr;
}

// 变量查找：递归向上搜索作用域链
std::tuple<std::shared_ptr<Object>, bool> Environment::get(const std::string &name) {
    auto it = store.find(name);
    if (it != store.end()) {
        // 在当前作用域找到
        return std::make_tuple(it->second, true); 
    }

    if (outer) {
        // 递归去父作用域找
        return outer->get(name); 
    }

    // 彻底找不到
    return std::make_tuple(nullptr, false); 
}

// 设置变量（当前作用域）
std::shared_ptr<Object> Environment::set(std::string name, std::shared_ptr<Object> val) {
    store[name] = val;
    return val;
}

std::shared_ptr<Environment> Environment::clone() const {
    return std::make_shared<Environment>(Environment{*this});
}

// 创建一个嵌套作用域，将当前的 outer 复制一份
std::shared_ptr<Environment> new_enclosed_environment(const Environment &outer) {
    auto env = std::make_shared<Environment>();
    env->outer = outer.clone();
    return env;
}

// =============================================================================
// 哈希支持 (HashKey & Hash Object)
// =============================================================================

// 用于 std::map 的排序规则
bool HashKey::operator<(const HashKey &other) const {
    // std::tie 方便地进行多字段联合比较
    return std::tie(type, value) < std::tie(other.type, other.value);
}

// 哈希表对象展示方法
std::string Hash::inspect() const {
    std::string out = "{";
    int counter = 0;
    for (const auto &kv: pairs) {
        if (counter++ > 0) out.append(", ");
        const auto[_, pair] = kv;
        out += pair.key->inspect() + ": " + pair.value->inspect();
    }
    out.append("}");
    return out;
}

// =============================================================================
// 各种 Object 子类的实现
// =============================================================================

/**
 * @brief 函数对象（用于解释器模式）
 * 包含了形参、函数体以及定义该函数时的词法环境（闭包实现的基础）
 */
std::string Function::inspect() const {
    std::string out = "fn(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) out.append(", ");
        out += parameters[i]->string();
    }
    out.append(") {\n" + body->string() + "\n}");
    return out;
}

/**
 * @brief 数组对象
 */
std::string Array::inspect() const {
    std::string out = "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) out.append(", ");
        out += elements[i]->inspect();
    }
    out.append("]");
    return out;
}

/**
 * @brief 错误对象
 */
std::string Error::inspect() const {
    return "Error: " + message;
}

/**
 * @brief 整数对象及其哈希键生成
 */
HashKey Integer::hash_key() const {
    // 整数的哈希值就是它本身的值转换成 uint64
    return HashKey{type(), static_cast<uint64_t>(value)};
}

/**
 * @brief 布尔对象及其哈希键生成
 */
HashKey Boolean::hash_key() const {
    // true 对应 1，false 对应 0
    return HashKey{type(), static_cast<uint64_t>(value ? 1 : 0)};
}

// =============================================================================
// CompiledFunction & Closure (用于虚拟机模式)
// =============================================================================

/**
 * @brief 已编译函数
 * 记录字节码、局部变量数和参数个数。其 inspect() 返回内存地址。
 */
std::string CompiledFunction::inspect() const {
    auto address = reinterpret_cast<uintptr_t>(this);
    std::stringstream stream;
    stream << "0x" << std::hex << address;
    return "CompiledFunction[" + stream.str() + "]";
}

/**
 * @brief 闭包
 * 将已编译函数与其捕获的“自由变量”列表绑定。
 */
std::string Closure::inspect() const {
    auto address = reinterpret_cast<uintptr_t>(this);
    std::stringstream stream;
    stream << "0x" << std::hex << address;
    return "Closure[" + stream.str() + "]";
}

// =============================================================================
// 字符串哈希算法 (FNV-1a)
// =============================================================================

/**
 * @brief 为字符串生成哈希键
 * 采用 64 位 FNV-1a 算法，确保字符串可以作为哈希表的键。
 */
HashKey String::hash_key() const {
    const char* apStr = value.data();
    // 64位 FNV 偏移基准
    uint64_t hash = 14695981039346656037ULL; 

    for (uint32_t idx = 0; idx < value.length(); ++idx) {
        // FNV 质数乘法
        hash = 1099511628211ULL * (hash ^ static_cast<unsigned char>(apStr[idx]));
    }

    return HashKey{type(), hash};
}

// =============================================================================
// 辅助工具函数
// =============================================================================

// 将 ObjectType 枚举转换为易读的字符串映射表
std::map<ObjectType, std::string> objecttype_literals = {
        {ObjectType::INTEGER_OBJ,"INTEGER"},
        {ObjectType::BOOLEAN_OBJ,"BOOLEAN"},
        {ObjectType::NULL_OBJ,"NULL"},
        {ObjectType::RETURN_VALUE_OBJ,"RETURN_VALUE"},
        {ObjectType::ERROR_OBJ,"ERROR"},
        {ObjectType::FUNCTION_OBJ,"FUNCTION"},
        {ObjectType::STRING_OBJ,"STRING"},
        {ObjectType::BUILTIN_OBJ,"BUILTIN"},
        {ObjectType::ARRAY_OBJ,"ARRAY"},
        {ObjectType::HASH_OBJ,"HASH"},
        {ObjectType::COMPILED_FUNCTION_OBJ,"COMPILED_FUNCTION"},
        {ObjectType::CLOSURE_OBJ,"CLOSURE"},
};

std::string objecttype_literal(ObjectType t) {
    auto it = objecttype_literals.find(t);
    return (it == objecttype_literals.end()) ? "OBJECT_NOT_DEFINED" : it->second;
}

std::shared_ptr<Error> new_error(std::string message) {
    return std::make_shared<Error>(Error{std::move(message)});
}

// 检查是否发生了运行时错误
bool is_error(const std::shared_ptr<Object>& obj) {
    return obj && obj->type() == ObjectType::ERROR_OBJ;
}