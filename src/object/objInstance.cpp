#include "object/objInstance.h"

#include "memory/gc.h"
#include "object/objClass.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjInstance::ObjInstance(ObjClass *_klass, GC *_gc)
    : Obj{ObjType::INSTANCE, hashObj(this, ObjType::INSTANCE), _gc}
    , klass{_klass}
    , fields{_gc}
{}

ObjInstance::~ObjInstance() = default;

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
}

Value ObjInstance::getByField(ObjString *name, Value &value)
{
    if (fields.get(obj_val(name), value)) {
        return true_val;
    }
    if (klass->methods.get(obj_val(name), value)) {
        return true_val;
    }
    return false_val;
}

Value ObjInstance::setByField(ObjString *name, Value value)
{
    fields.insert(obj_val(name), value);
    return true_val;
}

Value ObjInstance::copy(GC *gc)
{
    ObjInstance *newObj = newObjInstance(klass, gc);
    gc->cache(obj_val(newObj));
    newObj->fields.copy(&fields);
    gc->releaseCache(1);
    return obj_val(newObj);
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