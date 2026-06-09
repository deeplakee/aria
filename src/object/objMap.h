#ifndef ARIA_OBJMAP_H
#define ARIA_OBJMAP_H

#include "object/object.h"
#include "value/valueHashTable.h"

namespace aria {

class ObjMap : public Obj
{
public:
    ObjMap() = delete;

    explicit ObjMap(GC *gc);

    ObjMap(Value *values, uint32_t count, GC *gc);

    ~ObjMap() override;

    String to_string() override;

    String representation() override;

    size_t obj_size() override { return sizeof(ObjMap); }

    Value get_by_field(ObjString *name, Value &value) override;

    Value get_by_index(Value k, Value &v) override;

    Value set_by_index(Value k, Value v) override;

    Value create_iter(GC *gc) override;

    Value copy(GC *gc) override;

    void blacken() override;

    ValueHashTable *map_;
    ValueHashTable cached_methods_;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_obj_map(Value value)
{
    return is_obj_type(value, ObjType::MAP);
}

inline ObjMap *as_obj_map(Value value)
{
    return as_Obj<ObjMap>(value);
}

ObjMap *new_ObjMap(GC *gc);

ObjMap *new_ObjMap(Value *values, uint32_t count, GC *gc);

} // namespace aria

#endif // ARIA_OBJMAP_H
