#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/value/value.h"
#include "src/object/objException.h"
#include "src/object/objString.h"


TEST(ValueTest, ValueNumber)
{
    double num = 3.14159265358979323846;
    aria::Value num_value = aria::NanBox::fromNumber(num);
    EXPECT_TRUE(aria::NanBox::isNumber(num_value));

    EXPECT_DOUBLE_EQ(aria::NanBox::toNumber(num_value), num);
}

TEST(ValueTest, InvalidValueNumber)
{
    aria::Value not_a_number = aria::NanBox::fromBool(true);
    EXPECT_FALSE(aria::NanBox::isNumber(not_a_number));
}

TEST(ValueTest, ValueBool)
{
    auto bool1 = aria::NanBox::fromBool(true);
    auto bool2 = aria::NanBox::fromBool(false);
    EXPECT_TRUE(aria::NanBox::isBool(bool1));
    EXPECT_TRUE(aria::NanBox::isBool(bool2));

    EXPECT_EQ(aria::NanBox::fromBool(true), aria::NanBox::TrueValue);
    EXPECT_EQ(aria::NanBox::fromBool(false), aria::NanBox::FalseValue);
}

TEST(ValueTest, InvalidValueBool)
{
    aria::Value not_a_bool = aria::NanBox::fromNumber(42);
    EXPECT_FALSE(aria::NanBox::isBool(not_a_bool));
}

TEST(ValueTest, ValueNil)
{
    aria::Value is_nil_val = aria::NanBox::NilValue;
    EXPECT_TRUE(aria::NanBox::isNil(is_nil_val));
}

TEST(ValueTest, InvalidValueNil)
{
    aria::Value not_a_nil = aria::NanBox::fromNumber(42);
    EXPECT_FALSE(aria::NanBox::isNil((not_a_nil)));
}

TEST_F(ValueTestFixture, ValueObj)
{
    const char *msg1 = "this is a spark test for ValueObj";

    auto str1 = aria::newObjString(msg1, gc);
    aria::Value str1_value = aria::NanBox::fromObj(str1);
    EXPECT_TRUE(aria::NanBox::isObj(str1_value));

    auto e1 = aria::newObjException(msg1,gc);
    aria::Value e1_value = aria::NanBox::fromObj(e1);
    EXPECT_TRUE(aria::NanBox::isObj(e1_value));
}

TEST(ValueTest, InvalidValueObj)
{
    aria::Value not_a_obj = aria::NanBox::fromNumber(42);
    EXPECT_FALSE(aria::NanBox::isObj((not_a_obj)));
}