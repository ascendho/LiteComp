#include <benchmark/benchmark.h>

#include "benchmarks/cases.hpp"
#include "benchmarks/helpers.hpp"

using namespace litecomp_bench;

static void BM_EndToEnd_FibRecursive20(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kFibRecursive20);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_FibRecursive20);

static void BM_EndToEnd_ClosureChain(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kClosureChain);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_ClosureChain);

static void BM_EndToEnd_ArrayHashBuiltin(benchmark::State& state) {
    for (auto _ : state) {
        auto program = parse_or_exit(kArrayHashBuiltin);
        auto code = compile_or_exit(program);
        run_vm_or_exit(code);
    }
}
BENCHMARK(BM_EndToEnd_ArrayHashBuiltin);
