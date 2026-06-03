#ifndef ARIA_OBJBOUNDMETHOD_H
#define ARIA_OBJBOUNDMETHOD_H

#include "funDef.h"
#include "object/object.h"

namespace aria {

class ObjNativeFn;
class ObjFunction;

class ObjBoundMethod : public Obj
{
public:
    ObjBoundMethod() = delete;

    ObjBoundMethod(Value receiver, ObjFunction *method, GC *gc);

    ObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc);

    ~ObjBoundMethod() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjBoundMethod); }

    void blacken() override;

    Value receiver_;
    BoundMethodType method_type_;
    ObjFunction *method_;
    ObjNativeFn *native_method_;
};

inline bool is_obj_bound_method(Value value)
{
    return is_obj_type(value, ObjType::BOUND_METHOD);
}

inline ObjBoundMethod *as_obj_bound_method(Value value)
{
    return as_Obj<ObjBoundMethod>(value);
}

ObjBoundMethod *new_ObjBoundMethod(Value receiver, ObjFunction *method, GC *gc);

ObjBoundMethod *new_ObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc);

} // namespace aria

#endif //ARIA_OBJBOUNDMETHOD_H
