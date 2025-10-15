#ifndef OBJBOUNDMETHOD_H
#define OBJBOUNDMETHOD_H

#include "funDef.h"
#include "object/object.h"

namespace aria {

class ObjNativeFn;
class ObjFunction;

class ObjBoundMethod : public Obj
{
public:
    ObjBoundMethod() = delete;

    ObjBoundMethod(Value _receiver, ObjFunction *_method, GC *_gc);

    ObjBoundMethod(Value _receiver, ObjNativeFn *_method, GC *_gc);

    ~ObjBoundMethod() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjBoundMethod); }

    void blacken() override;

    Value receiver;
    BoundMethodType methodType;
    ObjFunction *method;
    ObjNativeFn *native_method;
};

inline bool is_ObjBoundMethod(Value value)
{
    return isObjType(value, ObjType::BOUND_METHOD);
}

inline ObjBoundMethod *as_ObjBoundMethod(Value value)
{
    return dynamic_cast<ObjBoundMethod *>(as_obj(value));
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjFunction *method, GC *gc);

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc);

} // namespace aria

#endif //OBJBOUNDMETHOD_H
