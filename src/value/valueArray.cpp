#include "value/valueArray.h"
#include "memory/gc.h"

#include <cstring>

namespace aria {
ValueArray::ValueArray(GC *gc)
    : capacity_{0}
    , count_{0}
    , values_{nullptr}
    , gc_{gc}
{}

ValueArray::ValueArray(const uint32_t begin, const uint32_t end, const ValueArray *other, GC *gc)
    : capacity_{0}
    , count_{0}
    , values_{nullptr}
    , gc_{gc}
{
    uint32_t size = end - begin;
    reserve(next_power_of_2(size));
    if (size > 0 && (other == nullptr || other->values_ == nullptr)) {
        fatal_error(
            ErrorCode::RESOURCE_LIST_CONSTRUCT_FAIL,
            "ValueArray constructor error: other is null but size > 0.");
    }
    if (size != 0) {
        Value *from = other->values_ + begin;
        memcpy(values_, from, size * sizeof(Value));
    }

    count_ = size;
}

ValueArray::ValueArray(Value *values, uint32_t count, GC *gc)
    : capacity_{0}
    , count_{0}
    , values_{nullptr}
    , gc_{gc}
{
    reserve(next_power_of_2(count));
    if (count > 0 && values == nullptr) {
        fatal_error(
            ErrorCode::RESOURCE_LIST_CONSTRUCT_FAIL,
            "ValueArray constructor error: values is null but count > 0.");
    }
    if (count != 0) {
        memcpy(values_, values, count * sizeof(Value));
    }
    count_ = count;
}

ValueArray::~ValueArray()
{
    gc_->free_array<Value>(values_, capacity_);
}

void ValueArray::push(Value value)
{
    if (capacity_ < count_ + 1) {
        uint64_t new_capacity = GC::grow_capacity(capacity_);
        if (new_capacity > UINT32_MAX) {
            fatal_error(ErrorCode::RESOURCE_LIST_OVERFLOW, "Too many values in a list");
        }
        reserve(new_capacity);
    }
    values_[count_] = value;
    count_++;
}

Value ValueArray::pop()
{
    if (count_ > 0) {
        count_--;
        return values_[count_];
    }
    return NanBox::NilValue;
}

bool ValueArray::remove(uint32_t index)
{
    if (index > count_) {
        return false;
    }
    count_--;
    for (uint32_t i = index; i < count_; i++) {
        values_[i] = values_[i + 1];
    }
    return true;
}

bool ValueArray::insert(uint32_t index, Value v)
{
    if (index > count_) {
        return false;
    }
    if (capacity_ < count_ + 1) {
        reserve(GC::grow_capacity(capacity_));
    }
    for (uint32_t i = count_; i > index; i--) {
        values_[i] = values_[i - 1];
    }
    values_[index] = v;
    count_++;
    return true;
}

void ValueArray::extend(ValueArray *other)
{
    uint32_t new_count = count_ + other->count_;
    uint32_t new_capacity = next_power_of_2(new_count);
    reserve(new_capacity);
    memcpy(values_ + count_, other->values_, other->count_ * sizeof(Value));
    count_ = new_count;
}

void ValueArray::copy(ValueArray *other)
{
    reserve(other->capacity_);
    memcpy(values_, other->values_, other->count_ * sizeof(Value));
    count_ = other->count_;
}

void ValueArray::clear()
{
    count_ = 0;
    reserve(0);
}

void ValueArray::reverse()
{
    for (uint32_t i = 0; i < count_ / 2; i++) {
        swap(values_[i], values_[count_ - 1 - i]);
    }
}

uint32_t ValueArray::size() const
{
    return count_;
}

void ValueArray::reserve(const uint32_t new_capacity)
{
    values_ = gc_->resize_array<Value>(values_, capacity_, new_capacity);
    capacity_ = new_capacity;
}

bool ValueArray::equals(ValueArray *other) const
{
    if (count_ != other->count_) {
        return false;
    }
    for (uint32_t i = 0; i < count_; i++) {
        if (!values_equal(values_[i], other->values_[i])) {
            return false;
        }
    }
    return true;
}

String ValueArray::to_string() const
{
    String list_str = "[";
    if (count_ == 0) {
        list_str += "]";
    }
    for (uint32_t i = 0; i < count_; i++) {
        Value elem = values_[i];
        list_str += value_representation(elem);
        if (i != count_ - 1) {
            list_str += ",";
        } else {
            list_str += "]";
        }
    }
    return list_str;
}

void ValueArray::mark()
{
    for (int i = 0; i < count_; i++) {
        mark_value(values_[i]);
    }
}

} // namespace aria
