#ifndef ARIA_OBJFUNCTION_H
#define ARIA_OBJFUNCTION_H

#include "object/funDef.h"
#include "object/object.h"

namespace aria {

class ValueHashTable;
class ObjUpvalue;
class Chunk;

class ObjFunction : public Obj
{
public:
    ObjFunction(
        FunctionType _type,
        ObjString *_location,
        ObjString *_name,
        int _arity,
        ValueHashTable *_globals,
        bool _acceptsVarargs,
        GC *_gc);

    ObjFunction(FunctionType _type, ObjString *_location, ObjString *_name, GC *_gc);

    ObjFunction(
        FunctionType _type,
        ObjString *_location,
        ObjString *_name,
        ValueHashTable *_globals,
        GC *_gc);

    ~ObjFunction() override;

    using Obj::toString;

    String toString(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjFunction); }

    void blacken() override;

    Value op_call(AriaEnv *env, int argCount) override;

    void initUpvalues(GC *gc);

    ObjString *location;
    ObjString *name;
    Chunk *chunk;
    int arity;
    FunctionType type;
    ObjUpvalue **upvalues;
    int upvalueCount;
    bool acceptsVarargs;
};

inline bool isObjFunction(Value value)
{
    return isObjType(value, ObjType::FUNCTION);
}

inline ObjFunction *asObjFunction(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjFunction *>(NanBox::toObj(value));
#else
    return static_cast<ObjFunction *>(NanBox::toObj(value));
#endif
}

// function object for runfile mode
ObjFunction *newObjFunction(FunctionType type, ObjString *location, ObjString *name, GC *gc);

// function object for repl mode
ObjFunction *newObjFunction(
    FunctionType type, ObjString *location, ObjString *name, ValueHashTable *globals, GC *gc);

// function object for normal function
ObjFunction *newObjFunction(
    FunctionType type,
    ObjString *location,
    ObjString *name,
    int arity,
    ValueHashTable *globals,
    bool acceptsVarargs,
    GC *gc);

} // namespace aria

#endif //ARIA_OBJFUNCTION_H
