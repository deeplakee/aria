#ifndef NANBOXING_H
#define NANBOXING_H

#include <cstdint>
#if __cplusplus >= 202002L // C++20 or higher version
#include <bit>
#endif

namespace aria {
// Forward declaration of class 'Obj'
// 'Obj' is the base class of all objects like string,function...
// To use 'nan_t', you need to define class 'Obj'
class Obj;

using nan_boxing_t = uint64_t;

#if __cplusplus >= 202002L // C++20 or higher version

inline nan_boxing_t numToValue(double num)
{
    return std::bit_cast<nan_boxing_t>(num);
}

inline double valueToNum(nan_boxing_t value)
{
    return std::bit_cast<double>(value);
}

#else // C++versions below 20

union Num_Value {
    double num;
    nan_t value;
};

inline nan_t numToValue(double num)
{
    Num_Value tmp;
    tmp.num = num;
    return tmp.value;
}

inline double valueToNum(nan_t value)
{
    Num_Value tmp;
    tmp.value = value;
    return tmp.num;
}

#endif

constexpr uint64_t sign_bit = 0x8000000000000000;
constexpr uint64_t qnan = 0x7ffc000000000000;
constexpr uint64_t tag_nil = 0x0000000000000001;
constexpr uint64_t tag_false = 0x0000000000000002;
constexpr uint64_t tag_true = 0x0000000000000003;

constexpr nan_boxing_t true_val = qnan | tag_true;
constexpr nan_boxing_t false_val = qnan | tag_false;
constexpr nan_boxing_t nil_val = qnan | tag_nil;

inline nan_boxing_t bool_val(bool b)
{
    return b ? true_val : false_val;
}

inline nan_boxing_t number_val(double num)
{
    return numToValue(num);
}

inline nan_boxing_t obj_val(Obj *obj)
{
    return sign_bit | qnan | reinterpret_cast<uint64_t>(obj);
}

inline bool as_bool(nan_boxing_t value)
{
    return value == true_val;
}

inline double as_number(nan_boxing_t value)
{
    return valueToNum(value);
}

inline Obj *as_obj(nan_boxing_t value)
{
    return reinterpret_cast<Obj *>((value) & ~(sign_bit | qnan));
}

inline bool is_bool(nan_boxing_t value)
{
    return (value | 1) == true_val;
}

inline bool is_true_val(nan_boxing_t value)
{
    return value == true_val;
}

inline bool is_false_val(nan_boxing_t value)
{
    return value == false_val;
}

inline bool is_nil(nan_boxing_t value)
{
    return value == nil_val;
}

inline bool is_number(nan_boxing_t value)
{
    return (value & qnan) != qnan;
}

inline bool is_obj(nan_boxing_t value)
{
    return (value & (qnan | sign_bit)) == (qnan | sign_bit);
}

} // namespace aria

#endif // NANBOXING_H
