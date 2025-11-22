#ifndef ARIA_VALUESTACK_H
#define ARIA_VALUESTACK_H

#include "util/util.h"
#include "value/value.h"

namespace aria {
class GC;

class ValueStack
{
public:
    ValueStack();

    explicit ValueStack(uint32_t size);

    ~ValueStack();

    void push(Value value)
    {
#ifdef DEBUG_MODE
        assert(top < DEFAULT_STACK_SIZE);
#endif
        stack[top] = value;
        top++;
    }

    Value pop()
    {
#ifdef DEBUG_MODE
        assert(top > 0);
#endif
        top--;
        return stack[top];
    }

    void reset() { top = 0; }

    void pop_n(uint32_t count)
    {
#ifdef DEBUG_MODE
        assert(top >= count);
#endif
        top -= count;
    }

    Value peek(uint32_t depth = 0)
    {
#ifdef DEBUG_MODE
        assert(top > depth);
#endif
        return stack[top - 1 - depth];
    }

    uint32_t size() const { return top; }

    void resize(uint32_t newSize) { top = newSize; }

    Value &operator[](uint32_t index) { return stack[index]; }

    const Value &operator[](uint32_t index) const { return stack[index]; }

    Value *getTopPtr() { return &stack[top]; }

    void setTopVal(Value v) { stack[top - 1] = v; }

    Value *base() { return stack; }

    bool Exist(Value v);

    String toString() const;

    void display(uint32_t base, StringView currentFnName);

    void mark();

private:
    using stack_size_t = uint16_t;
    static constexpr int DEFAULT_STACK_SIZE = UINT16_MAX;

    Value *stack;
    uint32_t max;
    uint32_t top;
};
} // namespace aria

#endif //ARIA_VALUESTACK_H
