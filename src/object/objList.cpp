#include "object/objList.h"

#include "memory/gc.h"
#include "object/objException.h"
#include "object/objIterator.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

#include <cassert>

namespace aria {

#define genException(code, msg) gc_->running_vm_->new_exception((code), (msg))

ObjList::ObjList(GC *gc)
    : Obj{ObjType::LIST, hash_obj(this, ObjType::LIST), gc}
    , list_{new ValueArray{gc}}
    , cached_methods_{gc}
{}

ObjList::ObjList(Value *values, uint32_t count, GC *gc)
    : Obj{ObjType::LIST, hash_obj(this, ObjType::LIST), gc}
    , list_{new ValueArray{values, count, gc}}
    , cached_methods_{gc}
{}

ObjList::ObjList(uint32_t begin, uint32_t end, const ObjList *other, GC *gc)
    : Obj{ObjType::LIST, hash_obj(this, ObjType::LIST), gc}
    , list_{new ValueArray{begin, end, other->list_, gc}}
    , cached_methods_{gc}
{}

ObjList::~ObjList()
{
    delete list_;
}

String ObjList::to_string()
{
    if (PrintGuard::is_cycle(this)) {
        return "[...]";
    }
    PrintGuard guard(this);
    return list_->to_string();
}

String ObjList::representation()
{
    return to_string();
}

void ObjList::blacken()
{
    list_->mark();
    cached_methods_.mark();
}

Value ObjList::get_by_field(ObjString *name, Value &value)
{
    if (cached_methods_.get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc_->list_methods_->get(NanBox::fromObj(name), value)) {
        assert(is_obj_native_fn(value) && "list builtin method is nativeFn");
        auto boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_native_fn(value), gc_);
        value = NanBox::fromObj(boundMethod);
        GcTempRootGuard guard{gc_, value};
        cached_methods_.insert(NanBox::fromObj(name), value);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjList::get_by_index(Value k, Value &v)
{
    if (!NanBox::isNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list_->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    v = (*list_)[index];
    return NanBox::TrueValue;
}

Value ObjList::set_by_index(Value k, Value v)
{
    if (!NanBox::isNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list_->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    (*list_)[index] = v;
    return NanBox::TrueValue;
}

Value ObjList::create_iter(GC *gc)
{
    return NanBox::fromObj(new_ObjIterator(this, gc));
}

Value ObjList::copy(GC *gc)
{
    ObjList *newObj = new_ObjList(gc);
    GcTempRootGuard guard{gc, NanBox::fromObj(newObj)};
    newObj->list_->copy(list_);
    return NanBox::fromObj(newObj);
}

ObjList *new_ObjList(GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(gc);
    log_obj_allocation(obj);
    return obj;
}

ObjList *new_ObjList(Value *values, uint32_t count, GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(values, count, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjList *new_ObjList(uint32_t begin, uint32_t end, ObjList *other, GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(begin, end, other, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
