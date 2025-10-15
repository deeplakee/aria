#ifndef OBJMODULE_H
#define OBJMODULE_H

#include "object/object.h"

namespace aria {

class ObjFunction;
class ValueHashTable;
class ObjModule : public Obj
{
public:
    ObjModule() = delete;

    ObjModule(ObjFunction *_module, GC *_gc);

    ~ObjModule() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjModule); }

    Value getByField(ObjString *name, Value &value) override;

    void blacken() override;

    ObjString *name;
    ValueHashTable *module;
};

inline bool is_ObjModule(Value value)
{
    return isObjType(value, ObjType::MODULE);
}

inline ObjModule *as_ObjModule(Value value)
{
    return dynamic_cast<ObjModule *>(as_obj(value));
}

ObjModule *newObjModule(ObjFunction *module, GC *gc);

} // namespace aria

#endif //OBJMODULE_H
