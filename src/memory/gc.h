#ifndef ARIA_GC_H
#define ARIA_GC_H

#include "aria.h"
#include "error/error.h"
#include "memory/stringPool.h"
#include "object/object.h"
#include "util/lock.h"
#include "util/util.h"
#include "value/valueStack.h"

#include <type_traits>

namespace aria {

class StringPool;
class ValueArray;
class ValueHashTable;
class ValueStack;
class ObjString;
class FunctionContext;

template<typename T>
concept DerivedFromObj = std::is_base_of_v<Obj, T>;

template<typename T>
concept Trivial = std::is_trivial_v<T>;

class GC
{
public:
    GC();

    ~GC();

    static uint64_t grow_capacity(uint32_t capacity) { return capacity < 8 ? 8 : capacity * 2; }

    void mark_roots();

    void trace_references();

    void sweep();

    void collect_garbage();

    template<Trivial T>
    T *reallocate(T *pointer, size_t old_count, size_t new_count)
    {
        bytes_allocated_ += (new_count - old_count) * sizeof(T);
#ifdef DEBUG_STRESS_GC
        if (new_count > old_count) {
            collect_garbage();
        }
#endif
        if (new_count > old_count && bytes_allocated_ > next_gc_) {
            collect_garbage();
        }

        if (new_count == 0) {
            free(pointer);
            return nullptr;
        }

        void *result = realloc(pointer, new_count * sizeof(T));
        if (!result) {
            fatal_error(ErrorCode::RESOURCE_MEMORY_EXHAUSTED, "Memory allocation failed");
        }

        return static_cast<T *>(result);
    }

    template<Trivial T>
    T *allocate_array(size_t count)
    {
        return reallocate<T>(nullptr, 0, count);
    }

    template<Trivial T>
    T *resize_array(T *pointer, size_t old_count, size_t new_count)
    {
        return reallocate<T>(pointer, old_count, new_count);
    }

    template<Trivial T>
    void free_array(T *pointer, size_t old_count)
    {
        reallocate<T>(pointer, old_count, 0);
    }

    template<DerivedFromObj T, typename... Args>
    T *allocate_object(Args &&...args)
    {
        bytes_allocated_ += sizeof(T);
#ifdef DEBUG_STRESS_GC
        collect_garbage();
#endif
        if (bytes_allocated_ > next_gc_) {
            collect_garbage();
        }
        T *obj = nullptr;
        try {
            obj = new T(std::forward<Args>(args)...);
        } catch ([[maybe_unused]] std::bad_alloc &e) {
            fatal_error(ErrorCode::RESOURCE_MEMORY_EXHAUSTED, "Memory allocation failed");
        }
        if constexpr (std::is_same_v<T, ObjString>) {
            obj->next_ = interned_string_list_;
            interned_string_list_ = obj;
        } else {
            obj->next_ = object_list_;
            object_list_ = obj;
        }
        return obj;
    }

    void free_object(Obj *obj);

    void free_all_objects();

    bool intern_string(ObjString *obj);

    ObjString *find_interned_string(const char *chars, size_t length, uint32_t hash);

    void push_temp_root(Value v) const { temp_root_stack_->push(v); }

    void pop_temp_root(int n = 1) const { temp_root_stack_->pop_n(n); }

    void attach_vm(AriaVM *vm) { running_vm_ = vm; }

    void attach_compiler(FunctionContext *ctx) { compiling_context_ = ctx; }

    void push_grey(Obj *obj) { grey_stack_.push(obj); }

    static constexpr int k_gc_initial_size = 1024 * 1024;
    static constexpr int k_gc_heap_grow_factor = 2;
    static constexpr int k_gc_buffer_size = 1024 * 4;

    size_t bytes_allocated_;
    size_t next_gc_;

    Obj *object_list_;
    Obj *interned_string_list_;

    ValueStack *temp_root_stack_;
    StringPool *intern_pool_;
    ValueHashTable *list_methods_;
    ValueHashTable *map_methods_;
    ValueHashTable *string_methods_;
    ValueHashTable *iterator_methods_;

    char *string_op_buffer_;

    Lock gc_lock_;
    bool in_gc_;

    Stack<Obj *> grey_stack_;
    AriaVM *running_vm_;
    FunctionContext *compiling_context_;
};

// RAII guard for temporary GC roots
class GcTempRootGuard
{
public:
    GcTempRootGuard(GC *gc, Value v)
        : gc_{gc}
        , count_{1}
    {
        gc_->push_temp_root(v);
    }

    explicit GcTempRootGuard(GC *gc)
        : gc_{gc}
        , count_{0}
    {}

    void push(Value v)
    {
        gc_->push_temp_root(v);
        count_++;
    }

    ~GcTempRootGuard() { gc_->pop_temp_root(count_); }

    GcTempRootGuard(const GcTempRootGuard &) = delete;
    GcTempRootGuard &operator=(const GcTempRootGuard &) = delete;
    GcTempRootGuard(GcTempRootGuard &&) = delete;
    GcTempRootGuard &operator=(GcTempRootGuard &&) = delete;

private:
    GC *gc_;
    int count_;
};

} // namespace aria

#endif //ARIA_GC_H
