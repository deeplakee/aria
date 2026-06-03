#ifndef ARIA_VALUEHASHTABLE_H
#define ARIA_VALUEHASHTABLE_H

#include "value/value.h"

namespace aria {
class GC;
class ObjList;

struct KVPair
{
    Value key;
    Value value;
};

inline void init_kv_pair(KVPair *pair, Value key, Value value)
{
    pair->key = key;
    pair->value = value;
}

inline void init_kv_pair(KVPair *pair)
{
    pair->key = NanBox::NilValue;
    pair->value = NanBox::NilValue;
}

class ValueHashTable
{
public:
    ValueHashTable() = delete;

    explicit ValueHashTable(GC *gc);

    ValueHashTable(const Value *values, uint32_t count, GC *gc);

    ~ValueHashTable();

    bool insert(Value k, Value v);

    bool get(Value k, Value &v) const;

    [[nodiscard]] bool has(Value k) const;

    [[nodiscard]] int size() const { return count_; }

    [[nodiscard]] bool empty() const { return count_ == 0; }

    bool remove(Value k);

    void copy(const ValueHashTable *other);

    bool equals(const ValueHashTable *other) const;

    void clear();

    String to_string() const;

    void mark();

    int64_t get_next_index(int64_t pre) const;

    Value get_by_index(int64_t index) const;

    [[nodiscard]] ObjList *create_pair(uint32_t index) const;

    [[nodiscard]] ObjList *create_pair_list() const;

    [[nodiscard]] ObjList *create_key_list() const;

    [[nodiscard]] ObjList *create_value_list() const;

private:
    static constexpr double k_table_max_load = 0.75;
    static constexpr uint8_t k_empty = 0b10000000;
    static constexpr uint8_t k_deleted = 0b11111110;

    uint32_t count_;
    uint32_t used_;
    uint32_t capacity_;
    KVPair *entry_;
    uint8_t *ctrl_;
    GC *gc_;

    static uint8_t get_hash_h2(uint32_t h) { return (h >> 25) & 0x7F; }

    static uint32_t get_hash_h1(uint32_t h) { return h & 0x1FFFFFF; }

    static bool ctrl_is_full(uint8_t ctrl) { return (ctrl & 0b10000000) == 0; }

    static bool ctrl_not_full(uint8_t ctrl) { return (ctrl & 0b10000000) == 0b10000000; }

    int64_t find_exist(Value key) const;

    uint32_t find_position(Value key, uint32_t hash) const;

    static uint32_t find_new(
        const KVPair *h_entry, const uint8_t *h_ctrl, uint32_t h_capacity, Value key);

    void adjust_capacity(uint32_t new_capacity);

    template<typename F>
    ObjList *collect_entries(F &&selector) const;
};

} // namespace aria

#endif //ARIA_VALUEHASHTABLE_H
