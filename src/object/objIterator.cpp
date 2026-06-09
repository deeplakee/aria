#include "object/objIterator.h"

#include "memory/gc.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "value/valueHashTable.h"

#include <cassert>

namespace aria {

ObjIterator::ObjIterator(Iterator *iter, GC *gc)
    : Obj{ObjType::ITERATOR, hash_obj(this, ObjType::ITERATOR), gc}
    , iter_{iter}
    , cached_methods_{new ValueHashTable{gc}}
{}

ObjIterator::~ObjIterator()
{
    delete iter_;
    delete cached_methods_;
}

String ObjIterator::to_string()
{
    return format("<iter {}>", iter_->typeString());
}

void ObjIterator::blacken()
{
    iter_->blacken();
    cached_methods_->mark();
}

Value ObjIterator::get_by_field(ObjString *name, Value &value)
{
    if (cached_methods_->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc_->iterator_methods_->get(NanBox::fromObj(name), value)) {
        assert(is_obj_native_fn(value) && "iterator builtin method is nativeFn");
        auto boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_native_fn(value), gc_);
        value = NanBox::fromObj(boundMethod);
        GcTempRootGuard guard{gc_, value};
        cached_methods_->insert(NanBox::fromObj(name), value);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

ObjIterator *new_ObjIterator(Iterator *iter, GC *gc)
{
    auto *obj = gc->allocate_object<ObjIterator>(iter, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjIterator *new_ObjIterator(ObjList *list, GC *gc)
{
    auto iter = new ListIterator{list};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjIterator *new_ObjIterator(ObjMap *map, GC *gc)
{
    auto iter = new MapIterator{map};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjIterator *new_ObjIterator(ObjString *str, GC *gc)
{
    auto iter = new StringIterator{str};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
