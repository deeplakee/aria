#ifndef ARIA_VALUEARRAY_H
#define ARIA_VALUEARRAY_H

#include "value/value.h"

namespace aria {

class ValueStack;

class GC;

class ValueArray
{
public:
    ValueArray() = delete;

    explicit ValueArray(GC *gc);

    // Constructs a new ValueArray by copying elements from another ValueArray within the specified range.
    ValueArray(uint32_t begin, uint32_t end, const ValueArray *other, GC *gc);

    ValueArray(Value *values, uint32_t count, GC *gc);

    ~ValueArray();

    Value &operator[](uint32_t index)
    {
#ifdef DEBUG_MODE
        assert(index < size());
#endif
        return values_[index];
    }

    const Value &operator[](uint32_t index) const
    {
#ifdef DEBUG_MODE
        assert(index < size());
#endif
        return values_[index];
    }

    void push(Value value);

    Value pop();

    bool remove(uint32_t index);

    bool insert(uint32_t index, Value v);

    void extend(ValueArray *other);

    void copy(ValueArray *other);

    void clear();

    void reverse();

    [[nodiscard]] uint32_t size() const;

    [[nodiscard]] bool empty() const { return count_ == 0; }

    void reserve(uint32_t new_capacity);

    bool equals(ValueArray *other) const;

    String to_string() const;

    void mark();

private:
    uint32_t capacity_;
    uint32_t count_;
    Value *values_;
    GC *gc_;
};
} // namespace aria

#endif //ARIA_VALUEARRAY_H
