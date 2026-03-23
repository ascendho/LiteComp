#pragma once


#include <string>

namespace litecomp_bench {

inline const std::string kFibRecursive20 = R"(
declare fib = func(n) {
    if (n == 0) { return 0; }
    if (n == 1) { return 1; }
    fib(n - 1) + fib(n - 2);
};
fib(20);
)";

inline const std::string kClosureChain = R"(
declare adder = func(a) { func(b) { a + b } };
declare f = adder(10);
f(32);
)";

inline const std::string kArrayHashBuiltin = R"(
declare arr = [1, 2, 3, 4, 5];
declare h = {"k": len(arr), 1: first(arr), 2: last(arr)};
h["k"] + h[1] + h[2];
)";

}  // namespace litecomp_bench

