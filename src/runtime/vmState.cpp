#include "runtime/vmState.h"
#include "runtime/vm.h"

namespace aria {

VMState VMStateHelper::saveState(AriaVM *vm)
{
    VMState state;
    state.CframeCount = vm->c_frame_count_;
    state.EframeCount = vm->e_frame_count_;
    state.RmoduleCount = vm->r_module_count_;
    state.frame = vm->frame_;
    state.stackSize = vm->stack_.size();
    state.flags = vm->flags_;
    state.E_REG = vm->e_reg_;
    return state;
}

void VMStateHelper::restoreState(AriaVM *vm, const VMState &state)
{
    if (!state.frame || state.CframeCount <= 0) {
        vm->report_runtime_fatal_error(ErrorCode::RUNTIME_INVALID_STATE, "Invalid VMState");
        return;
    }
    vm->c_frame_count_ = state.CframeCount;
    vm->e_frame_count_ = state.EframeCount;
    vm->r_module_count_ = state.RmoduleCount;
    vm->stack_.resize(state.stackSize);
    vm->frame_ = state.frame;
    vm->flags_ = state.flags;
    vm->e_reg_ = state.E_REG;
}

} // namespace aria
