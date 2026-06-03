#include "value/valueStack.h"

#include <sstream>

namespace aria {

ValueStack::ValueStack()
    : ValueStack{k_default_stack_size}
{}

ValueStack::ValueStack(uint32_t size)
    : stack_{new Value[size]}
    , max_{size}
    , top_{0}
{}

ValueStack::~ValueStack()
{
    delete[] stack_;
}

bool ValueStack::exist(Value v)
{
    for (uint32_t slot = 0; slot < top_; slot++) {
        if (values_same(v, stack_[slot])) {
            return true;
        }
    }
    return false;
}

String ValueStack::to_string() const
{
    std::stringstream oss;
    for (int slot = 0; slot < top_; slot++) {
        print(oss, "[ {} ]", value_string(stack_[slot]));
    }
    return oss.str();
}

void ValueStack::display(uint32_t base, StringView current_fn_name)
{
    std::stringstream oss;
    oss << "stack:  ";
    uint64_t offset = 0;
    for (uint32_t slot = 0; slot < top_; slot++) {
        if (slot == base) {
            offset = oss.tellp();
        }
        print(oss, "[ {} ]", value_representation(stack_[slot]));
    }
    println("{}", oss.str());
    String arrow_line(offset, ' ');
    // println("{}↑ frame base", arrow_line);
    println("{}↑ frame base of {}", arrow_line, current_fn_name);
}

void ValueStack::mark()
{
    for (int i = 0; i < top_; i++) {
        mark_value(stack_[i]);
    }
}

} // namespace aria
