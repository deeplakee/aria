#ifndef ARIA_VMSTATE_H
#define ARIA_VMSTATE_H

#include "runtime/callFrame.h"
#include "runtime/exceptionFrame.h"

namespace aria {

class AriaVM;

struct VMState
{
    int CframeCount;
    int EframeCount;
    int RmoduleCount;
    CallFrame *frame;
    uint32_t stackSize;
    uint8_t flags;
    Value E_REG;

    VMState()
        : CframeCount{0}
        , EframeCount{0}
        , RmoduleCount{0}
        , frame{nullptr}
        , stackSize{0}
        , flags{0}
        , E_REG{NanBox::NilValue}
    {}
};

class VMStateHelper
{
public:
    static VMState saveState(AriaVM *vm);

    static void restoreState(AriaVM *vm, const VMState &state);
};

} // namespace aria

#endif //ARIA_VMSTATE_H
