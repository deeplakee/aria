#include "object/objMap.h"

#include "memory/gc.h"
#include "object/objIterator.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

#include <cassert>

namespace aria {

#define genException(code, msg) gc_->running_vm_->new_exception((code), (msg))

ObjMap::ObjMap(GC *gc)
    : Obj{ObjType::MAP, hash_obj(this, ObjType::MAP), gc}
    , map_{new ValueHashTable{gc}}
    , cached_methods_{gc}
{}

ObjMap::ObjMap(Value *values, uint32_t count, GC *gc)
    : Obj{ObjType::MAP, hash_obj(this, ObjType::MAP), gc}
    , map_{new ValueHashTable{values, count, gc}}
    , cached_methods_{gc}
{}

ObjMap::~ObjMap()
{
    delete map_;
}

String ObjMap::to_string()
{
    if (PrintGuard::is_cycle(this)) {
        return "{...}";
    }
    PrintGuard guard(this);
    return map_->to_string();
}

String ObjMap::representation()
{
    return to_string();
}

Value ObjMap::get_by_field(ObjString *name, Value &value)
{
    if (cached_methods_.get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc_->map_methods_->get(NanBox::fromObj(name), value)) {
        assert(is_obj_native_fn(value) && "map builtin method is nativeFn");
        auto boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_native_fn(value), gc_);
        value = NanBox::fromObj(boundMethod);
        GcTempRootGuard guard{gc_, value};
        cached_methods_.insert(NanBox::fromObj(name), value);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjMap::get_by_index(Value k, Value &v)
{
    if (map_->get(k, v)) {
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjMap::set_by_index(Value k, Value v)
{
    map_->insert(k, v);
    return NanBox::TrueValue;
}

Value ObjMap::create_iter(GC *gc)
{
    return NanBox::fromObj(new_ObjIterator(this, gc));
}

Value ObjMap::copy(GC *gc)
{
    ObjMap *newObj = new_ObjMap(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(newObj)};
    newObj->map_->copy(map_);
    return NanBox::fromObj(newObj);
}

void ObjMap::blacken()
{
    map_->mark();
    cached_methods_.mark();
}

ObjMap *new_ObjMap(GC *gc)
{
    auto obj = gc->allocate_object<ObjMap>(gc);
    log_obj_allocation(obj);
    return obj;
}

ObjMap *new_ObjMap(Value *values, uint32_t count, GC *gc)
{
    auto obj = gc->allocate_object<ObjMap>(values, count, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
