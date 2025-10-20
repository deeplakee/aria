#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "common.h"
#include <utility>

namespace aria {

struct Breakpoint
{
    String path;
    uint32_t line;
    uint8_t *ins;

    Breakpoint()
        : line{0}
        , ins{nullptr}
    {}

    Breakpoint(String path, uint32_t line)
        : path{std::move(path)}
        , line{line}
        , ins{nullptr}
    {}
};

} // namespace aria

#endif //BREAKPOINT_H
