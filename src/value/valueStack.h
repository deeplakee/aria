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
        assert(top_ < k_default_stack_size);
#endif
        stack_[top_] = value;
        top_++;
    }

    Value pop()
    {
#ifdef DEBUG_MODE
        assert(top_ > 0);
#endif
        top_--;
        return stack_[top_];
    }

    void reset() { top_ = 0; }

    void pop_n(uint32_t count)
    {
#ifdef DEBUG_MODE
        assert(top_ >= count);
#endif
        top_ -= count;
    }

    Value peek(uint32_t depth = 0)
    {
#ifdef DEBUG_MODE
        assert(top_ > depth);
#endif
        return stack_[top_ - 1 - depth];
    }

    uint32_t size() const { return top_; }

    void resize(uint32_t new_size) { top_ = new_size; }

    Value &operator[](uint32_t index) { return stack_[index]; }

    const Value &operator[](uint32_t index) const { return stack_[index]; }

    Value *get_top_ptr() { return &stack_[top_]; }

    void set_top_val(Value v) { stack_[top_ - 1] = v; }

    Value *base() { return stack_; }

    bool exist(Value v);

    String to_string() const;

    void display(uint32_t base, StringView current_fn_name);

    void mark();

private:
    using stack_size_t = uint16_t;
    static constexpr int k_default_stack_size = UINT16_MAX;

    Value *stack_;
    uint32_t max_;
    uint32_t top_;
};
} // namespace aria

#endif //ARIA_VALUESTACK_H
