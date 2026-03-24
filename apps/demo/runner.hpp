#pragma once

// =============================================================================
// Application entry/demo layer
// This file contains app-facing wiring for demo, regression, or REPL scenarios.
// =============================================================================


#include <vector>

#include "apps/demo/types.hpp"

int run_demo_cases(const std::vector<DemoCase>& cases);

