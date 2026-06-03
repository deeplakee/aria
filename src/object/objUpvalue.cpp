#include "object/objUpvalue.h"

#include "memory/gc.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjUpvalue::ObjUpvalue(Value *location, GC *gc)
    : Obj{ObjType::STRING, hash_obj(this, ObjType::UPVALUE), gc}
    , location_{location}
    , closed_{NanBox::NilValue}
    , next_upvalue_{nullptr}
{}

ObjUpvalue::~ObjUpvalue() = default;

String ObjUpvalue::to_string()
{
    return String{"upvalue:" + value_string(*location_)};
}

void ObjUpvalue::blacken()
{
    mark_value(closed_);
}

ObjUpvalue *new_ObjUpvalue(Value *location, GC *gc)
{
    auto obj = gc->allocate_object<ObjUpvalue>(location, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
