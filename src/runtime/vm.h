#ifndef VM_H
#define VM_H

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

enum class interpretResult { SUCCESS, SRC_FILE_ERROR, COMPILE_ERROR, RUNTIME_ERROR };

class AriaVM
{
public:
    AriaVM();

    ~AriaVM();

    interpretResult interpret(String srcFilePath, String source);

    interpretResult interpret(String source);

    interpretResult interpretFromFile(const String &srcFilePath);

    Value runFunction(ObjFunction *fun, int argCount, const Value *args);

    void setDebugger(AriaDebugger *_debugger);

    void defineNativeFn(
        const char *name, int arity, NativeFn_t function, bool acceptsVarargs = false) const;

    void defineNativeVar(const char *name, Value value) const;

    Value newException(const char *msg);

    Value newException(ErrorCode code, const char *msg);

    Value newException(ErrorCode code, const String &msg);

    void throwException(ObjException *e);

    void throwException(ErrorCode code, ObjException *e);

    void throwException(ErrorCode code, const char *message);

    void throwException(ErrorCode code, const String &message);

    void reportRuntimeFatalError(ErrorCode code, const char *msg) const;

    GC *gc;

private:
    friend class ObjFunction;
    friend class GC;
    friend class VMStateHelper;

    enum flagIndex {
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
    void set_##what##_flag() { flags = flags | (1 << index_##what); } \
    void unset_##what##_flag() { flags = flags & ~(1 << index_##what); } \
    [[nodiscard]] bool get_##what##_flag() const { return (flags & (1 << index_##what)); }

    defFlag(err);

#undef defFlag

    static constexpr int FRAME_SIZE = 256;

    CallFrame *Cframes;
    ExceptionFrame *Eframes;
    Value *Rmodules;
    int CframeCount;
    int EframeCount;
    int RmoduleCount;
    CallFrame *frame;
    Chunk *chunk;

    Value E_REG;
    uint8_t flags;

    ValueHashTable *builtIn;
    ValueHashTable *cachedModules;
    ValueStack stack;
    ObjUpvalue *openUpvalues;
    String ariaDir;
    ValueHashTable *globals;
    AriaDebugger *debugger;

    ExceptionFrame *currentEframe() { return &Eframes[EframeCount - 1]; }

    Value *currentRmodule() { return &Rmodules[RmoduleCount - 1]; }

    void updateCallFrame();

    void pushCallFrame(ObjFunction *_function, uint8_t *_ip, Value *_stakBase);

    void pushExceptionFrame(int CframeCount_, int RmoduleCount_, uint8_t *_ip, uint32_t _stackSize);

    void pushRunningModule(const ObjFunction *_function);

    void popCallFrame();

    void popExceptionFrame();

    void popRunningModule();

    void packVarargs(int argCount, int arity);

    ObjModule *cacheModule(ObjFunction *moduleFn);

    Value createCallFrame(ObjFunction *function);

    Value returnFromCurrentModule(Value result);

    Value returnFromCurrentFrame(Value result);

    Value callValue(Value callee, int argCount);

    Value callModule(ObjFunction *module);

    Value callFunction(ObjFunction *function, int argCount);

    Value callNativeFn(ObjNativeFn *native, int argCount);

    Value callNewInstance(ObjClass *klass, int argCount);

    Value callBoundMethod(const ObjBoundMethod *method, int argCount);

    ObjUpvalue *captureUpvalue(Value *local);

    void closeUpvalues(const Value *last);

    void registerNative() const;

    bool isModuleRunning(const String &path) const;

    ObjModule *getCachedModule(ObjString *path);

    ObjFunction *loadModule(const String &path, ObjString *moduleName);

    Value run(int retFrame = 0);

    void reset();

    void unwindToCatchPoint();

    void maybeDebugStep(uint32_t offset) const;
};

} // namespace aria

#endif //VM_H
