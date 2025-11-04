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
        stack.push(obj_val(this));
        return map->toString(&stack);
    }

    if (printStack->Exist(obj_val(this))) {
        return "{...}";
    }
    printStack->push(obj_val(this));
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
    if (cachedMethods->get(obj_val(name), value)) {
        return true_val;
    }
    if (gc->mapBuiltins->get(obj_val(name), value)) {
        assert(is_ObjNativeFn(value) && "map builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(obj_val(this), as_ObjNativeFn(value), gc);
        value = obj_val(boundMethod);
        gc->cache(value);
        cachedMethods->insert(obj_val(name), value);
        gc->releaseCache(1);
        return true_val;
    }
    return false_val;
}

Value ObjMap::getByIndex(Value k, Value &v)
{
    if (map->get(k, v)) {
        return true_val;
    }
    return false_val;
}

Value ObjMap::setByIndex(Value k, Value v)
{
    map->insert(k, v);
    return true_val;
}

Value ObjMap::createIter(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

Value ObjMap::copy(GC *gc)
{
    ObjMap *newObj = newObjMap(gc);
    gc->cache(obj_val(newObj));
    newObj->map->copy(map);
    gc->releaseCache(1);
    return obj_val(newObj);
}

void ObjMap::blacken()
{
    map->mark();
    cachedMethods->mark();
}

ObjMap *newObjMap(GC *gc)
{
    auto obj = gc->allocate_object<ObjMap>(gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object MAP)", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}

ObjMap *newObjMap(Value *values, uint32_t count, GC *gc)
{
    auto obj = gc->allocate_object<ObjMap>(values, count, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object MAP)", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}

} // namespace aria
