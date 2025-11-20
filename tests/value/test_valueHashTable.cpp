#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objException.h"
#include "src/object/objString.h"
#include "src/util/util.h"
#include "value/valueHashTable.h"

#include <filesystem>

TEST_F(ValueTestFixture, ValueHashTableOperation1)
{
    constexpr auto factor = 1.75;
    auto map = aria::ValueHashTable{gc};

    for (int i = 0; i < 100; ++i) {
        auto str_i = newObjString(aria::format("{}", i), gc);
        map.insert(aria::NanBox::fromObj(str_i), aria::NanBox::fromNumber(i * factor));
    }

    EXPECT_EQ(map.size(), 100);

    for (int i = 0; i < 100; ++i) {
        auto str_i = newObjString(aria::format("{}", i), gc);
        aria::Value v = aria::NanBox::NilValue;
        bool result = map.get(aria::NanBox::fromObj(str_i), v);
        EXPECT_TRUE(result);
        EXPECT_EQ(v, aria::NanBox::fromNumber(i * factor));
    }

    for (int i = 0; i < 100; ++i) {
        auto str_i = newObjString(aria::format("{}", i), gc);
        bool result = map.remove(aria::NanBox::fromObj(str_i));
        EXPECT_TRUE(result);
        EXPECT_EQ(map.size(), 100 - i - 1);
    }
}