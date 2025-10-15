#include "value/value.h"
#include "object/objInstance.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/object.h"
#include "util/hash.h"
#include "value/valueArray.h"
#include "value/valueStack.h"

namespace aria {

String valueTypeString(Value value)
{
    if (is_bool(value)) {
        return "bool";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return "number";
    }
    if (is_obj(value)) {
        switch (as_obj(value)->type) {
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

String valueString(Value value, ValueStack *printStack)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return "true";
        }
        return "false";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return std::format("{}", as_number(value));
    }
    if (is_obj(value)) {
        return as_obj(value)->toString(printStack);
    }
    return "unknown value";
}

String valueRepresentation(Value value, ValueStack *printStack)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return "true";
        }
        return "false";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return std::format("{}", as_number(value));
    }
    if (is_obj(value)) {
        return as_obj(value)->representation(printStack);
    }
    return "unknown value";
}

bool valuesEqual(Value a, Value b)
{
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    if (is_ObjList(a) && is_ObjList(b)) {
        return as_ObjList(a)->list->equals(as_ObjList(b)->list);
    }
    if (is_ObjMap(a) && is_ObjMap(b)) {
        return as_ObjMap(a)->map->equals(as_ObjMap(b)->map);
    }
    if (is_ObjInstance(a) && is_ObjInstance(b)) {
        ObjInstance *aInstance = as_ObjInstance(a);
        ObjInstance *bInstance = as_ObjInstance(b);
        if (aInstance->klass != bInstance->klass) {
            return false;
        }
        return aInstance->fields.equals(&bInstance->fields);
    }
    return a == b;
}

uint32_t valueHash(Value value)
{
    if (is_bool(value))
        return static_cast<uint32_t>(as_bool(value)) | 0x200030611;
    if (is_nil(value))
        return 0;
    if (is_number(value))
        return hashNumber(as_number(value));
    return as_obj(value)->hash;
}

void markValue(Value value)
{
    if (is_obj(value)) {
        as_obj(value)->mark();
    }
}

} // namespace aria
