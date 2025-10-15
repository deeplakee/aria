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

ObjClass *newObjClass(ObjString *name, GC *gc)
{
    auto *obj = gc->allocate_object<ObjClass>(name, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object CLASS)", toVoidPtr(obj), sizeof(ObjClass));
#endif
    return obj;
}

} // namespace aria