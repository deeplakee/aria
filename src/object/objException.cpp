#include "object/objException.h"
#include "memory/gc.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjException::ObjException(ErrorCode code, const char *msg, GC *gc)
    : Obj{ObjType::EXCEPTION, hash_obj(this, ObjType::EXCEPTION), gc}
    , msg_{new_ObjString(msg, gc)}
    , code_{code}
{}

ObjException::ObjException(ErrorCode code, ObjString *msg, GC *gc)
    : Obj{ObjType::EXCEPTION, hash_obj(this, ObjType::EXCEPTION), gc}
    , msg_{msg}
    , code_{code}
{}

ObjException::~ObjException() = default;

String ObjException::to_string()
{
    return format("<exception: {}>", msg_->c_str());
}

void ObjException::blacken()
{
    msg_->mark();
}

const char *ObjException::what() const
{
    return msg_->c_str();
}

ObjException *new_ObjException(ErrorCode code, const char *msg, GC *gc)
{
    auto obj = gc->allocate_object<ObjException>(code, msg, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjException *new_ObjException(ErrorCode code, ObjString *msg, GC *gc)
{
    auto obj = gc->allocate_object<ObjException>(code, msg, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
