#ifndef VALUE_H
#define VALUE_H

#include "common.h"
#include "value/nanBoxing.h"

namespace aria {

class ValueStack;

using Value = nan_boxing_t;

String valueTypeString(Value value);

String valueString(Value value, ValueStack *printStack = nullptr);

String valueRepresentation(Value value, ValueStack *printStack = nullptr);

inline bool valuesSame(Value a, Value b)
{
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    return a == b;
}

bool valuesEqual(Value a, Value b);

uint32_t valueHash(Value value);

void markValue(Value value);

inline bool isFalsey(Value value)
{
    return is_nil(value) || (is_bool(value) && !as_bool(value));
}

} // namespace aria

#endif // VALUE_H
