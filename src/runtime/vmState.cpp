#include "runtime/vmState.h"
#include "runtime/vm.h"

namespace aria {

VMState VMStateHelper::saveState(AriaVM *vm)
{
    VMState state;
    state.CframeCount = vm->CframeCount;
    state.EframeCount = vm->EframeCount;
    state.RmoduleCount = vm->RmoduleCount;
    state.frame = vm->frame;
    state.stackSize = vm->stack.size();
    state.flags = vm->flags;
    state.E_REG = vm->E_REG;
    return state;
}

void VMStateHelper::restoreState(AriaVM *vm, const VMState &state)
{
    if (!state.frame || state.CframeCount <= 0) {
        vm->reportRuntimeFatalError(ErrorCode::RUNTIME_INVALID_STATE, "Invalid VMState");
        return;
    }
    vm->CframeCount = state.CframeCount;
    vm->EframeCount = state.EframeCount;
    vm->RmoduleCount = state.RmoduleCount;
    vm->stack.resize(state.stackSize);
    vm->frame = state.frame;
    vm->flags = state.flags;
    vm->E_REG = state.E_REG;
}

} // namespace aria
