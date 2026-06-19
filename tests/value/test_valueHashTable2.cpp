#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/object/objList.h"
#include "src/object/objString.h"
#include "src/util/util.h"
#include "src/value/valueArray.h"
#include "src/value/valueHashTable.h"

using namespace aria;

class ValueHashTableTest : public ValueTestFixture
{
};

// 空表操作
TEST_F(ValueHashTableTest, EmptyTable)
{
    ValueHashTable table{gc};
    EXPECT_EQ(table.size(), 0);
    EXPECT_TRUE(table.empty());

    Value v = NanBox::NilValue;
    EXPECT_FALSE(table.get(NanBox::fromNumber(1), v));
    EXPECT_FALSE(table.has(NanBox::fromNumber(1)));
    EXPECT_FALSE(table.remove(NanBox::fromNumber(1)));
}

// 重复 key 覆盖
TEST_F(ValueHashTableTest, DuplicateKeyOverwrite)
{
    ValueHashTable table{gc};
    auto key = NanBox::fromObj(new_ObjString("key", gc));

    EXPECT_TRUE(table.insert(key, NanBox::fromNumber(1)));
    EXPECT_EQ(table.size(), 1);

    // 重复插入 → 覆盖，size 不变
    EXPECT_FALSE(table.insert(key, NanBox::fromNumber(2)));
    EXPECT_EQ(table.size(), 1);

    Value v = NanBox::NilValue;
    EXPECT_TRUE(table.get(key, v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 2.0);
}

// 缺失 key 的 get/has/remove
TEST_F(ValueHashTableTest, MissingKey)
{
    ValueHashTable table{gc};
    table.insert(NanBox::fromObj(new_ObjString("a", gc)), NanBox::fromNumber(1));

    auto missing = NanBox::fromObj(new_ObjString("b", gc));
    Value v = NanBox::NilValue;
    EXPECT_FALSE(table.get(missing, v));
    EXPECT_FALSE(table.has(missing));
    EXPECT_FALSE(table.remove(missing));
    EXPECT_EQ(table.size(), 1);
}

// 删除后重新插入
TEST_F(ValueHashTableTest, RemoveThenReinsert)
{
    ValueHashTable table{gc};
    auto key = NanBox::fromObj(new_ObjString("k", gc));

    table.insert(key, NanBox::fromNumber(10));
    EXPECT_TRUE(table.remove(key));
    EXPECT_TRUE(table.empty());

    // 重新插入同一个 key
    table.insert(key, NanBox::fromNumber(20));
    EXPECT_EQ(table.size(), 1);
    Value v = NanBox::NilValue;
    EXPECT_TRUE(table.get(key, v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 20.0);
}

// 删除墓碑槽后，重插一个因哈希冲突被挤到后续槽的 key，
// find_position 必须穿透墓碑继续探测到已有槽，而非在墓碑处重复插入。
// 回归 bug：find_position 遇 k_deleted 立即返回，导致同 key 出现在两个槽、
// count_ 虚高、后续 remove 残留孤儿条目。
TEST_F(ValueHashTableTest, ReinsertCollidedKeyAfterTombstone)
{
    ValueHashTable table{gc};

    // 搜索两个 number key，使其 h1 对 capacity=8 取模相同（哈希冲突，线性探测挤位）。
    // 插入首个 key 后 capacity 由 0 扩到 8。
    double key_a = -1;
    double key_b = -1;
    bool found = false;
    for (int i = 0; i < 100000 && !found; ++i) {
        auto a = NanBox::fromNumber(i);
        for (int j = i + 1; j < 100000 && !found; ++j) {
            auto b = NanBox::fromNumber(j);
            if ((value_hash(a) & 7u) == (value_hash(b) & 7u)) {
                key_a = i;
                key_b = j;
                found = true;
            }
        }
    }
    ASSERT_TRUE(found) << "未找到哈希冲突的 number key";

    auto a = NanBox::fromNumber(key_a);
    auto b = NanBox::fromNumber(key_b);

    // a 落首选槽，b 冲突挤到下一槽。
    table.insert(a, NanBox::fromNumber(1));
    table.insert(b, NanBox::fromNumber(2));
    EXPECT_EQ(table.size(), 2);

    // 删除 a：其首选槽变为墓碑，b 仍在后续槽。
    EXPECT_TRUE(table.remove(a));
    EXPECT_EQ(table.size(), 1);

    // 重新插入 b：旧实现在墓碑槽重复插入 b，size 错误地变为 2。
    // 修复后应识别 b 已存在，原地更新，size 保持 1。
    EXPECT_FALSE(table.insert(b, NanBox::fromNumber(3)));
    EXPECT_EQ(table.size(), 1);

    Value v = NanBox::NilValue;
    EXPECT_TRUE(table.get(b, v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 3.0);
}

// copy 合并
TEST_F(ValueHashTableTest, CopyMergesEntries)
{
    ValueHashTable src{gc};
    ValueHashTable dst{gc};

    src.insert(NanBox::fromObj(new_ObjString("a", gc)), NanBox::fromNumber(1));
    src.insert(NanBox::fromObj(new_ObjString("b", gc)), NanBox::fromNumber(2));

    dst.insert(NanBox::fromObj(new_ObjString("c", gc)), NanBox::fromNumber(3));

    dst.copy(&src);
    EXPECT_EQ(dst.size(), 3);

    Value v = NanBox::NilValue;
    EXPECT_TRUE(dst.get(NanBox::fromObj(new_ObjString("a", gc)), v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 1.0);
    EXPECT_TRUE(dst.get(NanBox::fromObj(new_ObjString("b", gc)), v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 2.0);
    EXPECT_TRUE(dst.get(NanBox::fromObj(new_ObjString("c", gc)), v));
    EXPECT_DOUBLE_EQ(NanBox::toNumber(v), 3.0);
}

// equals
TEST_F(ValueHashTableTest, Equals)
{
    ValueHashTable a{gc};
    ValueHashTable b{gc};

    a.insert(NanBox::fromObj(new_ObjString("x", gc)), NanBox::fromNumber(10));
    a.insert(NanBox::fromObj(new_ObjString("y", gc)), NanBox::fromNumber(20));

    b.insert(NanBox::fromObj(new_ObjString("y", gc)), NanBox::fromNumber(20));
    b.insert(NanBox::fromObj(new_ObjString("x", gc)), NanBox::fromNumber(10));

    EXPECT_TRUE(a.equals(&b));

    // 不等的情况
    ValueHashTable c{gc};
    c.insert(NanBox::fromObj(new_ObjString("x", gc)), NanBox::fromNumber(99));
    EXPECT_FALSE(a.equals(&c));
}

// to_string
TEST_F(ValueHashTableTest, ToString)
{
    ValueHashTable table{gc};
    EXPECT_EQ(table.to_string(), "{}");

    table.insert(NanBox::fromNumber(1), NanBox::fromNumber(2));
    String s = table.to_string();
    // 应包含 key 和 value 的表示
    EXPECT_NE(s.find(":"), String::npos);
    EXPECT_NE(s.find("{"), String::npos);
    EXPECT_NE(s.find("}"), String::npos);
}

// get_next_index 迭代器
TEST_F(ValueHashTableTest, IteratorProtocol)
{
    ValueHashTable table{gc};
    table.insert(NanBox::fromNumber(1), NanBox::fromNumber(10));
    table.insert(NanBox::fromNumber(2), NanBox::fromNumber(20));

    int count = 0;
    for (int64_t idx = table.get_next_index(-1); idx != -2; idx = table.get_next_index(idx)) {
        EXPECT_GE(idx, 0);
        count++;
    }
    EXPECT_EQ(count, 2);
}

// 空表迭代
TEST_F(ValueHashTableTest, EmptyTableIteration)
{
    ValueHashTable table{gc};
    EXPECT_EQ(table.get_next_index(-1), -2);
}

// 使用 number key 的混合类型
TEST_F(ValueHashTableTest, MixedKeyTypes)
{
    ValueHashTable table{gc};
    auto strKey = NanBox::fromObj(new_ObjString("hello", gc));
    auto numKey = NanBox::fromNumber(42);
    auto boolKey = NanBox::TrueValue;

    table.insert(strKey, NanBox::fromNumber(1));
    table.insert(numKey, NanBox::fromNumber(2));
    table.insert(boolKey, NanBox::fromNumber(3));
    EXPECT_EQ(table.size(), 3);

    Value v = NanBox::NilValue;
    EXPECT_TRUE(table.get(strKey, v));
    EXPECT_TRUE(table.get(numKey, v));
    EXPECT_TRUE(table.get(boolKey, v));
}

// 大量插入触发多次扩容
TEST_F(ValueHashTableTest, ManyInsertions)
{
    ValueHashTable table{gc};
    constexpr int N = 500;

    for (int i = 0; i < N; i++) {
        table.insert(NanBox::fromNumber(i), NanBox::fromNumber(i * 2));
    }
    EXPECT_EQ(table.size(), N);

    for (int i = 0; i < N; i++) {
        Value v = NanBox::NilValue;
        EXPECT_TRUE(table.get(NanBox::fromNumber(i), v));
        EXPECT_DOUBLE_EQ(NanBox::toNumber(v), i * 2.0);
    }
}

// clear
TEST_F(ValueHashTableTest, Clear)
{
    ValueHashTable table{gc};
    table.insert(NanBox::fromNumber(1), NanBox::fromNumber(2));
    table.insert(NanBox::fromNumber(3), NanBox::fromNumber(4));
    EXPECT_EQ(table.size(), 2);

    table.clear();
    EXPECT_EQ(table.size(), 0);
    EXPECT_TRUE(table.empty());
}

// create_key_list / create_value_list / create_pair_list
TEST_F(ValueHashTableTest, CreateLists)
{
    ValueHashTable table{gc};
    table.insert(NanBox::fromNumber(1), NanBox::fromNumber(10));
    table.insert(NanBox::fromNumber(2), NanBox::fromNumber(20));

    ObjList *keys = table.create_key_list();
    EXPECT_EQ(keys->list_->size(), 2);

    ObjList *values = table.create_value_list();
    EXPECT_EQ(values->list_->size(), 2);

    ObjList *pairs = table.create_pair_list();
    EXPECT_EQ(pairs->list_->size(), 2);
    // 每个 pair 是一个 [key, value] 的 list
    for (uint32_t i = 0; i < pairs->list_->size(); i++) {
        auto pairVal = (*pairs->list_)[i];
        EXPECT_TRUE(is_obj_list(pairVal));
        auto pairList = as_obj_list(pairVal);
        EXPECT_EQ(pairList->list_->size(), 2);
    }
}
