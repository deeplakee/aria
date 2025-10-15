#include "value/valueStack.h"

#include <sstream>

namespace aria {

ValueStack::ValueStack()
    : ValueStack{DEFAULT_STACK_SIZE}
{}

ValueStack::ValueStack(uint32_t size)
    : stack{new Value[size]}
    , max{size}
    , top{0}
{}

ValueStack::~ValueStack()
{
    delete[] stack;
}

bool ValueStack::Exist(Value v)
{
    for (uint32_t slot = 0; slot < top; slot++) {
        if (valuesSame(v, stack[slot])) {
            return true;
        }
    }
    return false;
}

String ValueStack::toString() const
{
    std::stringstream oss;
    for (int slot = 0; slot < top; slot++) {
        print(oss, "[ {} ]", valueString(stack[slot]));
    }
    return oss.str();
}

void ValueStack::display(uint32_t base, StringView currentFnName)
{
    std::stringstream oss;
    oss << "stack:  ";
    uint64_t offset = 0;
    for (uint32_t slot = 0; slot < top; slot++) {
        if (slot == base) {
            offset = oss.tellp();
        }
        print(oss, "[ {} ]", valueRepresentation(stack[slot]));
    }
    println("{}", oss.str());
    String arrowLine(offset, ' ');
    // println("{}↑ frame base", arrowLine);
    println("{}↑ frame base of {}", arrowLine, currentFnName);
}

void ValueStack::mark()
{
    for (int i = 0; i < top; i++) {
        markValue(stack[i]);
    }
}

} // namespace aria
