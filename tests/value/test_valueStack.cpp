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
    auto val_1 = aria::nil_val;
    auto val_2 = aria::true_val;
    auto val_3 = aria::false_val;
    auto val_4 = aria::obj_val(aria::newObjString("hello", gc));
    auto val_5 = aria::false_val;
    stack.push(val_1);
    stack.push(val_2);
    stack.push(val_3);
    stack.push(val_4);
    stack.push(val_5);
    EXPECT_EQ(stack.size(), 5);
    EXPECT_EQ(stack.pop(), aria::false_val);
}