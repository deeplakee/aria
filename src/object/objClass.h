#ifndef OBJCLASS_H
#define OBJCLASS_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ObjFunction;

class ObjClass : public Obj
{
public:
    ObjClass() = delete;

    ObjClass(ObjString *_name, GC *_gc);

    ~ObjClass() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjClass); }

    void blacken() override;

    ObjString *name;
    ValueHashTable methods;
    ObjClass *superKlass;
    ObjFunction *initMethod;
};

inline bool is_ObjClass(Value value)
{
    return isObjType(value, ObjType::CLASS);
}

inline ObjClass *as_ObjClass(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjClass *>(as_obj(value));
#else
    return static_cast<ObjClass *>(as_obj(value));
#endif
}

ObjClass *newObjClass(ObjString *name, GC *gc);

} // namespace aria

#endif //OBJCLASS_H
