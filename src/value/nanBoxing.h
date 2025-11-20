#ifndef NANBOXING_H
#define NANBOXING_H

#include <bit>
#include <cstdint>

namespace aria {
// Forward declaration of class 'Obj'
// 'Obj' is the base class of all objects like string,function...
// To use 'nan_t', you need to define class 'Obj'
class Obj;

using NanBox_t = uint64_t;

class NanBox
{
    //==============================
    //  NaN Boxing 基本常量
    //==============================
private:
    static constexpr NanBox_t SignBit = 0x8000000000000000;
    static constexpr NanBox_t QNaN = 0x7ffc000000000000;
    static constexpr NanBox_t TagNil = 0x0000000000000001;
    static constexpr NanBox_t TagFalse = 0x0000000000000002;
    static constexpr uint64_t TagTrue = 0x0000000000000003;

public:
    static constexpr NanBox_t TrueValue = QNaN | TagTrue;
    static constexpr NanBox_t FalseValue = QNaN | TagFalse;
    static constexpr NanBox_t NilValue = QNaN | TagNil;

    //==============================
    //  基本构造函数
    //==============================

    static NanBox_t fromBool(bool b) { return b ? TrueValue : FalseValue; }

    static NanBox_t fromNumber(double num) { return std::bit_cast<NanBox_t>(num); }

    static NanBox_t fromObj(Obj *obj) { return SignBit | QNaN | reinterpret_cast<uint64_t>(obj); }

    //==============================
    //  取值函数
    //==============================

    static bool toBool(NanBox_t v) { return v == TrueValue; }

    static double toNumber(NanBox_t v) { return std::bit_cast<double>(v); }

    static Obj *toObj(NanBox_t v) { return reinterpret_cast<Obj *>(v & ~(SignBit | QNaN)); }

    //==============================
    //  类型检测
    //==============================

    static bool isBool(NanBox_t v) { return (v | 1) == TrueValue; }

    static bool isTrue(NanBox_t v) { return v == TrueValue; }

    static bool isFalse(NanBox_t v) { return v == FalseValue; }

    static bool isNil(NanBox_t v) { return v == NilValue; }

    static bool isNumber(NanBox_t v) { return (v & QNaN) != QNaN; }

    static bool isObj(NanBox_t v) { return (v & (QNaN | SignBit)) == (QNaN | SignBit); }
};

} // namespace aria

#endif // NANBOXING_H
