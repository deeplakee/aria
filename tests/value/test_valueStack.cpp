#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objException.h"
#include "src/object/objString.h"
#include "src/util/util.h"
#include "value/valueStack.h"

#include <filesystem>

TEST_F(ValueTestFixture, ValueStackOperation1)
{
    aria::ValueStack stack;
    auto val_1 = aria::NanBox::NilValue;
    auto val_2 = aria::NanBox::TrueValue;
    auto val_3 = aria::NanBox::FalseValue;
    auto val_4 = aria::NanBox::fromObj(aria::newObjString("hello", gc));
    auto val_5 = aria::NanBox::FalseValue;
    stack.push(val_1);
    stack.push(val_2);
    stack.push(val_3);
    stack.push(val_4);
    stack.push(val_5);
    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.pop(), aria::NanBox::FalseValue);
}