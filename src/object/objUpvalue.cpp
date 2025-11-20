#include "object/objUpvalue.h"

#include "memory/gc.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjUpvalue::ObjUpvalue(Value *_location, GC *_gc)
    : Obj{ObjType::STRING, hashObj(this, ObjType::UPVALUE), _gc}
    , location{_location}
    , closed{NanBox::NilValue}
    , nextUpvalue{nullptr}
{}

ObjUpvalue::~ObjUpvalue() = default;

String ObjUpvalue::toString(ValueStack *printStack)
{
    return String{"upvalue:" + valueString(*location)};
}

void ObjUpvalue::blacken()
{
    markValue(closed);
}

ObjUpvalue *newObjUpvalue(Value *location, GC *gc)
{
    ObjUpvalue *obj = gc->allocate_object<ObjUpvalue>(location, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object UPVALUE)", toVoidPtr(obj), sizeof(ObjUpvalue));
#endif
    return obj;
}

} // namespace aria