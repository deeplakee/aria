#ifndef OBJITERATOR_H
#define OBJITERATOR_H

#include "object/iterator.h"
#include "object/object.h"

namespace aria {
class ValueHashTable;
class ObjList;
class ObjMap;

class ObjIterator : public Obj
{
public:
    ObjIterator() = delete;

    explicit ObjIterator(Iterator *_iter, GC *_gc);

    ~ObjIterator() override;

    using Obj::toString;

    using Obj::representation;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjIterator); }

    void blacken() override;

    Value getByField(ObjString *name, Value &value) override;

    Iterator *iter;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_ObjIterator(Value value)
{
    return isObjType(value, ObjType::ITERATOR);
}

inline ObjIterator *as_ObjIterator(Value value)
{
    return dynamic_cast<ObjIterator *>(as_obj(value));
}

ObjIterator *newObjIterator(Iterator *iter, GC *gc);

ObjIterator *newObjIterator(ObjList *list, GC *gc);

ObjIterator *newObjIterator(ObjMap *map, GC *gc);

ObjIterator *newObjIterator(ObjString *str, GC *gc);

} // namespace aria

#endif //OBJITERATOR_H
