#ifndef OBJUPVALUE_H
#define OBJUPVALUE_H

#include "object/object.h"

namespace aria {

class ObjUpvalue : public Obj
{
public:
    ObjUpvalue() = delete;

    ObjUpvalue(Value *_location, GC *_gc);

    ~ObjUpvalue() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjUpvalue); }

    void blacken() override;

    Value *location;
    Value closed;
    ObjUpvalue *nextUpvalue;
};

inline bool is_ObjUpvalue(Value value)
{
    return isObjType(value, ObjType::UPVALUE);
}

inline ObjUpvalue *as_ObjUpvalue(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjUpvalue *>(NanBox::toObj(value));
#else
    return static_cast<ObjUpvalue *>(NanBox::toObj(value));
#endif
}

ObjUpvalue *newObjUpvalue(Value *location, GC *gc);

} // namespace aria

#endif //OBJUPVALUE_H
