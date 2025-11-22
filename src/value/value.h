#ifndef ARIA_VALUE_H
#define ARIA_VALUE_H

#include "common.h"
#include "value/nanBoxing.h"

namespace aria {

class ValueStack;

using Value = NanBox_t;

String valueTypeString(Value value);

String valueString(Value value, ValueStack *printStack = nullptr);

String valueRepresentation(Value value, ValueStack *printStack = nullptr);

inline bool valuesSame(Value a, Value b)
{
    if (NanBox::isNumber(a) && NanBox::isNumber(b)) {
        return NanBox::toNumber(a) == NanBox::toNumber(b);
    }
    return a == b;
}

bool valuesEqual(Value a, Value b);

uint32_t valueHash(Value value);

void markValue(Value value);

inline bool isFalsey(Value value)
{
    return NanBox::isNil(value) || (NanBox::isBool(value) && !NanBox::toBool(value));
}

} // namespace aria

#endif //ARIA_VALUE_H
