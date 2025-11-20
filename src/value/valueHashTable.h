#ifndef VALUEHASHTABLE_H
#define VALUEHASHTABLE_H

#include "object/objList.h"
#include "value/value.h"

namespace aria {
class GC;

struct KVPair
{
    Value key;
    Value value;
};

inline void initKVPair(KVPair *pair, Value _key, Value _value)
{
    pair->key = _key;
    pair->value = _value;
}

inline void initKVPair(KVPair *pair)
{
    pair->key = NanBox::NilValue;
    pair->value = NanBox::NilValue;
}

class ValueHashTable
{
public:
    ValueHashTable() = delete;

    explicit ValueHashTable(GC *_gc);

    ValueHashTable(const Value *_values, uint32_t _count, GC *_gc);

    ~ValueHashTable();

    bool insert(Value k, Value v);

    bool get(Value k, Value &v) const;

    [[nodiscard]] bool has(Value k) const;

    [[nodiscard]] int size() const { return count; }

    [[nodiscard]] bool empty() const { return count == 0; }

    bool remove(Value k);

    void copy(const ValueHashTable *other);

    bool equals(const ValueHashTable *other) const;

    void clear();

    String toString(ValueStack *printStack = nullptr) const;

    void mark();

    int64_t getNextIndex(int64_t pre) const;

    Value getByIndex(int64_t index) const;

    [[nodiscard]] ObjList *createPair(uint32_t index) const;

    [[nodiscard]] ObjList *createPairList() const;

    [[nodiscard]] ObjList *createKeyList() const;

    [[nodiscard]] ObjList *createValueList() const;

private:
    static constexpr double TABLE_MAX_LOAD = 0.75;
    static constexpr uint8_t kEmpty = 0b10000000;
    static constexpr uint8_t kDeleted = 0b11111110;

    uint32_t count;
    uint32_t used;
    uint32_t capacity;
    KVPair *entry;
    uint8_t *ctrl;
    GC *gc;

    static uint8_t getHashH2(uint32_t h) { return (h >> 25) & 0x7F; }

    static uint32_t getHashH1(uint32_t h) { return h & 0x1FFFFFF; }

    static bool ctrlIsFull(uint8_t ctrl) { return (ctrl & 0b10000000) == 0; }

    static bool ctrlNotFull(uint8_t ctrl) { return (ctrl & 0b10000000) == 0b10000000; }

    int64_t findExist(Value key) const;

    uint32_t findPosition(Value key, uint32_t hash) const;

    static uint32_t findNew(
        const KVPair *h_entry, const uint8_t *h_ctrl, uint32_t h_capacity, Value key);

    void adjustCapacity(uint32_t newCapacity);
};

} // namespace aria

#endif // VALUEHASHTABLE_H
