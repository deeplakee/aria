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

    explicit ValueArray(GC *_gc);

    // Constructs a new ValueArray by copying elements from another ValueArray within the specified range.
    ValueArray(uint32_t begin, uint32_t end, const ValueArray *other, GC *_gc);

    ValueArray(Value *_values, uint32_t _count, GC *_gc);

    ~ValueArray();

    Value &operator[](uint32_t index)
    {
#ifdef DEBUG_MODE
        assert(index < size());
#endif
        return values[index];
    }

    const Value &operator[](uint32_t index) const
    {
#ifdef DEBUG_MODE
        assert(index < size());
#endif
        return values[index];
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

    [[nodiscard]] bool empty() const { return count == 0; }

    void reserve(uint32_t newCapacity);

    bool equals(ValueArray *other) const;

    String toString(ValueStack *printStack = nullptr) const;

    void mark();

private:
    uint32_t capacity;
    uint32_t count;
    Value *values;
    GC *gc;
};
} // namespace aria

#endif //ARIA_VALUEARRAY_H
