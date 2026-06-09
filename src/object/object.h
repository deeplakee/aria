#ifndef ARIA_OBJECT_H
#define ARIA_OBJECT_H

#include "aria.h"
#include "common.h"
#include "error/ErrorCode.h"
#include "util/util.h"
#include "value/value.h"

namespace aria {

class GC;
class ObjString;
class ObjFunction;
class ObjNativeFn;
class ObjUpvalue;
class ObjClass;
class ObjInstance;
class ObjBoundMethod;
class ObjList;
class ObjMap;
class ObjModule;
class ObjIterator;
class ObjException;

enum class ObjType : uint8_t {
    BASE,
    STRING,
    FUNCTION,
    NATIVE_FN,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    LIST,
    MAP,
    MODULE,
    ITERATOR,
    EXCEPTION,
};

// RAII guard for cycle detection in to_string/repr
class PrintGuard
{
public:
    explicit PrintGuard(const Obj *obj) { stack_().push_back(obj); }
    ~PrintGuard() { stack_().pop_back(); }

    PrintGuard(const PrintGuard &) = delete;
    PrintGuard &operator=(const PrintGuard &) = delete;

    static bool is_cycle(const Obj *obj)
    {
        return std::find(stack_().begin(), stack_().end(), obj) != stack_().end();
    }

private:
    static List<const Obj *> &stack_()
    {
        static thread_local List<const Obj *> s;
        return s;
    }
};

class Obj
{
public:
    Obj *next_;
    GC *gc_;
    uint32_t hash_;
    ObjType type_;
    bool is_marked_;

    static const char *obj_type_str_[];

    static const char *type_to_str(ObjType t) { return obj_type_str_[static_cast<uint8_t>(t)]; }

    Obj() = delete;

    Obj(ObjType type, uint32_t hash, GC *gc)
        : next_{nullptr}
        , gc_{gc}
        , hash_{hash}
        , type_{type}
        , is_marked_{false}
    {}

    virtual ~Obj() = default;

    virtual String to_string() { return value_type_string(NanBox::fromObj(this)); }

    virtual String representation() { return this->to_string(); }

    virtual size_t obj_size() = 0;

    void mark();

    Value new_exception(ErrorCode code, const char *msg);

    virtual void blacken() = 0;

    virtual Value op_call(AriaEnv *env, int arg_count);

    virtual Value get_by_field(ObjString *name, Value &value) { return NanBox::FalseValue; }

    virtual Value set_by_field(ObjString *name, Value value) { return NanBox::FalseValue; }

    virtual Value get_by_index(Value k, Value &v) { return NanBox::FalseValue; }

    virtual Value set_by_index(Value k, Value v) { return NanBox::FalseValue; }

    virtual Value create_iter(GC *gc) { return NanBox::NilValue; }

    virtual Value copy(GC *gc) { return NanBox::NilValue; }
};

inline ObjType get_obj_type(const Value value)
{
    return NanBox::toObj(value)->type_;
}

inline bool is_obj_type(const Value value, const ObjType type)
{
    return NanBox::isObj(value) && get_obj_type(value) == type;
}

// Type-to-ObjType mapping
template<typename T>
struct ObjTypeMap;

#define DEFINE_OBJ_TYPE_MAP(CppType, EnumType) \
    template<>                                  \
    struct ObjTypeMap<CppType>                  \
    {                                           \
        static constexpr ObjType value = EnumType; \
    };

DEFINE_OBJ_TYPE_MAP(ObjString, ObjType::STRING)
DEFINE_OBJ_TYPE_MAP(ObjFunction, ObjType::FUNCTION)
DEFINE_OBJ_TYPE_MAP(ObjNativeFn, ObjType::NATIVE_FN)
DEFINE_OBJ_TYPE_MAP(ObjUpvalue, ObjType::UPVALUE)
DEFINE_OBJ_TYPE_MAP(ObjClass, ObjType::CLASS)
DEFINE_OBJ_TYPE_MAP(ObjInstance, ObjType::INSTANCE)
DEFINE_OBJ_TYPE_MAP(ObjBoundMethod, ObjType::BOUND_METHOD)
DEFINE_OBJ_TYPE_MAP(ObjList, ObjType::LIST)
DEFINE_OBJ_TYPE_MAP(ObjMap, ObjType::MAP)
DEFINE_OBJ_TYPE_MAP(ObjModule, ObjType::MODULE)
DEFINE_OBJ_TYPE_MAP(ObjIterator, ObjType::ITERATOR)
DEFINE_OBJ_TYPE_MAP(ObjException, ObjType::EXCEPTION)

#undef DEFINE_OBJ_TYPE_MAP

template<typename T>
bool is_a(const Value value)
{
    return is_obj_type(value, ObjTypeMap<T>::value);
}

// Unified cast function template for all Obj subclasses
template<typename T>
T *as_Obj(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<T *>(NanBox::toObj(value));
#else
    return static_cast<T *>(NanBox::toObj(value));
#endif
}

// Log object allocation in debug mode
inline void log_obj_allocation(Obj *obj)
{
#ifdef DEBUG_LOG_GC
    println(
        "{:p} allocate bytes {} (object {})",
        to_void_ptr(obj),
        obj->obj_size(),
        Obj::type_to_str(obj->type_));
#endif
}

} // namespace aria

#endif //ARIA_OBJECT_H
