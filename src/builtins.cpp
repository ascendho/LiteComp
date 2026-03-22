#include "litecomp/builtins.hpp"

#include <algorithm>

std::shared_ptr<Object> __len(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 1) {
        return new_error("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    // 数组或者字符串
    if (auto array = std::dynamic_pointer_cast<Array>(args.at(0))) {
        return std::make_shared<Integer>(array->elements.size());
    } else if (auto str = std::dynamic_pointer_cast<String>(args.at(0))) {
        return std::make_shared<Integer>(str->value.size());
    }

    return new_error("argument to 'len' not supported, got " + objecttype_literal(args.at(0)->type()));
}

std::shared_ptr<Object> __first(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 1) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'first' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        if (!arr->elements.empty()) {
            return arr->elements.at(0);
        }
    }

    return get_null_ref();
}

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

            // 忽略第一个元素
            for (auto i = std::next(arr->elements.begin()); i != arr->elements.end(); ++i) {
                new_elements.push_back(*i);
            }
            auto new_array = std::make_shared<Array>(Array{});
            new_array->elements = std::move(new_elements);

            return new_array;
        }
    }

    return get_null_ref();
}

std::shared_ptr<Object> __push(std::vector<std::shared_ptr<Object>> args) {
    if (args.size() != 2) {
        return new_error("Wrong Number of Arguments. got=" + std::to_string(args.size()) + ", want=2");
    }

    if (args.at(0)->type() != ObjectType::ARRAY_OBJ) {
        return new_error("Argument to 'push' must be ARRAY, got " + objecttype_literal(args.at(0)->type()));
    }

    if (auto arr = std::dynamic_pointer_cast<Array>(args.at(0))) {
        // 复制现有的数组元素
        auto new_array = std::make_shared<Array>(Array(*arr));

        // 添加新元素
        new_array->elements.push_back(std::move(args.at(1)));

        return new_array;
    }

    return get_null_ref();
}

std::shared_ptr<Object> __print(std::vector<std::shared_ptr<Object>> args) {
    for (const auto &arg: args) {
        std::cout << arg->inspect();
    }

    return get_null_ref();
}

std::shared_ptr<Object> __println(std::vector<std::shared_ptr<Object>> args) {
    for (const auto &arg: args) {
        std::cout << arg->inspect();
    }

    std::cout << std::endl;

    return get_null_ref();
}

std::array<std::string, COUNT> builtinsNames = {
        "len", "print", "println", "first", "last", "rest", "push"};

// 存储内置函数指针
std::array<builtin_fn, COUNT> builtins = {
        __len, __print, __println, __first, __last, __rest, __push};

std::shared_ptr<Builtin> getBuiltinByName(const std::string &name) {
    auto fn = std::find(builtinsNames.begin(), builtinsNames.end(), name);

    // 找不到内置函数
    if (fn == builtinsNames.end()) {
        return nullptr;
    }

    auto index = std::distance(builtinsNames.begin(), fn);

    return std::make_shared<Builtin>(Builtin{builtins[static_cast<long unsigned>(index)]});
}

std::shared_ptr<Builtin> getBuiltinByIndex(int index) {
    return std::make_shared<Builtin>(Builtin{builtins[static_cast<long unsigned>(index)]});
}