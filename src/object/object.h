#ifndef ARIA_OBJECT_H
#define ARIA_OBJECT_H

#include "aria.h"
#include "common.h"
#include "value/value.h"

namespace aria {

class ObjString;
class GC;

enum class ObjType : uint8_t {
    BASE,
    STRING,
    FUNCTION,
    NATIVE_FN,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    LIST,
    MAP,
    MODULE,
    ITERATOR,
    EXCEPTION,
};

class Obj
{
public:
    Obj *next;
    GC *gc;
    uint32_t hash;
    ObjType type;
    bool isMarked;

    static const char *objTypeStr[];

    static const char *type2Str(ObjType t) { return objTypeStr[static_cast<uint8_t>(t)]; }

    Obj() = delete;

    Obj(ObjType _type, uint32_t _hash, GC *_gc)
        : next{nullptr}
        , gc{_gc}
        , hash{_hash}
        , type{_type}
        , isMarked{false}
    {}

    virtual ~Obj() = default;

    virtual String toString(ValueStack *printStack)
    {
        return valueTypeString(NanBox::fromObj(this));
    }

    String toString() { return toString(nullptr); }

    virtual String representation(ValueStack *printStack) { return this->toString(printStack); }

    String representation() { return this->representation(nullptr); }

    virtual size_t objSize() = 0;

    void mark();

    virtual void blacken() = 0;

    virtual Value op_call(AriaEnv *env, int argCount);

    virtual Value getByField(ObjString *name, Value &value) { return NanBox::FalseValue; }

    virtual Value setByField(ObjString *name, Value value) { return NanBox::FalseValue; }

    virtual Value getByIndex(Value k, Value &v) { return NanBox::FalseValue; }

    virtual Value setByIndex(Value k, Value v) { return NanBox::FalseValue; }

    virtual Value createIter(GC *gc) { return NanBox::NilValue; }

    virtual Value copy(GC *gc) { return NanBox::NilValue; }
};

inline ObjType getObjType(const Value value)
{
    return NanBox::toObj(value)->type;
}

inline bool isObjType(const Value value, const ObjType type)
{
    return NanBox::isObj(value) && getObjType(value) == type;
}

} // namespace aria

#endif //ARIA_OBJECT_H
