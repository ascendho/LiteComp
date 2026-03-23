#pragma once


#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "litecomp/object.hpp"

// 内置函数个数
#define COUNT 7

// 用数组存储内置函数名称便于索引
extern std::array<std::string, COUNT> builtinsNames;

// 获取数组/字符串长度
std::shared_ptr<Object> __len(std::vector<std::shared_ptr<Object>> args);

// 获取第一个元素
std::shared_ptr<Object> __first(std::vector<std::shared_ptr<Object>> args);

// 获取最后一个元素
std::shared_ptr<Object> __last(std::vector<std::shared_ptr<Object>> args);

// 获取余下的元素
std::shared_ptr<Object> __rest(std::vector<std::shared_ptr<Object>> args);

// 添加元素到数组末尾
std::shared_ptr<Object> __push(std::vector<std::shared_ptr<Object>> args);

// 打印
std::shared_ptr<Object> __print(std::vector<std::shared_ptr<Object>> args);

// 打印（自动换行）
std::shared_ptr<Object> __println(std::vector<std::shared_ptr<Object>> args);

std::shared_ptr<Builtin> getBuiltinByName(const std::string &name);

std::shared_ptr<Builtin> getBuiltinByIndex(int index);

