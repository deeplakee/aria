#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objList.h"
#include "src/object/objString.h"
#include "src/value/valueArray.h"

using namespace aria;

class ObjListTest : public ObjectTestFixture
{
};

// 创建空列表
TEST_F(ObjListTest, CreateEmpty)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    EXPECT_TRUE(is_obj_list(NanBox::fromObj(list)));
    EXPECT_EQ(list->list_->size(), 0);
    EXPECT_TRUE(list->list_->empty());
}

// 从 Value 数组创建
TEST_F(ObjListTest, CreateFromArray)
{
    Value vals[] = {NanBox::fromNumber(1), NanBox::fromNumber(2), NanBox::fromNumber(3)};
    ObjList *list = new_ObjList(vals, 3, gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    EXPECT_EQ(list->list_->size(), 3);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*list->list_)[0]), 1.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*list->list_)[2]), 3.0);
}

// push 元素
TEST_F(ObjListTest, PushElements)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    list->list_->push(NanBox::fromNumber(10));
    list->list_->push(NanBox::fromNumber(20));
    list->list_->push(NanBox::fromObj(new_ObjString("hello", gc)));

    EXPECT_EQ(list->list_->size(), 3);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*list->list_)[0]), 10.0);
    EXPECT_TRUE(is_obj_string((*list->list_)[2]));
}

// pop 元素
TEST_F(ObjListTest, PopElements)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    list->list_->push(NanBox::fromNumber(1));
    list->list_->push(NanBox::fromNumber(2));

    auto v = list->list_->pop();
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 2.0);
    EXPECT_EQ(list->list_->size(), 1);
}

// 切片构造
TEST_F(ObjListTest, SliceConstruction)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    list->list_->push(NanBox::fromNumber(10));
    list->list_->push(NanBox::fromNumber(20));
    list->list_->push(NanBox::fromNumber(30));
    list->list_->push(NanBox::fromNumber(40));

    ObjList *slice = new_ObjList(1, 3, list, gc);
    GcTempRootGuard guard2{gc, NanBox::fromObj(slice)};
    EXPECT_EQ(slice->list_->size(), 2);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*slice->list_)[0]), 20.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*slice->list_)[1]), 30.0);
}

// to_string
TEST_F(ObjListTest, ToString)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    list->list_->push(NanBox::fromNumber(1));
    list->list_->push(NanBox::fromNumber(2));
    list->list_->push(NanBox::fromNumber(3));

    String s = list->to_string();
    EXPECT_NE(s.find("["), String::npos);
    EXPECT_NE(s.find("]"), String::npos);
}

// 空列表 to_string
TEST_F(ObjListTest, EmptyListToString)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    String s = list->to_string();
    EXPECT_NE(s.find("["), String::npos);
    EXPECT_NE(s.find("]"), String::npos);
}

// 混合类型元素
TEST_F(ObjListTest, MixedTypes)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    list->list_->push(NanBox::fromNumber(1));
    list->list_->push(NanBox::TrueValue);
    list->list_->push(NanBox::NilValue);
    list->list_->push(NanBox::fromObj(new_ObjString("test", gc)));

    EXPECT_EQ(list->list_->size(), 4);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*list->list_)[0]), 1.0);
    EXPECT_EQ((*list->list_)[1], NanBox::TrueValue);
    EXPECT_TRUE(NanBox::isNil((*list->list_)[2]));
    EXPECT_TRUE(is_obj_string((*list->list_)[3]));
}

// 大量元素
TEST_F(ObjListTest, ManyElements)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    for (int i = 0; i < 1000; i++) {
        list->list_->push(NanBox::fromNumber(i));
    }
    EXPECT_EQ(list->list_->size(), 1000);
    EXPECT_DOUBLE_EQ(NanBox::toNumber((*list->list_)[999]), 999.0);
}

// is_obj_list 类型检查
TEST_F(ObjListTest, TypeCheck)
{
    ObjList *list = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(list)};
    Value v = NanBox::fromObj(list);
    EXPECT_TRUE(is_obj_list(v));
    EXPECT_FALSE(is_obj_string(v));
    EXPECT_TRUE(NanBox::isObj(v));
}
