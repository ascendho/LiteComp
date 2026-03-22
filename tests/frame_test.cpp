#include <gtest/gtest.h>

#include "litecomp/frame.hpp"
#include "litecomp/object.hpp"

TEST(FrameModule, NewFrameInitialState) {
    auto fn = std::make_shared<CompiledFunction>(CompiledFunction(make(OpType::OpTrue)));
    auto cl = std::make_shared<Closure>(Closure(fn));
    auto frame = new_frame(cl, 3);

    ASSERT_NE(frame, nullptr);
    EXPECT_EQ(frame->ip, -1);
    EXPECT_EQ(frame->base_pointer, 3);
    EXPECT_EQ(frame->instructions(), fn->instructions);
}
