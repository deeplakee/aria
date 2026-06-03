#ifndef ARIA_OBJLIST_H
#define ARIA_OBJLIST_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ValueArray;

class ObjList : public Obj
{
public:
    ObjList() = delete;

    explicit ObjList(GC *gc);

    ObjList(Value *values, uint32_t count, GC *gc);

    ObjList(uint32_t begin, uint32_t end, const ObjList *other, GC *gc);

    ~ObjList() override;

    String to_string() override;

    String representation() override;

    size_t obj_size() override { return sizeof(ObjList); }

    void blacken() override;

    Value get_by_field(ObjString *name, Value &value) override;

    Value get_by_index(Value k, Value &v) override;

    Value set_by_index(Value k, Value v) override;

    Value create_iter(GC *gc) override;

    Value copy(GC *gc) override;

    ValueArray *list_;
    ValueHashTable cached_methods_;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_obj_list(Value value)
{
    return is_obj_type(value, ObjType::LIST);
}

inline ObjList *as_obj_list(Value value)
{
    return as_Obj<ObjList>(value);
}

ObjList *new_ObjList(GC *gc);

ObjList *new_ObjList(Value *values, uint32_t count, GC *gc);

ObjList *new_ObjList(uint32_t begin, uint32_t end, ObjList *other, GC *gc);

} // namespace aria

#endif //ARIA_OBJLIST_H
