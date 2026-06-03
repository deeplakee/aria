#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objString.h"
#include "src/value/valueStack.h"

using namespace aria;

class ValueStackTest : public ValueTestFixture
{
};

// peek 不同深度
TEST_F(ValueStackTest, PeekDepth)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    stack.push(NanBox::fromNumber(2));
    stack.push(NanBox::fromNumber(3));

    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(0)), 3.0);   // 栈顶
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(1)), 2.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(2)), 1.0);   // 栈底
}

// pop_n 批量弹出
TEST_F(ValueStackTest, PopN)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    stack.push(NanBox::fromNumber(2));
    stack.push(NanBox::fromNumber(3));
    stack.push(NanBox::fromNumber(4));
    stack.push(NanBox::fromNumber(5));

    stack.pop_n(3);
    EXPECT_EQ(stack.size(), 2);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(0)), 2.0);
}

// reset 清空
TEST_F(ValueStackTest, Reset)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    stack.push(NanBox::fromNumber(2));
    stack.push(NanBox::fromNumber(3));

    stack.reset();
    EXPECT_EQ(stack.size(), 0);
}

// operator[] 随机访问
TEST_F(ValueStackTest, IndexAccess)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(10));
    stack.push(NanBox::fromNumber(20));
    stack.push(NanBox::fromNumber(30));

    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack[0]), 10.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack[1]), 20.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack[2]), 30.0);
}

// push/pop 交替操作
TEST_F(ValueStackTest, PushPopAlternating)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    EXPECT_EQ(stack.size(), 1);

    auto v = stack.pop();
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 1.0);
    EXPECT_EQ(stack.size(), 0);

    stack.push(NanBox::fromNumber(2));
    stack.push(NanBox::fromNumber(3));
    EXPECT_EQ(stack.size(), 2);

    v = stack.pop();
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 3.0);
    v = stack.pop();
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 2.0);
    EXPECT_EQ(stack.size(), 0);
}

// resize
TEST_F(ValueStackTest, Resize)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    stack.push(NanBox::fromNumber(2));
    stack.push(NanBox::fromNumber(3));

    stack.resize(1);
    EXPECT_EQ(stack.size(), 1);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(0)), 1.0);
}

// set_top_val
TEST_F(ValueStackTest, SetTopVal)
{
    ValueStack stack;
    stack.push(NanBox::fromNumber(1));
    stack.set_top_val(NanBox::fromNumber(99));

    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(0)), 99.0);
    EXPECT_EQ(stack.size(), 1);
}

// 混合类型
TEST_F(ValueStackTest, MixedTypes)
{
    ValueStack stack;
    stack.push(NanBox::NilValue);
    stack.push(NanBox::TrueValue);
    stack.push(NanBox::FalseValue);
    stack.push(NanBox::fromNumber(3.14));
    stack.push(NanBox::fromObj(new_ObjString("hello", gc)));

    EXPECT_EQ(stack.size(), 5);
    EXPECT_TRUE(is_obj_string(stack.peek(0)));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(stack.peek(1)), 3.14);
    EXPECT_EQ(stack.peek(2), NanBox::FalseValue);
    EXPECT_EQ(stack.peek(3), NanBox::TrueValue);
    EXPECT_TRUE(NanBox::isNil(stack.peek(4)));
}
