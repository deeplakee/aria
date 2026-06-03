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

    explicit StringPool(GC *gc);

    ~StringPool();

    bool insert(ObjString *obj);

    ObjString *find_exist(const char *chars, size_t length, uint32_t hash);

    bool remove(ObjString *obj);

    void mark();

private:
    static constexpr double k_table_max_load = 0.75;
    inline static ObjStringPtr k_tombstone = reinterpret_cast<ObjString *>(-1);

    uint32_t count_;
    uint32_t capacity_;
    ObjStringPtr *table_;
    GC *gc_;

    static ObjStringPtr *find_position(ObjStringPtr *s_table, ObjString *s, uint32_t table_capacity);

    void adjust_capacity(uint32_t new_capacity);
};

} // namespace aria

#endif //ARIA_STRINGPOOL_H
