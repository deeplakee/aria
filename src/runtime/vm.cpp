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
#include <sstream>
#include <utility>

namespace aria {

AriaVM::AriaVM()
    : gc_{new GC{}}
    , c_frames_{new CallFrame[k_frame_size]}
    , e_frames_{new ExceptionFrame[k_frame_size]}
    , r_modules_{new Value[k_frame_size]}
    , c_frame_count_{0}
    , e_frame_count_{0}
    , r_module_count_{0}
    , frame_{nullptr}
    , chunk_{nullptr}
    , e_reg_{NanBox::NilValue}
    , flags_{0}
    , built_in_{new ValueHashTable{gc_}}
    , cached_modules_{new ValueHashTable{gc_}}
    , open_upvalues_{nullptr}
    , globals_{new ValueHashTable{gc_}}
    , debugger_{nullptr}
{
    gc_->attach_vm(this);
    register_native();
}

AriaVM::~AriaVM()
{
    delete[] c_frames_;
    delete[] e_frames_;
    delete[] r_modules_;
    delete built_in_;
    delete cached_modules_;
    delete globals_;
    delete gc_;
}

InterpretResult AriaVM::run_source(String sourceLocation, String source)
{
#ifdef DEBUG_MODE
    println("aria directory: {}", aria_dir_);
    println("source file path: {}", sourceLocation);
#endif
    auto script = Compiler::compile(std::move(sourceLocation), std::move(source), gc_, globals_);
    if (script == nullptr) {
        return InterpretResult::COMPILE_ERROR;
    }
    reset();
    try {
        stack_.push(NanBox::fromObj(script));
        call_module(script);
        if (get_err_flag()) {
            return InterpretResult::RUNTIME_ERROR;
        }
        run();
        return InterpretResult::SUCCESS;
    } catch (const ariaException &e) {
        error(e.what());
    }
    return InterpretResult::RUNTIME_ERROR;
}

InterpretResult AriaVM::interpret(String srcFilePath, String source)
{
    aria_dir_ = get_program_directory();
    return run_source(get_absolute_path(get_working_directory(), srcFilePath), std::move(source));
}

InterpretResult AriaVM::interpret(String source)
{
    aria_dir_ = get_program_directory();
    return run_source("<stdin>", std::move(source));
}

InterpretResult AriaVM::interpret_from_file(const String &srcFilePath)
{
    try {
        String code = read_file(srcFilePath);
        return interpret(srcFilePath, code);
    } catch (const std::runtime_error &e) {
        error(e.what());
        return InterpretResult::SRC_FILE_ERROR;
    }
}

Value AriaVM::run_function(ObjFunction *fun, int argCount, const Value *args)
{
    if (fun == nullptr) {
        report_runtime_fatal_error(ErrorCode::RUNTIME_NULL_REFERENCE, "Invalid function pointer");
    }
    stack_.push(NanBox::fromObj(fun));
    if (argCount > 0 && args == nullptr) {
        report_runtime_fatal_error(ErrorCode::RUNTIME_NULL_REFERENCE, "Invalid arguments pointer");
    }
    for (int i = 0; i < argCount; i++) {
        stack_.push(args[i]);
    }
    call_function(fun, argCount);
    return run(c_frame_count_ - 1);
}

void AriaVM::set_debugger(AriaDebugger *_debugger)
{
    debugger_ = _debugger;
}

Value AriaVM::new_exception(ErrorCode code, const char *msg)
{
    set_err_flag();
    auto e = NanBox::fromObj(new_ObjException(code, msg, gc_));
    e_reg_ = e;
    return e;
}

Value AriaVM::new_exception(ErrorCode code, const String &msg)
{
    return new_exception(code, msg.c_str());
}

void AriaVM::update_call_frame()
{
    if (c_frame_count_ > 0) {
        frame_ = &c_frames_[c_frame_count_ - 1];
        chunk_ = frame_->function->chunk_;
    } else {
        frame_ = nullptr;
        chunk_ = nullptr;
    }
}

void AriaVM::push_call_frame(ObjFunction *_function, uint8_t *_ip, Value *_stakBase)
{
    if (c_frame_count_ >= k_frame_size) {
        throw_exception(ErrorCode::RUNTIME_STACK_OVERFLOW, "Call stack overflow.");
        return;
    }
    c_frames_[c_frame_count_].init(_function, _ip, _stakBase);
    c_frame_count_++;
    frame_ = &c_frames_[c_frame_count_ - 1];
    chunk_ = frame_->function->chunk_;
}

void AriaVM::push_exception_frame(
    int c_frame_count, int r_module_count, uint8_t *ip, uint32_t stack_size)
{
    e_frames_[e_frame_count_].init(c_frame_count, r_module_count, ip, stack_size);
    e_frame_count_++;
}

void AriaVM::push_running_module(const ObjFunction *_function)
{
    if (r_module_count_ >= k_frame_size) {
        throw_exception(ErrorCode::RUNTIME_STACK_OVERFLOW, "Module stack overflow.");
        return;
    }
    r_modules_[r_module_count_] = NanBox::fromObj(_function->location_);
    r_module_count_++;
}

void AriaVM::pop_call_frame()
{
    c_frame_count_--;
    if (c_frame_count_ == 0) {
        frame_ = nullptr;
        chunk_ = nullptr;
    } else {
        frame_ = &c_frames_[c_frame_count_ - 1];
        chunk_ = frame_->function->chunk_;
    }
}

void AriaVM::pop_exception_frame()
{
    e_frame_count_--;
}

void AriaVM::pop_running_module()
{
    r_module_count_--;
#ifdef DEBUG_MODE
    println("module exit: from {} to {}", r_module_count_, r_module_count_ - 1);
#endif
}

void AriaVM::pack_varargs(int argCount, int arity)
{
    int count = argCount - arity;
    if (count == 0) {
        ObjList *emptyList = new_ObjList(nullptr, 0, gc_);
        stack_.push(NanBox::fromObj(emptyList));
    } else {
        Value *start = stack_.get_top_ptr() - count;
        ObjList *list = new_ObjList(start, count, gc_);
        stack_.pop_n(count);
        stack_.push(NanBox::fromObj(list));
    }
}

ObjModule *AriaVM::cache_module(ObjFunction *moduleFn)
{
    GcTempRootGuard guard{gc_, NanBox::fromObj(moduleFn)};
    auto module = NanBox::fromObj(new_ObjModule(moduleFn, gc_));
    guard.push(module);
    cached_modules_->insert(NanBox::fromObj(moduleFn->location_), module);
    return as_obj_module(module);
}

Value AriaVM::create_call_frame(ObjFunction *function)
{
    if (c_frame_count_ == k_frame_size) {
        return new_exception(ErrorCode::RUNTIME_STACK_OVERFLOW, "Stack overflow.");
    }
    // Normal function frame base includes callee itself at slot 0.
    auto arity = function->accepts_varargs_ ? function->arity_ + 2 : function->arity_ + 1;
    push_call_frame(function, function->chunk_->codes_, stack_.get_top_ptr() - arity);
    return NanBox::NilValue;
}

Value AriaVM::return_from_current_module(Value result)
{
    if (r_module_count_ > 1) {
        result = NanBox::fromObj(cache_module(frame_->function));
    }
    pop_running_module();
    return result;
}

Value AriaVM::return_from_current_frame(Value result)
{
#ifdef DEBUG_MODE
    assert(c_frame_count_ > 0 && "returnFromCurrentFrame called with empty frame stack.");
    assert(r_module_count_ > 0 && "returnFromCurrentFrame called with empty running module stack.");
#endif
    close_upvalues(frame_->stakBase);
    if (frame_->function->type_ == FunctionType::SCRIPT) {
        result = return_from_current_module(result);
    }
    pop_call_frame();
    return result;
}

Value AriaVM::call_value(Value callee, int argCount)
{
    if (NanBox::isObj(callee)) {
        switch (get_obj_type(callee)) {
        case ObjType::FUNCTION:
            return call_function(as_obj_function(callee), argCount);
        case ObjType::NATIVE_FN:
            return call_native_fn(as_obj_native_fn(callee), argCount);
        case ObjType::CLASS:
            return call_new_instance(as_obj_class(callee), argCount);
        case ObjType::BOUND_METHOD:
            return call_bound_method(as_obj_bound_method(callee), argCount);
        default:
            return NanBox::toObj(callee)->op_call(this, argCount);
        }
    }
    return new_exception(ErrorCode::RUNTIME_INVALID_CALL, "Invalid call operation.");
}

Value AriaVM::call_module(ObjFunction *module)
{
    if (r_module_count_ == k_frame_size) {
        return new_exception(ErrorCode::RUNTIME_STACK_OVERFLOW, "RunningModule Stack overflow.");
    }
    push_running_module(module);
    return call_function(module, 0);
}

Value AriaVM::call_function(ObjFunction *function, int argCount)
{
    if (function->accepts_varargs_ && argCount >= function->arity_) {
        pack_varargs(argCount, function->arity_);
    } else if (argCount == function->arity_) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", function->arity_, argCount);
        return new_exception(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    return create_call_frame(function);
}

Value AriaVM::call_native_fn(ObjNativeFn *native, int argCount)
{
    if (native->accepts_varargs_ && argCount >= native->arity_) {
        pack_varargs(argCount, native->arity_);
    } else if (argCount == native->arity_) {
        // pass
    } else {
        String msg = format("Expected {} arguments but got {}.", native->arity_, argCount);
        return new_exception(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    // Native function args pointer starts from first argument.
    auto arity = native->accepts_varargs_ ? native->arity_ + 1 : native->arity_;
    Value result = native->function_(this, argCount, stack_.get_top_ptr() - arity);
    stack_.pop_n(arity + 1);
    stack_.push(result);
    return result;
}

Value AriaVM::call_new_instance(ObjClass *klass, int argCount)
{
    stack_[stack_.size() - argCount - 1] = NanBox::fromObj(new_ObjInstance(klass, gc_));
    if (klass->init_method_ != nullptr) {
        return call_function(klass->init_method_, argCount);
    }
    if (argCount != 0) {
        String msg = format("Expected 0 argument but got {}.", argCount);
        return new_exception(ErrorCode::RUNTIME_MISMATCH_ARG_COUNT, msg);
    }
    return NanBox::NilValue;
}

Value AriaVM::call_bound_method(const ObjBoundMethod *method, const int argCount)
{
    if (method->method_type_ == BoundMethodType::FUNCTION) {
        stack_[stack_.size() - argCount - 1] = method->receiver_;
        return call_function(method->method_, argCount);
    }
    if (method->method_type_ == BoundMethodType::NATIVE_FN) {
        stack_[stack_.size() - argCount - 1] = method->receiver_;
        return call_native_fn(method->native_method_, argCount);
    }
    return new_exception(ErrorCode::RUNTIME_INVALID_CALL, "Unknown bound method type.");
}

ObjUpvalue *AriaVM::capture_upvalue(Value *local)
{
    ObjUpvalue *prevUpvalue = nullptr;
    ObjUpvalue *upvalue = open_upvalues_;
    while (upvalue != nullptr && upvalue->location_ > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next_upvalue_;
    }

    if (upvalue != nullptr && upvalue->location_ == local) {
        return upvalue;
    }
    ObjUpvalue *createdUpvalue = new_ObjUpvalue(local, gc_);
    createdUpvalue->next_upvalue_ = upvalue;

    if (prevUpvalue == nullptr) {
        open_upvalues_ = createdUpvalue;
    } else {
        prevUpvalue->next_upvalue_ = createdUpvalue;
    }
    return createdUpvalue;
}

void AriaVM::close_upvalues(const Value *last)
{
    while (open_upvalues_ != nullptr && open_upvalues_->location_ >= last) {
        ObjUpvalue *upvalue = open_upvalues_;
        upvalue->closed_ = *upvalue->location_;
        upvalue->location_ = &upvalue->closed_;
        open_upvalues_ = upvalue->next_upvalue_;
    }
}

void AriaVM::register_native() const
{
    for (const auto &entry : Native::nativeFnTable()) {
        define_native_fn(entry.name, entry.arity, entry.fn, entry.acceptsVarargs);
    }

    for (auto &var : Native::nativeVarTable()) {
        define_native_var(var.name, var.initializer(gc_));
    }
}

void AriaVM::define_native_fn(
    const char *name, int arity, NativeFn_t function, bool acceptsVarargs) const
{
    Value key = NanBox::fromObj(new_ObjString(name, gc_));
    GcTempRootGuard guard{gc_, key};
    Value value = NanBox::fromObj(new_ObjNativeFn(
        FunctionType::FUNCTION, function, as_obj_string(key), arity, acceptsVarargs, gc_));
    guard.push(value);
    built_in_->insert(key, value);
}

void AriaVM::define_native_var(const char *name, Value value) const
{
    Value key = NanBox::fromObj(new_ObjString(name, gc_));
    GcTempRootGuard guard{gc_, key};
    guard.push(value);
    built_in_->insert(key, value);
}

bool AriaVM::is_module_running(const String &path) const
{
    for (int i = 0; i < r_module_count_; i++) {
        if (path == as_c_string(r_modules_[i])) {
            return true;
        }
    }
    return false;
}

ObjModule *AriaVM::get_cached_module(ObjString *path)
{
    Value value;
    if (cached_modules_->get(NanBox::fromObj(path), value)) {
        return as_obj_module(value);
    }
    return nullptr;
}

ObjFunction *AriaVM::load_module(const String &path, ObjString *module_name)
{
    String source;
    try {
        source = read_file(path);
    } catch ([[maybe_unused]] const std::exception &e) {
        return nullptr;
    }
    return Compiler::compile(path, module_name->c_str(), std::move(source), gc_);
}

void AriaVM::mark_gc_roots()
{
    stack_.mark();

    mark_value(e_reg_);

    for (int i = 0; i < c_frame_count_; i++) {
        c_frames_[i].function->mark();
    }

    for (int i = 0; i < r_module_count_; i++) {
        mark_value(r_modules_[i]);
    }

    for (ObjUpvalue *p = open_upvalues_; p != nullptr; p = p->next_upvalue_) {
        p->mark();
    }

    built_in_->mark();
    cached_modules_->mark();
    if (globals_ != nullptr) {
        globals_->mark();
    }
}

template<NumericBinOp op>
bool AriaVM::numeric_bin_op()
{
    if (!NanBox::isNumber(stack_.peek(0)) || !NanBox::isNumber(stack_.peek(1))) {
        throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers.");
        return false;
    }
    double b = NanBox::toNumber(stack_.pop());

    if constexpr (op == NumericBinOp::DIV || op == NumericBinOp::MOD) {
        if (is_zero(b)) {
            if constexpr (op == NumericBinOp::DIV) {
                throw_exception(ErrorCode::RUNTIME_DIVISION_BY_ZERO, "Divide by zero.");
            } else {
                throw_exception(ErrorCode::RUNTIME_MODULO_BY_ZERO, "Modulo by zero.");
            }
            return false;
        }
    }

    double a = NanBox::toNumber(stack_.pop());

    if constexpr (op == NumericBinOp::GT) {
        stack_.push(NanBox::fromBool(a > b));
    } else if constexpr (op == NumericBinOp::GE) {
        stack_.push(NanBox::fromBool(a >= b));
    } else if constexpr (op == NumericBinOp::LT) {
        stack_.push(NanBox::fromBool(a < b));
    } else if constexpr (op == NumericBinOp::LE) {
        stack_.push(NanBox::fromBool(a <= b));
    } else if constexpr (op == NumericBinOp::SUB) {
        stack_.push(NanBox::fromNumber(a - b));
    } else if constexpr (op == NumericBinOp::MUL) {
        stack_.push(NanBox::fromNumber(a * b));
    } else if constexpr (op == NumericBinOp::DIV) {
        stack_.push(NanBox::fromNumber(a / b));
    } else if constexpr (op == NumericBinOp::MOD) {
        stack_.push(NanBox::fromNumber(std::fmod(a, b)));
    }
    return true;
}

Value AriaVM::run(int retFrame)
{
    if (retFrame < 0 || retFrame >= c_frame_count_) {
        report_runtime_fatal_error(ErrorCode::RUNTIME_INVALID_FRAME, "Invalid retFrame index");
    }
    for (;;) {
        auto codeOffset = static_cast<uint32_t>(frame_->ip - chunk_->codes_);
        maybe_debug_step(codeOffset);
#ifdef DEBUG_TRACE_EXECUTION
        stack_.display(frame_->stakBase - stack_.base(), frame_->function->to_string());
        Disassembler::disassembleInstruction(chunk_, codeOffset, true);
#endif

        switch (frame_->readOpcode()) {
        case opCode::LOAD_CONST:
            stack_.push(frame_->readConstant());
            break;
        case opCode::LOAD_NIL:
            stack_.push(NanBox::NilValue);
            break;
        case opCode::LOAD_TRUE:
            stack_.push(NanBox::TrueValue);
            break;
        case opCode::LOAD_FALSE:
            stack_.push(NanBox::FalseValue);
            break;
        case opCode::LOAD_LOCAL: {
            uint16_t offset = frame_->readWord();
            stack_.push(frame_->stakBase[offset]);
            break;
        }
        case opCode::STORE_LOCAL: {
            uint16_t offset = frame_->readWord();
            frame_->stakBase[offset] = stack_.peek();
            break;
        }
        case opCode::LOAD_UPVALUE: {
            uint16_t slot = frame_->readWord();
            Value val = *(frame_->function->upvalues_[slot]->location_);
            stack_.push(val);
            break;
        }
        case opCode::STORE_UPVALUE: {
            uint16_t slot = frame_->readWord();
            *frame_->function->upvalues_[slot]->location_ = stack_.peek();
            break;
        }
        case opCode::CLOSE_UPVALUE: {
            close_upvalues(stack_.get_top_ptr() - 1);
            stack_.pop(); // pop captured upvalue variable
            break;
        }
        case opCode::DEF_GLOBAL: {
            ObjString *name = frame_->readObjString();
            if (!chunk_->globals_->insert(NanBox::fromObj(name), stack_.peek())) {
                String msg = format("Existed variable '{}'.", name->c_str());
                throw_exception(ErrorCode::RUNTIME_EXISTED_VARIABLE, msg);
                break;
            }
            stack_.pop();
            break;
        }
        case opCode::LOAD_GLOBAL: {
            ObjString *name = frame_->readObjString();
            Value value = NanBox::NilValue;
            if (!chunk_->globals_->get(NanBox::fromObj(name), value)) {
                if (!built_in_->get(NanBox::fromObj(name), value)) {
                    String msg = format("Undefined variable '{}'.", name->c_str());
                    throw_exception(ErrorCode::RUNTIME_UNDEFINED_VARIABLE, msg);
                    break;
                }
            }
            stack_.push(value);
            break;
        }
        case opCode::STORE_GLOBAL: {
            ObjString *name = frame_->readObjString();
            if (chunk_->globals_->insert(NanBox::fromObj(name), stack_.peek())) {
                chunk_->globals_->remove(NanBox::fromObj(name));
                String msg = format("Undefined variable '{}'.", name->c_str());
                throw_exception(ErrorCode::RUNTIME_UNDEFINED_VARIABLE, msg);
                break;
            }
            break;
        }
        case opCode::LOAD_FIELD: {
            if (!NanBox::isObj(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_INVALID_FIELD_OP, "Only objects have fields.");
                break;
            }
            Obj *obj = NanBox::toObj(stack_.peek());
            ObjString *name = frame_->readObjString();
            Value value;

            if (auto result = obj->get_by_field(name, value); NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does no have attribute {}.",
                    obj->representation(),
                    name->c_str());
                throw_exception(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                break;
            }
            stack_.set_top_val(value);
            break;
        }
        case opCode::STORE_FIELD: {
            if (!NanBox::isObj(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_INVALID_FIELD_OP, "Only objects have fields.");
                break;
            }
            Obj *obj = NanBox::toObj(stack_.pop());
            ObjString *propertyName = frame_->readObjString();

            if (auto result = obj->set_by_field(propertyName, stack_.peek());
                NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does no support store field operation.",
                    value_type_string(stack_.peek()));
                throw_exception(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                break;
            }
            break;
        }
        case opCode::LOAD_SUBSCR: {
            if (!NanBox::isObj(stack_.peek(1))) {
                throw_exception(
                    ErrorCode::RUNTIME_INVALID_INDEX_OP, "Only objects support index operation.");
                break;
            }
            Obj *obj = NanBox::toObj(stack_.peek(1));
            Value index = stack_.peek();
            Value value;

            auto result = obj->get_by_index(index, value);

            if (NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does not support subscript access with index '{}'.",
                    value_type_string(NanBox::fromObj(obj)),
                    value_string(index));
                throw_exception(ErrorCode::RUNTIME_INVALID_INDEX_OP, msg);
                break;
            }
            if (get_err_flag()) {
                if (!is_obj_exception(result)) {
                    report_runtime_fatal_error(ErrorCode::INTERNAL_UNKNOWN, "Invalid return value");
                }
                throw_exception(as_obj_exception(result));
                break;
            }

            stack_.pop_n(2);
            stack_.push(value);
            break;
        }
        case opCode::STORE_SUBSCR: {
            if (!NanBox::isObj(stack_.peek(1))) {
                throw_exception(
                    ErrorCode::RUNTIME_INVALID_INDEX_OP, "Only objects support index operation.");
                break;
            }
            Value index = stack_.peek();
            Obj *obj = NanBox::toObj(stack_.peek(1));
            Value value = stack_.peek(2);

            auto result = obj->set_by_index(index, value);

            if (NanBox::isFalse(result)) {
                String msg = format(
                    "this {} object does not support subscript assignment with index '{}'.",
                    value_type_string(NanBox::fromObj(obj)),
                    value_string(index));
                throw_exception(ErrorCode::RUNTIME_INVALID_INDEX_OP, msg);
                break;
            }
            if (get_err_flag()) {
                if (!is_obj_exception(result)) {
                    report_runtime_fatal_error(ErrorCode::RUNTIME_UNKNOWN, "Invalid return value");
                }
                throw_exception(as_obj_exception(result));
                break;
            }

            stack_.pop_n(2);
            break;
        }
        case opCode::EQUAL: {
            Value b = stack_.pop();
            Value a = stack_.pop();
            stack_.push(NanBox::fromBool(values_same(a, b)));
            break;
        }
        case opCode::NOT_EQUAL: {
            Value b = stack_.pop();
            Value a = stack_.pop();
            stack_.push(NanBox::fromBool(!values_same(a, b)));
            break;
        }
        case opCode::GREATER:
            numeric_bin_op<NumericBinOp::GT>();
            break;
        case opCode::GREATER_EQUAL:
            numeric_bin_op<NumericBinOp::GE>();
            break;
        case opCode::LESS:
            numeric_bin_op<NumericBinOp::LT>();
            break;
        case opCode::LESS_EQUAL:
            numeric_bin_op<NumericBinOp::LE>();
            break;
        case opCode::ADD: {
            if (NanBox::isNumber(stack_.peek()) && NanBox::isNumber(stack_.peek(1))) {
                double b = NanBox::toNumber(stack_.pop());
                double a = NanBox::toNumber(stack_.pop());
                stack_.push(NanBox::fromNumber(a + b));
                break;
            }
            if (is_obj_string(stack_.peek()) && is_obj_string(stack_.peek(1))) {
                ObjString *b = as_obj_string(stack_.peek(0));
                ObjString *a = as_obj_string(stack_.peek(1));
                ObjString *result = concatenate_string(a, b, gc_);
                if (result == nullptr) {
                    fatal_error(
                        ErrorCode::RESOURCE_STRING_OVERFLOW,
                        "String concatenation result exceeds maximum length。");
                }
                stack_.pop_n(2);
                stack_.push(NanBox::fromObj(result));
                break;
            }
            throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Operands must be numbers or strings.");
            break;
        }
        case opCode::SUBTRACT:
            numeric_bin_op<NumericBinOp::SUB>();
            break;
        case opCode::MULTIPLY:
            numeric_bin_op<NumericBinOp::MUL>();
            break;
        case opCode::DIVIDE:
            numeric_bin_op<NumericBinOp::DIV>();
            break;
        case opCode::MOD:
            numeric_bin_op<NumericBinOp::MOD>();
            break;
        case opCode::NOT:
            stack_.push(NanBox::fromBool(is_falsey(stack_.pop())));
            break;
        case opCode::NEGATE: {
            if (!NanBox::isNumber(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Operand must be number.");
                break;
            }
            stack_.push(NanBox::fromNumber(-NanBox::toNumber(stack_.pop())));
            break;
        }
        case opCode::POP:
            stack_.pop();
            break;
        case opCode::POP_N:
            stack_.pop_n(frame_->readByte());
            break;
        case opCode::PRINT:
            std::cout << value_string(stack_.pop()) << std::endl;
            break;
        case opCode::NOP:
            break;
        case opCode::JUMP_FWD: {
            const uint16_t offset = frame_->readWord();
            frame_->ip -= offset;
            break;
        }
        case opCode::JUMP_BWD: {
            const uint16_t offset = frame_->readWord();
            frame_->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE: {
            const uint16_t offset = frame_->readWord();
            if (!is_falsey(stack_.pop()))
                frame_->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE_NOPOP: {
            const uint16_t offset = frame_->readWord();
            if (!is_falsey(stack_.peek(0)))
                frame_->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE: {
            const uint16_t offset = frame_->readWord();
            if (is_falsey(stack_.pop()))
                frame_->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE_NOPOP: {
            const uint16_t offset = frame_->readWord();
            if (is_falsey(stack_.peek(0)))
                frame_->ip += offset;
            break;
        }
        case opCode::CALL: {
            int argCount = frame_->readByte();
            auto callee = stack_.peek(argCount);
            auto result = call_value(callee, argCount);
            if (get_err_flag()) {
                if (!is_obj_exception(result)) {
                    report_runtime_fatal_error(ErrorCode::RUNTIME_UNKNOWN, "Invalid return value");
                }
                throw_exception(as_obj_exception(result));
            }
            break;
        }
        case opCode::CLOSURE: {
            ObjFunction *fun = as_obj_function(frame_->readConstant());
            for (int i = 0; i < fun->upvalue_count_; i++) {
                uint8_t isLocal = frame_->readByte();
                uint16_t index = frame_->readWord();
                if (isLocal) {
                    fun->upvalues_[i] = capture_upvalue(frame_->stakBase + index);
                } else {
                    fun->upvalues_[i] = frame_->function->upvalues_[index];
                }
            }
            break;
        }
        case opCode::MAKE_CLASS: {
            ObjClass *klass = new_ObjClass(frame_->readObjString(), gc_);
            stack_.push(NanBox::fromObj(klass));
            break;
        }
        case opCode::INHERIT: {
            if (!is_obj_class(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Superclass must be a class.");
                break;
            }
            ObjClass *superKlass = as_obj_class(stack_.pop());
            ObjClass *klass = as_obj_class(stack_.peek());
            klass->super_klass_ = superKlass;
            klass->methods_.copy(&superKlass->methods_);
            break;
        }
        case opCode::MAKE_METHOD: {
            Value methodName = NanBox::fromObj(frame_->readObjString());
            Value method = stack_.peek(0);
            ObjClass *klass = as_obj_class(stack_.peek(1));
            as_obj_function(method)->enclosing_class_ = klass;
            klass->methods_.insert(methodName, method);
            stack_.pop();
            break;
        }
        case opCode::MAKE_INIT_METHOD: {
            Value method = stack_.pop();
            ObjClass *klass = as_obj_class(stack_.peek());
            as_obj_function(method)->enclosing_class_ = klass;
            klass->init_method_ = as_obj_function(method);
            break;
        }
        case opCode::INVOKE_METHOD: {
            error("Invoke_method not implemented");
            exit(1);
        }
        case opCode::LOAD_SUPER_METHOD: {
            ObjString *methodName = frame_->readObjString();
            ObjInstance *instance = as_obj_instance(stack_.peek());
            ObjClass *klass = frame_->function->enclosing_class_;
            Value superMethod = NanBox::NilValue;
            if (auto result = instance->getSuperMethod(klass, methodName, superMethod);
                NanBox::isFalse(result)) {
                String msg = format("Superclass has no method '{}", methodName->to_string());
                throw_exception(ErrorCode::RUNTIME_INVALID_FIELD_OP, msg);
                break;
            }
            stack_.set_top_val(superMethod);
            break;
        }
        case opCode::MAKE_LIST: {
            int listSize = frame_->readWord();
            ObjList *list = new_ObjList(stack_.get_top_ptr() - listSize, listSize, gc_);
            stack_.pop_n(listSize);
            stack_.push(NanBox::fromObj(list));
            break;
        }
        case opCode::MAKE_MAP: {
            int mapSize = frame_->readWord();
            ObjMap *map = new_ObjMap(stack_.get_top_ptr() - mapSize * 2, mapSize, gc_);
            stack_.pop_n(mapSize * 2);
            stack_.push(NanBox::fromObj(map));
            break;
        }
        case opCode::IMPORT: {
            ObjString *moduleName = frame_->readObjString();
            const char *currentModuleName = as_c_string(*current_rmodule());
            String absoluteModulePath;
            absoluteModulePath = get_absolute_module_path(currentModuleName, moduleName->c_str());
            if (absoluteModulePath.empty()) {
                throw_exception(ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Invalid module path.");
                break;
            }
#ifdef DEBUG_MODE
            println("Importing module {}", absoluteModulePath);
#endif
            if (is_module_running(absoluteModulePath)) {
                throw_exception(
                    ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Circular module import detected.");
                break;
            }

            ObjString *path = new_ObjString(absoluteModulePath, gc_);
            if (auto module = get_cached_module(path)) {
                module->name_ = moduleName;
                stack_.push(NanBox::fromObj(module));
                break;
            }
            if (auto moduleFn = load_module(absoluteModulePath, moduleName)) {
                stack_.push(NanBox::fromObj(moduleFn));
                call_module(moduleFn);
                break;
            }
            throw_exception(ErrorCode::RUNTIME_MODULE_INIT_ERROR, "Import module error.");
            break;
        }
        case opCode::GET_ITER: {
            if (!NanBox::isObj(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterable object");
                break;
            }
            Value iter = NanBox::toObj(stack_.peek())->create_iter(gc_);
            if (NanBox::isNil(iter)) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Expected iterable object");
                break;
            }
            stack_.set_top_val(iter);
            break;
        }
        case opCode::ITER_HAS_NEXT: {
            if (!is_obj_iterator(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterator object");
                break;
            }
            ObjIterator *iterator = as_obj_iterator(stack_.peek());
            Value result = NanBox::fromBool(iterator->iter_->hasNext());
            stack_.set_top_val(result);
            break;
        }
        case opCode::ITER_GET_NEXT: {
            if (!is_obj_iterator(stack_.peek())) {
                throw_exception(ErrorCode::RUNTIME_TYPE_ERROR, "Expected an iterator object");
                break;
            }
            ObjIterator *iterator = as_obj_iterator(stack_.peek());
            Value nextVal = iterator->iter_->next();
            stack_.pop();
            stack_.push(nextVal);
            break;
        }
        case opCode::SETUP_EXCEPT: {
            uint16_t offset = frame_->readWord();
            uint8_t *newIp = frame_->ip + offset;
            if (e_frame_count_ == k_frame_size) {
                report_runtime_fatal_error(
                    ErrorCode::RUNTIME_STACK_OVERFLOW, "Exception Stack overflow.");
            }
            push_exception_frame(c_frame_count_, r_module_count_, newIp, stack_.size());
            break;
        }
        case opCode::END_EXCEPT: {
            pop_exception_frame();
            break;
        }
        case opCode::THROW: {
            set_err_flag();
            e_reg_ = stack_.pop();
            if (e_frame_count_ == 0) {
                report_runtime_fatal_error(
                    ErrorCode::RUNTIME_UNCAUGHT_EXCEPTION, value_string(e_reg_).c_str());
            }
            unwind_to_catch_point();
            break;
        }
        case opCode::RETURN: {
            Value result = stack_.pop();
            stack_.resize(static_cast<uint32_t>(frame_->stakBase - stack_.base()));
            result = return_from_current_frame(result);
            if (c_frame_count_ == retFrame) {
                return result;
            }
            stack_.push(result);
            break;
        }
        default:
            report_runtime_fatal_error(ErrorCode::RUNTIME_INVALID_INSTRUCTION, "Invalid opcode.");
        }
    }
}

void AriaVM::reset()
{
    stack_.reset();
    c_frame_count_ = 0;
    e_frame_count_ = 0;
    open_upvalues_ = nullptr;
    update_call_frame();
    e_reg_ = NanBox::NilValue;
    flags_ = 0;
}

void AriaVM::unwind_to_catch_point()
{
    auto ef = current_eframe();
    c_frame_count_ = ef->CframeCount;
    update_call_frame();
    r_module_count_ = ef->RmoduleCount;
    frame_->ip = ef->ip;
    stack_.resize(ef->stackSize);
    stack_.push(e_reg_);
    e_reg_ = NanBox::NilValue;
    unset_err_flag();
}

void AriaVM::throw_exception(ObjException *e)
{
    throw_exception(e->code_, e);
}

void AriaVM::throw_exception(ErrorCode code, ObjException *e)
{
    e_reg_ = NanBox::fromObj(e);
    if (e_frame_count_ == 0) {
        report_runtime_fatal_error(code, e->what());
    }
    unwind_to_catch_point();
}

void AriaVM::throw_exception(ErrorCode code, const char *message)
{
    throw_exception(new_ObjException(code, message, gc_));
}

void AriaVM::throw_exception(ErrorCode code, const String &message)
{
    throw_exception(new_ObjException(code, message.c_str(), gc_));
}

void AriaVM::report_runtime_fatal_error(ErrorCode code, const char *msg) const
{
    std::ostringstream oss;
    println(oss, runtime_error(msg));
    for (int i = c_frame_count_ - 1; i >= 0; i--) {
        const CallFrame *cf = &c_frames_[i];
        const ObjFunction *function = cf->function;
        const char *location = function->location_->c_str();
        const char *fun_name = function->name_->c_str();
        const size_t offset = static_cast<uint32_t>(cf->ip - function->chunk_->codes_ - 1);
        uint32_t line = function->chunk_->lines_[offset];
        println(oss, "{}:{} in {}", location, line, fun_name);
    }
    throw ariaRuntimeException(code, oss.str());
}

void AriaVM::maybe_debug_step(const uint32_t offset) const
{
    if (debugger_) {
        debugger_->hookBeforeExec(frame_, offset);
    }
}

} // namespace aria