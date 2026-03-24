// =============================================================================
// Benchmark module
// This file defines benchmark inputs, helpers, or benchmark registration logic.
// =============================================================================
#include <benchmark/benchmark.h>

#include <string>

#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

namespace {

std::string build_arithmetic_chain_program(int terms) {
    std::string program;
    program.reserve(static_cast<size_t>(terms) * 4);
    program.append("1");

    for (int i = 1; i < terms; ++i) {
        program.append(" + 1");
    }

    return program;
}

}  // namespace

// [基准项] BM_Scaling_Parse_ArithmeticChain：执行性能测量主循环
static void BM_Scaling_Parse_ArithmeticChain(benchmark::State& state) {
    const int terms = static_cast<int>(state.range(0));
    const auto source = build_arithmetic_chain_program(terms);

    state.SetLabel("terms=" + std::to_string(terms));
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(source));
    }
}
BENCHMARK(BM_Scaling_Parse_ArithmeticChain)->RangeMultiplier(2)->Range(8, 1024);

// [基准项] BM_Scaling_Compile_ArithmeticChain：执行性能测量主循环
static void BM_Scaling_Compile_ArithmeticChain(benchmark::State& state) {
    const int terms = static_cast<int>(state.range(0));
    const auto source = build_arithmetic_chain_program(terms);
    const auto program = parse_or_exit(source);

    state.SetLabel("terms=" + std::to_string(terms));
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Scaling_Compile_ArithmeticChain)->RangeMultiplier(2)->Range(8, 1024);

// [基准项] BM_Scaling_VM_ArithmeticChain：执行性能测量主循环
static void BM_Scaling_VM_ArithmeticChain(benchmark::State& state) {
    const int terms = static_cast<int>(state.range(0));
    const auto source = build_arithmetic_chain_program(terms);
    const auto program = parse_or_exit(source);
    const auto code = compile_or_exit(program);

    state.SetLabel("terms=" + std::to_string(terms));
    for (auto _ : state) {
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_Scaling_VM_ArithmeticChain)->RangeMultiplier(2)->Range(8, 1024);
