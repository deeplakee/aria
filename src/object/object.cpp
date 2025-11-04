#include "object/object.h"
#include "runtime/vm.h"

namespace aria {

const char *Obj::objTypeStr[]
    = {"BASE",
       "STRING",
       "FUNCTION",
       "NATIVE_FN",
       "UPVALUE",
       "CLASS",
       "INSTANCE",
       "BOUND_METHOD",
       "LIST",
       "MAP",
       "MODULE",
       "ITERATOR",
       "EXCEPTION"};

void Obj::mark()
{
    if (isMarked)
        return;
#ifdef DEBUG_MODE
    assert(type != ObjType::BASE && "Invalid Object Type");
#endif
#ifdef DEBUG_LOG_GC
    print("{:p} mark {}\n", toVoidPtr(this), this->toString());
#endif
    isMarked = true;
    gc->addToGrey(this);
}

Value Obj::op_call(AriaEnv *env, int argCount)
{
    String msg = format("Cannot call object of {}", valueRepresentation(obj_val(this)));
    return env->newException(msg.c_str());
}

} // namespace aria
