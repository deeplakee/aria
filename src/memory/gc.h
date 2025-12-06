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

    static uint64_t growCapacity(uint32_t capacity) { return capacity < 8 ? 8 : capacity * 2; }

    void markRoots();

    void traceReferences();

    void sweep();

    void collectGarbage();

    template<Trivial T>
    T *reallocate(T *pointer, size_t oldCount, size_t newCount)
    {
        bytesAllocated += (newCount - oldCount) * sizeof(T);
#ifdef DEBUG_STRESS_GC
        if (newCount > oldCount) {
            collectGarbage();
        }
#endif
        if (newCount > oldCount && bytesAllocated > nextGC) {
            collectGarbage();
        }

        if (newCount == 0) {
            free(pointer);
            return nullptr;
        }

        void *result = realloc(pointer, newCount * sizeof(T));
        if (!result) {
            fatalError(ErrorCode::RESOURCE_MEMORY_EXHAUSTED, "Memory allocation failed");
        }

        return static_cast<T *>(result);
    }

    template<Trivial T>
    T *allocateArray(size_t count)
    {
        return reallocate<T>(nullptr, 0, count);
    }

    template<Trivial T>
    T *resizeArray(T *pointer, size_t oldCount, size_t newCount)
    {
        return reallocate<T>(pointer, oldCount, newCount);
    }

    template<Trivial T>
    void freeArray(T *pointer, size_t oldCount)
    {
        reallocate<T>(pointer, oldCount, 0);
    }

    template<DerivedFromObj T, typename... Args>
    T *allocateObject(Args &&...args)
    {
        bytesAllocated += sizeof(T);
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        if (bytesAllocated > nextGC) {
            collectGarbage();
        }
        T *obj = nullptr;
        try {
            obj = new T(std::forward<Args>(args)...);
        } catch ([[maybe_unused]] std::bad_alloc &e) {
            fatalError(ErrorCode::RESOURCE_MEMORY_EXHAUSTED, "Memory allocation failed");
        }
        if constexpr (std::is_same_v<T, ObjString>) {
            obj->next = internedStringList;
            internedStringList = obj;
        } else {
            obj->next = objectList;
            objectList = obj;
        }
        return obj;
    }

    void freeObject(Obj *obj);

    void freeAllObjects();

    bool internString(ObjString *obj);

    ObjString *findInternedString(const char *chars, size_t length, uint32_t hash);

    void pushTempRoot(Value v) const { tempRootStack->push(v); }

    void popTempRoot(int n = 1) const { tempRootStack->pop_n(n); }

    void attachVM(AriaVM *vm) { runningVM = vm; }

    void attachCompiler(FunctionContext *ctx) { compilingContext = ctx; }

    void pushGrey(Obj *obj) { greyStack.push(obj); }

    static constexpr int GC_INITIAL_SIZE = 1024 * 1024;
    static constexpr int GC_HEAP_GROW_FACTOR = 2;
    static constexpr int GC_BUFFER_SIZE = 1024 * 4;

    size_t bytesAllocated;
    size_t nextGC;

    Obj *objectList;
    Obj *internedStringList;

    ValueStack *tempRootStack;
    StringPool *internPool;
    ValueHashTable *listMethods;
    ValueHashTable *mapMethods;
    ValueHashTable *stringMethods;
    ValueHashTable *iteratorMethods;

    char *stringOpBuffer;

    Lock gcLock;
    bool inGC;

    Stack<Obj *> greyStack;
    AriaVM *runningVM;
    FunctionContext *compilingContext;
};

} // namespace aria

#endif //ARIA_GC_H
