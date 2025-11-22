#include "runtime/vm.h"

#include "chunk/chunk.h"
#include "chunk/disassembler.h"
#include "compile/compiler.h"
#include "error/error.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objException.h"
#include "object/objFunction.h"
#include "object/objInstance.h"
#include "object/objIterator.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/objModule.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "object/objUpvalue.h"
#include "runtime/native.h"
#include "value/valueHashTable.h"

// debugger header files
#include "debugger/debugger.h"

// std header files
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace aria {

AriaVM::AriaVM()
    : gc{new GC{}}
    , Cframes{new CallFrame[FRAME_SIZE]}
    , Eframes{new ExceptionFrame[FRAME_SIZE]}
    , Rmodules{new Value[FRAME_SIZE]}
    , CframeCount{0}
    , EframeCount{0}
    , RmoduleCount{0}
    , frame{nullptr}
    , chunk{nullptr}
    , E_REG{NanBox::NilValue}
    , flags{0}
    , builtIn{new ValueHashTable{gc}}
    , cachedModules{new ValueHashTable{gc}}
    , openUpvalues{nullptr}
    , globals{new ValueHashTable{gc}}
    , debugger{nullptr}
{
    gc->bindVM(this);
    registerNative();
}

AriaVM::~AriaVM()
{
    delete[] Cframes;
    delete[] Eframes;
    delete[] Rmodules;
    delete builtIn;
    delete cachedModules;
    delete globals;
    delete gc;
}

interpretResult AriaVM::interpret(String srcFilePath, String source)
{
    ariaDir = getProgramDirectory();
    srcFilePath = getAbsolutePath(getWorkingDirectory(), srcFilePath);
#ifdef DEBUG_MODE
    println("aria directory: {}", ariaDir);
    println("source file path: {}", srcFilePath);
#endif
    auto script = Compiler::compile(srcFilePath, std::move(source), gc, globals);
    if (script == nullptr) {
        return interpretResult::COMPILE_ERROR;
    }
    reset();
    try {
        stack.push(NanBox::fromObj(script));
        callModule(script);
        if (getErrFlag()) {
            return interpretResult::RUNTIME_ERROR;
        }
        run();
        return interpretResult::SUCCESS;
    } catch (const ariaException &e) {
        error(e.what());
    }
    return interpretResult::RUNTIME_ERROR;
}

interpretResult AriaVM::interpret(String source)
{
    std::ofstream out(tmp_aria_file_path);
    out << source;
    out.close();
    return interpret(tmp_aria_file_path, std::move(source));
}

interpretResult AriaVM::interpretFromFile(const String &srcFilePath)
{
    try {
        String code = readFile(srcFilePath);
        return interpret(srcFilePath, code);
    } catch (const std::runtime_error &e) {
        error(e.what());
        return interpretResult::SRC_FILE_ERROR;
    }
}

Value AriaVM::runFunction(ObjFunction *fun, int argCount, const Value *args)
{
    stack.push(NanBox::fromObj(fun));
    if (fun == nullptr) {
        reportRuntimeFatalError(ErrorCode::RUNTIME_NULL_REFERENCE, "Invalid function pointer");
    }
    if (argCount > 0 && args == nullptr) {
        reportRuntimeFatalError(ErrorCode::RUNTIME_NULL_REFERENCE, "Invalid arguments pointer");
    }
    for (int i = 0; i < argCount; i++) {
        stack.push(args[i]);
    }
    callFunction(fun, argCount);
    return run(CframeCount - 1);
}

void AriaVM::setDebugger(AriaDebugger *_debugger)
{
    debugger = _debugger;
}

Value AriaVM::newException(const char *msg)
{
    setErrFlag();
    auto e = NanBox::fromObj(newObjException(msg, gc));
    E_REG = e;
    return e;
}

Value AriaVM::newException(ErrorCode code, const char *msg)
{
    setErrFlag();
    auto e = NanBox::fromObj(newObjException(code, msg, gc));
    E_REG = e;
    return e;
}

Value AriaVM::newException(ErrorCode code, const String &msg)
{
    return newException(code, msg.c_str());
}

void AriaVM::updateCallFrame()
{
    if (CframeCount > 0) {
        frame = &Cframes[CframeCount - 1];
        chunk = frame->function->chunk;
    } else {
        frame = nullptr;
        chunk = nullptr;
    }
}

void AriaVM::pushCallFrame(ObjFunction *_function, uint8_t *_ip, Value *_stakBase)
{
    Cframes[CframeCount].init(_function, _ip, _stakBase);
    CframeCount++;
    frame = &Cframes[CframeCount - 1];
    chunk = frame->function->chunk;
}

void AriaVM::pushExceptionFrame(
    int CframeCount_, int RmoduleCount_, uint8_t *_ip, uint32_t _stackSize)
{
    Eframes[EframeCount].init(CframeCount_, RmoduleCount_, _ip, _stackSize);
    EframeCount++;
}

void AriaVM::pushRunningModule(const ObjFunction *_function)
{
    Rmodules[RmoduleCount] = NanBox::fromObj(_function->location);
    RmoduleCount++;
}

void AriaVM::popCallFrame()
{
    CframeCount--;
    if (CframeCount == 0) {
        frame = nullptr;
        chunk = nullptr;
    } else {
        frame = &Cframes[CframeCount - 1];
        chunk = frame->function->chunk;
    }
}

void AriaVM::popExceptionFrame()
{
    EframeCount--;
}

void AriaVM::popRunningModule()
{
    RmoduleCount--;
#ifdef DEBUG_MODE
    println("module exit: from {} to {}", RmoduleCount, RmoduleCount - 1);
#endif
}

void AriaVM::packVarargs(int argCount, int arity)
{
    int count = argCount - arity;
    if (count == 0) {
        ObjList *emptyList = newObjList(nullptr, 0, gc);
        stack.push(NanBox::fromObj(emptyList));
    } else {
        Value *start = stack.getTopPtr() - count;
        ObjList *list = newObjList(start, count, gc);
        stack.pop_n(count);
        stack.push(NanBox::fromObj(list));
    }
}

ObjModule *AriaVM::cacheModule(ObjFunction *moduleFn)
{
    gc->cache(NanBox::fromObj(moduleFn));
    auto module = NanBox::fromObj(newObjModule(moduleFn, gc));
    gc->cache(module);
    cachedModules->insert(NanBox::fromObj(moduleFn->location), module);
    gc->releaseCache(2);
    return asObjModule(module);
}

Value AriaVM::createCallFrame(ObjFunction *function)
{
    if (CframeCount == FRAME_SIZE) {
        return newException(ErrorCode::RUNTIME_STACK_OVERFLOW, "Stack overflow.");
    }
    // Normal function frame base includes callee itself at slot 0.
    auto arity = function->acceptsVarargs ? function->arity + 2 : function->arity + 1;
    pushCallFrame(function, function->chunk->codes, stack.getTopPtr() - arity);
    return NanBox::NilValue;
}

Value AriaVM::returnFromCurrentModule(Value result)
{
    if (RmoduleCount > 1) {
        result = NanBox::fromObj(cacheModule(frame->function));
    }
    popRunningModule();
    return result;
}

Value AriaVM::returnFromCurrentFrame(Value result)
{
#ifdef DEBUG_MODE
    assert(CframeCount > 0 && "returnFromCurrentFrame called with empty frame stack.");
    assert(RmoduleCount > 0 && "returnFromCurrentFrame called with empty running module stack.");
#endif
    closeUpvalues(frame->stakBase);
    if (frame->function->type == FunctionType::SCRIPT) {
        result = returnFromCurrentModule(result);
    }
    popCallFrame();
    return result;
}

Value AriaVM::callValue(Value callee, int argCount)
{
    if (NanBox::isObj(callee)) {
        switch (getObjType(callee)) {
        case ObjType::FUNCTION:
            return callFunction(asObjFunction(callee), argCount);
        case ObjType::NATIVE_FN:
            return callNativeFn(asObjNativeFn(callee), argCount);
        case ObjType::CLASS:
            return callNewInstance(asObjClass(callee), argCount);
        case ObjType::BOUND_METHOD:
            return callBoundMethod(asObjBoundMethod(callee), argCount);
        default:
            return NanBox::toObj(callee)->op_call(this, argCount);
        }
    }
    return newException("Invalid call operation.");
}

Value AriaVM::callModule(ObjFunction *module)
{
    if (RmoduleCount == FRAME_SIZE) {
        return newException(ErrorCode::RUNTIME_STACK_OVERFLOW, "RunningModule Stack overflow.");
    }
    pushRunningModule(module);
    return callFunction(module, 0);
}

Value AriaVM::callFunction(ObjFunction *function, int argCount)
{
    if (function->acceptsVarargs && argCount >= function->arity) {
        packVarargs(argCount, function->arity);
    } else if (argCount == function->arity) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", function->arity, argCount);
        return newException(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    return createCallFrame(function);
}

Value AriaVM::callNativeFn(ObjNativeFn *native, int argCount)
{
    if (native->acceptsVarargs && argCount >= native->arity) {
        packVarargs(argCount, native->arity);
    } else if (argCount == native->arity) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", native->arity, argCount);
        return newException(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    // Native function args pointer starts from first argument.
    auto arity = native->acceptsVarargs ? native->arity + 1 : native->arity;
    Value result = native->function(this, argCount, stack.getTopPtr() - arity);
    stack.pop_n(arity + 1);
    stack.push(result);
    return result;
}

Value AriaVM::callNewInstance(ObjClass *klass, int argCount)
{
    stack[stack.size() - argCount - 1] = NanBox::fromObj(newObjInstance(klass, gc));
    if (klass->initMethod != nullptr) {
        return callFunction(klass->initMethod, argCount);
    }
    if (argCount != 0) {
        String msg = format("Expected 0 argument but got {}.", argCount);
        return newException(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    return NanBox::NilValue;
}

Value AriaVM::callBoundMethod(const ObjBoundMethod *method, const int argCount)
{
    if (method->methodType == BoundMethodType::FUNCTION) {
        stack[stack.size() - argCount - 1] = method->receiver;
        return callFunction(method->method, argCount);
    }
    if (method->methodType == BoundMethodType::NATIVE_FN) {
        stack[stack.size() - argCount - 1] = method->receiver;
        return callNativeFn(method->native_method, argCount);
    }
    return newException(ErrorCode::RUNTIME_UNKNOWN, "Unknown bound method type.");
}

ObjUpvalue *AriaVM::captureUpvalue(Value *local)
{
    ObjUpvalue *prevUpvalue = nullptr;
    ObjUpvalue *upvalue = openUpvalues;
    while (upvalue != nullptr && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->nextUpvalue;
    }

    if (upvalue != nullptr && upvalue->location == local) {
        return upvalue;
    }
    ObjUpvalue *createdUpvalue = newObjUpvalue(local, gc);
    createdUpvalue->nextUpvalue = upvalue;

    if (prevUpvalue == nullptr) {
        openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->nextUpvalue = createdUpvalue;
    }
    return createdUpvalue;
}

void AriaVM::closeUpvalues(const Value *last)
{
    while (openUpvalues != nullptr && openUpvalues->location >= last) {
        ObjUpvalue *upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = upvalue->nextUpvalue;
    }
}

void AriaVM::registerNative() const
{
    for (const auto &entry : Native::nativeFnTable()) {
        defineNativeFn(entry.name, entry.arity, entry.fn, entry.acceptsVarargs);
    }

    for (auto &var : Native::nativeVarTable()) {
        defineNativeVar(var.name, var.initializer(gc));
    }
}

void AriaVM::defineNativeFn(
    const char *name, int arity, NativeFn_t function, bool acceptsVarargs) const
{
    Value key = NanBox::fromObj(newObjString(name, gc));
    gc->cache(key);
    Value value = NanBox::fromObj(
        newObjNativeFn(FunctionType::FUNCTION, function, asObjString(key), arity, acceptsVarargs, gc));
    gc->cache(value);
    builtIn->insert(key, value);
    gc->releaseCache(2);
}

void AriaVM::defineNativeVar(const char *name, Value value) const
{
    Value key = NanBox::fromObj(newObjString(name, gc));
    gc->cache(key);
    gc->cache(value);
    builtIn->insert(key, value);
    gc->releaseCache(2);
}

bool AriaVM::isModuleRunning(const String &path) const
{
    for (int i = 0; i < RmoduleCount; i++) {
        if (path == asCString(Rmodules[i])) {
            return true;
        }
    }
    return false;
}

ObjModule *AriaVM::getCachedModule(ObjString *path)
{
    Value value;
    if (cachedModules->get(NanBox::fromObj(path), value)) {
        return asObjModule(value);
    }
    return nullptr;
}

ObjFunction *AriaVM::loadModule(const String &path, ObjString *moduleName)
{
    String source;
    try {
        source = readFile(path);
    } catch ([[maybe_unused]] const std::exception &e) {
        return nullptr;
    }
    return Compiler::compile(path, moduleName->C_str_ref(), std::move(source), gc);
}

Value AriaVM::run(int retFrame)
{
    if (retFrame < 0 || retFrame >= CframeCount) {
        reportRuntimeFatalError(ErrorCode::RUNTIME_INVALID_FRAME, "Invalid retFrame index");
    }
    for (;;) {
        auto codeOffset = static_cast<uint32_t>(frame->ip - chunk->codes);
        maybeDebugStep(codeOffset);
#ifdef DEBUG_TRACE_EXECUTION
        stack.display(frame->stakBase - stack.base(), frame->function->toString());
        Disassembler::disassembleInstruction(chunk, codeOffset, true);
#endif

        switch (frame->readOpcode()) {
        case opCode::LOAD_CONST:
            stack.push(frame->readConstant());
            break;
        case opCode::LOAD_NIL:
            stack.push(NanBox::NilValue);
            break;
        case opCode::LOAD_TRUE:
            stack.push(NanBox::TrueValue);
            break;
        case opCode::LOAD_FALSE:
            stack.push(NanBox::FalseValue);
            break;
        case opCode::LOAD_LOCAL: {
            uint16_t offset = frame->readWord();
            stack.push(frame->stakBase[offset]);
            break;
        }
        case opCode::STORE_LOCAL: {
            uint16_t offset = frame->readWord();
            frame->stakBase[offset] = stack.peek();
            break;
        }
        case opCode::LOAD_UPVALUE: {
            uint16_t slot = frame->readWord();
            Value val = *(frame->function->upvalues[slot]->location);
            stack.push(val);
            break;
        }
        case opCode::STORE_UPVALUE: {
            uint16_t slot = frame->readWord();
            *frame->function->upvalues[slot]->location = stack.peek();
            break;
        }
        case opCode::CLOSE_UPVALUE: {
            closeUpvalues(stack.getTopPtr() - 1);
            stack.pop(); // pop captured upvalue variable
            break;
        }
        case opCode::DEF_GLOBAL: {
            ObjString *name = frame->readObjString();
            if (!chunk->globals->insert(NanBox::fromObj(name), stack.peek())) {
                String msg = format("Existed variable '{}'.", name->C_str_ref());
                throwException(ErrorCode::RUNTIME_EXISTED_VARIABLE, msg);
                break;
            }
            stack.pop();
            break;
        }
        case opCode::LOAD_GLOBAL: {
            ObjString *name = frame->readObjString();
            Value value = NanBox::NilValue;
            if (!chunk->globals->get(NanBox::fromObj(name), value)) {
                if (!builtIn->get(NanBox::fromObj(name), value)) {
                    String msg = format("Undefined variable '{}'.", name->C_str_ref());
                    throwException(ErrorCode::RUNTIME_UNDEFINED_VARIABLE, msg);
                    break;
                }
            }
            stack.push(value);
            break;
        }
        case opCode::STORE_GLOBAL: {
            ObjString *name = frame->readObjString();
            if (chunk->globals->insert(NanBox::fromObj(name), stack.peek())) {
                chunk->globals->remove(NanBox::fromObj(name));
                String msg = format("Undefined variable '{}'.", name->C_str_ref());
                throwException(ErrorCode::RUNTIME_UNDEFINED_VARIABLE, msg);
                break;
            }
            break;
        }
        case opCode::LOAD_FIELD: {
            if (!NanBox::isObj(stack.peek())) {
                throwException(ErrorCode::RUNTIME_INVALID_FIELD_OP, "Only objects have fields.");
                break;
            }
            Obj *obj = NanBox::toObj(stack.peek());
            ObjString *name = frame->readObjString();
            Value value;

            if (auto result = obj->getByField(name, value); NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does no have attribute {}.",
                    obj->representation(),
                    name->C_str_ref());
                throwException(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                break;
            }
            stack.setTopVal(value);
            break;
        }
        case opCode::STORE_FIELD: {
            if (!NanBox::isObj(stack.peek())) {
                throwException(ErrorCode::RUNTIME_INVALID_FIELD_OP, "Only objects have fields.");
                break;
            }
            Obj *obj = NanBox::toObj(stack.pop());
            ObjString *propertyName = frame->readObjString();

            if (auto result = obj->setByField(propertyName, stack.peek()); NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does no support store field operation.",
                    valueTypeString(stack.peek()));
                throwException(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                break;
            }
            break;
        }
        case opCode::LOAD_SUBSCR: {
            if (!NanBox::isObj(stack.peek(1))) {
                throwException(
                    ErrorCode::RUNTIME_INVALID_INDEX_OP, "Only objects support index operation.");
                break;
            }
            Obj *obj = NanBox::toObj(stack.peek(1));
            Value index = stack.peek();
            Value value;

            auto result = obj->getByIndex(index, value);

            if (NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does not support subscript access with index '{}'.",
                    valueTypeString(NanBox::fromObj(obj)),
                    valueString(index));
                throwException(ErrorCode::RUNTIME_INVALID_INDEX_OP, msg);
                break;
            }
            if (getErrFlag()) {
                if (!isObjException(result)) {
                    reportRuntimeFatalError(ErrorCode::RUNTIME_UNKNOWN, "Invalid return value");
                }
                throwException(asObjException(result));
                break;
            }

            stack.pop_n(2);
            stack.push(value);
            break;
        }
        case opCode::STORE_SUBSCR: {
            if (!NanBox::isObj(stack.peek(1))) {
                throwException(
                    ErrorCode::RUNTIME_INVALID_INDEX_OP, "Only objects support index operation.");
                break;
            }
            Value index = stack.peek();
            Obj *obj = NanBox::toObj(stack.peek(1));
            Value value = stack.peek(2);

            auto result = obj->setByIndex(index, value);

            if (NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does not support subscript assignment with index '{}'.",
                    valueTypeString(NanBox::fromObj(obj)),
                    valueString(index));
                throwException(ErrorCode::RUNTIME_INVALID_INDEX_OP, msg);
                break;
            }
            if (getErrFlag()) {
                if (!isObjException(result)) {
                    reportRuntimeFatalError(ErrorCode::RUNTIME_UNKNOWN, "Invalid return value");
                }
                throwException(asObjException(result));
                break;
            }

            stack.pop_n(2);
            break;
        }
        case opCode::EQUAL: {
            Value b = stack.pop();
            Value a = stack.pop();
            stack.push(NanBox::fromBool(valuesSame(a, b)));
            break;
        }
        case opCode::NOT_EQUAL: {
            Value b = stack.pop();
            Value a = stack.pop();
            stack.push(NanBox::fromBool(!valuesSame(a, b)));
            break;
        }
        case opCode::GREATER: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromBool(a > b));
            break;
        }
        case opCode::GREATER_EQUAL: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromBool(a >= b));
            break;
        }
        case opCode::LESS: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromBool(a < b));
            break;
        }
        case opCode::LESS_EQUAL: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromBool(a <= b));
            break;
        }
        case opCode::ADD: {
            if (NanBox::isNumber(stack.peek()) && NanBox::isNumber(stack.peek(1))) {
                double b = NanBox::toNumber(stack.pop());
                double a = NanBox::toNumber(stack.pop());
                stack.push(NanBox::fromNumber(a + b));
                break;
            }
            if (isObjString(stack.peek()) && isObjString(stack.peek(1))) {
                ObjString *b = asObjString(stack.peek(0));
                ObjString *a = asObjString(stack.peek(1));
                ObjString *result = concatenateString(a, b, gc);
                if (result == nullptr) {
                    fatalError(
                        ErrorCode::RESOURCE_STRING_OVERFLOW,
                        "String concatenation result exceeds maximum length。");
                }
                stack.pop_n(2);
                stack.push(NanBox::fromObj(result));
                break;
            }
            throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers or strings.");
            break;
        }
        case opCode::SUBTRACT: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromNumber(a - b));
            break;
        }
        case opCode::MULTIPLY: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromNumber(a * b));
            break;
        }
        case opCode::DIVIDE: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            if (isZero(b)) {
                throwException(ErrorCode::RUNTIME_DIVISION_BY_ZERO, "Divide by zero.");
                break;
            }
            double a = NanBox::toNumber(stack.pop());
            stack.push(NanBox::fromNumber(a / b));
            break;
        }
        case opCode::MOD: {
            if (!NanBox::isNumber(stack.peek(0)) || !NanBox::isNumber(stack.peek(1))) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
                break;
            }
            double b = NanBox::toNumber(stack.pop());
            double a = NanBox::toNumber(stack.pop());
            if (isZero(b)) {
                throwException(ErrorCode::RUNTIME_MODULO_BY_ZERO, "Modulo by zero.");
                break;
            }
            stack.push(NanBox::fromNumber(std::fmod(a, b)));
            break;
        }
        case opCode::NOT:
            stack.push(NanBox::fromBool(isFalsey(stack.pop())));
            break;
        case opCode::NEGATE: {
            if (!NanBox::isNumber(stack.peek())) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Operand must be number.");
                break;
            }
            stack.push(NanBox::fromNumber(-NanBox::toNumber(stack.pop())));
            break;
        }
        case opCode::POP:
            stack.pop();
            break;
        case opCode::POP_N:
            stack.pop_n(frame->readByte());
            break;
        case opCode::PRINT:
            std::cout << valueString(stack.pop()) << std::endl;
            break;
        case opCode::NOP:
            break;
        case opCode::JUMP_FWD: {
            const uint16_t offset = frame->readWord();
            frame->ip -= offset;
            break;
        }
        case opCode::JUMP_BWD: {
            const uint16_t offset = frame->readWord();
            frame->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE: {
            const uint16_t offset = frame->readWord();
            if (!isFalsey(stack.pop()))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE_NOPOP: {
            const uint16_t offset = frame->readWord();
            if (!isFalsey(stack.peek(0)))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE: {
            const uint16_t offset = frame->readWord();
            if (isFalsey(stack.pop()))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE_NOPOP: {
            const uint16_t offset = frame->readWord();
            if (isFalsey(stack.peek(0)))
                frame->ip += offset;
            break;
        }
        case opCode::CALL: {
            int argCount = frame->readByte();
            auto callee = stack.peek(argCount);
            auto result = callValue(callee, argCount);
            if (getErrFlag()) {
                if (!isObjException(result)) {
                    reportRuntimeFatalError(ErrorCode::RUNTIME_UNKNOWN, "Invalid return value");
                }
                throwException(asObjException(result));
            }
            break;
        }
        case opCode::CLOSURE: {
            ObjFunction *fun = asObjFunction(frame->readConstant());
            for (int i = 0; i < fun->upvalueCount; i++) {
                uint8_t isLocal = frame->readByte();
                uint16_t index = frame->readWord();
                if (isLocal) {
                    fun->upvalues[i] = captureUpvalue(frame->stakBase + index);
                } else {
                    fun->upvalues[i] = frame->function->upvalues[index];
                }
            }
            break;
        }
        case opCode::MAKE_CLASS: {
            ObjClass *klass = newObjClass(frame->readObjString(), gc);
            stack.push(NanBox::fromObj(klass));
            break;
        }
        case opCode::INHERIT: {
            if (!isObjClass(stack.peek())) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Superclass must be a class.");
                break;
            }
            ObjClass *superKlass = asObjClass(stack.pop());
            ObjClass *klass = asObjClass(stack.peek());
            klass->superKlass = superKlass;
            klass->methods.copy(&superKlass->methods);
            break;
        }
        case opCode::MAKE_METHOD: {
            Value methodName = NanBox::fromObj(frame->readObjString());
            Value method = stack.peek(0);
            ObjClass *klass = asObjClass(stack.peek(1));
            klass->methods.insert(methodName, method);
            stack.pop();
            break;
        }
        case opCode::MAKE_INIT_METHOD: {
            Value method = stack.pop();
            ObjClass *klass = asObjClass(stack.peek());
            klass->initMethod = asObjFunction(method);
            break;
        }
        case opCode::INVOKE_METHOD: {
            error("Invoke_method not implemented");
            exit(1);
        }
        case opCode::LOAD_SUPER_METHOD: {
            ObjString *methodName = frame->readObjString();
            Value instance = stack.peek();
            ObjClass *superKlass = asObjInstance(instance)->klass->superKlass;
            Value superMethod = NanBox::NilValue;
            if (!superKlass->methods.get(NanBox::fromObj(methodName), superMethod)) {
                if (superKlass->initMethod != nullptr && methodName->length == 4
                    && memcmp(methodName->C_str_ref(), "init", 4) == 0) {
                    superMethod = NanBox::fromObj(superKlass->initMethod);
                } else {
                    String msg = format(
                        "Superclass '{}' has no method '{}",
                        superKlass->toString(),
                        methodName->toString());
                    throwException(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                    break;
                }
            }
            ObjFunction *unpackedMethod = asObjFunction(superMethod);
            ObjBoundMethod *boundMethod = newObjBoundMethod(instance, unpackedMethod, gc);
            superMethod = NanBox::fromObj(boundMethod);

            stack.setTopVal(superMethod);
            break;
        }
        case opCode::MAKE_LIST: {
            int listSize = frame->readWord();
            ObjList *list = newObjList(stack.getTopPtr() - listSize, listSize, gc);
            stack.pop_n(listSize);
            stack.push(NanBox::fromObj(list));
            break;
        }
        case opCode::MAKE_MAP: {
            int mapSize = frame->readWord();
            ObjMap *map = newObjMap(stack.getTopPtr() - mapSize * 2, mapSize, gc);
            stack.pop_n(mapSize * 2);
            stack.push(NanBox::fromObj(map));
            break;
        }
        case opCode::IMPORT: {
            ObjString *moduleName = frame->readObjString();
            const char *currentModuleName = asCString(*currentRmodule());
            String absoluteModulePath;
            absoluteModulePath = getAbsoluteModulePath(currentModuleName, moduleName->C_str_ref());
            if (absoluteModulePath.empty()) {
                throwException(ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Invalid module path.");
                break;
            }
#ifdef DEBUG_MODE
            println("Importing module {}", absoluteModulePath);
#endif
            if (isModuleRunning(absoluteModulePath)) {
                throwException(
                    ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Circular module import detected.");
                break;
            }

            ObjString *path = newObjString(absoluteModulePath, gc);
            if (auto module = getCachedModule(path)) {
                module->name = moduleName;
                stack.push(NanBox::fromObj(module));
                break;
            }
            if (auto moduleFn = loadModule(absoluteModulePath, moduleName)) {
                stack.push(NanBox::fromObj(moduleFn));
                callModule(moduleFn);
                break;
            }
            throwException(ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Import module error.");
            break;
        }
        case opCode::GET_ITER: {
            if (!NanBox::isObj(stack.peek())) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterable object");
                break;
            }
            Value iter = NanBox::toObj(stack.peek())->createIter(gc);
            if (NanBox::isNil(iter)) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Expected iterable object");
                break;
            }
            stack.setTopVal(iter);
            break;
        }
        case opCode::ITER_HAS_NEXT: {
            if (!isObjIterator(stack.peek())) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterator object");
                break;
            }
            ObjIterator *iterator = asObjIterator(stack.peek());
            Value result = NanBox::fromBool(iterator->iter->hasNext());
            stack.setTopVal(result);
            break;
        }
        case opCode::ITER_GET_NEXT: {
            if (!isObjIterator(stack.peek())) {
                throwException(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterator object");
                break;
            }
            ObjIterator *iterator = asObjIterator(stack.peek());
            Value nextVal = iterator->iter->next();
            stack.pop();
            stack.push(nextVal);
            break;
        }
        case opCode::SETUP_EXCEPT: {
            uint16_t offset = frame->readWord();
            uint8_t *newIp = frame->ip + offset;
            if (EframeCount == FRAME_SIZE) {
                reportRuntimeFatalError(
                    ErrorCode::RUNTIME_STACK_OVERFLOW, "Exception Stack overflow.");
            }
            pushExceptionFrame(CframeCount, RmoduleCount, newIp, stack.size());
            break;
        }
        case opCode::END_EXCEPT: {
            popExceptionFrame();
            break;
        }
        case opCode::THROW: {
            setErrFlag();
            E_REG = stack.pop();
            if (EframeCount == 0) {
                reportRuntimeFatalError(
                    ErrorCode::RUNTIME_UNCAUGHT_EXCEPTION, valueString(E_REG).c_str());
            }
            unwindToCatchPoint();
            break;
        }
        case opCode::RETURN: {
            Value result = stack.pop();
            stack.resize(static_cast<uint32_t>(frame->stakBase - stack.base()));
            result = returnFromCurrentFrame(result);
            if (CframeCount == retFrame) {
                return result;
            }
            stack.push(result);
            break;
        }
        default:
            reportRuntimeFatalError(ErrorCode::RUNTIME_INVALID_INSTRUCTION, "Invalid opcode.");
        }
    }
}

void AriaVM::reset()
{
    stack.reset();
    CframeCount = 0;
    EframeCount = 0;
    openUpvalues = nullptr;
    updateCallFrame();
    E_REG = NanBox::NilValue;
    flags = 0;
}

void AriaVM::unwindToCatchPoint()
{
    auto ef = currentEframe();
    CframeCount = ef->CframeCount;
    updateCallFrame();
    RmoduleCount = ef->RmoduleCount;
    frame->ip = ef->ip;
    stack.resize(ef->stackSize);
    stack.push(E_REG);
    E_REG = NanBox::NilValue;
    unsetErrFlag();
}

void AriaVM::throwException(ObjException *e)
{
    throwException(e->code, e);
}

void AriaVM::throwException(ErrorCode code, ObjException *e)
{
    E_REG = NanBox::fromObj(e);
    if (EframeCount == 0) {
        reportRuntimeFatalError(code, e->what());
    }
    unwindToCatchPoint();
}

void AriaVM::throwException(ErrorCode code, const char *message)
{
    throwException(newObjException(code, message, gc));
}

void AriaVM::throwException(ErrorCode code, const String &message)
{
    throwException(newObjException(code, message.c_str(), gc));
}

void AriaVM::reportRuntimeFatalError(ErrorCode code, const char *msg) const
{
    std::ostringstream oss;
    println(oss, runtimeError(msg));
    for (int i = CframeCount - 1; i >= 0; i--) {
        const CallFrame *frame = &Cframes[i];
        const ObjFunction *function = frame->function;
        const char *location = function->location->C_str_ref();
        const char *funName = function->name->C_str_ref();
        const size_t offset = static_cast<uint32_t>(frame->ip - function->chunk->codes - 1);
        uint32_t line = function->chunk->lines[offset];
        println(oss, "{}:{} in {}", location, line, funName);
    }
    throw ariaRuntimeException(code, oss.str());
}

void AriaVM::maybeDebugStep(const uint32_t offset) const
{
    if (debugger) {
        debugger->hookBeforeExec(frame, offset);
    }
}

} // namespace aria