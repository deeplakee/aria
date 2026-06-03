#include "memory/stringPool.h"
#include "error/error.h"
#include "memory/gc.h"
#include "object/objString.h"

#include <cstring>

namespace aria {

StringPool::StringPool(GC *gc)
    : count_{0}
    , capacity_{0}
    , table_{nullptr}
    , gc_{gc}
{}

StringPool::~StringPool()
{
    gc_->free_array<ObjStringPtr>(table_, capacity_);
}

bool StringPool::insert(ObjString *obj)
{
    if (count_ + 1 > capacity_ * k_table_max_load) {
        uint64_t new_capacity = GC::grow_capacity(capacity_);
        if (new_capacity > UINT32_MAX) {
            fatal_error(ErrorCode::RESOURCE_STRING_POOL_FULL, "Aria string pool is full");
        }
        adjust_capacity(new_capacity);
    }
    ObjStringPtr *dest = find_position(table_, obj, capacity_);

    if (*dest == obj) {
        return false;
    }

    *dest = obj;
    count_++;
    return true;
}

ObjString *StringPool::find_exist(const char *chars, size_t length, uint32_t hash)
{
    if (count_ == 0 || capacity_ == 0) {
        return nullptr;
    }

    uint32_t index = hash & (capacity_ - 1);
    for (;;) {
        ObjStringPtr *each_entry = &table_[index];
        if (*each_entry == nullptr) {
            return nullptr;
        }
        if (*each_entry == k_tombstone) {
            continue;
        }
        ObjStringPtr obj_str = *each_entry;
        if (obj_str->length_ == length && obj_str->hash_ == hash
            && memcmp(obj_str->c_str(), chars, length) == 0) {
            return *each_entry;
        }

        index = (index + 1) & (capacity_ - 1);
    }
}

bool StringPool::remove(ObjString *obj)
{
    if (count_ == 0) {
        return false;
    }
    uint32_t index = obj->hash_ & (capacity_ - 1);
    for (;;) {
        ObjStringPtr *entry = &table_[index];
        if (*entry == nullptr) {
            return false; // not found
        }
        if (*entry != k_tombstone) {
            ObjStringPtr candidate = *entry;
            if (candidate->length_ == obj->length_ && candidate->hash_ == obj->hash_
                && memcmp(candidate->c_str(), obj->c_str(), obj->length_) == 0) {
                *entry = k_tombstone;
                count_--;
                return true;
            }
        }
        index = (index + 1) & (capacity_ - 1);
    }
}

void StringPool::mark()
{
    for (auto i = 0; i < capacity_; i++) {
        if (table_[i] != nullptr && table_[i] != k_tombstone) {
            table_[i]->mark();
        }
    }
}

ObjStringPtr *StringPool::find_position(ObjStringPtr *s_table, ObjString *s, uint32_t table_capacity)
{
    uint32_t index = s->hash_ & (table_capacity - 1);

    for (;;) {
        ObjStringPtr *each_ptr = &s_table[index];
        if (*each_ptr == nullptr || *each_ptr == k_tombstone) {
            return each_ptr;
        }

        ObjStringPtr obj_str = *each_ptr;
        if (obj_str->length_ == s->length_ && obj_str->hash_ == s->hash_
            && memcmp(obj_str->c_str(), s->c_str(), s->length_) == 0) {
            return each_ptr;
        }
        index = (index + 1) & (table_capacity - 1);
    }
}

void StringPool::adjust_capacity(uint32_t new_capacity)
{
    ObjStringPtr *new_table = gc_->allocate_array<ObjStringPtr>(new_capacity);
    for (uint32_t i = 0; i < new_capacity; i++) {
        new_table[i] = nullptr;
    }

    count_ = 0;
    for (uint32_t i = 0; i < capacity_; i++) {
        ObjStringPtr src = table_[i];
        if (src == nullptr || src == k_tombstone)
            continue;

        ObjStringPtr *dest = find_position(new_table, src, new_capacity);
        *dest = src;
        count_++;
    }

    gc_->free_array<ObjStringPtr>(table_, capacity_);
    table_ = new_table;
    capacity_ = new_capacity;
}

} // namespace aria
