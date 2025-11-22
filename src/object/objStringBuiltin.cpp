#include "object/objNativeFn.h"
#include "object/objString.h"
#include "runtime/vm.h"
#include "util/nativeUtil.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"

#include <cstring>

namespace aria {

static Value builtin_length(AriaEnv *env, int argCount, Value *args)
{
    ObjString *self = asObjString(args[-1]);
    return NanBox::fromNumber(static_cast<double>(self->length));
}

static Value builtin_at(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_INTEGER(args[0], index, Argument);
    CHECK_RANGE(index, 0, self->length, Index);
    char a = self->C_str_ref()[index];
    return NanBox::fromObj(NEW_OBJSTRING(a));
}

static Value builtin_substr(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_INTEGER(args[0], start, Start index);
    CHECK_INTEGER(args[1], end, End index);
    auto size = self->length;
    CHECK_RANGE(start, 0, size, Start index);
    CHECK_RANGE(end, 0, size, End index);

    if (end < start) {
        return env->newException(
            ErrorCode::RUNTIME_OUT_OF_BOUNDS, "Start index should be smaller than end index");
    }

    ObjString *str = NEW_OBJSTRING(self->C_str(), end - start);
    return NanBox::fromObj(str);
}

static Value builtin_findstr(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_OBJSTRING(args[0], Argument);
    const ObjString *substr = asObjString(args[0]);
    const char *result = strstr(self->C_str_ref(), substr->C_str_ref());
    if (result == nullptr) {
        return NanBox::fromNumber(-1);
    }
    return NanBox::fromNumber(static_cast<double>(result - self->C_str_ref()));
}

static Value builtin_concat(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_OBJSTRING(args[0], Argument);
    const ObjString *b = asObjString(args[0]);
    ObjString *concatenatedStr = concatenateString(self, b, env->gc);
    if (concatenatedStr == nullptr) {
        fatalError(
            ErrorCode::RESOURCE_STRING_OVERFLOW,
            "String concatenation result exceeds maximum length。");
    }
    return NanBox::fromObj(concatenatedStr);
}

static Value builtin_startWith(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_OBJSTRING(args[0], Argument);
    const ObjString *substr = asObjString(args[0]);
    if (substr->length > self->length) {
        return NanBox::FalseValue;
    }

    int result = memcmp(self->C_str_ref(), substr->C_str_ref(), substr->length) == 0;
    return NanBox::fromBool(result);
}

static Value builtin_endWith(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    CHECK_OBJSTRING(args[0], Argument);
    const ObjString *substr = asObjString(args[0]);
    if (substr->length > self->length) {
        return NanBox::FalseValue;
    }

    auto result = memcmp(
                      self->C_str_ref() + self->length - substr->length,
                      substr->C_str_ref(),
                      substr->length)
                  == 0;
    return NanBox::fromBool(result);
}

static Value builtin_reverse(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    auto length = self->length;
    const char *src = self->C_str_ref();
    BUILTIN_INIT_BUFFER(dst, length);
    for (int i = 0; i < length; i++) {
        dst[i] = src[length - i - 1];
    }
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_upper(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    auto length = self->length;
    const char *src = self->C_str_ref();
    BUILTIN_INIT_BUFFER(dst, length);
    for (int i = 0; i < length; i++) {
        dst[i] = static_cast<char>(toupper(src[i]));
    }
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_lower(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    auto length = self->length;
    const char *src = self->C_str_ref();
    BUILTIN_INIT_BUFFER(dst, length);
    for (int i = 0; i < length; i++) {
        dst[i] = static_cast<char>(tolower(src[i]));
    }
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_trim(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    const char *start = self->C_str_ref();
    while (*start && isspace(*start)) {
        start++;
    }
    const char *end = self->C_str_ref() + self->length - 1;
    while (end > start && isspace(*end)) {
        end--;
    }
    size_t length = end - start + 1;
    BUILTIN_INIT_BUFFER(dst, length);
    memcpy(dst, start, length);
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_ltrim(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    const char *start = self->C_str_ref();
    while (*start && isspace(*start)) {
        start++;
    }
    const char *end = self->C_str_ref() + self->length - 1;
    size_t length = end - start + 1;
    BUILTIN_INIT_BUFFER(dst, length);
    memcpy(dst, start, length);
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_rtrim(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    const char *start = self->C_str_ref();
    const char *end = self->C_str_ref() + self->length - 1;
    while (end > start && isspace(*end)) {
        end--;
    }
    size_t length = end - start + 1;
    BUILTIN_INIT_BUFFER(dst, length);
    memcpy(dst, start, length);
    ObjString *newStrObj = NEW_OBJSTRING(dst, length);
    BUILTIN_DESTROY_BUFFER(dst, length);
    return NanBox::fromObj(newStrObj);
}

static Value builtin_split(AriaEnv *env, int argCount, Value *args)
{
    auto self = asObjString(args[-1]);
    auto gc = env->gc;
    CHECK_OBJSTRING(args[0], Argument);
    if (self->length == 0) {
        return NanBox::fromObj(NEW_OBJLIST());
    }
    const ObjString *delim = asObjString(args[0]);
    const char *srcStr = self->C_str_ref();
    const char *delimStr = delim->C_str_ref();
    ObjList *objlist = NEW_OBJLIST();
    gc->cache(NanBox::fromObj(objlist));

    if (delim->length == 0) {
        for (int i = 0; srcStr[i] != '\0'; i++) {
            if (!isspace(srcStr[i])) {
                ObjString *i_str = NEW_OBJSTRING(srcStr[i]);
                gc->cache(NanBox::fromObj(i_str));
                objlist->list->push(NanBox::fromObj(i_str));
                gc->releaseCache(1);
            }
        }
    } else {
        const size_t delim_len = strlen(delimStr);
        char *start = self->C_str();
        char *end = strstr(start, delimStr);

        while (end != nullptr) {
            size_t token_len = end - start;
            if (token_len > 0) {
                ObjString *i_str = NEW_OBJSTRING(start, token_len);
                gc->cache(NanBox::fromObj(i_str));
                objlist->list->push(NanBox::fromObj(i_str));
                gc->releaseCache(1);
            }
            start = end + delim_len;
            end = strstr(start, delimStr);
        }

        if (strlen(start) > 0) {
            ObjString *i_str = NEW_OBJSTRING(start, strlen(start));
            gc->cache(NanBox::fromObj(i_str));
            objlist->list->push(NanBox::fromObj(i_str));
            gc->releaseCache(1);
        }
    }

    gc->releaseCache(1);
    return NanBox::fromObj(objlist);
}

static Value builtin___add__(AriaEnv *env, int argCount, Value *args)
{
    const ObjString *left = asObjString(args[-1]);
    const ObjString *right = asObjString(args[0]);
    auto concatenatedStr = concatenateString(left, right, env->gc);
    if (concatenatedStr == nullptr) {
        fatalError(
            ErrorCode::RESOURCE_STRING_OVERFLOW,
            "String concatenation result exceeds maximum length。");
    }
    return NanBox::fromObj(concatenatedStr);
}

void ObjString::init(GC *_gc, ValueHashTable *builtins)
{
    bindBuiltinMethod(builtins, "length", builtin_length, 0, _gc);
    bindBuiltinMethod(builtins, "at", builtin_at, 1, _gc);
    bindBuiltinMethod(builtins, "substr", builtin_substr, 2, _gc);
    bindBuiltinMethod(builtins, "findstr", builtin_findstr, 1, _gc);
    bindBuiltinMethod(builtins, "concat", builtin_concat, 1, _gc);
    bindBuiltinMethod(builtins, "startWith", builtin_startWith, 1, _gc);
    bindBuiltinMethod(builtins, "endWith", builtin_endWith, 1, _gc);
    bindBuiltinMethod(builtins, "reverse", builtin_reverse, 0, _gc);
    bindBuiltinMethod(builtins, "upper", builtin_upper, 0, _gc);
    bindBuiltinMethod(builtins, "lower", builtin_lower, 0, _gc);
    bindBuiltinMethod(builtins, "trim", builtin_trim, 0, _gc);
    bindBuiltinMethod(builtins, "ltrim", builtin_ltrim, 0, _gc);
    bindBuiltinMethod(builtins, "rtrim", builtin_rtrim, 0, _gc);
    bindBuiltinMethod(builtins, "split", builtin_split, 1, _gc); // split()
    bindBuiltinMethod(builtins, "__add__", builtin___add__, 1, _gc);
}

} // namespace aria
