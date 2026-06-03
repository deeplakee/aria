#ifndef ARIA_OBJUPVALUE_H
#define ARIA_OBJUPVALUE_H

#include "object/object.h"

namespace aria {

class ObjUpvalue : public Obj
{
public:
    ObjUpvalue() = delete;

    ObjUpvalue(Value *location, GC *gc);

    ~ObjUpvalue() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjUpvalue); }

    void blacken() override;

    Value *location_;
    Value closed_;
    ObjUpvalue *next_upvalue_;
};

inline bool is_obj_upvalue(Value value)
{
    return is_obj_type(value, ObjType::UPVALUE);
}

inline ObjUpvalue *as_obj_upvalue(Value value)
{
    return as_Obj<ObjUpvalue>(value);
}

ObjUpvalue *new_ObjUpvalue(Value *location, GC *gc);

} // namespace aria

#endif //ARIA_OBJUPVALUE_H
