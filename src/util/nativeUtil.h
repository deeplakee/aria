#ifndef ARIA_NATIVEUTIL_H
#define ARIA_NATIVEUTIL_H

namespace aria {

#define CHECK_OBJLIST(val, what) \
    if (!isObjList(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an list"); \
    }

#define CHECK_OBJMAP(val, what) \
    if (!isObjMap(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a map"); \
    }

#define CHECK_OBJSTRING(val, what) \
    if (!isObjString(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a map"); \
    }

#define CHECK_INTEGER(val, int_result, what) \
    int int_result = -1; \
    do { \
        if (!NanBox::isNumber(val)) { \
            return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an integer"); \
        } \
        auto _double_val_ = NanBox::toNumber(val); \
        (int_result) = static_cast<int>(_double_val_); \
        if ((int_result) != _double_val_) { \
            return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an integer"); \
        } \
    } while (0)

#define CHECK_NUMBER(val, int_result, what) \
    if (!NanBox::isNumber(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a number"); \
    }

#define CHECK_BOOL(val, int_result, what) \
    if (!NanBox::isBool(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a boolean"); \
    }

#define CHECK_NIL(val, int_result, what) \
    if (!NanBox::isNil(val)) { \
        return env->newException(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a nil"); \
    }

#define CHECK_RANGE(index, start, end, what) \
    if ((index) >= (end) || (index) < (start)) { \
        return env->newException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, #what " out of range"); \
    }

#define NEW_OBJLIST(...) newObjList(__VA_ARGS__ __VA_OPT__(, ) env->gc)

#define NEW_OBJMAP(...) newObjMap(__VA_ARGS__ __VA_OPT__(, ) env->gc)

#define NEW_OBJSTRING(...) newObjString(__VA_ARGS__ __VA_OPT__(, ) env->gc)

#define NEW_OBJSTRING_FROM_RAW(...) newObjStringFromRaw(__VA_ARGS__ __VA_OPT__(, ) env->gc)

#define BUILTIN_INIT_BUFFER(bufferName, length) \
    bool __##bufferName##__useBuffer__ = length < GC::GC_BUFFER_SIZE; \
    char *bufferName = __##bufferName##__useBuffer__ ? env->gc->stringOpBuffer \
                                     : env->gc->allocateArray<char>(length + 1); \
    bufferName[length] = '\0';

#define BUILTIN_DESTROY_BUFFER(bufferName, length) \
    if (!__##bufferName##__useBuffer__) { \
        env->gc->freeArray<char>(bufferName, length + 1); \
    }

} // namespace aria

#endif //ARIA_NATIVEUTIL_H
