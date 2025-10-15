#ifndef CALLFRAME_H
#define CALLFRAME_H

#include "common.h"
#include "value/value.h"

namespace aria {

class ObjFunction;

struct CallFrame
{
    CallFrame(ObjFunction *_function, uint8_t *_ip, Value *_stakBase)
        : function{_function}
        , ip{_ip}
        , stakBase{_stakBase}
    {}

    CallFrame()
        : CallFrame(nullptr, nullptr, nullptr)
    {}

    ~CallFrame() = default;

    void init(ObjFunction *_function, uint8_t *_ip, Value *_stakBase)
    {
        this->function = _function;
        this->ip = _ip;
        this->stakBase = _stakBase;
    }

    void copy(CallFrame *other)
    {
        this->function = other->function;
        this->ip = other->ip;
        this->stakBase = other->stakBase;
    }

    ObjFunction *function;
    uint8_t *ip;
    Value *stakBase;
};

} // namespace aria

#endif //CALLFRAME_H
