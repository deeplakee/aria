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
        FunctionType type,
        NativeFn_t function,
        ObjString *name,
        int arity,
        bool acceptsVarargs,
        GC *gc);

    ~ObjNativeFn() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjNativeFn); }

    void blacken() override;

    FunctionType type_;
    NativeFn_t function_;
    ObjString *name_;
    int arity_;
    bool accepts_varargs_;
};

inline bool is_obj_native_fn(Value value)
{
    return is_obj_type(value, ObjType::NATIVE_FN);
}

inline ObjNativeFn *as_obj_native_fn(Value value)
{
    return as_Obj<ObjNativeFn>(value);
}

inline NativeFn_t as_native_fn(Value value)
{
    return as_obj_native_fn(value)->function_;
}

ObjNativeFn *new_ObjNativeFn(
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
