#include "memory/stringPool.h"
#include "error/error.h"
#include "memory/gc.h"
#include "object/objString.h"

namespace aria {

StringPool::StringPool(GC *_gc)
    : count{0}
    , capacity{0}
    , table{nullptr}
    , gc{_gc}
{}

StringPool::~StringPool()
{
    gc->free_array<ObjStringPtr>(table, capacity);
}

bool StringPool::insert(ObjString *obj)
{
    if (count + 1 > capacity * TABLE_MAX_LOAD) {
        uint64_t newCapacity = GC::grow_capacity(capacity);
        if (newCapacity > UINT32_MAX) {
            fatalError(ErrorCode::RESOURCE_STRING_POOL_FULL, "Aria string pool is full");
        }
        adjustCapacity(newCapacity);
    }
    ObjStringPtr *dest = findPosition(table, obj, capacity);

    if (*dest == obj) {
        return false;
    }

    *dest = obj;
    count++;
    return true;
}

ObjString *StringPool::findExist(const char *chars, size_t length, uint32_t hash)
{
    if (count == 0 || capacity == 0) {
        return nullptr;
    }

    uint32_t index = hash & (capacity - 1);
    for (;;) {
        ObjStringPtr *eachEntry = &table[index];
        if (*eachEntry == nullptr) {
            return nullptr;
        }
        if (*eachEntry == TOMBSTONE) {
            continue;
        }
        ObjStringPtr objStr = *eachEntry;
        if (objStr->length == length && objStr->hash == hash
            && memcmp(objStr->C_str_ref(), chars, length) == 0) {
            return *eachEntry;
        }

        index = (index + 1) & (capacity - 1);
    }
}

bool StringPool::remove(ObjString *obj)
{
    if (count == 0) {
        return false;
    }
    uint32_t index = obj->hash & (capacity - 1);
    for (;;) {
        ObjStringPtr *entry = &table[index];
        if (*entry == nullptr) {
            return false; // not found
        }
        if (*entry != TOMBSTONE) {
            ObjStringPtr candidate = *entry;
            if (candidate->length == obj->length && candidate->hash == obj->hash
                && memcmp(candidate->C_str_ref(), obj->C_str_ref(), obj->length) == 0) {
                *entry = TOMBSTONE;
                count--;
                return true;
            }
        }
        index = (index + 1) & (capacity - 1);
    }
}

ObjStringPtr *StringPool::findPosition(ObjStringPtr *s_table, ObjString *s, uint32_t table_capacity)
{
    uint32_t index = s->hash & (table_capacity - 1);

    for (;;) {
        ObjStringPtr *eachPtr = &s_table[index];
        if (*eachPtr == nullptr || *eachPtr == TOMBSTONE) {
            return eachPtr;
        }

        ObjStringPtr objStr = *eachPtr;
        if (objStr->length == s->length && objStr->hash == s->hash
            && memcmp(objStr->C_str_ref(), s->C_str_ref(), s->length) == 0) {
            return eachPtr;
        }
        index = (index + 1) & (table_capacity - 1);
    }
}

void StringPool::adjustCapacity(uint32_t newCapacity)
{
    ObjStringPtr *newTable = gc->allocate_array<ObjStringPtr>(newCapacity);
    for (uint32_t i = 0; i < newCapacity; i++) {
        newTable[i] = nullptr;
    }

    count = 0;
    for (uint32_t i = 0; i < capacity; i++) {
        ObjStringPtr src = table[i];
        if (src == nullptr || src == TOMBSTONE)
            continue;

        ObjStringPtr *dest = findPosition(newTable, src, newCapacity);
        *dest = src;
        count++;
    }

    gc->free_array<ObjStringPtr>(table, capacity);
    table = newTable;
    capacity = newCapacity;
}

} // namespace aria
