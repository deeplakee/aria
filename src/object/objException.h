#ifndef ARIA_OBJEXCEPTION_H
#define ARIA_OBJEXCEPTION_H

#include "error/ErrorCode.h"
#include "object/object.h"

namespace aria {

class ObjException : public Obj
{
public:
    ObjException() = delete;

    ObjException(ErrorCode code, const char *msg, GC *gc);

    ObjException(ErrorCode code, ObjString *msg, GC *gc);

    ~ObjException() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjException); }

    void blacken() override;

    const char *what() const;

    ObjString *msg_;
    ErrorCode code_;
};

inline bool is_obj_exception(Value value)
{
    return is_obj_type(value, ObjType::EXCEPTION);
}

inline ObjException *as_obj_exception(Value value)
{
    return as_Obj<ObjException>(value);
}

ObjException *new_ObjException(ErrorCode code, const char *msg, GC *gc);

ObjException *new_ObjException(ErrorCode code,ObjString *msg, GC *gc);

} // namespace aria

#endif //ARIA_OBJEXCEPTION_H
