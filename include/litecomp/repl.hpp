#pragma once

#include <vector>
#include <string>
#include <string_view>

/**
 * @brief REPL (交互式解释器) 模块
 * 该头文件定义了交互式环境所需的提示符常量及核心接口函数。
 */

// PROMPT: 用户交互界面的输入提示符（例如显示为 ">> "），提示用户在此输入代码。
inline constexpr std::string_view PROMPT{">> "};

// ERRORSIGN: 当解析代码出错时，打印在错误消息上方的视觉横幅。
inline constexpr std::string_view ERRORSIGN{R"(-------- ERROR --------  )"};

/**
 * @brief 启动 REPL 交互式解释器。
 * 
 * 此函数会进入一个无限循环，执行以下步骤：
 * 1. 打印提示符 ">> "。
 * 2. 读取用户从标准输入 (std::cin) 输入的一行代码。
 * 3. 调用词法分析器 (Lexer) 和解析器 (Parser) 处理代码。
 * 4. 如果解析成功，将其传递给编译器 (Compiler) 或解释器 (Evaluator) 执行。
 * 5. 将结果或错误打印到屏幕上。
 * 6. 重复上述过程，直到用户退出。
 */
void launchRepl();

/**
 * @brief 格式化输出语法解析器产生的错误。
 * 
 * @param errors 一个包含解析过程中发现的所有错误描述字符串的 vector。
 * 该函数会在控制台上打印 ERRORSIGN，并逐行显示具体的错误信息，帮助用户定位语法错误。
 */
void printParserErrors(std::vector<std::string> errors);