#include "object/objNativeFn.h"

#include "memory/gc.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueHashTable.h"

namespace aria {

ObjNativeFn::ObjNativeFn(
    FunctionType type,
    NativeFn_t function,
    ObjString *name,
    int arity,
    bool acceptsVarargs,
    GC *gc)
    : Obj{ObjType::NATIVE_FN, hash_obj(this, ObjType::NATIVE_FN), gc}
    , type_{type}
    , function_{function}
    , name_{name}
    , arity_{arity}
    , accepts_varargs_{acceptsVarargs}
{}

ObjNativeFn::~ObjNativeFn() = default;

String ObjNativeFn::to_string()
{
    return format("<nativeFn {}>", name_->c_str());
}

void ObjNativeFn::blacken()
{
    if (name_ != nullptr) {
        name_->mark();
    }
}

ObjNativeFn *new_ObjNativeFn(
    FunctionType type, NativeFn_t function, ObjString *name, int arity, bool acceptsVarargs, GC *gc)
{
    auto obj = gc->allocate_object<ObjNativeFn>(type, function, name, arity, acceptsVarargs, gc);
    log_obj_allocation(obj);
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
    GcTempRootGuard guard{gc};
    const Value name_val = NanBox::fromObj(new_ObjString(name, gc));
    guard.push(name_val);
    const Value method_val = NanBox::fromObj(new_ObjNativeFn(
        FunctionType::METHOD, fn, as_obj_string(name_val), arity, acceptsVarargs, gc));
    guard.push(method_val);
    methodTab->insert(name_val, method_val);
}

} // namespace aria
