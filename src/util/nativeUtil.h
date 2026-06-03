#ifndef ARIA_NATIVEUTIL_H
#define ARIA_NATIVEUTIL_H

namespace aria {

#define CHECK_OBJLIST(val, what) \
    if (!is_obj_list(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an list"); \
    }

#define CHECK_OBJMAP(val, what) \
    if (!is_obj_map(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a map"); \
    }

#define CHECK_OBJSTRING(val, what) \
    if (!is_obj_string(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a string"); \
    }

#define CHECK_INTEGER(val, int_result, what) \
    int int_result = -1; \
    do { \
        if (!NanBox::isNumber(val)) { \
            return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an integer"); \
        } \
        auto _double_val_ = NanBox::toNumber(val); \
        (int_result) = static_cast<int>(_double_val_); \
        if ((int_result) != _double_val_) { \
            return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be an integer"); \
        } \
    } while (0)

#define CHECK_NUMBER(val, int_result, what) \
    if (!NanBox::isNumber(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a number"); \
    }

#define CHECK_BOOL(val, int_result, what) \
    if (!NanBox::isBool(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a boolean"); \
    }

#define CHECK_NIL(val, int_result, what) \
    if (!NanBox::isNil(val)) { \
        return env->new_exception(ErrorCode::RUNTIME_TYPE_ERROR, #what " must be a nil"); \
    }

#define CHECK_RANGE(index, start, end, what) \
    if ((index) >= (end) || (index) < (start)) { \
        return env->new_exception(ErrorCode::RUNTIME_OUT_OF_BOUNDS, #what " out of range"); \
    }

#define NEW_OBJLIST(...) new_ObjList(__VA_ARGS__ __VA_OPT__(, ) env->gc_)

#define NEW_OBJMAP(...) new_ObjMap(__VA_ARGS__ __VA_OPT__(, ) env->gc_)

#define NEW_OBJSTRING(...) new_ObjString(__VA_ARGS__ __VA_OPT__(, ) env->gc_)

#define NEW_OBJSTRING_FROM_RAW(...) new_obj_string_from_raw(__VA_ARGS__ __VA_OPT__(, ) env->gc_)

#define BUILTIN_INIT_BUFFER(bufferName, length) \
    bool __##bufferName##__useBuffer__ = length < GC::k_gc_buffer_size; \
    char *bufferName = __##bufferName##__useBuffer__ ? env->gc_->string_op_buffer_ \
                                     : env->gc_->allocate_array<char>(length + 1); \
    bufferName[length] = '\0';

#define BUILTIN_DESTROY_BUFFER(bufferName, length) \
    if (!__##bufferName##__useBuffer__) { \
        env->gc_->free_array<char>(bufferName, length + 1); \
    }

} // namespace aria

#endif //ARIA_NATIVEUTIL_H
