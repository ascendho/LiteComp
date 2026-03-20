#include <chrono>
#include <iostream>

#include "compiler.hpp"
#include "parser.hpp"
#include "vm.hpp"

int main() {
    std::string src = R"(
declare term=27;
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


    auto l = Lexer(src);
    auto p = Parser(std::move(l));
    auto program = p.parseProgram();

    auto comp = new_compiler();

    auto err = comp->compile(program);
    if (err) {
        std::cerr << "Compile Error: " << err->message << std::endl;
        return EXIT_FAILURE;
    }

    auto machine = VM(comp->bytecode());

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();

    // Execute benchmark run
    err = machine.run();
    if (err) {
        std::cerr << "Virtual Machine Error: " << err->message << std::endl;
        return EXIT_FAILURE;
    }

    auto t2 = high_resolution_clock::now();

    duration<double, std::milli> benchmark_time = t2 - t1;

    auto result = machine.last_popped_stack_elem();

    std::cout << "result=" << result->inspect() << ", duration=" << benchmark_time.count() / 1000
              << std::endl;

    return EXIT_SUCCESS;
}
