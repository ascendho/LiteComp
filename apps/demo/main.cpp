// =============================================================================
// Application entry/demo layer
// This file contains app-facing wiring for demo, regression, or REPL scenarios.
// =============================================================================
#include "apps/demo/cases.hpp"
#include "apps/demo/runner.hpp"

// 程序入口：组织初始化并返回进程退出码
int main() {
    return run_demo_cases(build_demo_cases());
}
