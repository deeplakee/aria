#include "object/objMap.h"
#include "object/objNativeFn.h"
#include "runtime/vm.h"
#include "util/nativeUtil.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"

namespace aria {
static Value builtin_insert(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    self->map->insert(args[0], args[1]);
    return NanBox::NilValue;
}

static Value builtin_get(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    Value value = NanBox::NilValue;
    self->map->get(args[0], value);
    return value;
}

static Value builtin_remove(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    self->map->remove(args[0]);
    return NanBox::NilValue;
}

static Value builtin_has(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    bool result = self->map->has(args[0]);
    return NanBox::fromBool(result);
}

static Value builtin_size(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    return NanBox::fromNumber(self->map->size());
}

static Value builtin_empty(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    return NanBox::fromBool(self->map->empty());
}

static Value builtin_clear(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    self->map->clear();
    return NanBox::NilValue;
}

static Value builtin_keys(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    ObjList *list = self->map->createKeyList();
    return NanBox::fromObj(list);
}

static Value builtin_values(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    ObjList *newList = self->map->createValueList();
    return NanBox::fromObj(newList);
}

Value builtin_pairs(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    ObjList *list = self->map->createPairList();
    return NanBox::fromObj(list);
}

static Value builtin_equals(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjMap(args[-1]);
    CHECK_OBJMAP(args[0], Argument);
    bool result = self->map->equals(as_ObjMap(args[0])->map);
    return NanBox::fromBool(result);
}

void ObjMap::init(GC *_gc, ValueHashTable *builtins)
{
    bindBuiltinMethod(builtins, "insert", builtin_insert, 2, _gc);
    bindBuiltinMethod(builtins, "get", builtin_get, 1, _gc);
    bindBuiltinMethod(builtins, "remove", builtin_remove, 1, _gc);
    bindBuiltinMethod(builtins, "has", builtin_has, 1, _gc);
    bindBuiltinMethod(builtins, "size", builtin_size, 0, _gc);
    bindBuiltinMethod(builtins, "empty", builtin_empty, 0, _gc);
    bindBuiltinMethod(builtins, "clear", builtin_clear, 0, _gc);
    bindBuiltinMethod(builtins, "keys", builtin_keys, 0, _gc);
    bindBuiltinMethod(builtins, "values", builtin_values, 0, _gc);
    bindBuiltinMethod(builtins, "pairs", builtin_pairs, 0, _gc);
    bindBuiltinMethod(builtins, "equals", builtin_equals, 1, _gc);
}

} // namespace aria
