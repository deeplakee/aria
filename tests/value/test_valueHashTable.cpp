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
        map.insert(obj_val(str_i), aria::number_val(i * factor));
    }

    EXPECT_EQ(map.size(), 100);

    for (int i = 0; i < 100; ++i) {
        auto str_i = newObjString(aria::format("{}", i), gc);
        aria::Value v = aria::nil_val;
        bool result = map.get(aria::obj_val(str_i), v);
        EXPECT_TRUE(result);
        EXPECT_EQ(v, aria::number_val(i * factor));
    }

    for (int i = 0; i < 100; ++i) {
        auto str_i = newObjString(aria::format("{}", i), gc);
        bool result = map.remove(aria::obj_val(str_i));
        EXPECT_TRUE(result);
        EXPECT_EQ(map.size(), 100 - i - 1);
    }
}