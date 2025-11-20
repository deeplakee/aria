#ifndef OBJMAP_H
#define OBJMAP_H

#include "object/object.h"

namespace aria {

class ValueHashTable;

class ObjMap : public Obj
{
public:
    ObjMap() = delete;

    explicit ObjMap(GC *_gc);

    ObjMap(Value *_values, uint32_t _count, GC *_gc);

    ~ObjMap() override;

    using Obj::toString;

    using Obj::representation;

    String toString(ValueStack *printStack) override;

    String representation(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjMap); }

    Value getByField(ObjString *name, Value &value) override;

    Value getByIndex(Value k, Value &v) override;

    Value setByIndex(Value k, Value v) override;

    Value createIter(GC *gc) override;

    Value copy(GC *gc) override;

    void blacken() override;

    ValueHashTable *map;
    ValueHashTable *cachedMethods;

    static void init(GC *_gc,ValueHashTable *builtins);
};

inline bool is_ObjMap(Value value)
{
    return isObjType(value, ObjType::MAP);
}

inline ObjMap *as_ObjMap(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjMap *>(NanBox::toObj(value));
#else
    return static_cast<ObjMap *>(NanBox::toObj(value));
#endif
}

ObjMap *newObjMap(GC *gc);

ObjMap *newObjMap(Value *values, uint32_t count, GC *gc);

} // namespace aria

#endif //OBJMAP_H
