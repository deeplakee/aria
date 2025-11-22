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
        switch (NanBox::toObj(value)->type) {
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
        return NanBox::toObj(value)->toString(printStack);
    }
    return "unknown value";
}

String valueRepresentation(Value value, ValueStack *printStack)
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
        auto obj = NanBox::toObj(value);
        return NanBox::toObj(value)->representation(printStack);
    }
    return "unknown value";
}

bool valuesEqual(Value a, Value b)
{
    if (NanBox::isNumber(a) && NanBox::isNumber(b)) {
        return NanBox::toNumber(a) == NanBox::toNumber(b);
    }
    if (isObjList(a) && isObjList(b)) {
        return asObjList(a)->list->equals(asObjList(b)->list);
    }
    if (isObjMap(a) && isObjMap(b)) {
        return asObjMap(a)->map->equals(asObjMap(b)->map);
    }
    if (isObjInstance(a) && isObjInstance(b)) {
        ObjInstance *aInstance = asObjInstance(a);
        ObjInstance *bInstance = asObjInstance(b);
        if (aInstance->klass != bInstance->klass) {
            return false;
        }
        return aInstance->fields.equals(&bInstance->fields);
    }
    return a == b;
}

uint32_t valueHash(Value value)
{
    if (NanBox::isBool(value))
        return static_cast<uint32_t>(NanBox::toBool(value)) | 0x1da55dda;
    if (NanBox::isNil(value))
        return 0;
    if (NanBox::isNumber(value))
        return hashNumber(NanBox::toNumber(value));
    return NanBox::toObj(value)->hash;
}

void markValue(Value value)
{
    if (NanBox::isObj(value)) {
        NanBox::toObj(value)->mark();
    }
}

} // namespace aria
