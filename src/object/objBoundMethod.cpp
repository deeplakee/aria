#include "object/objBoundMethod.h"

#include "memory/gc.h"
#include "objFunction.h"
#include "objNativeFn.h"
#include "util/hash.h"

namespace aria {

ObjBoundMethod::ObjBoundMethod(Value receiver, ObjFunction *method, GC *gc)
    : Obj{ObjType::BOUND_METHOD, hash_obj(this, ObjType::BOUND_METHOD), gc}
    , receiver_{receiver}
    , method_type_{BoundMethodType::FUNCTION}
    , method_{method}
    , native_method_{nullptr}
{}

ObjBoundMethod::ObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc)
    : Obj{ObjType::BOUND_METHOD, hash_obj(this, ObjType::BOUND_METHOD), gc}
    , receiver_{receiver}
    , method_type_{BoundMethodType::NATIVE_FN}
    , method_{nullptr}
    , native_method_{method}
{}

ObjBoundMethod::~ObjBoundMethod() = default;

String ObjBoundMethod::to_string()
{
    String objStr = value_string(receiver_);
    String methodStr;
    if (method_type_ == BoundMethodType::FUNCTION) {
        methodStr = method_->to_string();
    } else if (method_type_ == BoundMethodType::NATIVE_FN) {
        methodStr = native_method_->to_string();
    } else {
        methodStr = "unknownMethod";
    }
    return format("<{}:{}>", objStr, methodStr);
}

void ObjBoundMethod::blacken()
{
    mark_value(receiver_);
    if (method_ != nullptr) {
        method_->mark();
    }
    if (native_method_ != nullptr) {
        native_method_->mark();
    }
}

ObjBoundMethod *new_ObjBoundMethod(Value receiver, ObjFunction *method, GC *gc)
{
    auto obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
    log_obj_allocation(obj);
    return obj;
}

ObjBoundMethod *new_ObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc)
{
    auto obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
