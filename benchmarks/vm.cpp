// =============================================================================
// Benchmark module
// This file defines benchmark inputs, helpers, or benchmark registration logic.
// =============================================================================
#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

// [基准项] BM_VM_FibRecursive20：执行性能测量主循环
static void BM_VM_FibRecursive20(benchmark::State& state) {
    auto program = parse_or_exit(kFibRecursive20);
    auto code = compile_or_exit(program);

    for (auto _ : state) {
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_VM_FibRecursive20);

// [基准项] BM_VM_ClosureChain：执行性能测量主循环
static void BM_VM_ClosureChain(benchmark::State& state) {
    auto program = parse_or_exit(kClosureChain);
    auto code = compile_or_exit(program);

    for (auto _ : state) {
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_VM_ClosureChain);

// [基准项] BM_VM_ArrayHashBuiltin：执行性能测量主循环
static void BM_VM_ArrayHashBuiltin(benchmark::State& state) {
    auto program = parse_or_exit(kArrayHashBuiltin);
    auto code = compile_or_exit(program);

    for (auto _ : state) {
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_VM_ArrayHashBuiltin);
