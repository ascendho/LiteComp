#include <iostream>

#include "litecomp/repl.hpp"
#include "litecomp/builtins.hpp"
#include "litecomp/bytecode.hpp"
#include "litecomp/compiler.hpp"
#include "litecomp/lexer.hpp"
#include "litecomp/object.hpp"
#include "litecomp/parser.hpp"
#include "litecomp/vm.hpp"

/**
 * @brief 启动 REPL (Read-Eval-Print Loop) 交互式环境。
 * 该函数实现了从用户输入到程序执行的完整闭环。
 */
void launchRepl() {
    std::string line;               // 存储用户输入的每一行字符串
    std::shared_ptr<Error> err;     // 存储运行过程中可能产生的错误对象

    // --- 持久化状态 (状态在多次输入之间保留) ---
    // 下面这三个变量定义在 while 循环外，确保用户在上一行定义的变量，在下一行还能继续使用。
    
    // 1. 全局常量池：存储所有的数字、字符串和编译后的函数对象。
    std::vector<std::shared_ptr<Object>> constants;
    
    // 2. 全局变量存储：对应字节码中的 OpSetGlobal 和 OpGetGlobal，大小由 GLOBALSSIZE 定义。
    std::array<std::shared_ptr<Object>, GLOBALSSIZE> globals;
    
    // 3. 符号表：记录变量名与索引、作用域的映射关系。
    auto symbol_table = new_symbol_table();

    // --- 初始化内置函数 ---
    // 在 REPL 启动之初，将系统自带的内置函数（如 len, print）注册到符号表中。
    for (int i = 0; i < static_cast<int>(builtinsNames.size()); i++) {
        symbol_table->define_builtin(i, builtinsNames[i]);
    }

    // --- REPL 主循环 ---
    while (true) {
        // 1. 打印提示符 ">> "
        std::cout << PROMPT;

        // 2. 获取用户输入的一行
        if (!std::getline(std::cin, line)) {
            break; // 如果读取失败（如 Ctrl+D），退出循环
        }

        // 3. 词法分析 (Lexer)：将字符串拆解为 Token 流
        auto l = Lexer(line);
        
        // 4. 语法解析 (Parser)：将 Token 流转换为 抽象语法树 (AST)
        auto p = Parser(std::move(l));
        auto program = p.parseProgram();

        // 检查 Parser 是否报错
        if (!p.errors().empty()) {
            printParserErrors(p.errors());
            continue; // 跳过本次循环，等待下次输入
        }

        // 如果用户只是按了回车没有输入任何语句，直接跳过
        if (program->statements.empty()) {
            continue;
        }

        // 5. 编译 (Compiler)：将 AST 转换为 字节码 (Bytecode)
        // 使用 new_compiler_with_state 确保新的编译循环能识别之前已经定义的变量和常量。
        auto comp = new_compiler_with_state(symbol_table, constants);
        err = comp->compile(program);
        
        if (err) {
            // 编译失败：使用 ANSI 转义码打印红色错误信息 (\033[31;1m)
            std::cout << "\033[31;1mCompile Failed: " << err->message << "\033[0m"
                      << std::endl;
            continue;
        }

        // 获取本次编译生成的字节码
        auto code = comp->bytecode();

        // --- 常量池同步 ---
        // 编译器在本次循环中可能会产生新的常量（如新的函数定义或数字），
        // 我们需要把这些新产生的常量追加到持久化的常量池 constants 中。
        auto new_constants = code->constants;
        for (int i = constants.size(); i < static_cast<int>(new_constants.size()); i++) {
            constants.push_back(new_constants.at(i)->clone());
        }

        // 6. 虚拟机执行 (VM)：运行字节码并得到结果
        // 将持久化的 globals 传入 VM，使其能访问之前定义的全局变量。
        auto machine = VM(std::move(code), globals);
        err = machine.run();
        
        if (err) {
            // 运行时失败：使用 ANSI 转义码打印红色错误信息
            std::cout << "\033[31;1mVirtual Machine Failed to Execute: " << err->message << "\033[0m"
                      << std::endl;
            continue;
        }

        // --- 全局变量同步 ---
        // 虚拟机在运行过程中可能修改了全局变量（如 let a = 1;），
        // 我们需要把修改后的全局状态复制回持久化的 globals 数组中。
        std::copy(std::begin(machine.globals), std::end(machine.globals), std::begin(globals));

        // 如果需要显示表达式的结果，可以取消下面代码的注释：
        // auto last_popped = machine.last_popped_stack_elem();
        // std::cout << last_popped->inspect() << std::endl;
    }
}

/**
 * @brief 格式化输出 Parser 错误信息。
 * 
 * @param errors 错误消息字符串向量。
 */
void printParserErrors(std::vector<std::string> errors) {
    // 打印红色加粗的标题
    std::cout << "\033[31;1mParser Error:\033[0m" << std::endl;

    // 逐行打印带缩进的错误内容
    for (const auto &msg: errors) {
        std::cout << "\t" << "\033[31;1m" << msg << "\033[0m" << std::endl;
    }
}