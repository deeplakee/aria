#ifndef HASH_H
#define HASH_H

#include "common.h"
#include "util/util.h"
#include "object/object.h"
#include "value/value.h"

#include <cstring>

namespace aria {

inline uint32_t hashNumber(double value)
{
    uint32_t hash = 0;
    uint64_t temp;
    memcpy(&temp, &value, sizeof(double));
    hash = static_cast<uint32_t>(temp ^ (temp >> 32));
    hash = hash ^ (hash >> 16);
    hash = hash ^ (hash << 15);
    return hash;
}

inline uint32_t hashObj(Obj *obj, ObjType type)
{
    uint32_t hash = 2166136261U;

    uintptr_t ptr_val = reinterpret_cast<uintptr_t>(obj);
    hash ^= static_cast<uint32_t>(ptr_val & 0xFF);
    hash *= 16777619U;

    ptr_val >>= 8;
    while (ptr_val > 0) {
        hash ^= static_cast<uint32_t>(ptr_val & 0xFF);
        hash *= 16777619U;
        ptr_val >>= 8;
    }

    hash ^= static_cast<uint32_t>(type);
    hash *= 16777619U;

    return hash;
}

// inline uint32_t hashString(const char *str, const size_t length, uint32_t hash = 2166136261u)
// {
//     for (size_t i = 0; i < length; i++) {
//         hash ^= static_cast<uint8_t>(str[i]);
//         hash *= 16777619;
//     }
//     return hash;
// }

inline uint32_t hashString(const char *str, const size_t length, uint32_t hash = 2166136261u)
{
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + static_cast<uint8_t>(str[i]);
    }
    return hash;
}

// constexpr uint32_t kHASH_P = 31; // 质数基数，可换成131, 1315423911等
// constexpr uint32_t kHASH_MOD = 0xFFFFFFFFu; // 32位环
//
// inline uint32_t hashString(const char* str, size_t len) {
//     uint32_t h = 2166136261u;
//     for (size_t i = 0; i < len; i++) {
//         h = (h * kHASH_P + static_cast<uint8_t>(str[i])) & kHASH_MOD;
//     }
//     return h;
// }
//
// // 用于组合两个 finalized hash
// inline uint32_t hashString(uint32_t hashB, size_t lenB,uint32_t hashA) {
//     uint64_t powP = 1;
//     for (size_t i = 0; i < lenB; ++i)
//         powP = (powP * kHASH_P) & kHASH_MOD;
//     return (hashA * powP + hashB) & kHASH_MOD;
// }



} // namespace aria

#endif //HASH_H
