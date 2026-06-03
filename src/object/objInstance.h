#ifndef ARIA_OBJINSTANCE_H
#define ARIA_OBJINSTANCE_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ObjClass;

class ObjInstance : public Obj
{
public:
    ObjInstance() = delete;

    ObjInstance(ObjClass *klass, GC *gc);

    ~ObjInstance() override;

    String to_string() override;

    String representation() override;

    size_t obj_size() override { return sizeof(ObjInstance); }

    void blacken() override;

    Value get_by_field(ObjString *name, Value &value) override;

    Value set_by_field(ObjString *name, Value value) override;

    Value copy(GC *gc) override;

    Value getSuperMethod(ObjClass *methodKlass, ObjString *methodName, Value &superMethod);

    ObjClass *klass_;
    ValueHashTable fields_;
    ValueHashTable cached_methods_;
};

inline bool is_obj_instance(Value value)
{
    return is_obj_type(value, ObjType::INSTANCE);
}

inline ObjInstance *as_obj_instance(Value value)
{
    return as_Obj<ObjInstance>(value);
}

ObjInstance *new_ObjInstance(ObjClass *klass, GC *gc);

} // namespace aria

#endif //ARIA_OBJINSTANCE_H
