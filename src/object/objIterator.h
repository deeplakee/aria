#ifndef ARIA_OBJITERATOR_H
#define ARIA_OBJITERATOR_H

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

    explicit ObjIterator(Iterator *iter, GC *gc);

    ~ObjIterator() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjIterator); }

    void blacken() override;

    Value get_by_field(ObjString *name, Value &value) override;

    Iterator *iter_;
    ValueHashTable *cached_methods_;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_obj_iterator(Value value)
{
    return is_obj_type(value, ObjType::ITERATOR);
}

inline ObjIterator *as_obj_iterator(Value value)
{
    return as_Obj<ObjIterator>(value);
}

ObjIterator *new_ObjIterator(Iterator *iter, GC *gc);

ObjIterator *new_ObjIterator(ObjList *list, GC *gc);

ObjIterator *new_ObjIterator(ObjMap *map, GC *gc);

ObjIterator *new_ObjIterator(ObjString *str, GC *gc);

} // namespace aria

#endif //ARIA_OBJITERATOR_H
