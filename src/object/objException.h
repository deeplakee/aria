#ifndef OBJEXCEPTION_H
#define OBJEXCEPTION_H

#include "error/ErrorCode.h"
#include "object/object.h"

namespace aria {

class ObjException : public Obj
{
public:
    ObjException() = delete;

    ObjException(const char *_msg, GC *_gc);

    ObjException(const char *_msg, GC *_gc, ErrorCode _code);

    ObjException(ObjString *_msg, GC *_gc);

    ~ObjException() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjException); }

    void blacken() override;

    const char *what() const;

    ObjString *msg;
    ErrorCode code;
};

inline bool is_ObjException(Value value)
{
    return isObjType(value, ObjType::EXCEPTION);
}

inline ObjException *as_ObjException(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjException *>(NanBox::toObj(value));
#else
    return static_cast<ObjException *>(NanBox::toObj(value));
#endif
}

ObjException *newObjException(const char *msg, GC *gc);

ObjException *newObjException(ErrorCode code, const char *msg, GC *gc);

ObjException *newObjException(ObjString *msg, GC *gc);

} // namespace aria

#endif //OBJEXCEPTION_H
