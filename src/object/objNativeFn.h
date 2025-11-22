#ifndef ARIA_OBJNATIVEFN_H
#define ARIA_OBJNATIVEFN_H

#include "object/funDef.h"
#include "object/object.h"

namespace aria {

class ValueHashTable;
class GC;
class ObjString;

class ObjNativeFn : public Obj
{
public:
    ObjNativeFn() = delete;

    ObjNativeFn(
        FunctionType _type,
        NativeFn_t _function,
        ObjString *_name,
        int _arity,
        bool _acceptsVarargs,
        GC *_gc);

    ~ObjNativeFn() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjNativeFn); }

    void blacken() override;

    FunctionType type;
    NativeFn_t function;
    ObjString *name;
    int arity;
    bool acceptsVarargs;
};

inline bool isObjNativeFn(Value value)
{
    return isObjType(value, ObjType::NATIVE_FN);
}

inline ObjNativeFn *asObjNativeFn(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjNativeFn *>(NanBox::toObj(value));
#else
    return static_cast<ObjNativeFn *>(NanBox::toObj(value));
#endif
}

inline NativeFn_t asNativeFn(Value value)
{
    return asObjNativeFn(value)->function;
}

ObjNativeFn *newObjNativeFn(
    FunctionType type, NativeFn_t function, ObjString *name, int arity, bool acceptsVarargs, GC *gc);

void bindBuiltinMethod(
    ValueHashTable *methodTab,
    const char *name,
    NativeFn_t fn,
    int arity,
    GC *gc,
    bool acceptsVarargs = false);

} // namespace aria

#endif //ARIA_OBJNATIVEFN_H
