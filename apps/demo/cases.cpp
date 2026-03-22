#include "apps/demo/cases.hpp"

std::vector<DemoCase> build_demo_cases() {
    return {
        {"arithmetic + precedence", "1 + 2 * 3 - 4", "3"},
        {"boolean + if/else", "if (!(1 > 2)) { 100 } else { 200 }", "100"},
        {"global bindings", "declare x = 7; declare y = 8; x + y", "15"},
        {"function + return", "declare add = func(a, b) { return a + b; }; add(20, 22)", "42"},
        {"closure", "declare mk = func(a) { func(b) { a + b } }; mk(9)(33)", "42"},
        {"string", "\"lite\" + \"comp\"", "litecomp"},
        {"array + index", "declare a = [10, 20, 30]; a[1]", "20"},
        {"array out-of-range", "[1, 2][99]", "null"},
        {"hash + key lookup", "declare h = {\"k\": 42, 1: 7}; h[\"k\"] + h[1]", "49"},
        {"hash missing key", "{1: 2}[3]", "null"},
        {"builtins len/first/last", "declare a = [1, 2, 3, 4]; len(a) + first(a) + last(a)", "9"},
        {"builtins rest/push", "declare a = [1, 2, 3]; len(push(rest(a), 99))", "3"},
        {"recursion", "declare fib = func(n) { if (n == 0) { return 0; } if (n == 1) { return 1; } fib(n - 1) + fib(n - 2); }; fib(10)", "55"},
    };
}
