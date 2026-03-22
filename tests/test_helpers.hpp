#ifndef LITECOMP_TEST_HELPERS_HPP
#define LITECOMP_TEST_HELPERS_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "litecomp/compiler.hpp"
#include "litecomp/lexer.hpp"
#include "litecomp/object.hpp"
#include "litecomp/parser.hpp"
#include "litecomp/vm.hpp"

namespace test_helpers {

inline std::shared_ptr<Program> parse_program(const std::string& input, std::vector<std::string>* errors) {
    Lexer lexer(input);
    Parser parser(std::move(lexer));
    auto program = parser.parseProgram();
    if (errors != nullptr) {
        *errors = parser.errors();
    }
    return program;
}

inline std::shared_ptr<Error> compile_program(
    const std::string& input,
    std::shared_ptr<Compiler>* out_compiler,
    std::vector<std::string>* parser_errors) {
    auto program = parse_program(input, parser_errors);
    if (parser_errors != nullptr && !parser_errors->empty()) {
        return new_error("parser error");
    }

    auto compiler = newCompiler();
    auto err = compiler->compile(program);
    if (out_compiler != nullptr) {
        *out_compiler = compiler;
    }
    return err;
}

inline std::pair<std::string, std::string> eval_input(const std::string& input) {
    std::vector<std::string> parser_errors;
    std::shared_ptr<Compiler> compiler;

    auto compile_err = compile_program(input, &compiler, &parser_errors);
    if (!parser_errors.empty()) {
        return {"", parser_errors.front()};
    }
    if (compile_err) {
        return {"", compile_err->message};
    }

    VM vm(compiler->bytecode());
    auto vm_err = vm.run();
    if (vm_err) {
        return {"", vm_err->message};
    }

    return {vm.last_popped_stack_elem()->inspect(), ""};
}

}  // namespace test_helpers

#endif
