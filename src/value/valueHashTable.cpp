#include "value/valueHashTable.h"
#include "memory/gc.h"
#include "object/objList.h"
#include "value/valueArray.h"

namespace aria {
ValueHashTable::ValueHashTable(GC *_gc)
    : count{0}
    , used{0}
    , capacity{0}
    , entry{nullptr}
    , ctrl{nullptr}
    , gc{_gc}
{}

ValueHashTable::ValueHashTable(const Value *_values, uint32_t _count, GC *_gc)
    : count{0}
    , used{0}
    , capacity{0}
    , entry{nullptr}
    , ctrl{nullptr}
    , gc{_gc}
{
    if (_count > 0 && _values == nullptr) {
        fatalError(
            ErrorCode::RESOURCE_MAP_CONSTRUCT_FAIL,
            "ValueHashTable constructor error: _values is null but _count > 0.");
    }
    if (_values != nullptr) {
        for (uint32_t i = 0; i < _count; ++i) {
            auto k = _values[2 * i];
            auto v = _values[2 * i + 1];
            insert(k, v);
        }
    }
}

ValueHashTable::~ValueHashTable()
{
    gc->freeArray<KVPair>(entry, capacity);
    gc->freeArray<uint8_t>(ctrl, capacity);
}

bool ValueHashTable::insert(Value k, Value v)
{
    if (used + 1 > capacity * TABLE_MAX_LOAD) {
        uint64_t newCapacity = GC::growCapacity(capacity);
        if (newCapacity > UINT32_MAX) {
            fatalError(ErrorCode::RESOURCE_MAP_OVERFLOW, "Too many values in a map");
        }
        adjustCapacity(newCapacity);
    }
    uint32_t hash = valueHash(k);
    uint32_t destIndex = findPosition(k, hash);
    const bool notFull = ctrlNotFull(ctrl[destIndex]);
    if (notFull) {
        count++;
        if (ctrl[destIndex] == kEmpty) {
            used++;
        }
    }

    ctrl[destIndex] = getHashH2(hash);
    entry[destIndex].key = k;
    entry[destIndex].value = v;
    return notFull;
}

bool ValueHashTable::get(Value k, Value &v) const
{
    if (count == 0) {
        return false;
    }
    int64_t destIndex = findExist(k);
    if (destIndex == -1) {
        return false;
    }
    v = entry[destIndex].value;
    return true;
}

bool ValueHashTable::has(Value k) const
{
    if (count == 0) {
        return false;
    }
    int64_t destIndex = findExist(k);
    if (destIndex == -1) {
        return false;
    }
    return true;
}

bool ValueHashTable::remove(Value k)
{
    if (count == 0) {
        return false;
    }

    int64_t destIndex = findExist(k);
    if (destIndex == -1) {
        return false;
    }

    ctrl[destIndex] = kDeleted;
    entry[destIndex].key = NanBox::NilValue;
    entry[destIndex].value = NanBox::NilValue;
    count--;
    return true;
}

void ValueHashTable::copy(const ValueHashTable *other)
{
    auto otherEntry = other->entry;
    auto otherCtrl = other->ctrl;
    for (int i = 0; i < other->capacity; i++) {
        if (ctrlNotFull(otherCtrl[i])) {
            continue;
        }
        insert(otherEntry[i].key, otherEntry[i].value);
    }
}

bool ValueHashTable::equals(const ValueHashTable *other) const
{
    if (count != other->count) {
        return false;
    }
    for (int i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        Value v = NanBox::NilValue;
        if (!other->get(entry[i].key, v)) {
            return false;
        }
        if (!valuesEqual(entry[i].value, v)) {
            return false;
        }
    }
    return true;
}

void ValueHashTable::clear()
{
    gc->freeArray<KVPair>(entry, capacity);
    gc->freeArray<uint8_t>(ctrl, capacity);
    count = 0;
    used = 0;
    capacity = 0;
    entry = nullptr;
}

String ValueHashTable::toString(ValueStack *printStack) const
{
    String str;
    str.reserve(count * 6);
    str += "{";
    bool first = true;
    for (int i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        if (!first) {
            str += ",";
        }
        first = false;
        str += valueRepresentation(entry[i].key, printStack);
        str += ":";
        str += valueRepresentation(entry[i].value, printStack);
    }
    str += "}";
    return str;
    // 大表可以使用 ostringstream，避免频繁拷贝
}

int64_t ValueHashTable::findExist(Value key) const
{
    uint32_t hash = valueHash(key);
    uint8_t h2 = getHashH2(hash);
    uint32_t index = hash & (capacity - 1);

    for (;;) {
        auto c = ctrl[index];
        if (c == kEmpty) {
            return -1;
        }
        if (c == kDeleted) {
            // pass
        } else if (c == h2 && valuesSame(entry[index].key, key)) {
            return index;
        }
        index = (index + 1) & (capacity - 1);
    }
}

uint32_t ValueHashTable::findPosition(Value key, uint32_t hash) const
{
    uint8_t h2 = getHashH2(hash);
    uint32_t index = hash & (capacity - 1);

    for (;;) {
        auto c = ctrl[index];
        if (c == kEmpty) {
            return index;
        }
        if (c == kDeleted) {
            return index;
        }
        if (c == h2 && valuesSame(entry[index].key, key)) {
            return index;
        }
        index = (index + 1) & (capacity - 1);
    }
}

uint32_t ValueHashTable::findNew(
    const KVPair *h_entry, const uint8_t *h_ctrl, uint32_t h_capacity, Value key)
{
    uint32_t hash = valueHash(key);
    uint8_t h2 = getHashH2(hash);
    uint32_t index = hash & (h_capacity - 1);

    for (;;) {
        auto c = h_ctrl[index];
        if (c == kEmpty) {
            return index;
        }
        if (c == kDeleted) {
            return index;
        }
        if (c == h2 && valuesSame(h_entry[index].key, key)) {
            return index;
        }
        index = (index + 1) & (h_capacity - 1);
    }
}

void ValueHashTable::adjustCapacity(const uint32_t newCapacity)
{
    auto *newEntry = gc->allocateArray<KVPair>(newCapacity);
    auto *newCtrl = gc->allocateArray<uint8_t>(newCapacity);
    for (int i = 0; i < newCapacity; i++) {
        initKVPair(&newEntry[i]);
        newCtrl[i] = kEmpty;
    }

    used = 0;
    for (int i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        uint32_t destIndex = findNew(newEntry, newCtrl, newCapacity, entry[i].key);
        initKVPair(newEntry + destIndex, entry[i].key, entry[i].value);
        newCtrl[destIndex] = ctrl[i];
        used++;
    }

    gc->freeArray<KVPair>(entry, capacity);
    gc->freeArray<uint8_t>(ctrl, capacity);
    entry = newEntry;
    ctrl = newCtrl;
    capacity = newCapacity;
    count = used;
}

void ValueHashTable::mark()
{
    for (int i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        markValue(entry[i].key);
        markValue(entry[i].value);
    }
}

int64_t ValueHashTable::getNextIndex(const int64_t pre) const
{
    // -2 means reach the end
    if (pre == -2) {
        return -2;
    }
    // -1 means begin
    if (pre == -1) {
        for (int i = 0; i < capacity; i++) {
            if (ctrlNotFull(ctrl[i])) {
                continue;
            }
            return i;
        }
        return -2;
    }
    // Check if pre is within valid range
    if (pre < 0 || pre >= capacity) {
        return -2;
    }
    for (auto i = pre + 1; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        return i;
    }
    return -2;
}

Value ValueHashTable::getByIndex(const int64_t index) const
{
    if (index < 0 || index >= capacity) {
        return NanBox::NilValue;
    }
    ObjList *obj = createPair(index);
    return NanBox::fromObj(obj);
}

ObjList *ValueHashTable::createPair(const uint32_t index) const
{
    ObjList *list = newObjList(gc);
    gc->pushTempRoot(NanBox::fromObj(list));
    list->list->push(entry[index].key);
    list->list->push(entry[index].value);
    gc->popTempRoot(1);
    return list;
}

ObjList *ValueHashTable::createPairList() const
{
    ObjList *list = newObjList(gc);
    gc->pushTempRoot(NanBox::fromObj(list));
    list->list->reserve(nextPowerOf2(count));
    for (uint32_t i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        auto pair = NanBox::fromObj(createPair(i));
        list->list->push(pair);
    }
    gc->popTempRoot(1);
    return list;
}

ObjList *ValueHashTable::createKeyList() const
{
    ObjList *list = newObjList(gc);
    gc->pushTempRoot(NanBox::fromObj(list));
    list->list->reserve(nextPowerOf2(count));
    for (uint32_t i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        list->list->push(entry[i].key);
    }
    gc->popTempRoot(1);
    return list;
}

ObjList *ValueHashTable::createValueList() const
{
    ObjList *list = newObjList(gc);
    gc->pushTempRoot(NanBox::fromObj(list));
    list->list->reserve(nextPowerOf2(count));
    for (uint32_t i = 0; i < capacity; i++) {
        if (ctrlNotFull(ctrl[i])) {
            continue;
        }
        list->list->push(entry[i].value);
    }
    gc->popTempRoot(1);
    return list;
}

} // namespace aria
