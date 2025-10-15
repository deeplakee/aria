#include "object/objException.h"
#include "memory/gc.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {

ObjException::ObjException(const char *_msg, GC *_gc)
    : Obj{ObjType::EXCEPTION, hashObj(this, ObjType::EXCEPTION), _gc}
    , msg{newObjString(_msg, _gc)}
    , code{ErrorCode::INTERNAL_UNKNOWN}
{}

ObjException::ObjException(const char *_msg, GC *_gc, ErrorCode _code)
    : Obj{ObjType::EXCEPTION, hashObj(this, ObjType::EXCEPTION), _gc}
    , msg{newObjString(_msg, _gc)}
    , code{_code}
{}

ObjException::ObjException(ObjString *_msg, GC *_gc)
    : Obj{ObjType::EXCEPTION, hashObj(this, ObjType::EXCEPTION), _gc}
    , msg{_msg}
    , code{ErrorCode::INTERNAL_UNKNOWN}
{}

ObjException::~ObjException() = default;

String ObjException::toString(ValueStack *printStack)
{
    return format("<exception: {}>", msg->C_str_ref());
}

void ObjException::blacken()
{
    msg->mark();
}

const char *ObjException::what() const
{
    return msg->C_str_ref();
}

ObjException *newObjException(const char *msg, GC *gc)
{
    auto obj = gc->allocate_object<ObjException>(msg, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object EXCEPTION)", toVoidPtr(obj), sizeof(ObjException));
#endif
    return obj;
}

ObjException *newObjException(ErrorCode code, const char *msg, GC *gc)
{
    auto obj = gc->allocate_object<ObjException>(msg, gc, code);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object EXCEPTION)", toVoidPtr(obj), sizeof(ObjException));
#endif
    return obj;
}

ObjException *newObjException(ObjString *msg, GC *gc)
{
    auto obj = gc->allocate_object<ObjException>(msg, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object EXCEPTION)", toVoidPtr(obj), sizeof(ObjException));
#endif
    return obj;
}

} // namespace aria
