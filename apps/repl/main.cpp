// =============================================================================
// Application entry/demo layer
// This file contains app-facing wiring for demo, regression, or REPL scenarios.
// =============================================================================
#include "litecomp/repl.hpp"

// 程序入口：组织初始化并返回进程退出码
int main() {
    try {
        std::string line;

        launchRepl();
    }
    catch (const std::exception &) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}