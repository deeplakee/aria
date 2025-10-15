#include "object/objList.h"

#include "memory/gc.h"
#include "object/objException.h"
#include "object/objIterator.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "util/util.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

namespace aria {

#define genException(code, msg) gc->runningVM->newException((code), (msg))

ObjList::ObjList(GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{_gc}}
{}

ObjList::ObjList(Value *_values, uint32_t _count, GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{_values, _count, _gc}}
{}

ObjList::ObjList(uint32_t begin, uint32_t end, const ObjList *other, GC *_gc)
    : Obj{ObjType::LIST, hashObj(this, ObjType::LIST), _gc}
    , list{new ValueArray{begin, end, other->list, _gc}}
{}

ObjList::~ObjList()
{
    delete list;
}

String ObjList::toString(ValueStack *printStack)
{
    if (printStack == nullptr) {
        ValueStack stack;
        stack.push(obj_val(this));
        return list->toString(&stack);
    }

    if (printStack->Exist(obj_val(this))) {
        return "[...]";
    }
    printStack->push(obj_val(this));
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
}

Value ObjList::getByField(ObjString *name, Value &value)
{
    if (gc->listBuiltins->get(obj_val(name), value)) {
        return true_val;
    }
    return false_val;
}

Value ObjList::getByIndex(Value k, Value &v)
{
    if (!is_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    v = (*list)[index];
    return true_val;
}

Value ObjList::setByIndex(Value k, Value v)
{
    if (!is_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of list must be a integer");
    }
    if (index < 0 || index >= list->size()) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    (*list)[index] = v;
    return true_val;
}

Value ObjList::createIter(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

Value ObjList::copy(GC *gc)
{
    ObjList *newObj = newObjList(gc);
    gc->cache(obj_val(newObj));
    newObj->list->copy(list);
    gc->releaseCache(1);
    return obj_val(newObj);
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