#pragma once

// =============================================================================
// Application entry/demo layer
// This file contains app-facing wiring for demo, regression, or REPL scenarios.
// =============================================================================


#include <string>

struct DemoCase {
    std::string feature;
    std::string input;
    std::string expected;
};

