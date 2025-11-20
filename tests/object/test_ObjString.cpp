#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "object/objString.h"

TEST_F(ObjectTestFixture, ValidObjectString)
{
    const char *msg1 = "Hello World";
    auto str1 = aria::newObjString(msg1, gc);
    aria::Value str1_value = aria::NanBox::fromObj(str1);
    EXPECT_TRUE(aria::is_ObjString(str1_value));

    std::ostringstream oss;
    aria::print(oss, aria::as_ObjString(str1_value)->toString());
    EXPECT_STREQ(oss.str().c_str(), msg1);
}

TEST(ObjectTest, InvalidObjectString)
{
    aria::Value not_a_string = aria::NanBox::fromNumber(42);
    EXPECT_FALSE(aria::is_ObjString(not_a_string));
}

TEST_F(ObjectTestFixture, ObjectStringCreation)
{
    const char *msg1 = "Hello World";
    aria::String msg2 = aria::String{"Hello World"};
    char msg3[] = "Hello World";
    char msg4 = 'H';
    auto str1 = aria::newObjString(msg1, gc);
    auto str2 = aria::newObjString(msg2, gc);
    auto str3 = aria::newObjString(msg3, gc);
    auto str4 = aria::newObjString(msg4, gc);
    auto str5 = aria::newObjString(msg3, strlen(msg3), gc);

    EXPECT_TRUE(str1 != nullptr);
    EXPECT_TRUE(str2 != nullptr);
    EXPECT_TRUE(str3 != nullptr);
    EXPECT_TRUE(str4 != nullptr);
    EXPECT_TRUE(str5 != nullptr);

    EXPECT_EQ(str1, str2);
    EXPECT_EQ(str2, str3);
    EXPECT_EQ(str3, str5);

    EXPECT_TRUE(aria::valuesEqual(aria::NanBox::fromObj(str1), aria::NanBox::fromObj(str2)));
}