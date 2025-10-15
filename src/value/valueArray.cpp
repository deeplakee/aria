#include "value/valueArray.h"
#include "memory/gc.h"

#include <cstring>

namespace aria {
ValueArray::ValueArray(GC *_gc)
    : capacity{0}
    , count{0}
    , values{nullptr}
    , gc{_gc}
{}

ValueArray::ValueArray(const uint32_t begin, const uint32_t end, const ValueArray *other, GC *_gc)
    : capacity{0}
    , count{0}
    , values{nullptr}
    , gc{_gc}
{
    uint32_t size = end - begin;
    reserve(next_power_of_2(size));
    if (size > 0 && (other == nullptr || other->values == nullptr)) {
        fatalError(
            ErrorCode::RESOURCE_LIST_CONSTRUCT_FAIL,
            "ValueArray constructor error: other is null but size > 0.");
    }
    if (size != 0) {
        Value *from = other->values + begin;
        memcpy(values, from, size * sizeof(Value));
    }

    count = size;
}

ValueArray::ValueArray(Value *_values, uint32_t _count, GC *_gc)
    : capacity{0}
    , count{0}
    , values{nullptr}
    , gc{_gc}
{
    reserve(next_power_of_2(_count));
    if (_count > 0 && _values == nullptr) {
        fatalError(
            ErrorCode::RESOURCE_LIST_CONSTRUCT_FAIL,
            "ValueArray constructor error: _values is null but _count > 0.");
    }
    if (_count != 0) {
        memcpy(values, _values, _count * sizeof(Value));
    }
    count = _count;
}

ValueArray::~ValueArray()
{
    gc->free_array<Value>(values, capacity);
}

void ValueArray::push(Value value)
{
    if (capacity < count + 1) {
        uint64_t newCapacity = GC::grow_capacity(capacity);
        if (newCapacity > UINT32_MAX) {
            fatalError(ErrorCode::RESOURCE_LIST_OVERFLOW, "Too many values in a list");
        }
        reserve(newCapacity);
    }
    values[count] = value;
    count++;
}

Value ValueArray::pop()
{
    if (count > 0) {
        count--;
        return values[count];
    }
    return nil_val;
}

bool ValueArray::remove(uint32_t index)
{
    if (index > count) {
        return false;
    }
    count--;
    for (uint32_t i = index; i < count; i++) {
        values[i] = values[i + 1];
    }
    return true;
}

bool ValueArray::insert(uint32_t index, Value v)
{
    if (index > count) {
        return false;
    }
    if (capacity < count + 1) {
        reserve(GC::grow_capacity(capacity));
    }
    for (uint32_t i = count; i > index; i--) {
        values[i] = values[i - 1];
    }
    values[index] = v;
    count++;
    return true;
}

void ValueArray::extend(ValueArray *other)
{
    uint32_t newSize = next_power_of_2(count + other->count);
    reserve(newSize);
    memcpy(values + count, other->values, other->count * sizeof(Value));
    count = newSize;
}

void ValueArray::copy(ValueArray *other)
{
    reserve(other->capacity);
    memcpy(values, other->values, other->count * sizeof(Value));
    count = other->count;
}

void ValueArray::clear()
{
    count = 0;
    reserve(0);
}

void ValueArray::reverse()
{
    for (uint32_t i = 0; i < count / 2; i++) {
        swap(values[i], values[count - 1 - i]);
    }
}

uint32_t ValueArray::size() const
{
    return count;
}

void ValueArray::reserve(const uint32_t newCapacity)
{
    uint32_t oldCapacity = capacity;
    values = gc->grow_array<Value>(values, oldCapacity, newCapacity);
    capacity = newCapacity;
}

bool ValueArray::equals(ValueArray *other) const
{
    if (count != other->count) {
        return false;
    }
    for (uint32_t i = 0; i < count; i++) {
        if (!valuesEqual(values[i], other->values[i])) {
            return false;
        }
    }
    return true;
}

String ValueArray::toString(ValueStack *printStack) const
{
    String listStr = "[";
    if (count == 0) {
        listStr += "]";
    }
    for (uint32_t i = 0; i < count; i++) {
        Value elem = values[i];
        listStr += valueRepresentation(elem, printStack);
        if (i != count - 1) {
            listStr += ",";
        } else {
            listStr += "]";
        }
    }
    return listStr;
}

void ValueArray::mark()
{
    for (int i = 0; i < count; i++) {
        markValue(values[i]);
    }
}

} // namespace aria
