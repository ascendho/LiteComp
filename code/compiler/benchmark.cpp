#include <chrono>
#include <iostream>

#include "compiler.hpp"
#include "parser.hpp"
#include "vm.hpp"

int main() {

    // 将要输入的源程序
    std::string src = R"(
declare term=10;
print("term = ", term,", ");

declare Fib = func(x) {
    if (x == 0) {
        return 0;
    } else {
        if (x == 1) {
            return 1;
        } else {
            return Fib(x - 1) + Fib(x - 2);
        }
    }
};

Fib(term);
)";


    auto l = Lexer(src);                         // 将源程序投喂给词法分析器
    auto p = Parser(std::move(l));                 // 语法分析器处理token流
    auto program = p.parseProgram();

    auto compiler = newCompiler();              // 编译器
    auto err = compiler->compile(program);   // 编译程序生成字节码
    if (err) {
        std::cerr << "Compile Error: " << err->message << std::endl;
        return EXIT_FAILURE;
    }

    auto machine = VM(compiler->bytecode());    // 虚拟机

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    // 记录起始时间
    auto startTime = high_resolution_clock::now();

    // 执行基准测试代码
    err = machine.run();
    if (err) {
        std::cerr << "Virtual Machine Error: " << err->message << std::endl;
        return EXIT_FAILURE;
    }

    // 记录终止时间
    auto endTime = high_resolution_clock::now();

    // 计算运行时间
    duration<double, std::milli> elapsedTime = endTime - startTime;

    auto result = machine.last_popped_stack_elem(); // 弹出运算结果

    // 打印信息
    std::cout << "result = " << result->inspect() << ", elapsed time = " << elapsedTime.count() / 1000
              << "s" << std::endl;

    return EXIT_SUCCESS;
}