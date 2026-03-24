// =============================================================================
// Benchmark module
// This file defines benchmark inputs, helpers, or benchmark registration logic.
// =============================================================================
#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

// [基准项] BM_Parser_FibRecursive20：执行性能测量主循环
static void BM_Parser_FibRecursive20(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kFibRecursive20));
    }
}
BENCHMARK(BM_Parser_FibRecursive20);

// [基准项] BM_Parser_ClosureChain：执行性能测量主循环
static void BM_Parser_ClosureChain(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kClosureChain));
    }
}
BENCHMARK(BM_Parser_ClosureChain);

// [基准项] BM_Parser_ArrayHashBuiltin：执行性能测量主循环
static void BM_Parser_ArrayHashBuiltin(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kArrayHashBuiltin));
    }
}
BENCHMARK(BM_Parser_ArrayHashBuiltin);
