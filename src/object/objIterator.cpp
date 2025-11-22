#include "object/objIterator.h"

#include "memory/gc.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "value/valueHashTable.h"

#include <cassert>

namespace aria {

#define genException(code, msg) gc->runningVM->newException((code), (msg))

ObjIterator::ObjIterator(Iterator *iter, GC *_gc)
    : Obj{ObjType::ITERATOR, hashObj(this, ObjType::ITERATOR), _gc}
    , iter{iter}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjIterator::~ObjIterator()
{
    delete iter;
    delete cachedMethods;
}

String ObjIterator::toString(ValueStack *printStack)
{
    return format("<iter {}>", iter->typeString());
}

void ObjIterator::blacken()
{
    iter->blacken();
    cachedMethods->mark();
}

Value ObjIterator::getByField(ObjString *name, Value &value)
{
    if (cachedMethods->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc->iteratorBuiltins->get(NanBox::fromObj(name), value)) {
        assert(isObjNativeFn(value) && "iterator builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjNativeFn(value), gc);
        value = NanBox::fromObj(boundMethod);
        gc->cache(value);
        cachedMethods->insert(NanBox::fromObj(name), value);
        gc->releaseCache(1);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

ObjIterator *newObjIterator(Iterator *iter, GC *gc)
{
    auto *obj = gc->allocate_object<ObjIterator>(iter, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {}  bytes (object ITERATOR)", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjList *list, GC *gc)
{
    auto iter = new ListIterator{list};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {}  bytes (object ITERATOR)", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjMap *map, GC *gc)
{
    auto iter = new MapIterator{map};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {}  bytes (object ITERATOR)", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjString *str, GC *gc)
{
    auto iter = new StringIterator{str};
    auto obj = gc->allocate_object<ObjIterator>(iter, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {}  bytes (object ITERATOR)", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

} // namespace aria