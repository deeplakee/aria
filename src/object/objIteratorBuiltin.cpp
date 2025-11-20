#include "object/objNativeFn.h"
#include "object/objIterator.h"

namespace aria {

static Value builtin_hasNext(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjIterator(args[-1]);
    return NanBox::fromBool(self->iter->hasNext());
}

static Value builtin_next(AriaEnv *env, int argCount, Value *args)
{
    auto self = as_ObjIterator(args[-1]);
    return self->iter->next();
}

void ObjIterator::init(GC *_gc, ValueHashTable *builtins)
{
    bindBuiltinMethod(builtins, "hasNext", builtin_hasNext, 0, _gc);
    bindBuiltinMethod(builtins, "next", builtin_next, 0, _gc);
}

} // namespace aria
