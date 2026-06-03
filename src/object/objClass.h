#ifndef ARIA_OBJCLASS_H
#define ARIA_OBJCLASS_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ObjFunction;

class ObjClass : public Obj
{
public:
    ObjClass() = delete;

    ObjClass(ObjString *name, GC *gc);

    ~ObjClass() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjClass); }

    void blacken() override;

    bool getSuperMethod(ObjString *method_name, Value &method) const;

    ObjString *name_;
    ValueHashTable methods_;
    ObjClass *super_klass_;
    ObjFunction *init_method_;
};

inline bool is_obj_class(Value value)
{
    return is_obj_type(value, ObjType::CLASS);
}

inline ObjClass *as_obj_class(Value value)
{
    return as_Obj<ObjClass>(value);
}

ObjClass *new_ObjClass(ObjString *name, GC *gc);

} // namespace aria

#endif //ARIA_OBJCLASS_H
