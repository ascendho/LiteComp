#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

static void BM_Compiler_FibRecursive20(benchmark::State& state) {
    auto program = parse_or_exit(kFibRecursive20);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_FibRecursive20);

static void BM_Compiler_ClosureChain(benchmark::State& state) {
    auto program = parse_or_exit(kClosureChain);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_ClosureChain);

static void BM_Compiler_ArrayHashBuiltin(benchmark::State& state) {
    auto program = parse_or_exit(kArrayHashBuiltin);
    for (auto _ : state) {
        benchmark::DoNotOptimize(compile_or_exit(program));
    }
}
BENCHMARK(BM_Compiler_ArrayHashBuiltin);
