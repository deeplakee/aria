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
    aria::Value val_1 = aria::obj_val(str1);

    auto e1 = aria::newObjException(msg1, gc);
    aria::Value val_2 = aria::obj_val(e1);

    aria::Value val_3 = aria::nil_val;

    aria::Value val_4 = aria::true_val;

    aria::Value val_5 = aria::false_val;

    aria::Value val_6 = aria::number_val(123.456);

    auto str2 = aria::newObjString("oiyuweryuoiew\t\t\t", gc);

    aria::Value val_7 = aria::obj_val(str2);

    aria::Value val_8 = aria::obj_val(aria::newObjException(str2, gc));

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
        arr.push(aria::number_val(static_cast<double>(i) / 10));
        EXPECT_EQ(arr.size(), i - begin + 1);
    }
    for (int i = begin; i < end; i++) {
        aria::Value val_i = arr[i - begin];
        EXPECT_TRUE(aria::is_number(val_i));
        EXPECT_EQ(aria::as_number(val_i), static_cast<double>(i) / 10);
    }

    const int arr_size = arr.size();
    for (int i = 0; i < arr.size(); i++) {
        aria::Value val_i = arr.pop();
        EXPECT_EQ(arr.size(), arr_size - i - 1);
        EXPECT_TRUE(aria::is_number(val_i));
        EXPECT_EQ(aria::as_number(val_i), static_cast<double>(end - i - 1) / 10);
    }
}

TEST_F(ValueTestFixture, ValueArrayOperation3)
{
    aria::ValueArray arr = aria::ValueArray(gc);
    arr.push(aria::number_val(123.456));
    arr.push(aria::number_val(-123.456));
    arr.push(aria::true_val);
    arr.push(aria::false_val);
    arr.push(aria::nil_val);
    EXPECT_EQ(arr.size(), 5);

    std::ostringstream oss;
    aria::print(oss, "{}", arr.toString());
    EXPECT_STREQ(oss.str().c_str(), "[123.456,-123.456,true,false,nil]");
}