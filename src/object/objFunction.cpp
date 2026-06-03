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
    FunctionType type,
    ObjString *location,
    ObjString *name,
    int arity,
    ValueHashTable *globals,
    bool acceptsVarargs,
    GC *gc)
    : Obj{ObjType::FUNCTION, hash_obj(this, ObjType::FUNCTION), gc}
    , location_{location}
    , enclosing_class_{nullptr}
    , name_{name}
    , chunk_{new Chunk{globals, gc}}
    , arity_{arity}
    , type_{type}
    , upvalues_{nullptr}
    , upvalue_count_{0}
    , accepts_varargs_{acceptsVarargs}
{}

ObjFunction::ObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc)
    : Obj{ObjType::FUNCTION, hash_obj(this, ObjType::FUNCTION), gc}
    , location_{location}
    , enclosing_class_{nullptr}
    , name_{name}
    , chunk_{new Chunk{gc}}
    , arity_{0}
    , type_{type}
    , upvalues_{nullptr}
    , upvalue_count_{0}
    , accepts_varargs_{false}
{}

ObjFunction::ObjFunction(
    FunctionType type, ObjString *location, ObjString *name, ValueHashTable *globals, GC *gc)
    : Obj{ObjType::FUNCTION, hash_obj(this, ObjType::FUNCTION), gc}
    , location_{location}
    , enclosing_class_{nullptr}
    , name_{name}
    , chunk_{new Chunk{globals, gc}}
    , arity_{0}
    , type_{type}
    , upvalues_{nullptr}
    , upvalue_count_{0}
    , accepts_varargs_{false}
{}

ObjFunction::~ObjFunction()
{
    delete chunk_;
    if (upvalues_ != nullptr) {
        gc_->free_array<ObjUpvalue *>(upvalues_, upvalue_count_);
    }
}

String ObjFunction::to_string()
{
    auto funcName = name_->c_str();
    if (type_ == FunctionType::SCRIPT) {
        return format("<module {}>", funcName);
    }
    return format("<fn {}>", funcName);
}

void ObjFunction::blacken()
{
    location_->mark();
    if (enclosing_class_ != nullptr) {
        enclosing_class_->mark();
    }
    name_->mark();
    chunk_->consts_.mark();
    chunk_->globals_->mark();
    if (upvalues_ != nullptr) {
        for (int i = 0; i < upvalue_count_; i++) {
            if (upvalues_[i] == nullptr) {
                continue;
            }
            upvalues_[i]->mark();
        }
    }
}

Value ObjFunction::op_call(AriaEnv *env, int argCount)
{
    if (accepts_varargs_ && argCount >= arity_) {
        env->pack_varargs(argCount, arity_);
    } else if (argCount == arity_) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", arity_, argCount);
        return env->new_exception(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg.c_str());
    }
    return env->create_call_frame(this);
}

void ObjFunction::initUpvalues()
{
    if (upvalue_count_ == 0) {
        return;
    }
    upvalues_ = gc_->allocate_array<ObjUpvalue *>(upvalue_count_);
    for (int i = 0; i < upvalue_count_; i++) {
        upvalues_[i] = nullptr;
    }
}

ObjFunction *new_ObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc)
{
    auto obj = gc->allocate_object<ObjFunction>(type, location, name, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjFunction *new_ObjFunction(
    FunctionType type, ObjString *location, ObjString *name, ValueHashTable *globals, GC *gc)
{
    auto obj = gc->allocate_object<ObjFunction>(type, location, name, globals, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjFunction *new_ObjFunction(
    FunctionType type,
    ObjString *location,
    ObjString *name,
    int arity,
    ValueHashTable *globals,
    bool acceptsVarargs,
    GC *gc)
{
    auto obj
        = gc->allocate_object<ObjFunction>(type, location, name, arity, globals, acceptsVarargs, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
