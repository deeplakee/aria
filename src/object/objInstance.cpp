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

ObjInstance::ObjInstance(ObjClass *_klass, GC *_gc)
    : Obj{ObjType::INSTANCE, hashObj(this, ObjType::INSTANCE), _gc}
    , klass{_klass}
    , fields{_gc}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjInstance::~ObjInstance()
{
    delete cachedMethods;
}

String ObjInstance::toString(ValueStack *printStack)
{
    return format("<{} instance>", klass->name->C_str_ref());
}

String ObjInstance::representation(ValueStack *printStack)
{
    return format("<{} instance repr>", klass->name->C_str_ref());
}

void ObjInstance::blacken()
{
    klass->mark();
    fields.mark();
    cachedMethods->mark();
}

Value ObjInstance::getByField(ObjString *name, Value &value)
{
    if (fields.get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (cachedMethods->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (klass->methods.get(NanBox::fromObj(name), value)) {
        ObjBoundMethod *boundMethod = nullptr;
        if (isObjNativeFn(value)) {
            boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjNativeFn(value), gc);
        } else {
            boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjFunction(value), gc);
        }
        value = NanBox::fromObj(boundMethod);
        gc->cache(value);
        cachedMethods->insert(NanBox::fromObj(name), value);
        gc->releaseCache(1);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjInstance::setByField(ObjString *name, Value value)
{
    fields.insert(NanBox::fromObj(name), value);
    return NanBox::TrueValue;
}

Value ObjInstance::copy(GC *gc)
{
    ObjInstance *newObj = newObjInstance(klass, gc);
    gc->cache(NanBox::fromObj(newObj));
    newObj->fields.copy(&fields);
    gc->releaseCache(1);
    return NanBox::fromObj(newObj);
}

ObjInstance *newObjInstance(ObjClass *klass, GC *gc)
{
    auto *obj = gc->allocate_object<ObjInstance>(klass, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object INSTANCE)", toVoidPtr(obj), sizeof(ObjInstance));
#endif
    return obj;
}

} // namespace aria