#include "object/objClass.h"

#include "memory/gc.h"
#include "object/objFunction.h"
#include "object/objString.h"
#include "util/hash.h"
#include "util/util.h"

namespace aria {
ObjClass::ObjClass(ObjString *name, GC *gc)
    : Obj{ObjType::CLASS, hash_obj(this, ObjType::CLASS), gc}
    , name_{name}
    , methods_{gc}
    , super_klass_{nullptr}
    , init_method_{nullptr}
{}

ObjClass::~ObjClass() = default;

String ObjClass::to_string()
{
    return format("<class {}>", name_->c_str());
}

void ObjClass::blacken()
{
    name_->mark();
    methods_.mark();
    if (super_klass_ != nullptr) {
        super_klass_->mark();
    }
    if (init_method_ != nullptr) {
        init_method_->mark();
    }
}

bool ObjClass::getSuperMethod(ObjString *method_name, Value &method) const
{
    if (super_klass_ == nullptr) {
        return false;
    }

    if (method_name->length_ == k_init_fun_name_len
        && memcmp(method_name->c_str(), k_init_fun_name, k_init_fun_name_len) == 0) {
        if (super_klass_->init_method_ != nullptr) {
            method = NanBox::fromObj(super_klass_->init_method_);
            return true;
        }
        return false;
    }

    return super_klass_->methods_.get(NanBox::fromObj(method_name), method);
}

ObjClass *new_ObjClass(ObjString *name, GC *gc)
{
    auto obj = gc->allocate_object<ObjClass>(name, gc);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
