#pragma once


#include <iostream>
#include <memory>
#include <string>

#include "litecomp/bytecode.hpp"
#include "litecomp/compiler.hpp"
#include "litecomp/parser.hpp"
#include "litecomp/vm.hpp"

namespace litecomp_bench {

inline std::shared_ptr<Program> parse_or_exit(const std::string& source) {
    Lexer lexer(source);
    Parser parser(std::move(lexer));
    auto program = parser.parseProgram();

    auto errs = parser.errors();
    if (!errs.empty()) {
        std::cerr << "Benchmark parser error: " << errs.front() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return program;
}

inline std::shared_ptr<Bytecode> compile_or_exit(const std::shared_ptr<Program>& program) {
    auto compiler = newCompiler();
    auto err = compiler->compile(program);
    if (err) {
        std::cerr << "Benchmark compile error: " << err->message << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return compiler->bytecode();
}

inline void run_vm_or_exit(const std::shared_ptr<Bytecode>& code) {
    auto cloned = std::make_shared<Bytecode>(*code);
    VM vm(std::move(cloned));
    auto err = vm.run();
    if (err) {
        std::cerr << "Benchmark VM error: " << err->message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace litecomp_bench

