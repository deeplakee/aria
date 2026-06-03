#include "value/valueHashTable.h"
#include "memory/gc.h"
#include "object/objList.h"
#include "value/valueArray.h"

namespace aria {
ValueHashTable::ValueHashTable(GC *gc)
    : count_{0}
    , used_{0}
    , capacity_{0}
    , entry_{nullptr}
    , ctrl_{nullptr}
    , gc_{gc}
{}

ValueHashTable::ValueHashTable(const Value *values, uint32_t count, GC *gc)
    : count_{0}
    , used_{0}
    , capacity_{0}
    , entry_{nullptr}
    , ctrl_{nullptr}
    , gc_{gc}
{
    if (count > 0 && values == nullptr) {
        fatal_error(
            ErrorCode::RESOURCE_MAP_CONSTRUCT_FAIL,
            "ValueHashTable constructor error: values is null but count > 0.");
    }
    if (values != nullptr) {
        for (uint32_t i = 0; i < count; ++i) {
            auto k = values[2 * i];
            auto v = values[2 * i + 1];
            insert(k, v);
        }
    }
}

ValueHashTable::~ValueHashTable()
{
    gc_->free_array<KVPair>(entry_, capacity_);
    gc_->free_array<uint8_t>(ctrl_, capacity_);
}

bool ValueHashTable::insert(Value k, Value v)
{
    if (used_ + 1 > capacity_ * k_table_max_load) {
        uint64_t new_capacity = GC::grow_capacity(capacity_);
        if (new_capacity > UINT32_MAX) {
            fatal_error(ErrorCode::RESOURCE_MAP_OVERFLOW, "Too many values in a map");
        }
        adjust_capacity(new_capacity);
    }
    uint32_t hash = value_hash(k);
    uint32_t dest_index = find_position(k, hash);
    const bool not_full = ctrl_not_full(ctrl_[dest_index]);
    if (not_full) {
        count_++;
        if (ctrl_[dest_index] == k_empty) {
            used_++;
        }
    }

    ctrl_[dest_index] = get_hash_h2(hash);
    entry_[dest_index].key = k;
    entry_[dest_index].value = v;
    return not_full;
}

bool ValueHashTable::get(Value k, Value &v) const
{
    if (count_ == 0) {
        return false;
    }
    int64_t dest_index = find_exist(k);
    if (dest_index == -1) {
        return false;
    }
    v = entry_[dest_index].value;
    return true;
}

bool ValueHashTable::has(Value k) const
{
    if (count_ == 0) {
        return false;
    }
    int64_t dest_index = find_exist(k);
    if (dest_index == -1) {
        return false;
    }
    return true;
}

bool ValueHashTable::remove(Value k)
{
    if (count_ == 0) {
        return false;
    }

    int64_t dest_index = find_exist(k);
    if (dest_index == -1) {
        return false;
    }

    ctrl_[dest_index] = k_deleted;
    entry_[dest_index].key = NanBox::NilValue;
    entry_[dest_index].value = NanBox::NilValue;
    count_--;
    return true;
}

void ValueHashTable::copy(const ValueHashTable *other)
{
    auto other_entry = other->entry_;
    auto other_ctrl = other->ctrl_;
    for (int i = 0; i < other->capacity_; i++) {
        if (ctrl_not_full(other_ctrl[i])) {
            continue;
        }
        insert(other_entry[i].key, other_entry[i].value);
    }
}

bool ValueHashTable::equals(const ValueHashTable *other) const
{
    if (count_ != other->count_) {
        return false;
    }
    for (int i = 0; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        Value v = NanBox::NilValue;
        if (!other->get(entry_[i].key, v)) {
            return false;
        }
        if (!values_equal(entry_[i].value, v)) {
            return false;
        }
    }
    return true;
}

void ValueHashTable::clear()
{
    gc_->free_array<KVPair>(entry_, capacity_);
    gc_->free_array<uint8_t>(ctrl_, capacity_);
    count_ = 0;
    used_ = 0;
    capacity_ = 0;
    entry_ = nullptr;
    ctrl_ = nullptr;
}

String ValueHashTable::to_string() const
{
    String str;
    str.reserve(count_ * 6);
    str += "{";
    bool first = true;
    for (int i = 0; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        if (!first) {
            str += ",";
        }
        first = false;
        str += value_representation(entry_[i].key);
        str += ":";
        str += value_representation(entry_[i].value);
    }
    str += "}";
    return str;
}

int64_t ValueHashTable::find_exist(Value key) const
{
    uint32_t hash = value_hash(key);
    uint8_t h2 = get_hash_h2(hash);
    uint32_t index = hash & (capacity_ - 1);

    for (;;) {
        auto c = ctrl_[index];
        if (c == k_empty) {
            return -1;
        }
        if (c == k_deleted) {
            // pass
        } else if (c == h2 && values_same(entry_[index].key, key)) {
            return index;
        }
        index = (index + 1) & (capacity_ - 1);
    }
}

uint32_t ValueHashTable::find_position(Value key, uint32_t hash) const
{
    uint8_t h2 = get_hash_h2(hash);
    uint32_t index = hash & (capacity_ - 1);

    for (;;) {
        auto c = ctrl_[index];
        if (c == k_empty) {
            return index;
        }
        if (c == k_deleted) {
            return index;
        }
        if (c == h2 && values_same(entry_[index].key, key)) {
            return index;
        }
        index = (index + 1) & (capacity_ - 1);
    }
}

uint32_t ValueHashTable::find_new(
    const KVPair *h_entry, const uint8_t *h_ctrl, uint32_t h_capacity, Value key)
{
    uint32_t hash = value_hash(key);
    uint8_t h2 = get_hash_h2(hash);
    uint32_t index = hash & (h_capacity - 1);

    for (;;) {
        auto c = h_ctrl[index];
        if (c == k_empty) {
            return index;
        }
        if (c == k_deleted) {
            return index;
        }
        if (c == h2 && values_same(h_entry[index].key, key)) {
            return index;
        }
        index = (index + 1) & (h_capacity - 1);
    }
}

void ValueHashTable::adjust_capacity(const uint32_t new_capacity)
{
    auto *new_entry = gc_->allocate_array<KVPair>(new_capacity);
    auto *new_ctrl = gc_->allocate_array<uint8_t>(new_capacity);
    for (int i = 0; i < new_capacity; i++) {
        init_kv_pair(&new_entry[i]);
        new_ctrl[i] = k_empty;
    }

    used_ = 0;
    for (int i = 0; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        uint32_t dest_index = find_new(new_entry, new_ctrl, new_capacity, entry_[i].key);
        init_kv_pair(new_entry + dest_index, entry_[i].key, entry_[i].value);
        new_ctrl[dest_index] = ctrl_[i];
        used_++;
    }

    gc_->free_array<KVPair>(entry_, capacity_);
    gc_->free_array<uint8_t>(ctrl_, capacity_);
    entry_ = new_entry;
    ctrl_ = new_ctrl;
    capacity_ = new_capacity;
    count_ = used_;
}

void ValueHashTable::mark()
{
    for (int i = 0; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        mark_value(entry_[i].key);
        mark_value(entry_[i].value);
    }
}

int64_t ValueHashTable::get_next_index(const int64_t pre) const
{
    // -2 means reach the end
    if (pre == -2) {
        return -2;
    }
    // -1 means begin
    if (pre == -1) {
        for (int i = 0; i < capacity_; i++) {
            if (ctrl_not_full(ctrl_[i])) {
                continue;
            }
            return i;
        }
        return -2;
    }
    // Check if pre is within valid range
    if (pre < 0 || pre >= capacity_) {
        return -2;
    }
    for (auto i = pre + 1; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        return i;
    }
    return -2;
}

Value ValueHashTable::get_by_index(const int64_t index) const
{
    if (index < 0 || index >= capacity_) {
        return NanBox::NilValue;
    }
    ObjList *obj = create_pair(index);
    return NanBox::fromObj(obj);
}

ObjList *ValueHashTable::create_pair(const uint32_t index) const
{
    ObjList *list = new_ObjList(gc_);
    GcTempRootGuard guard{gc_, NanBox::fromObj(list)};
    list->list_->push(entry_[index].key);
    list->list_->push(entry_[index].value);
    return list;
}

template<typename F>
ObjList *ValueHashTable::collect_entries(F &&selector) const
{
    ObjList *list = new_ObjList(gc_);
    GcTempRootGuard guard{gc_, NanBox::fromObj(list)};
    list->list_->reserve(next_power_of_2(count_));
    for (uint32_t i = 0; i < capacity_; i++) {
        if (ctrl_not_full(ctrl_[i])) {
            continue;
        }
        selector(list, i);
    }
    return list;
}

ObjList *ValueHashTable::create_pair_list() const
{
    return collect_entries([this](ObjList *list, uint32_t i) {
        list->list_->push(NanBox::fromObj(create_pair(i)));
    });
}

ObjList *ValueHashTable::create_key_list() const
{
    return collect_entries([this](ObjList *list, uint32_t i) {
        list->list_->push(entry_[i].key);
    });
}

ObjList *ValueHashTable::create_value_list() const
{
    return collect_entries([this](ObjList *list, uint32_t i) {
        list->list_->push(entry_[i].value);
    });
}

} // namespace aria
