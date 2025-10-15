#include "runtime/native.h"
#include "common.h"
#include "memory/gc.h"
#include "object/objList.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "value/valueArray.h"

#include <ctime>
#include <random>
#include <cstring>

namespace aria {

Value Native::_aria_clock_(AriaEnv *env, int argCount, Value *args)
{
    return number_val(static_cast<double>(::clock()) / CLOCKS_PER_SEC);
}

Value Native::_aria_random_(AriaEnv *env, int argCount, Value *args)
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<> dis(0, UINT32_MAX);

    return number_val(dis(gen));
}

Value Native::_aria_println_(AriaEnv *env, int argCount, Value *args)
{
    ObjList *list = as_ObjList(args[0]);
    String formatStr = valueString((*list->list)[0]);

    if (argCount == 1) {
        std::cout << valueString((*list->list)[0]) << '\n';
        return nil_val;
    }

    try {
        String result;
        int argIndex = 1;

        for (size_t i = 0; i < formatStr.length(); ++i) {
            if (formatStr[i] == '{' && i + 1 < formatStr.length() && formatStr[i + 1] == '}') {
                if (argIndex < argCount) {
                    result += valueString((*list->list)[argIndex++]);
                } else {
                    result += "{}";
                }
                i++; // skip '}'
            } else {
                result += formatStr[i];
            }
        }

        std::cout << result << '\n';
    } catch (const std::exception &e) {
        return env->newException(ErrorCode::RUNTIME_UNKNOWN, e.what());
    }

    return list->list->size();
}

Value Native::_aria_readline_(AriaEnv *env, int argCount, Value *args)
{
    String line;
    std::getline(std::cin, line);
    ObjString *objLineStr = newObjString(line, env->gc);
    return obj_val(objLineStr);
}

Value Native::_aria_typeof_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = newObjString(valueTypeString(args[0]), env->gc);
    return obj_val(str);
}

Value Native::_aria_str_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = newObjString(valueString(args[0]), env->gc);
    return obj_val(str);
}

Value Native::_aria_repr_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = newObjString(valueRepresentation(args[0]), env->gc);
    return obj_val(str);
}

Value Native::_aria_num_(AriaEnv *env, int argCount, Value *args)
{
    if (!is_ObjString(args[0])) {
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, "argument must be a string");
    }
    try {
        return number_val(std::stod(as_ObjString(args[0])->C_str_ref()));
    } catch ([[maybe_unused]] const std::exception &e) {
        return env->newException(ErrorCode::RUNTIME_UNKNOWN, "Conversion failed");
    }
}

Value Native::_aria_bool_(AriaEnv *env, int argCount, Value *args)
{
    if (!is_ObjString(args[0])) {
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, "argument must be a string");
    }
    ObjString *str = as_ObjString(args[0]);
    if (str->length == 4 && memcmp(str->C_str_ref(), "true", 4) == 0) {
        return true_val;
    }
    if (str->length == 5 && memcmp(str->C_str_ref(), "false", 5) == 0) {
        return false_val;
    }
    return env->newException(ErrorCode::RUNTIME_UNKNOWN, "Invalid boolean string");
}

Value Native::_aria_copy_(AriaEnv *env, int argCount, Value *args)
{
    if (is_obj(args[0])) {
        return as_obj(args[0])->copy(env->gc);
    }
    return nil_val;
}

Value Native::_aria_equals_(AriaEnv *env, int argCount, Value *args)
{
    return bool_val(valuesEqual(args[0], args[1]));
}

Value Native::_aria_iter_(AriaEnv *env, int argCount, Value *args)
{
    if (is_obj(args[0])) {
        return as_obj(args[0])->createIter(env->gc);
    }
    return nil_val;
}

Value Native::_aria_exit_(AriaEnv *env, int argCount, Value *args)
{
    if (!is_number(args[0])) {
        exit(1);
    }
    exit(static_cast<int>(as_number(args[0])));
}

Value Native::_aria__foo__(AriaEnv *env, int argCount, Value *args)
{
    return nil_val;
}

List<NativeVarEntry> &Native::nativeVarTable()
{
    static List<NativeVarEntry> table = {
        {"__platform__", obj_wrap(newObjString(platform, gc))},
        {"__version__", obj_wrap(newObjString(version, gc))},
        //{"pi", num_wrap(pi)},
        //{"e", num_wrap(e)},
        {"_", val_wrap(nil_val)},
    };
    return table;
}

List<NativeFnEntry> &Native::nativeFnTable()
{
    static List<NativeFnEntry> table = {
        {"clock", 0, _aria_clock_},
        {"random", 0, _aria_random_},
        {"println", 0, _aria_println_, true},
        {"readline", 0, _aria_readline_},
        {"typeof", 1, _aria_typeof_},
        {"str", 1, _aria_str_},
        {"repr", 1, _aria_repr_},
        {"num", 1, _aria_num_},
        {"bool", 1, _aria_bool_},
        {"copy", 1, _aria_copy_},
        {"equals", 2, _aria_equals_},
        {"iter", 1, _aria_iter_},
        {"exit", 1, _aria_exit_},
        {"_foo_", 1, _aria__foo__},
    };
    return table;
}

} // namespace aria
