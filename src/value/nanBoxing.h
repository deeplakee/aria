#ifndef ARIA_NANBOXING_H
#define ARIA_NANBOXING_H

#include <bit>
#include <cstdint>

namespace aria {

class Obj;

namespace NanBox {

using NanBox_t = uint64_t;

//==============================
//  NaN Boxing 基本常量
//==============================

inline constexpr NanBox_t SignBit = 0x8000000000000000;
inline constexpr NanBox_t QNaN = 0x7ffc000000000000;
inline constexpr NanBox_t TagNil = 0x0000000000000001;
inline constexpr NanBox_t TagFalse = 0x0000000000000002;
inline constexpr uint64_t TagTrue = 0x0000000000000003;
inline constexpr NanBox_t TrueValue = QNaN | TagTrue;
inline constexpr NanBox_t FalseValue = QNaN | TagFalse;
inline constexpr NanBox_t NilValue = QNaN | TagNil;

//==============================
//  基本构造函数
//==============================

inline NanBox_t fromBool(bool b)
{
    return b ? TrueValue : FalseValue;
}

inline NanBox_t fromNumber(double num)
{
    return std::bit_cast<NanBox_t>(num);
}

inline NanBox_t fromObj(Obj *obj)
{
    return SignBit | QNaN | reinterpret_cast<uint64_t>(obj);
}

//==============================
//  取值函数
//==============================

inline bool toBool(NanBox_t v)
{
    return v == TrueValue;
}

inline double toNumber(NanBox_t v)
{
    return std::bit_cast<double>(v);
}

inline Obj *toObj(NanBox_t v)
{
    return reinterpret_cast<Obj *>(v & ~(SignBit | QNaN));
}

//==============================
//  类型检测
//==============================

inline bool isBool(NanBox_t v)
{
    return (v | 1) == TrueValue;
}

inline bool isTrue(NanBox_t v)
{
    return v == TrueValue;
}

inline bool isFalse(NanBox_t v)
{
    return v == FalseValue;
}

inline bool isNil(NanBox_t v)
{
    return v == NilValue;
}

inline bool isNumber(NanBox_t v)
{
    return (v & QNaN) != QNaN;
}

inline bool isObj(NanBox_t v)
{
    return (v & (QNaN | SignBit)) == (QNaN | SignBit);
}

} // namespace NanBox

} // namespace aria

#endif //ARIA_NANBOXING_H
