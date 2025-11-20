#include "compile/ast.h"
#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objException.h"
#include "src/object/objString.h"
#include "src/value/valueArray.h"

#include <filesystem>

TEST_F(ValueTestFixture, ValueArrayOperation1)
{
    aria::ValueArray arr = aria::ValueArray(gc);

    const char *msg1 = "this is a spark test for ValueObj";

    auto str1 = aria::newObjString(msg1, gc);
    aria::Value val_1 = aria::NanBox::fromObj(str1);

    auto e1 = aria::newObjException(msg1, gc);
    aria::Value val_2 = aria::NanBox::fromObj(e1);

    aria::Value val_3 = aria::NanBox::NilValue;

    aria::Value val_4 = aria::NanBox::TrueValue;

    aria::Value val_5 = aria::NanBox::FalseValue;

    aria::Value val_6 = aria::NanBox::fromNumber(123.456);

    auto str2 = aria::newObjString("oiyuweryuoiew\t\t\t", gc);

    aria::Value val_7 = aria::NanBox::fromObj(str2);

    aria::Value val_8 = aria::NanBox::fromObj(aria::newObjException(str2, gc));

    arr.push(val_1);
    arr.push(val_2);
    arr.push(val_3);
    arr.push(val_4);
    arr.push(val_5);
    arr.push(val_6);
    arr.push(val_7);
    arr.push(val_8);

    EXPECT_EQ(arr.size(), 8);

    EXPECT_EQ(arr[0], val_1);
    EXPECT_EQ(arr[1], val_2);
    EXPECT_EQ(arr[2], val_3);
    EXPECT_EQ(arr[3], val_4);
    EXPECT_EQ(arr[4], val_5);
    EXPECT_EQ(arr[5], val_6);
    EXPECT_EQ(arr[6], val_7);
    EXPECT_EQ(arr[7], val_8);

    EXPECT_EQ(arr.pop(), val_8);
    EXPECT_EQ(arr.size(), 7);
    EXPECT_EQ(arr.pop(), val_7);
    EXPECT_EQ(arr.size(), 6);
}

TEST_F(ValueTestFixture, ValueArrayOperation2)
{
    aria::ValueArray arr = aria::ValueArray(gc);
    int begin = 10;
    int end = 1000;
    for (int i = begin; i < end; i++) {
        arr.push(aria::NanBox::fromNumber(static_cast<double>(i) / 10));
        EXPECT_EQ(arr.size(), i - begin + 1);
    }
    for (int i = begin; i < end; i++) {
        aria::Value val_i = arr[i - begin];
        EXPECT_TRUE(aria::NanBox::isNumber(val_i));
        EXPECT_EQ(aria::NanBox::toNumber(val_i), static_cast<double>(i) / 10);
    }

    const int arr_size = arr.size();
    for (int i = 0; i < arr.size(); i++) {
        aria::Value val_i = arr.pop();
        EXPECT_EQ(arr.size(), arr_size - i - 1);
        EXPECT_TRUE(aria::NanBox::isNumber(val_i));
        EXPECT_EQ(aria::NanBox::toNumber(val_i), static_cast<double>(end - i - 1) / 10);
    }
}

TEST_F(ValueTestFixture, ValueArrayOperation3)
{
    aria::ValueArray arr = aria::ValueArray(gc);
    arr.push(aria::NanBox::fromNumber(123.456));
    arr.push(aria::NanBox::fromNumber(-123.456));
    arr.push(aria::NanBox::TrueValue);
    arr.push(aria::NanBox::FalseValue);
    arr.push(aria::NanBox::NilValue);
    EXPECT_EQ(arr.size(), 5);

    std::ostringstream oss;
    aria::print(oss, "{}", arr.toString());
    EXPECT_STREQ(oss.str().c_str(), "[123.456,-123.456,true,false,nil]");
}