#include "object/object.h"
#include "runtime/vm.h"

namespace aria {

const char *Obj::obj_type_str_[]
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
    if (is_marked_)
        return;
#ifdef DEBUG_MODE
    assert(type_ != ObjType::BASE && "Invalid Object Type");
#endif
#ifdef DEBUG_LOG_GC
    print("{:p} mark {}\n", to_void_ptr(this), this->to_string());
#endif
    is_marked_ = true;
    gc_->push_grey(this);
}

Value Obj::op_call(AriaEnv *env, int arg_count)
{
    String msg = format("Cannot call object of {}", value_representation(NanBox::fromObj(this)));
    return env->new_exception(msg.c_str());
}

} // namespace aria
