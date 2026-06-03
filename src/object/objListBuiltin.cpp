#include "object/objList.h"
#include "object/objNativeFn.h"
#include "runtime/vm.h"
#include "util/nativeUtil.h"
#include "value/valueArray.h"

namespace aria {

static Value builtin_append(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    self->list_->push(args[0]);
    return NanBox::NilValue;
}

static Value builtin_extend(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_OBJLIST(args[0], Argument);
    GcTempRootGuard guard{env->gc_ ,args[0]};
    self->list_->extend(as_obj_list(args[0])->list_);
    return NanBox::NilValue;
}

static Value builtin_size(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    return NanBox::fromNumber(self->list_->size());
}

static Value builtin_empty(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    return NanBox::fromBool(self->list_->empty());
}

static Value builtin_pop(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    if (self->list_->empty()) {
        return env->new_exception(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "Pop from empty list");
    }
    self->list_->pop();
    return NanBox::NilValue;
}

static Value builtin_insert(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    Value value = args[1];
    bool result = self->list_->insert(index, value);
    return NanBox::fromBool(result);
}

static Value builtin_remove(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    CHECK_RANGE(index, 0, self->list_->size(), Index);
    bool result = self->list_->remove(index);
    return NanBox::fromBool(result);
}

static Value builtin_at(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    CHECK_RANGE(index, 0, self->list_->size(), Index);
    return (*self->list_)[index];
}

static Value builtin_clear(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    self->list_->clear();
    return NanBox::NilValue;
}

static Value builtin_slice(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_INTEGER(args[0], start, Start index);
    CHECK_INTEGER(args[1], end, End index);

    auto size = self->list_->size();
    CHECK_RANGE(start, 0, size, Start index);
    CHECK_RANGE(end, 0, size, End index);
    if (end < start) {
        return env->new_exception(
            ErrorCode::RUNTIME_OUT_OF_BOUNDS, "Start index should be smaller than end index");
    }
    ObjList *newList = NEW_OBJLIST(start, end, self);
    return NanBox::fromObj(newList);
}

static Value builtin_reverse(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    self->list_->reverse();
    return NanBox::NilValue;
}

static Value builtin_equals(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_obj_list(args[-1]);
    CHECK_OBJLIST(args[0], Argument);
    bool result = self->list_->equals(as_obj_list(args[0])->list_);
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
