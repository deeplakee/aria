#ifndef OBJINSTANCE_H
#define OBJINSTANCE_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ObjClass;

class ObjInstance : public Obj
{
public:
    ObjInstance() = delete;

    ObjInstance(ObjClass *_klass, GC *_gc);

    ~ObjInstance() override;

    String toString(ValueStack *printStack) override;

    String representation(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjInstance); }

    void blacken() override;

    Value getByField(ObjString *name, Value &value) override;

    Value setByField(ObjString *name, Value value) override;

    Value copy(GC *gc) override;

    ObjClass *klass;
    ValueHashTable fields;
};

inline bool is_ObjInstance(Value value)
{
    return isObjType(value, ObjType::INSTANCE);
}

inline ObjInstance *as_ObjInstance(Value value)
{
    return dynamic_cast<ObjInstance *>(as_obj(value));
}

ObjInstance *newObjInstance(ObjClass *klass, GC *gc);

} // namespace aria

#endif //OBJINSTANCE_H
