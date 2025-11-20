#ifndef NATIVE_H
#define NATIVE_H

#include "object/funDef.h"
#include "value/value.h"

namespace aria {

class GC;

using VarInitFn_t = Value (*)(GC *);

#define val_wrap(initExpr) [](GC *gc) { return (initExpr); }
#define num_wrap(initExpr) [](GC *gc) { return NanBox::fromNumber(initExpr); }
#define obj_wrap(initExpr) [](GC *gc) { return NanBox::fromObj(initExpr); }

struct NativeFnEntry
{
    const char *name;
    int arity;
    NativeFn_t fn;
    bool acceptsVarargs;

    NativeFnEntry(const char *_name, int _arity, NativeFn_t _fn, bool _acceptsVarargs = false)
        : name{_name}
        , arity{_arity}
        , fn{_fn}
        , acceptsVarargs{_acceptsVarargs}
    {}
};

struct NativeVarEntry
{
    const char *name;
    VarInitFn_t initializer; // 延迟执行函数

    NativeVarEntry(const char *_name, VarInitFn_t _initializer)
        : name{_name}
        , initializer{_initializer}
    {}
};

class Native
{
public:
    static constexpr double pi = 3.141592653589793238462643383279502884L;
    static constexpr double e = 2.718281828459045235360287471352662498L;

    static Value _aria_clock_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_random_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_println_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_readline_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_typeof_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_str_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_repr_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_num_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_bool_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_copy_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_equals_(AriaEnv *env, int argCount, Value *args);
    static Value _aria_iter_(AriaEnv *env, int argCount, Value *args);
    [[noreturn]] static Value _aria_exit_(AriaEnv *env, int argCount, Value *args);
    static Value _aria__foo__(AriaEnv *env, int argCount, Value *args);

    static List<NativeVarEntry> &nativeVarTable();

    static List<NativeFnEntry> &nativeFnTable();
};

} // namespace aria

#endif //NATIVE_H
