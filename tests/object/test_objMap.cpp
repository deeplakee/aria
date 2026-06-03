#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objList.h"
#include "src/object/objMap.h"
#include "src/object/objString.h"
#include "src/value/valueArray.h"

using namespace aria;

class ObjMapTest : public ObjectTestFixture
{
};

// 创建空 map
TEST_F(ObjMapTest, CreateEmpty)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    EXPECT_TRUE(is_obj_map(NanBox::fromObj(map)));
    EXPECT_EQ(map->map_->size(), 0);
    EXPECT_TRUE(map->map_->empty());
}

// 从 Value 数组创建（key-value 交替）
TEST_F(ObjMapTest, CreateFromArray)
{
    Value vals[] = {
        NanBox::fromObj(new_ObjString("a", gc)),
        NanBox::fromNumber(1),
        NanBox::fromObj(new_ObjString("b", gc)),
        NanBox::fromNumber(2),
    };
    ObjMap *map = new_ObjMap(vals, 2, gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    EXPECT_EQ(map->map_->size(), 2);
}

// set_by_index / get_by_index（用实际 key）
TEST_F(ObjMapTest, SetGetByKey)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    auto key = NanBox::fromObj(new_ObjString("x", gc));
    map->set_by_index(key, NanBox::fromNumber(42));

    Value v = NanBox::NilValue;
    Value result = map->get_by_index(key, v);
    EXPECT_EQ(result, NanBox::TrueValue);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 42.0);
}

// get_by_index 缺失 key
TEST_F(ObjMapTest, GetMissingKey)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    auto missing = NanBox::fromObj(new_ObjString("nope", gc));

    Value v = NanBox::NilValue;
    Value result = map->get_by_index(missing, v);
    EXPECT_EQ(result, NanBox::FalseValue);
}

// to_string
TEST_F(ObjMapTest, ToString)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    map->map_->insert(NanBox::fromObj(new_ObjString("a", gc)), NanBox::fromNumber(1));

    String s = map->to_string();
    EXPECT_NE(s.find("{"), String::npos);
    EXPECT_NE(s.find("}"), String::npos);
}

// 空 map to_string
TEST_F(ObjMapTest, EmptyMapToString)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    String s = map->to_string();
    EXPECT_EQ(s, "{}");
}

// 内部 map 操作
TEST_F(ObjMapTest, InternalMapOperations)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    auto key1 = NanBox::fromObj(new_ObjString("a", gc));
    auto key2 = NanBox::fromObj(new_ObjString("b", gc));

    map->map_->insert(key1, NanBox::fromNumber(1));
    map->map_->insert(key2, NanBox::fromNumber(2));
    EXPECT_EQ(map->map_->size(), 2);

    Value v = NanBox::NilValue;
    EXPECT_TRUE(map->map_->get(key1, v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 1.0);

    EXPECT_TRUE(map->map_->remove(key1));
    EXPECT_EQ(map->map_->size(), 1);
}

// 混合 value 类型
TEST_F(ObjMapTest, MixedValueTypes)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    map->map_->insert(
        NanBox::fromObj(new_ObjString("num", gc)), NanBox::fromNumber(42));
    map->map_->insert(
        NanBox::fromObj(new_ObjString("bool", gc)), NanBox::TrueValue);
    map->map_->insert(
        NanBox::fromObj(new_ObjString("nil", gc)), NanBox::NilValue);
    map->map_->insert(
        NanBox::fromObj(new_ObjString("str", gc)),
        NanBox::fromObj(new_ObjString("hello", gc)));

    EXPECT_EQ(map->map_->size(), 4);
}

// 类型检查
TEST_F(ObjMapTest, TypeCheck)
{
    ObjMap *map = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(map)};
    Value v = NanBox::fromObj(map);
    EXPECT_TRUE(is_obj_map(v));
    EXPECT_FALSE(is_obj_list(v));
    EXPECT_TRUE(NanBox::isObj(v));
}
