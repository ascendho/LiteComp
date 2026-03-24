cmake_minimum_required(VERSION 3.16)

project(LiteComp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to optimized builds for single-config generators (e.g. Unix Makefiles).
# Multi-config generators (e.g. Xcode, Ninja Multi-Config) choose config at build/run time.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

include(FetchContent)

add_library(litecomp_core STATIC
    src/ast.cpp
    src/builtins.cpp
    src/code.cpp
    src/compiler.cpp
    src/frame.cpp
    src/lexer.cpp
    src/object.cpp
    src/parser.cpp
    src/repl.cpp
    src/symbol_table.cpp
    src/token.cpp
    src/vm.cpp
)

target_include_directories(litecomp_core PUBLIC include)

add_executable(litecomp_repl apps/repl/main.cpp)
target_link_libraries(litecomp_repl PRIVATE litecomp_core)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(litecomp_benchmark
    benchmarks/main.cpp
    benchmarks/parser.cpp
    benchmarks/compiler.cpp
    benchmarks/vm.cpp
    benchmarks/end_to_end.cpp
    benchmarks/scaling.cpp
)
target_include_directories(litecomp_benchmark PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(litecomp_benchmark PRIVATE litecomp_core benchmark::benchmark)

add_executable(litecomp_regression apps/regression/main.cpp)
target_link_libraries(litecomp_regression PRIVATE litecomp_core)

enable_testing()
add_test(NAME regression COMMAND litecomp_regression)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(litecomp_demo
    apps/demo/main.cpp
    apps/demo/cases.cpp
    apps/demo/runner.cpp
)
target_include_directories(litecomp_demo PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(litecomp_demo PRIVATE litecomp_core)

add_executable(litecomp_unit_tests
    tests/ast_test.cpp
    tests/builtins_test.cpp
    tests/bytecode_test.cpp
    tests/code_test.cpp
    tests/compiler_test.cpp
    tests/frame_test.cpp
    tests/lexer_test.cpp
    tests/negative_golden_test.cpp
    tests/object_test.cpp
    tests/parser_test.cpp
    tests/repl_test.cpp
    tests/symbol_table_test.cpp
    tests/token_test.cpp
    tests/vm_test.cpp
)

target_include_directories(litecomp_unit_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(litecomp_unit_tests PRIVATE litecomp_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(litecomp_unit_tests)
