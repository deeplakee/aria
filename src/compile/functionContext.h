#ifndef ARIA_FUNCTIONCONTEXT_H
#define ARIA_FUNCTIONCONTEXT_H

#include "chunk/chunk.h"
#include "object/funDef.h"

namespace aria {

class ObjFunction;

constexpr int kUnsetDepth = -1;

struct Local
{
    String name;
    int depth;
    bool isCaptured;

    Local()
        : name{}
        , depth{0}
        , isCaptured{false}
    {}

    Local(String _name, int _depth, bool _isCaptured)
        : name{std::move(_name)}
        , depth{_depth}
        , isCaptured{_isCaptured}
    {}
};



struct Upvalue
{
    uint16_t index{0};
    bool isLocal{false};

    Upvalue(uint16_t _index, bool _isLocal)
        : index{_index}
        , isLocal{_isLocal}
    {}
};

struct ClassContext
{
    ClassContext()
        : enclosing{nullptr}
        , hasSuperClass{false}
    {}

    ClassContext(ClassContext *_enclosing, bool _hasSuperClass)
        : enclosing{_enclosing}
        , hasSuperClass{_hasSuperClass}
    {}

    ClassContext *enclosing;
    bool hasSuperClass;
};

class FunctionContext
{
public:
    // for global start up, compile a script
    FunctionContext(
        const String &_fnName, const String &_fnLocation, ValueHashTable *_globals, GC *_gc);

    // for local start up, compile a function
    FunctionContext(
        FunctionType _type,
        FunctionContext *_enclosing,
        const String &_fnName,
        int _arity,
        bool _acceptsVarargs);

    ~FunctionContext();

    ObjFunction *currentFunction();

    void beginScope();

    List<opCode> endScope();

    int popLocalsOnControlFlow();

    bool addLocal(String name);

    void finalizeLocal();

    [[nodiscard]] bool isDefined(StringView name) const;

    /**
     * Finds a local variable by name and returns its status/index.
     *
     * @param name The name of the variable to search for.
     * @return -
     *   - `-2` if the variable is not a local variable (not found).
     *   - `-1` if the variable is found but accessed in its own initializer (e.g., `var x = x + 1;`).
     *   - `>= 0` if the variable is found and valid, returning its stack index.
     */
    int findLocalVariable(StringView name);

    /**
     * @param name variable name
     * @return -
     *    - `>= 0` Index of upvalue variable in this scope
     *    - `-1` No upper value found
     *    - `-2` Too many closure variables in function.
     */
    int findUpvalueVariable(StringView name);

    /**
     * @param index
     * @param isLocal
     * @return -
     *    - `-2` Too many closure variables in function.
     *    - `>= 0` The index of upvalue in current scope
     */
    int addUpvalue(uint16_t index, bool isLocal);

    void mark();

    GC *gc;
    FunctionContext *enclosing;
    ClassContext *currentClass;
    ObjFunction *fun;
    Chunk *chunk;

    int scopeDepth;
    List<Local> locals;
    List<Upvalue> upvalues;

    Stack<int> loopDepths;
    Stack<List<uint32_t>> loopBreaks;
    Stack<List<uint32_t>> loopContinues;
};

} // namespace aria

#endif //ARIA_FUNCTIONCONTEXT_H
