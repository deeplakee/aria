#ifndef ARIA_OBJMODULE_H
#define ARIA_OBJMODULE_H

#include "object/object.h"

namespace aria {

class ObjFunction;
class ValueHashTable;
class ObjModule : public Obj
{
public:
    ObjModule() = delete;

    ObjModule(ObjFunction *module, GC *gc);

    ~ObjModule() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjModule); }

    Value get_by_field(ObjString *name, Value &value) override;

    void blacken() override;

    ObjString *name_;
    ValueHashTable *module_;
};

inline bool is_obj_module(Value value)
{
    return is_obj_type(value, ObjType::MODULE);
}

inline ObjModule *as_obj_module(Value value)
{
    return as_Obj<ObjModule>(value);
}

ObjModule *new_ObjModule(ObjFunction *module, GC *gc);

} // namespace aria

#endif //ARIA_OBJMODULE_H
