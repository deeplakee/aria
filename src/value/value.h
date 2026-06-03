#ifndef ARIA_VALUE_H
#define ARIA_VALUE_H

#include "common.h"
#include "value/nanBoxing.h"

namespace aria {

using Value = NanBox::NanBox_t;

String value_type_string(Value value);

String value_string(Value value);

String value_representation(Value value);

inline bool values_same(Value a, Value b)
{
    if (NanBox::isNumber(a) && NanBox::isNumber(b)) {
        return NanBox::toNumber(a) == NanBox::toNumber(b);
    }
    return a == b;
}

bool values_equal(Value a, Value b);

uint32_t value_hash(Value value);

void mark_value(Value value);

inline bool is_falsey(Value value)
{
    return NanBox::isNil(value) || (NanBox::isBool(value) && !NanBox::toBool(value));
}

} // namespace aria

#endif //ARIA_VALUE_H
