// =============================================================================
// Benchmark module
// This file defines benchmark inputs, helpers, or benchmark registration logic.
// =============================================================================
#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

// [基准项] BM_EndToEnd_FibRecursive20：执行性能测量主循环
static void BM_EndToEnd_FibRecursive20(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kFibRecursive20);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_FibRecursive20);

// [基准项] BM_EndToEnd_ClosureChain：执行性能测量主循环
static void BM_EndToEnd_ClosureChain(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kClosureChain);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_ClosureChain);

// [基准项] BM_EndToEnd_ArrayHashBuiltin：执行性能测量主循环
static void BM_EndToEnd_ArrayHashBuiltin(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kArrayHashBuiltin);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_ArrayHashBuiltin);
