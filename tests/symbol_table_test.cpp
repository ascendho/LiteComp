#include <gtest/gtest.h>

#include "litecomp/symbol_table.hpp"

TEST(SymbolTableModule, DefineGlobalAndLocalSymbols) {
    auto global = new_symbol_table();
    auto a = global->define("a");
    EXPECT_EQ(a.scope, SymbolScope::GlobalScope);
    EXPECT_EQ(a.index, 0);

    auto local = new_enclosed_symbol_table(global);
    auto b = local->define("b");
    EXPECT_EQ(b.scope, SymbolScope::LocalScope);
    EXPECT_EQ(b.index, 0);
}

TEST(SymbolTableModule, ResolveFreeSymbol) {
    auto global = new_symbol_table();
    global->define("a");

    auto first_local = new_enclosed_symbol_table(global);
    first_local->define("b");

    auto second_local = new_enclosed_symbol_table(first_local);
    auto [resolved, ok] = second_local->resolve("b");

    ASSERT_TRUE(ok);
    EXPECT_EQ(resolved.scope, SymbolScope::FreeScope);
    EXPECT_EQ(resolved.name, "b");
    ASSERT_EQ(second_local->free_symbols.size(), 1u);
    EXPECT_EQ(second_local->free_symbols[0].name, "b");
}
