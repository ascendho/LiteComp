#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

static void BM_Parser_FibRecursive20(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kFibRecursive20));
    }
}
BENCHMARK(BM_Parser_FibRecursive20);

static void BM_Parser_ClosureChain(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kClosureChain));
    }
}
BENCHMARK(BM_Parser_ClosureChain);

static void BM_Parser_ArrayHashBuiltin(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(parse_or_exit(kArrayHashBuiltin));
    }
}
BENCHMARK(BM_Parser_ArrayHashBuiltin);
