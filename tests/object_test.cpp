#include <gtest/gtest.h>

#include "litecomp/object.hpp"

TEST(ObjectModule, EnvironmentCopyAssignmentClearsOldEntries) {
    Environment src;
    src.set("a", std::make_shared<Integer>(Integer(1)));
    src.set("b", std::make_shared<Integer>(Integer(2)));

    Environment dst;
    dst.set("x", std::make_shared<Integer>(Integer(99)));

    dst = src;

    auto [a_obj, a_ok] = dst.get("a");
    auto [b_obj, b_ok] = dst.get("b");
    auto [x_obj, x_ok] = dst.get("x");

    EXPECT_TRUE(a_ok);
    EXPECT_TRUE(b_ok);
    EXPECT_FALSE(x_ok);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(a_obj)->value, 1);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(b_obj)->value, 2);
    EXPECT_EQ(x_obj, nullptr);
}

TEST(ObjectModule, HashMoveAssignmentReplacesContent) {
    HashKey key1{ObjectType::INTEGER_OBJ, 1};
    HashKey key2{ObjectType::INTEGER_OBJ, 2};

    HashPair pair1{std::make_shared<Integer>(Integer(1)), std::make_shared<String>(String("one"))};
    HashPair pair2{std::make_shared<Integer>(Integer(2)), std::make_shared<String>(String("two"))};

    Hash h1({{key1, pair1}});
    Hash h2({{key2, pair2}});

    h2 = std::move(h1);

    EXPECT_EQ(h2.pairs.size(), 1u);
    EXPECT_NE(h2.pairs.find(key1), h2.pairs.end());
    EXPECT_EQ(h2.pairs.find(key2), h2.pairs.end());
}

TEST(ObjectModule, NullIsSingletonByReference) {
    EXPECT_EQ(get_null_ref(), get_null_ref());
}

TEST(ObjectModule, HashKeyOrderingDistinguishesTypeAndValue) {
    HashKey int_one{ObjectType::INTEGER_OBJ, 1};
    HashKey bool_true{ObjectType::BOOLEAN_OBJ, 1};

    std::map<HashKey, HashPair> pairs;
    pairs[int_one] = HashPair{std::make_shared<Integer>(Integer(1)), std::make_shared<String>(String("int"))};
    pairs[bool_true] = HashPair{std::make_shared<Boolean>(Boolean(true)), std::make_shared<String>(String("bool"))};

    EXPECT_EQ(pairs.size(), 2u);
}

TEST(ObjectModule, EnvironmentGetResolvesThroughMultipleOuterScopes) {
    auto global = new_environment();
    global->set("a", std::make_shared<Integer>(Integer(42)));

    auto level1 = new_enclosed_environment(*global);
    auto level2 = new_enclosed_environment(*level1);

    auto [obj, ok] = level2->get("a");
    ASSERT_TRUE(ok);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<Integer>(obj)->value, 42);
}
