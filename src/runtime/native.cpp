#include "runtime/native.h"
#include "common.h"
#include "memory/gc.h"
#include "object/objList.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/nativeUtil.h"
#include "value/valueArray.h"

#include <cstring>
#include <ctime>
#include <random>

namespace aria {

Value Native::_aria_clock_(AriaEnv *env, int argCount, Value *args)
{
    return NanBox::fromNumber(static_cast<double>(::clock()) / CLOCKS_PER_SEC);
}

Value Native::_aria_random_(AriaEnv *env, int argCount, Value *args)
{
    CHECK_INTEGER(args[0], min, Argument);
    CHECK_INTEGER(args[1], max, Argument);
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dis;
    dis.param(
        std::uniform_int_distribution<uint32_t>::param_type{
            static_cast<uint32_t>(min), static_cast<uint32_t>(max)});

    return NanBox::fromNumber(dis(gen));
}

Value Native::_aria_println_(AriaEnv *env, int argCount, Value *args)
{
    ObjList *list = as_obj_list(args[0]);
    String formatStr = value_string((*list->list_)[0]);

    if (argCount == 1) {
        std::cout << value_string((*list->list_)[0]) << '\n';
        return NanBox::NilValue;
    }

    try {
        String result;
        int argIndex = 1;

        for (size_t i = 0; i < formatStr.length(); ++i) {
            if (formatStr[i] == '{' && i + 1 < formatStr.length() && formatStr[i + 1] == '}') {
                if (argIndex < argCount) {
                    result += value_string((*list->list_)[argIndex++]);
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
        return env->new_exception(ErrorCode::RUNTIME_UNKNOWN, e.what());
    }

    return list->list_->size();
}

Value Native::_aria_readline_(AriaEnv *env, int argCount, Value *args)
{
    String line;
    std::getline(std::cin, line);
    ObjString *objLineStr = new_ObjString(line, env->gc_);
    return NanBox::fromObj(objLineStr);
}

Value Native::_aria_typeof_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = new_ObjString(value_type_string(args[0]), env->gc_);
    return NanBox::fromObj(str);
}

Value Native::_aria_str_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = new_ObjString(value_string(args[0]), env->gc_);
    return NanBox::fromObj(str);
}

Value Native::_aria_repr_(AriaEnv *env, int argCount, Value *args)
{
    ObjString *str = new_ObjString(value_representation(args[0]), env->gc_);
    return NanBox::fromObj(str);
}

Value Native::_aria_num_(AriaEnv *env, int argCount, Value *args)
{
    CHECK_OBJSTRING(args[0], argument);
    try {
        return NanBox::fromNumber(std::stod(as_c_string(args[0])));
    } catch ([[maybe_unused]] const std::exception &e) {
        return env->new_exception(ErrorCode::RUNTIME_UNKNOWN, "Conversion failed");
    }
}

Value Native::_aria_bool_(AriaEnv *env, int argCount, Value *args)
{
    CHECK_OBJSTRING(args[0], argument);
    ObjString *str = as_obj_string(args[0]);
    if (str->length_ == 4 && memcmp(str->c_str(), "true", 4) == 0) {
        return NanBox::TrueValue;
    }
    if (str->length_ == 5 && memcmp(str->c_str(), "false", 5) == 0) {
        return NanBox::FalseValue;
    }
    return env->new_exception(ErrorCode::RUNTIME_UNKNOWN, "Invalid boolean string");
}

Value Native::_aria_copy_(AriaEnv *env, int argCount, Value *args)
{
    if (NanBox::isObj(args[0])) {
        return NanBox::toObj(args[0])->copy(env->gc_);
    }
    return NanBox::NilValue;
}

Value Native::_aria_equals_(AriaEnv *env, int argCount, Value *args)
{
    return NanBox::fromBool(values_equal(args[0], args[1]));
}

Value Native::_aria_iter_(AriaEnv *env, int argCount, Value *args)
{
    if (NanBox::isObj(args[0])) {
        return NanBox::toObj(args[0])->create_iter(env->gc_);
    }
    return NanBox::NilValue;
}

Value Native::_aria_exit_(AriaEnv *env, int argCount, Value *args)
{
    if (!NanBox::isNumber(args[0])) {
        exit(1);
    }
    exit(static_cast<int>(NanBox::toNumber(args[0])));
}

Value Native::_aria__foo__(AriaEnv *env, int argCount, Value *args)
{
    return NanBox::NilValue;
}

List<NativeVarEntry> &Native::nativeVarTable()
{
    static List<NativeVarEntry> table = {
        {"__platform__", obj_wrap(new_ObjString(k_platform, gc))},
        {"__version__", obj_wrap(new_ObjString(k_version, gc))},
        //{"pi", num_wrap(pi)},
        //{"e", num_wrap(e)},
        {"_", val_wrap(NanBox::NilValue)},
    };
    return table;
}

List<NativeFnEntry> &Native::nativeFnTable()
{
    static List<NativeFnEntry> table = {
        {"clock", 0, _aria_clock_},
        {"random", 2, _aria_random_},
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
