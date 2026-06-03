#include "value/value.h"
#include "object/objInstance.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/object.h"
#include "util/hash.h"
#include "value/valueArray.h"
#include "value/valueStack.h"

namespace aria {

String value_type_string(Value value)
{
    if (NanBox::isBool(value)) {
        return "bool";
    }
    if (NanBox::isNil(value)) {
        return "nil";
    }
    if (NanBox::isNumber(value)) {
        return "number";
    }
    if (NanBox::isObj(value)) {
        switch (NanBox::toObj(value)->type_) {
        case ObjType::STRING:
            return "string";
        case ObjType::FUNCTION:
            return "function";
        case ObjType::BOUND_METHOD:
            return "boundMethod";
        case ObjType::NATIVE_FN:
            return "nativeFn";
        case ObjType::CLASS:
            return "class";
        case ObjType::INSTANCE:
            return "instance";
        case ObjType::LIST:
            return "list";
        case ObjType::MAP:
            return "map";
        case ObjType::UPVALUE:
            return "upvalue";
        case ObjType::MODULE:
            return "module";
        case ObjType::ITERATOR:
            return "iterator";
        default:
            return "unknownObj";
        }
    }
    return "unknownType";
}

String value_string(Value value)
{
    if (NanBox::isBool(value)) {
        if (NanBox::toBool(value)) {
            return "true";
        }
        return "false";
    }
    if (NanBox::isNil(value)) {
        return "nil";
    }
    if (NanBox::isNumber(value)) {
        return std::format("{}", NanBox::toNumber(value));
    }
    if (NanBox::isObj(value)) {
        return NanBox::toObj(value)->to_string();
    }
    return "unknown value";
}

String value_representation(Value value)
{
    if (NanBox::isBool(value)) {
        if (NanBox::toBool(value)) {
            return "true";
        }
        return "false";
    }
    if (NanBox::isNil(value)) {
        return "nil";
    }
    if (NanBox::isNumber(value)) {
        return std::format("{}", NanBox::toNumber(value));
    }
    if (NanBox::isObj(value)) {
        return NanBox::toObj(value)->representation();
    }
    return "unknown value";
}

bool values_equal(Value a, Value b)
{
    if (NanBox::isNumber(a) && NanBox::isNumber(b)) {
        return NanBox::toNumber(a) == NanBox::toNumber(b);
    }
    if (is_obj_list(a) && is_obj_list(b)) {
        return as_obj_list(a)->list_->equals(as_obj_list(b)->list_);
    }
    if (is_obj_map(a) && is_obj_map(b)) {
        return as_obj_map(a)->map_->equals(as_obj_map(b)->map_);
    }
    if (is_obj_instance(a) && is_obj_instance(b)) {
        ObjInstance *a_instance = as_obj_instance(a);
        ObjInstance *b_instance = as_obj_instance(b);
        if (a_instance->klass_ != b_instance->klass_) {
            return false;
        }
        return a_instance->fields_.equals(&b_instance->fields_);
    }
    return a == b;
}

uint32_t value_hash(Value value)
{
    if (NanBox::isBool(value))
        return static_cast<uint32_t>(NanBox::toBool(value)) | 0x1da55dda;
    if (NanBox::isNil(value))
        return 0;
    if (NanBox::isNumber(value))
        return hash_number(NanBox::toNumber(value));
    return NanBox::toObj(value)->hash_;
}

void mark_value(Value value)
{
    if (NanBox::isObj(value)) {
        NanBox::toObj(value)->mark();
    }
}

} // namespace aria
