#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/value/value.h"
#include "src/object/objException.h"
#include "src/object/objString.h"


TEST(ValueTest, ValueNumber)
{
    double num = 3.14159265358979323846;
    aria::Value num_value = aria::number_val(num);
    EXPECT_TRUE(aria::is_number(num_value));

    EXPECT_DOUBLE_EQ(aria::as_number(num_value), num);
}

TEST(ValueTest, InvalidValueNumber)
{
    aria::Value not_a_number = aria::bool_val(true);
    EXPECT_FALSE(aria::is_number(not_a_number));
}

TEST(ValueTest, ValueBool)
{
    auto bool1 = aria::bool_val(true);
    auto bool2 = aria::bool_val(false);
    EXPECT_TRUE(aria::is_bool(bool1));
    EXPECT_TRUE(aria::is_bool(bool2));

    EXPECT_EQ(aria::bool_val(true), aria::true_val);
    EXPECT_EQ(aria::bool_val(false), aria::false_val);
}

TEST(ValueTest, InvalidValueBool)
{
    aria::Value not_a_bool = aria::number_val(42);
    EXPECT_FALSE(aria::is_bool(not_a_bool));
}

TEST(ValueTest, ValueNil)
{
    aria::Value is_nil_val = aria::nil_val;
    EXPECT_TRUE(aria::is_nil(is_nil_val));
}

TEST(ValueTest, InvalidValueNil)
{
    aria::Value not_a_nil = aria::number_val(42);
    EXPECT_FALSE(aria::is_nil((not_a_nil)));
}

TEST_F(ValueTestFixture, ValueObj)
{
    const char *msg1 = "this is a spark test for ValueObj";

    auto str1 = aria::newObjString(msg1, gc);
    aria::Value str1_value = aria::obj_val(str1);
    EXPECT_TRUE(aria::is_obj(str1_value));

    auto e1 = aria::newObjException(msg1,gc);
    aria::Value e1_value = aria::obj_val(e1);
    EXPECT_TRUE(aria::is_obj(e1_value));
}

TEST(ValueTest, InvalidValueObj)
{
    aria::Value not_a_obj = aria::number_val(42);
    EXPECT_FALSE(aria::is_obj((not_a_obj)));
}