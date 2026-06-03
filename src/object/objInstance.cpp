#include "object/objInstance.h"

#include "memory/gc.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objFunction.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjInstance::ObjInstance(ObjClass *klass, GC *gc)
    : Obj{ObjType::INSTANCE, hash_obj(this, ObjType::INSTANCE), gc}
    , klass_{klass}
    , fields_{gc}
    , cached_methods_{gc}
{}

ObjInstance::~ObjInstance() = default;

String ObjInstance::to_string()
{
    return format("<{} instance>", klass_->name_->c_str());
}

String ObjInstance::representation()
{
    return format("<{} instance repr>", klass_->name_->c_str());
}

void ObjInstance::blacken()
{
    klass_->mark();
    fields_.mark();
    cached_methods_.mark();
}

Value ObjInstance::get_by_field(ObjString *name, Value &value)
{
    if (fields_.get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (cached_methods_.get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (klass_->methods_.get(NanBox::fromObj(name), value)) {
        ObjBoundMethod *boundMethod = nullptr;
        if (is_obj_native_fn(value)) {
            boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_native_fn(value), gc_);
        } else {
            boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_function(value), gc_);
        }
        value = NanBox::fromObj(boundMethod);
        GcTempRootGuard guard{gc_, value};
        cached_methods_.insert(NanBox::fromObj(name), value);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjInstance::set_by_field(ObjString *name, Value value)
{
    fields_.insert(NanBox::fromObj(name), value);
    return NanBox::TrueValue;
}

Value ObjInstance::copy(GC *gc)
{
    ObjInstance *newObj = new_ObjInstance(klass_, gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(newObj)};
    newObj->fields_.copy(&fields_);
    return NanBox::fromObj(newObj);
}

Value ObjInstance::getSuperMethod(ObjClass *methodKlass, ObjString *methodName, Value &superMethod)
{

    if (!methodKlass->getSuperMethod(methodName, superMethod)) [[unlikely]] {
        return NanBox::FalseValue;
    }
    ObjBoundMethod *boundMethod
        = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_function(superMethod), gc_);
    superMethod = NanBox::fromObj(boundMethod);
    return NanBox::TrueValue;
}

ObjInstance *new_ObjInstance(ObjClass *klass, GC *gc)
{
    auto obj = gc->allocate_object<ObjInstance>(klass, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
