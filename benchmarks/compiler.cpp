// =============================================================================
// Benchmark module
// This file defines benchmark inputs, helpers, or benchmark registration logic.
// =============================================================================
#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

// [基准项] BM_Compiler_FibRecursive20：执行性能测量主循环
static void BM_Compiler_FibRecursive20(benchmark::State& state) {
    auto program = parse_or_exit(kFibRecursive20);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_FibRecursive20);

// [基准项] BM_Compiler_ClosureChain：执行性能测量主循环
static void BM_Compiler_ClosureChain(benchmark::State& state) {
    auto program = parse_or_exit(kClosureChain);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_ClosureChain);

// [基准项] BM_Compiler_ArrayHashBuiltin：执行性能测量主循环
static void BM_Compiler_ArrayHashBuiltin(benchmark::State& state) {
    auto program = parse_or_exit(kArrayHashBuiltin);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_ArrayHashBuiltin);
