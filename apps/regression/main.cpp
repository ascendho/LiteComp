// =============================================================================
// Application entry/demo layer
// This file contains app-facing wiring for demo, regression, or REPL scenarios.
// =============================================================================
#include <iostream>
#include <string>
#include <vector>

#include "litecomp/compiler.hpp"
#include "litecomp/object.hpp"
#include "litecomp/parser.hpp"
#include "litecomp/vm.hpp"

struct RegressionCase {
    std::string input;
    std::string expected;
};

// 程序入口：组织初始化并返回进程退出码
int main() {
    std::vector<RegressionCase> cases = {
            {"1 + 2", "3"},
            {"declare x = 5; x * 2", "10"},
            {"if (1 < 2) { 7 } else { 8 }", "7"},
            {"len([1, 2, 3])", "3"},
            {"declare add = func(a, b) { a + b }; add(4, 5)", "9"},
            {"declare mk = func(a) { func(b) { a + b } }; mk(3)(6)", "9"},
            {"[1, 2, 3][99]", "null"},
            {"{1: 2}[3]", "null"},
    };

    for (size_t i = 0; i < cases.size(); ++i) {
        auto lexer = Lexer(cases[i].input);
        auto parser = Parser(std::move(lexer));
        auto program = parser.parseProgram();

        auto parser_errors = parser.errors();
        if (!parser_errors.empty()) {
            std::cerr << "[FAIL] case " << i << " parser error: " << parser_errors.front() << std::endl;
            return EXIT_FAILURE;
        }

        auto compiler = newCompiler();
        auto compile_error = compiler->compile(program);
        if (compile_error) {
            std::cerr << "[FAIL] case " << i << " compile error: " << compile_error->message << std::endl;
            return EXIT_FAILURE;
        }

        auto vm = VM(compiler->bytecode());
        auto vm_error = vm.run();
        if (vm_error) {
            std::cerr << "[FAIL] case " << i << " vm error: " << vm_error->message << std::endl;
            return EXIT_FAILURE;
        }

        auto actual = vm.last_popped_stack_elem()->inspect();
        if (actual != cases[i].expected) {
            std::cerr << "[FAIL] case " << i << " expected=" << cases[i].expected
                      << " got=" << actual << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "All regression cases passed (" << cases.size() << ")." << std::endl;
    return EXIT_SUCCESS;
}
