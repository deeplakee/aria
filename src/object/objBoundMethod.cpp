#include "object/objBoundMethod.h"

#include "memory/gc.h"
#include "objFunction.h"
#include "objNativeFn.h"
#include "util/hash.h"

namespace aria {

ObjBoundMethod::ObjBoundMethod(Value _receiver, ObjFunction *_method, GC *_gc)
    : Obj{ObjType::BOUND_METHOD, hashObj(this, ObjType::BOUND_METHOD), _gc}
    , receiver{_receiver}
    , methodType{BoundMethodType::FUNCTION}
    , method{_method}
    , native_method{nullptr}
{}

ObjBoundMethod::ObjBoundMethod(Value _receiver, ObjNativeFn *_method, GC *_gc)
    : Obj{ObjType::BOUND_METHOD, hashObj(this, ObjType::BOUND_METHOD), _gc}
    , receiver{_receiver}
    , methodType{BoundMethodType::NATIVE_FN}
    , method{nullptr}
    , native_method{_method}
{}

ObjBoundMethod::~ObjBoundMethod() = default;

String ObjBoundMethod::toString(ValueStack *printStack)
{
    String objStr = valueString(receiver);
    String methodStr;
    if (methodType == BoundMethodType::FUNCTION) {
        methodStr = method->toString();
    } else if (methodType == BoundMethodType::NATIVE_FN) {
        methodStr = native_method->toString();
    } else {
        methodStr = "unknownMethod";
    }
    return format("<{}:{}>", objStr, methodStr);
}

void ObjBoundMethod::blacken()
{
    markValue(receiver);
    if (method != nullptr) {
        method->mark();
    }
    if (native_method != nullptr) {
        native_method->mark();
    }
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjFunction *method, GC *gc)
{
    auto obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object BOUND_METHOD)", toVoidPtr(obj), sizeof(ObjBoundMethod));
#endif
    return obj;
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc)
{
    auto obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object BOUND_METHOD)", toVoidPtr(obj), sizeof(ObjBoundMethod));
#endif
    return obj;
}

} // namespace aria
