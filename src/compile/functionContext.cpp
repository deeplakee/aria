#include "compile/functionContext.h"
#include "object/objFunction.h"
#include "object/objString.h"

namespace aria {

FunctionContext::FunctionContext(
    const String &_fnName, const String &_fnLocation, ValueHashTable *_globals, GC *_gc)
    : gc{_gc}
    , enclosing{nullptr}
    , currentClass{nullptr}
    , fun{nullptr}
    , scopeDepth{0}
{
    auto fnNameObj = newObjString(_fnName, gc);
    gc->cache(NanBox::fromObj(fnNameObj));
    auto fnLocationObj = newObjString(_fnLocation, gc);
    gc->cache(NanBox::fromObj(fnLocationObj));
    if (_globals == nullptr) {
        fun = newObjFunction(FunctionType::SCRIPT, fnLocationObj, fnNameObj, _gc);
    } else {
        fun = newObjFunction(FunctionType::SCRIPT, fnLocationObj, fnNameObj, _globals, _gc);
    }
    gc->releaseCache(2);
    chunk = fun->chunk;
    gc->bindCompilingCtx(this);
    locals.emplace_back();
}

FunctionContext::FunctionContext(
    FunctionType _type,
    FunctionContext *_enclosing,
    const String &_fnName,
    int _arity,
    bool _acceptsVarargs)
    : gc{_enclosing->gc}
    , enclosing{_enclosing}
    , currentClass{_enclosing->currentClass}
    , fun{nullptr}
    , scopeDepth{0}
{
    auto globals = enclosing->fun->chunk->globals;
    auto fnNameObj = newObjString(_fnName, gc);
    gc->cache(NanBox::fromObj(fnNameObj));
    _arity = _acceptsVarargs ? _arity - 1 : _arity;
    fun = newObjFunction(
        _type, enclosing->fun->location, fnNameObj, _arity, globals, _acceptsVarargs, gc);
    gc->releaseCache(1);
    chunk = fun->chunk;
    gc->bindCompilingCtx(this);
    locals.emplace_back();
    if (_type == FunctionType::METHOD || _type == FunctionType::INIT_METHOD) {
        locals[0].name = "this";
    }
}

FunctionContext::~FunctionContext()
{
    gc->bindCompilingCtx(this->enclosing);
}

ObjFunction *FunctionContext::currentFunction()
{
    return fun;
}

void FunctionContext::beginScope()
{
    scopeDepth++;
}

List<opCode> FunctionContext::endScope()
{
    scopeDepth--;
    List<opCode> ops;
    while (!locals.empty()) {
        if (locals.back().depth <= scopeDepth) {
            break;
        }
        auto op = locals.back().isCaptured ? opCode::CLOSE_UPVALUE : opCode::POP;
        ops.push_back(op);
        locals.pop_back();
    }
    return ops;
}

int FunctionContext::popLocalsOnControlFlow()
{
    const int loopDepth = loopDepths.top();
    int count = 0;
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; i--) {
        if (locals[i].depth <= loopDepth) {
            break;
        }
        count++;
    }
    return count;
}

bool FunctionContext::addLocal(String name)
{
    if (locals.size() == UINT16_MAX) {
        return false;
    }
    locals.emplace_back(std::move(name), kUnsetDepth, false);
    return true;
}

void FunctionContext::finalizeLocal()
{
    if (scopeDepth == 0) {
        return;
    }
    locals.back().depth = scopeDepth;
}

bool FunctionContext::isDefined(StringView name) const
{
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; i--) {
        const Local &local = locals[i];
        if (local.depth != kUnsetDepth && local.depth < scopeDepth) {
            return false;
        }
        if (name == local.name) {
            return true;
        }
    }
    return false;
}

int FunctionContext::findLocalVariable(StringView name)
{
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; i--) {
        Local *local = &locals[i];
        if (name == local->name) {
            return (local->depth == kUnsetDepth) ? -1 : i;
        }
    }
    return -2;
}
int FunctionContext::findUpvalueVariable(StringView name)
{
    if (enclosing == nullptr) {
        return -1;
    }

    int localIndex = enclosing->findLocalVariable(name);
    if (localIndex >= 0) {
        enclosing->locals[localIndex].isCaptured = true;
        return addUpvalue(static_cast<uint16_t>(localIndex), true);
    }

    int upvalueIndex = enclosing->findUpvalueVariable(name);
    if (upvalueIndex >= 0) {
        return addUpvalue(static_cast<uint16_t>(upvalueIndex), false);
    }

    return upvalueIndex;
}

int FunctionContext::addUpvalue(uint16_t index, bool isLocal)
{
    int upvalueCount = fun->upvalueCount;
    for (int i = 0; i < upvalues.size(); i++) {
        Upvalue *upvalue = &upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT16_MAX) {
        return -2;
    }

    upvalues.emplace_back(index, isLocal);
    return fun->upvalueCount++;
}

void FunctionContext::mark()
{
    FunctionContext *current = this;
    while (current != nullptr) {
        current->fun->mark();
        current = current->enclosing;
    }
}

} // namespace aria