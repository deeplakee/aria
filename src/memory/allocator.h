#ifndef ARIA_ALLOCATOR_H
#define ARIA_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <vector>

namespace aria {

template<typename T, size_t poolSize = 256>
class Allocator
{
public:
    Allocator() = default;

    ~Allocator()
    {
        for (void *pool : pools) {
            ::operator delete(pool, static_cast<std::align_val_t>(alignof(T)));
        }
    }

    template<typename... Args>
    T *allocate(Args &&...args)
    {
        if (freeList.empty()) {
            addPool();
        }

        T *p = freeList.back();
        freeList.pop_back();

        new (p) T(std::forward<Args>(args)...);
        return p;
    }

    void deallocate(T *p)
    {
#ifdef DEBUG_MODE
        if (!isFromPools(p)) {
            throw std::runtime_error("Attempt to deallocate pointer not from pool");
        }
        for (auto q : freeList) {
            if (q == p)
                throw std::runtime_error("Double free detected");
        }
#endif
        p->~T();
        freeList.push_back(p);
    }

private:
    std::vector<T *> freeList;
    std::vector<void *> pools;

    bool isFromPools(T *p)
    {
        for (void *raw : pools) {
            T *start = static_cast<T *>(raw);
            T *end = start + poolSize;

            if (p >= start && p < end)
                return true;
        }
        return false;
    }

    void addPool()
    {
        const size_t bytes = sizeof(T) * poolSize;

        void *rawMem = ::operator new(bytes, static_cast<std::align_val_t>(alignof(T)));
        pools.push_back(rawMem);

        T *poolStart = static_cast<T *>(rawMem);

        for (size_t i = 0; i < poolSize; ++i) {
            freeList.push_back(poolStart + i);
        }
    }
};

} // namespace aria

#endif //ARIA_ALLOCATOR_H
