#include "apps/demo/runner.hpp"

#include <iostream>

#include "litecomp/compiler.hpp"
#include "litecomp/object.hpp"
#include "litecomp/parser.hpp"
#include "litecomp/vm.hpp"

int run_demo_cases(const std::vector<DemoCase>& cases) {
    std::cout << "LiteComp feature demo" << std::endl;

    for (size_t i = 0; i < cases.size(); ++i) {
        auto lexer = Lexer(cases[i].input);
        auto parser = Parser(std::move(lexer));
        auto program = parser.parseProgram();

        auto parser_errors = parser.errors();
        if (!parser_errors.empty()) {
            std::cerr << "[FAIL] " << i << " (" << cases[i].feature << ") parser: "
                      << parser_errors.front() << std::endl;
            return EXIT_FAILURE;
        }

        auto compiler = newCompiler();
        auto compile_err = compiler->compile(program);
        if (compile_err) {
            std::cerr << "[FAIL] " << i << " (" << cases[i].feature << ") compile: "
                      << compile_err->message << std::endl;
            return EXIT_FAILURE;
        }

        VM vm(compiler->bytecode());
        auto vm_err = vm.run();
        if (vm_err) {
            std::cerr << "[FAIL] " << i << " (" << cases[i].feature << ") vm: "
                      << vm_err->message << std::endl;
            return EXIT_FAILURE;
        }

        auto actual = vm.last_popped_stack_elem()->inspect();
        const bool ok = (actual == cases[i].expected);

        std::cout << (ok ? "[PASS] " : "[FAIL] ") << cases[i].feature
                  << " => " << actual << std::endl;

        if (!ok) {
            std::cerr << "Expected: " << cases[i].expected << ", got: " << actual << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "All demo cases passed (" << cases.size() << ")." << std::endl;
    return EXIT_SUCCESS;
}
