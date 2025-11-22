#ifndef ARIA_CALLFRAME_H
#define ARIA_CALLFRAME_H

#include "chunk/chunk.h"
#include "chunk/code.h"
#include "common.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "value/value.h"

namespace aria {

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

    uint8_t readByte()
    {
        ip++;
        return ip[-1];
    }

    uint16_t readWord()
    {
        ip += 2;
        return static_cast<uint16_t>((ip[-1] << 8) | ip[-2]);
    }

    opCode readOpcode() { return static_cast<opCode>(readByte()); }

    Value readConstant() { return function->chunk->consts[readWord()]; }

    ObjString *readObjString() { return asObjString(readConstant()); }

    ObjFunction *function;
    uint8_t *ip;
    Value *stakBase;
};

} // namespace aria

#endif //ARIA_CALLFRAME_H
