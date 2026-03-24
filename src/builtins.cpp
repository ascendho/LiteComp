#include "litecomp/builtins.hpp"
#include <algorithm>

/**
 * @brief 实现 len(arg) 函数
 * 支持对象：字符串 (String) 和 数组 (Array)
 */
std::shared_ptr<Object> __len(std::vector<std::shared_ptr<Object>> args) {
    // 检查参数个数：必须且只能有 1 个参数
    if (args.size() != 1) {
        return new_error("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    // 尝试将参数转换为数组
    if (auto array = std::dynamic_pointer_cast<Array>(args.at(0))) {
        return std::make_shared<Integer>(array->elements.size());
    } 

    // 尝试将参数转换为字符串
    else if (auto str = std::dynamic_pointer_cast<String>(args.at(0))) {
        return std::make_shared<Integer>(str->value.size());
    }

    // 类型不匹配错误
    return new_error("argument to 'len' not supported, got " + objecttype_literal(args.at(0)->type()));
}

/**
 * @brief 实现 first(arr) 函数
 * 获取数组的第一个元素
 */
std::shared_ptr<Object> __first(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 1) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    // 必须是数组类型
    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'first' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        if (!arr->elements.empty()) {
            // 返回第一个元素的指针
            return arr->elements.at(0); 
        }
    }

    // 数组为空则返回 null
    return get_null_ref(); 
}

/**
 * @brief 实现 last(arr) 函数
 * 获取数组的最后一个元素
 */
std::shared_ptr<Object> __last(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 1) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'last' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        auto length = arr->elements.size();
        if (length > 0) {
            return arr->elements.at(length - 1);
        }
    }

    return get_null_ref();
}

/**
 * @brief 实现 rest(arr) 函数
 * 返回一个新数组，包含除第一个元素以外的所有元素（类似于 Lisp 的 cdr）
 */
std::shared_ptr<Object> __rest(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 1) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'rest' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        auto length = arr->elements.size();
        if (length > 0) {
            std::vector<std::shared_ptr<Object>> new_elements;

            // 拷贝从第二个元素开始的所有内容到新 vector
            for (auto i = std::next(arr->elements.begin()); i != arr->elements.end(); ++i) {
                new_elements.push_back(*i);
            }
            auto new_array = std::make_shared<Array>(Array{});
            new_array->elements = std::move(new_elements);

            // 返回新数组对象
            return new_array; 
        }
    }

    return get_null_ref();
}

/**
 * @brief 实现 push(arr, val) 函数
 * 向数组添加元素。注意：为了保证函数式编程的不可变性，它返回一个包含新元素的新数组，不修改原数组。
 */
std::shared_ptr<Object> __push(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 2) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=2");
    }

    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'push' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        // 利用拷贝构造函数复制现有数组内容
        auto new_array = std::make_shared<Array>(Array(*arr));

        // 在新副本末尾追加新元素
        new_array->elements.push_back(std::move(args.at(1)));

        return new_array;
    }

    return get_null_ref();
}

/**
 * @brief 实现 print(args...) 函数
 * 将传入的所有参数打印到标准输出
 */
std::shared_ptr<Object> __print(std::vector<std::shared_ptr<Object>> args) {
    for (const auto &arg: args) {
        std::cout << arg->inspect();
    }

    // print 函数通常返回空
    return get_null_ref(); 
}

/**
 * @brief 实现 println(args...) 函数
 * 打印参数并换行
 */
std::shared_ptr<Object> __println(std::vector<std::shared_ptr<Object>> args) {
    for (const auto &arg: args) {
        std::cout << arg->inspect();
    }

    std::cout << std::endl;

    return get_null_ref();
}

// --- 内置函数映射表 ---

// 函数名称列表
std::array<std::string, COUNT> builtinsNames = {
        "len", "print", "println", "first", "last", "rest", "push"};

// 函数指针列表（与上面的名称顺序一一对应）
std::array<builtin_fn, COUNT> builtins = {
        __len, __print, __println, __first, __last, __rest, __push};

/**
 * @brief 按名称查找内置函数
 * @param name 字符串形式的函数名（如 "len"）
 * @return 对应的 Builtin 对象指针，找不到则返回 nullptr
 */
std::shared_ptr<Builtin> getBuiltinByName(const std::string &name) {
    // 在内置名称数组中搜索
    auto fn = std::find(builtinsNames.begin(), builtinsNames.end(), name);

    if (fn == builtinsNames.end()) {
        return nullptr;
    }

    // 计算索引偏移
    auto index = std::distance(builtinsNames.begin(), fn);

    // 返回封装好的 Builtin 对象
    return std::make_shared<Builtin>(Builtin{builtins[static_cast<long unsigned>(index)]});
}

/**
 * @brief 按索引查找内置函数
 * 用于虚拟机（VM）快速访问。VM 指令流中直接存储的是索引数字。
 */
std::shared_ptr<Builtin> getBuiltinByIndex(int index) {
    if (index < 0 || index >= COUNT) return nullptr;
    return std::make_shared<Builtin>(Builtin{builtins[static_cast<long unsigned>(index)]});
}