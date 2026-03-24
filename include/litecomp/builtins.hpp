#pragma once

#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "litecomp/object.hpp"

/**
 * @brief 语言支持的内置函数总数
 * 目前定义了 7 个核心函数。
 */
#define COUNT 7

/**
 * @brief 内置函数名称注册表
 * 使用 extern 声明，在对应的 .cpp 文件中初始化。
 * 它按顺序存储了函数名，索引与字节码中的编号对应。
 */
extern std::array<std::string, COUNT> builtinsNames;

/**
 * @brief 内置函数：获取长度 (len)
 * 预期行为：返回字符串的字符数或数组的元素个数。
 * @param args 运行时传递的参数列表（包装在 std::shared_ptr<Object> 中）
 * @return 包含长度值的 Integer 对象
 */
std::shared_ptr<Object> __len(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：获取首个元素 (first)
 * 预期行为：针对数组对象，返回其索引为 0 的元素。
 */
std::shared_ptr<Object> __first(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：获取末尾元素 (last)
 * 预期行为：针对数组对象，返回其最后一个元素。
 */
std::shared_ptr<Object> __last(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：获取除首个元素外的余下部分 (rest)
 * 预期行为：类似于 Lisp 中的 cdr，返回一个包含原数组除第一个元素外所有内容的新数组。
 */
std::shared_ptr<Object> __rest(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：向数组末尾添加元素 (push)
 * 预期行为：接收一个数组和一个元素，返回一个包含新元素的新数组副本。
 */
std::shared_ptr<Object> __push(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：打印到标准输出 (print)
 * 预期行为：调用对象的 inspect() 方法并将其内容输出到控制台。
 */
std::shared_ptr<Object> __print(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 内置函数：打印并自动换行 (println)
 */
std::shared_ptr<Object> __println(std::vector<std::shared_ptr<Object>> args);

/**
 * @brief 工具函数：通过名称获取内置函数对象
 * 常用于解释器模式（Evaluator）在环境初始化时根据名称绑定函数。
 * @param name 函数名，如 "len"
 */
std::shared_ptr<Builtin> getBuiltinByName(const std::string &name);

/**
 * @brief 工具函数：通过索引获取内置函数对象
 * 常用于虚拟机模式（VM）。在编译阶段，符号表会将内置函数名转换为索引，
 * 虚拟机通过 OpGetBuiltin 指令和该索引进行 O(1) 复杂度的极速查找。
 * @param index 数组中的偏移量 (0 到 COUNT-1)
 */
std::shared_ptr<Builtin> getBuiltinByIndex(int index);