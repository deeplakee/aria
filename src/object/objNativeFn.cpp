#include "object/objNativeFn.h"

#include "memory/gc.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueHashTable.h"

namespace aria {

ObjNativeFn::ObjNativeFn(
    FunctionType _type,
    NativeFn_t _function,
    ObjString *_name,
    int _arity,
    bool _acceptsVarargs,
    GC *_gc)
    : Obj{ObjType::NATIVE_FN, hashObj(this, ObjType::NATIVE_FN), _gc}
    , type{_type}
    , function{_function}
    , name{_name}
    , arity{_arity}
    , acceptsVarargs{_acceptsVarargs}
{}

ObjNativeFn::~ObjNativeFn() = default;

String ObjNativeFn::toString(ValueStack *printStack)
{
    return format("<nativeFn {}>", name->C_str_ref());
}

void ObjNativeFn::blacken()
{
    if (name != nullptr) {
        name->mark();
    }
}

ObjNativeFn *newObjNativeFn(
    FunctionType type, NativeFn_t function, ObjString *name, int arity, bool acceptsVarargs, GC *gc)
{
    auto obj = gc->allocateObject<ObjNativeFn>(type, function, name, arity, acceptsVarargs, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object NATIVE_FN)", toVoidPtr(obj), sizeof(ObjNativeFn));
#endif
    return obj;
}

void bindBuiltinMethod(
    ValueHashTable *methodTab,
    const char *name,
    const NativeFn_t fn,
    const int arity,
    GC *gc,
    bool acceptsVarargs)
{
    const Value name_val = NanBox::fromObj(newObjString(name, gc));
    gc->pushTempRoot(name_val);
    const Value method_val = NanBox::fromObj(
        newObjNativeFn(FunctionType::METHOD, fn, asObjString(name_val), arity, acceptsVarargs, gc));
    gc->pushTempRoot(method_val);
    methodTab->insert(name_val, method_val);
    gc->popTempRoot(2);
}

} // namespace aria
