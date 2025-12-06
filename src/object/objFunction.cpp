#include "object/objFunction.h"

#include "chunk/chunk.h"
#include "objUpvalue.h"
#include "object/objList.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

namespace aria {

ObjFunction::ObjFunction(
    FunctionType _type,
    ObjString *_location,
    ObjString *_name,
    int _arity,
    ValueHashTable *_globals,
    bool _acceptsVarargs,
    GC *_gc)
    : Obj{ObjType::FUNCTION, hashObj(this, ObjType::FUNCTION), _gc}
    , location(_location)
    , name{_name}
    , chunk{new Chunk{_globals, _gc}}
    , arity{_arity}
    , type{_type}
    , upvalues{nullptr}
    , upvalueCount{0}
    , acceptsVarargs{_acceptsVarargs}
{
}

ObjFunction::ObjFunction(FunctionType _type, ObjString *_location, ObjString *_name, GC *_gc)
    : Obj{ObjType::FUNCTION, hashObj(this, ObjType::FUNCTION), _gc}
    , location(_location)
    , name{_name}
    , chunk{new Chunk{_gc}}
    , arity{0}
    , type{_type}
    , upvalues{nullptr}
    , upvalueCount{0}
    , acceptsVarargs{false}
{}

ObjFunction::ObjFunction(
    FunctionType _type, ObjString *_location, ObjString *_name, ValueHashTable *_globals, GC *_gc)
    : Obj{ObjType::FUNCTION, hashObj(this, ObjType::FUNCTION), _gc}
    , location(_location)
    , name{_name}
    , chunk{new Chunk{_globals, _gc}}
    , arity{0}
    , type{_type}
    , upvalues{nullptr}
    , upvalueCount{0}
    , acceptsVarargs{false}
{
}

ObjFunction::~ObjFunction()
{
    delete chunk;
    if (upvalues != nullptr) {
        gc->freeArray<ObjUpvalue *>(upvalues, upvalueCount);
    }
}

String ObjFunction::toString(ValueStack *printStack)
{
    auto f_name = name->C_str_ref();
    if (type == FunctionType::SCRIPT) {
        return format("<module {}>", f_name);
    }
    return format("<fn {}>", f_name);
}

void ObjFunction::blacken()
{
    location->mark();
    name->mark();
    chunk->consts.mark();
    chunk->globals->mark();
    if (upvalues != nullptr) {
        for (int i = 0; i < upvalueCount; i++) {
            if (upvalues[i] == nullptr) {
                continue;
            }
            upvalues[i]->mark();
        }
    }
}

Value ObjFunction::op_call(AriaEnv *env, int argCount)
{
    if (acceptsVarargs && argCount >= arity) {
        env->packVarargs(argCount, arity);
    } else if (argCount == arity) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", arity, argCount);
        return env->newException(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg.c_str());
    }
    return env->createCallFrame(this);
}

void ObjFunction::initUpvalues(GC *gc)
{
    if (upvalueCount == 0) {
        return;
    }
    upvalues = gc->allocateArray<ObjUpvalue *>(upvalueCount);
    for (int i = 0; i < upvalueCount; i++) {
        upvalues[i] = nullptr;
    }
}

ObjFunction *newObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc)
{
    auto obj = gc->allocateObject<ObjFunction>(type, location, name, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object FUNCTION)", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

ObjFunction *newObjFunction(
    FunctionType type, ObjString *location, ObjString *name, ValueHashTable *globals, GC *gc)
{
    auto obj = gc->allocateObject<ObjFunction>(type, location, name, globals, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object FUNCTION)", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

ObjFunction *newObjFunction(
    FunctionType type,
    ObjString *location,
    ObjString *name,
    int arity,
    ValueHashTable *globals,
    bool acceptsVarargs,
    GC *gc)
{
    auto obj
        = gc->allocateObject<ObjFunction>(type, location, name, arity, globals, acceptsVarargs, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object FUNCTION)", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

} // namespace aria