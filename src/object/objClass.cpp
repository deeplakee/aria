#include "object/objClass.h"

#include "memory/gc.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {
ObjClass::ObjClass(ObjString *_name, GC *_gc)
    : Obj{ObjType::CLASS, hashObj(this, ObjType::CLASS), _gc}
    , name{_name}
    , methods{_gc}
    , superKlass{nullptr}
    , initMethod{nullptr}
{}

ObjClass::~ObjClass() = default;

String ObjClass::toString(ValueStack *printStack)
{
    return format("<class {}>", name->C_str_ref());
}

void ObjClass::blacken()
{
    name->mark();
    methods.mark();
    if (superKlass != nullptr) {
        superKlass->mark();
    }
    if (initMethod != nullptr) {
        initMethod->mark();
    }
}

bool ObjClass::getSuperMethod(ObjString *methodName, Value &method) const
{
    if (superKlass == nullptr) {
        return false;
    }

    if (methodName->length == 4 && memcmp(methodName->C_str_ref(), "init", 4) == 0) {
        if (superKlass->initMethod != nullptr) {
            method = NanBox::fromObj(superKlass->initMethod);
            return true;
        }
        return false;
    }

    return superKlass->methods.get(NanBox::fromObj(methodName), method);
}

ObjClass *newObjClass(ObjString *name, GC *gc)
{
    auto *obj = gc->allocateObject<ObjClass>(name, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object CLASS)", toVoidPtr(obj), sizeof(ObjClass));
#endif
    return obj;
}

} // namespace aria