#ifndef ARIA_STRINGPOOL_H
#define ARIA_STRINGPOOL_H

#include "common.h"

namespace aria {

class GC;
class ObjString;

using ObjStringPtr = ObjString *;

class StringPool
{
public:
    StringPool() = delete;

    explicit StringPool(GC *_gc);

    ~StringPool();

    bool insert(ObjString *obj);

    ObjString *findExist(const char *chars, size_t length, uint32_t hash);

    bool remove(ObjString *obj);

    void mark();

private:
    static constexpr double TABLE_MAX_LOAD = 0.75;
    inline static ObjStringPtr TOMBSTONE = reinterpret_cast<ObjString *>(-1);

    uint32_t count;
    uint32_t capacity;
    ObjStringPtr *table;
    GC *gc;

    static ObjStringPtr *findPosition(ObjStringPtr *s_table, ObjString *s, uint32_t table_capacity);

    void adjustCapacity(uint32_t newCapacity);
};

} // namespace aria

#endif //ARIA_STRINGPOOL_H
