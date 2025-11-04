#ifndef OBJLIST_H
#define OBJLIST_H

#include "object/object.h"

namespace aria {

class ValueHashTable;
class ValueArray;

class ObjList : public Obj
{
public:
    ObjList() = delete;

    explicit ObjList(GC *_gc);

    ObjList(Value *_values, uint32_t _count, GC *_gc);

    ObjList(uint32_t begin, uint32_t end, const ObjList *other, GC *_gc);

    ~ObjList() override;

    using Obj::toString;

    using Obj::representation;

    String toString(ValueStack *printStack) override;

    String representation(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjList); }

    void blacken() override;

    Value getByField(ObjString *name, Value &value) override;

    Value getByIndex(Value k, Value &v) override;

    Value setByIndex(Value k, Value v) override;

    Value createIter(GC *gc) override;

    Value copy(GC *gc) override;

    ValueArray *list;
    ValueHashTable *cachedMethods;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_ObjList(Value value)
{
    return isObjType(value, ObjType::LIST);
}

inline ObjList *as_ObjList(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjList *>(as_obj(value));
#else
    return static_cast<ObjList *>(as_obj(value));
#endif
}

ObjList *newObjList(GC *gc);

ObjList *newObjList(Value *values, uint32_t count, GC *gc);

ObjList *newObjList(uint32_t begin, uint32_t end, ObjList *other, GC *gc);

} // namespace aria

#endif //OBJLIST_H
