#ifndef EXCEPTIONFRAME_H
#define EXCEPTIONFRAME_H

#include "common.h"

namespace aria {

struct ExceptionFrame
{
    ExceptionFrame(int CframeCount_, int RmoduleCount_, uint8_t *_ip, uint32_t _stackSize)
        : CframeCount{CframeCount_}
        , RmoduleCount{RmoduleCount_}
        , ip{_ip}
        , stackSize{_stackSize}
    {}

    ExceptionFrame()
        : ExceptionFrame{0, 0, nullptr, 0}
    {}

    void init(int CframeCount_, int RmoduleCount_, uint8_t *_ip, uint32_t _stackSize)
    {
        CframeCount = CframeCount_;
        RmoduleCount = RmoduleCount_;
        ip = _ip;
        stackSize = _stackSize;
    }

    int CframeCount;
    int RmoduleCount;
    uint8_t *ip;
    uint32_t stackSize;
};

} // namespace aria

#endif //EXCEPTIONFRAME_H
