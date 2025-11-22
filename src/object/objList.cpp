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

#define genException(code, msg) gc->runningVM->newException((code), (msg))

ObjList::ObjList(GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{_gc}}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjList::ObjList(Value *_values, uint32_t _count, GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{_values, _count, _gc}}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjList::ObjList(uint32_t begin, uint32_t end, const ObjList *other, GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{begin, end, other->list, _gc}}
    , cachedMethods{new ValueHashTable{_gc}}
{}

ObjList::~ObjList()
{
    delete list;
    delete cachedMethods;
}

String ObjList::toString(ValueStack *printStack)
{
    if (printStack == nullptr) {
        ValueStack stack;
        stack.push(NanBox::fromObj(this));
        return list->toString(&stack);
    }

    if (printStack->Exist(NanBox::fromObj(this))) {
        return "[...]";
    }
    printStack->push(NanBox::fromObj(this));
    String str = list->toString(printStack);
    printStack->pop();
    return str;
}

String ObjList::representation(ValueStack *printStack)
{
    return toString(printStack);
    return String{"<list>"};
}

void ObjList::blacken()
{
    list->mark();
    cachedMethods->mark();
}

Value ObjList::getByField(ObjString *name, Value &value)
{
    if (cachedMethods->get(NanBox::fromObj(name), value)) {
        return NanBox::TrueValue;
    }
    if (gc->listBuiltins->get(NanBox::fromObj(name), value)) {
        assert(isObjNativeFn(value) && "list builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjNativeFn(value), gc);
        value = NanBox::fromObj(boundMethod);
        gc->cache(value);
        cachedMethods->insert(NanBox::fromObj(name), value);
        gc->releaseCache(1);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjList::getByIndex(Value k, Value &v)
{
    if (!NanBox::isNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    v = (*list)[index];
    return NanBox::TrueValue;
}

Value ObjList::setByIndex(Value k, Value v)
{
    if (!NanBox::isNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    (*list)[index] = v;
    return NanBox::TrueValue;
}

Value ObjList::createIter(GC *gc)
{
    return NanBox::fromObj(newObjIterator(this, gc));
}

Value ObjList::copy(GC *gc)
{
    ObjList *newObj = newObjList(gc);
    gc->cache(NanBox::fromObj(newObj));
    newObj->list->copy(list);
    gc->releaseCache(1);
    return NanBox::fromObj(newObj);
}

ObjList *newObjList(GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate bytes {} (object LIST)", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

ObjList *newObjList(Value *values, uint32_t count, GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(values, count, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate bytes {} (object LIST)", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

ObjList *newObjList(uint32_t begin, uint32_t end, ObjList *other, GC *gc)
{
    auto obj = gc->allocate_object<ObjList>(begin, end, other, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate bytes {} (object LIST)", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

} // namespace aria