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

#define genException(code, msg) gc->runningVM->newException((code), (msg))

ObjMap::ObjMap(GC *_gc)
    : Obj{ObjType::MAP, hashObj(this, ObjType::MAP), _gc}
    , map{new ValueHashTable{_gc}}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjMap::ObjMap(Value *_values, uint32_t _count, GC *_gc)
    : Obj{ObjType::MAP, hashObj(this, ObjType::MAP), _gc}
    , map{new ValueHashTable{_values, _count, _gc}}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjMap::~ObjMap()
{
    delete map;
    delete cachedMethods;
}

String ObjMap::toString(ValueStack *printStack)
{
    if (printStack == nullptr) {
        ValueStack stack;
        stack.push(NanBox::fromObj(this));
        return map->toString(&stack);
    }

    if (printStack->Exist(NanBox::fromObj(this))) {
        return "{...}";
    }
    printStack->push(NanBox::fromObj(this));
    String str = map->toString(printStack);
    printStack->pop();
    return str;
}

String ObjMap::representation(ValueStack *printStack)
{
    return toString(printStack);
    return String{"<map>"};
}

Value ObjMap::getByField(ObjString *name, Value &value)
{
    if (cachedMethods->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc->mapMethods->get(NanBox::fromObj(name), value)) {
        assert(isObjNativeFn(value) && "map builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjNativeFn(value), gc);
        value = NanBox::fromObj(boundMethod);
        gc->pushTempRoot(value);
        cachedMethods->insert(NanBox::fromObj(name), value);
        gc->popTempRoot(1);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjMap::getByIndex(Value k, Value &v)
{
    if (map->get(k, v)) {
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjMap::setByIndex(Value k, Value v)
{
    map->insert(k, v);
    return NanBox::TrueValue;
}

Value ObjMap::createIter(GC *gc)
{
    return NanBox::fromObj(newObjIterator(this, gc));
}

Value ObjMap::copy(GC *gc)
{
    ObjMap *newObj = newObjMap(gc);
    gc->pushTempRoot(NanBox::fromObj(newObj));
    newObj->map->copy(map);
    gc->popTempRoot(1);
    return NanBox::fromObj(newObj);
}

void ObjMap::blacken()
{
    map->mark();
    cachedMethods->mark();
}

ObjMap *newObjMap(GC *gc)
{
    auto obj = gc->allocateObject<ObjMap>(gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object MAP)", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}

ObjMap *newObjMap(Value *values, uint32_t count, GC *gc)
{
    auto obj = gc->allocateObject<ObjMap>(values, count, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object MAP)", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}

} // namespace aria
