#include "object/objList.h"
#include "object/objNativeFn.h"
#include "runtime/vm.h"
#include "util/nativeUtil.h"
#include "value/valueArray.h"

namespace aria {

static Value builtin_append(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    self->list->push(args[0]);
    return NanBox::NilValue;
}

static Value builtin_extend(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_OBJLIST(args[0], Argument);
    env->gc->pushTempRoot(args[0]);
    self->list->extend(asObjList(args[0])->list);
    env->gc->popTempRoot(1);
    return NanBox::NilValue;
}

static Value builtin_size(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    return NanBox::fromNumber(self->list->size());
}

static Value builtin_empty(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    return NanBox::fromBool(self->list->empty());
}

static Value builtin_pop(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    if (self->list->empty()) {
        return env->newException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "Pop from empty list");
    }
    self->list->pop();
    return NanBox::NilValue;
}

static Value builtin_insert(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    Value value = args[1];
    bool result = self->list->insert(index, value);
    return NanBox::fromBool(result);
}

static Value builtin_remove(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    CHECK_RANGE(index, 0, self->list->size(), Index);
    bool result = self->list->remove(index);
    return NanBox::fromBool(result);
}

static Value builtin_at(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    CHECK_RANGE(index, 0, self->list->size(), Index);
    return (*self->list)[index];
}

static Value builtin_clear(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    self->list->clear();
    return NanBox::NilValue;
}

static Value builtin_slice(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_INTEGER(args[0], start, Start index);
    CHECK_INTEGER(args[1], end, End index);

    auto size = self->list->size();
    CHECK_RANGE(start, 0, size, Start index);
    CHECK_RANGE(end, 0, size, End index);
    if (end < start) {
        return env->newException(
            ErrorCode::RUNTIME_OUT_OF_BOUNDS, "Start index should be smaller than end index");
    }
    ObjList *newList = NEW_OBJLIST(start, end, self);
    return NanBox::fromObj(newList);
}

static Value builtin_reverse(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    self->list->reverse();
    return NanBox::NilValue;
}

static Value builtin_equals(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjList(args[-1]);
    CHECK_OBJLIST(args[0], Argument);
    bool result = self->list->equals(asObjList(args[0])->list);
    return NanBox::fromBool(result);
}

void ObjList::init(GC *_gc, ValueHashTable *builtins)
{
    bindBuiltinMethod(builtins, "append", builtin_append, 1, _gc);
    bindBuiltinMethod(builtins, "extend", builtin_extend, 1, _gc);
    bindBuiltinMethod(builtins, "size", builtin_size, 0, _gc);
    bindBuiltinMethod(builtins, "empty", builtin_empty, 0, _gc);
    bindBuiltinMethod(builtins, "pop", builtin_pop, 0, _gc);
    bindBuiltinMethod(builtins, "insert", builtin_insert, 2, _gc);
    bindBuiltinMethod(builtins, "remove", builtin_remove, 1, _gc);
    bindBuiltinMethod(builtins, "at", builtin_at, 1, _gc);
    bindBuiltinMethod(builtins, "clear", builtin_clear, 0, _gc);
    bindBuiltinMethod(builtins, "slice", builtin_slice, 2, _gc);
    bindBuiltinMethod(builtins, "reverse", builtin_reverse, 0, _gc);
    bindBuiltinMethod(builtins, "equals", builtin_equals, 1, _gc);
}

} // namespace aria
