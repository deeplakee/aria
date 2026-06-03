#ifndef ARIA_OBJFUNCTION_H
#define ARIA_OBJFUNCTION_H

#include "object/funDef.h"
#include "object/object.h"

namespace aria {
class ObjClass;

class ValueHashTable;
class ObjUpvalue;
class Chunk;

class ObjFunction : public Obj
{
public:
    ObjFunction(
        FunctionType type,
        ObjString *location,
        ObjString *name,
        int arity,
        ValueHashTable *globals,
        bool acceptsVarargs,
        GC *gc);

    ObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc);

    ObjFunction(
        FunctionType type,
        ObjString *location,
        ObjString *name,
        ValueHashTable *globals,
        GC *gc);

    ~ObjFunction() override;

    String to_string() override;

    size_t obj_size() override { return sizeof(ObjFunction); }

    void blacken() override;

    Value op_call(AriaEnv *env, int argCount) override;

    void initUpvalues();

    ObjString *location_;
    ObjClass *enclosing_class_;
    ObjString *name_;
    Chunk *chunk_;
    int arity_;
    FunctionType type_;
    ObjUpvalue **upvalues_;
    int upvalue_count_;
    bool accepts_varargs_;
};

inline bool is_obj_function(Value value)
{
    return is_obj_type(value, ObjType::FUNCTION);
}

inline ObjFunction *as_obj_function(Value value)
{
    return as_Obj<ObjFunction>(value);
}

// function object for runfile mode
ObjFunction *new_ObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc);

// function object for repl mode
ObjFunction *new_ObjFunction(
    FunctionType type, ObjString *location, ObjString *name, ValueHashTable *globals, GC *gc);

// function object for normal function
ObjFunction *new_ObjFunction(
    FunctionType type,
    ObjString *location,
    ObjString *name,
    int arity,
    ValueHashTable *globals,
    bool acceptsVarargs,
    GC *gc);

} // namespace aria

#endif //ARIA_OBJFUNCTION_H
