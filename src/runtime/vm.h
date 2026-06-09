#ifndef ARIA_VM_H
#define ARIA_VM_H

#include "aria.h"
#include "common.h"
#include "memory/gc.h"
#include "object/funDef.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objException.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "runtime/callFrame.h"
#include "runtime/exceptionFrame.h"

namespace aria {
class ObjModule;
class ObjUpvalue;
class ValueHashTable;
class AriaDebugger;

enum class InterpretResult { SUCCESS, SRC_FILE_ERROR, COMPILE_ERROR, RUNTIME_ERROR };

enum class NumericBinOp { GT, GE, LT, LE, SUB, MUL, DIV, MOD };

class AriaVM
{
public:
    AriaVM();

    ~AriaVM();

    InterpretResult interpret(String src_file_path, String source);

    InterpretResult interpret(String source);

    InterpretResult interpret_from_file(const String &src_file_path);

    Value run_function(ObjFunction *fun, int arg_count, const Value *args);

    void set_debugger(AriaDebugger *debugger);

    void define_native_fn(
        const char *name, int arity, NativeFn_t function, bool accepts_varargs = false) const;

    void define_native_var(const char *name, Value value) const;

    Value new_exception(ErrorCode code, const char *msg);

    Value new_exception(ErrorCode code, const String &msg);

    void throw_exception(ObjException *e);

    void throw_exception(ErrorCode code, ObjException *e);

    void throw_exception(ErrorCode code, const char *message);

    void throw_exception(ErrorCode code, const String &message);

    [[noreturn]]
    void report_runtime_fatal_error(ErrorCode code, const char *msg) const;

    void mark_gc_roots();

    GC *gc_;

private:
    InterpretResult run_source(String sourceLocation, String source);

    friend class ObjFunction;
    friend class VMStateHelper;

    enum FlagIndex {
        undefined0 = 0,
        index_err,
        undefined2,
        undefined3,
        undefined4,
        undefined5,
        undefined6,
        undefined7,
    };

#define defFlag(what) \
    void set_##what##_flag() { flags_ = flags_ | (1 << index_##what); } \
    void unset_##what##_flag() { flags_ = flags_ & ~(1 << index_##what); } \
    [[nodiscard]] bool get_##what##_flag() const { return (flags_ & (1 << index_##what)); }

    defFlag(err);

#undef defFlag

    static constexpr int k_frame_size = 256;

    CallFrame *c_frames_;
    ExceptionFrame *e_frames_;
    Value *r_modules_;
    int c_frame_count_;
    int e_frame_count_;
    int r_module_count_;
    CallFrame *frame_;
    Chunk *chunk_;

    Value e_reg_;
    uint8_t flags_;

    ValueHashTable *built_in_;
    ValueHashTable *cached_modules_;
    ValueStack stack_;
    ObjUpvalue *open_upvalues_;
    String aria_dir_;
    ValueHashTable *globals_;
    AriaDebugger *debugger_;

    ExceptionFrame *current_eframe() { return &e_frames_[e_frame_count_ - 1]; }

    Value *current_rmodule() { return &r_modules_[r_module_count_ - 1]; }

    void update_call_frame();

    void push_call_frame(ObjFunction *function, uint8_t *ip, Value *stack_base);

    void push_exception_frame(
        int c_frame_count, int r_module_count, uint8_t *ip, uint32_t stack_size);

    void push_running_module(const ObjFunction *function);

    void pop_call_frame();

    void pop_exception_frame();

    void pop_running_module();

    void pack_varargs(int arg_count, int arity);

    ObjModule *cache_module(ObjFunction *module_fn);

    Value create_call_frame(ObjFunction *function);

    Value return_from_current_module(Value result);

    Value return_from_current_frame(Value result);

    Value call_value(Value callee, int arg_count);

    Value call_module(ObjFunction *module);

    Value call_function(ObjFunction *function, int arg_count);

    Value call_native_fn(ObjNativeFn *native, int arg_count);

    Value call_new_instance(ObjClass *klass, int arg_count);

    Value call_bound_method(const ObjBoundMethod *method, int arg_count);

    ObjUpvalue *capture_upvalue(Value *local);

    void close_upvalues(const Value *last);

    template<NumericBinOp op>
    bool numeric_bin_op();

    void register_native() const;

    [[nodiscard]] bool is_module_running(const String &path) const;

    ObjModule *get_cached_module(ObjString *path);

    ObjFunction *load_module(const String &path, ObjString *module_name);

    Value run(int ret_frame = 0);

    void reset();

    void unwind_to_catch_point();

    void maybe_debug_step(uint32_t offset) const;
};

} // namespace aria

#endif //ARIA_VM_H
