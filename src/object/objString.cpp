#include "object/objString.h"

#include "memory/gc.h"
#include "object/objException.h"
#include "object/objIterator.h"
#include "object/objNativeFn.h"
#include "runtime/vm.h"
#include "util/hash.h"
#include "value/valueHashTable.h"

#include <cassert>

namespace aria {

#define genException(code, msg) gc->runningVM->newException((code), (msg))

ObjString::ObjString(char *_chars, size_t _length, uint32_t _hash, bool _ownChars, GC *_gc)
    : Obj{ObjType::STRING, _hash, _gc}
    , length{_length}
{
    if (length <= SHORT_CAPACITY) {
        isLong = false;
        memcpy(shortChars, _chars, length);
        if (_ownChars) {
            gc->free_array<char>(_chars, length + 1);
        }
    } else {
        isLong = true;
        if (_ownChars) {
            longChars = _chars;
        } else {
            longChars = gc->allocate_array<char>(length + 1);
            memcpy(longChars, _chars, length);
            longChars[length] = '\0';
        }
    }
}

ObjString::ObjString(const char *_chars, size_t _length, uint32_t _hash, GC *_gc)
    : Obj{ObjType::STRING, _hash, _gc}
    , length{_length}
{
    if (length <= SHORT_CAPACITY) {
        isLong = false;
        memcpy(shortChars, _chars, length);
    } else {
        isLong = true;
        longChars = gc->allocate_array<char>(length + 1);
        memcpy(longChars, _chars, length);
        longChars[length] = '\0';
    }
}

ObjString::~ObjString()
{
    if (isLong && longChars != nullptr) {
        gc->free_array<char>(longChars, length + 1);
    }
}

String ObjString::toString(ValueStack *printStack)
{
    return String{C_str_ref()};
}

String ObjString::representation(ValueStack *printStack)
{
    return String{format("'{}'", C_str_ref())};
}

Value ObjString::getByField(ObjString *name, Value &value)
{
    if (gc->stringBuiltins->get(obj_val(name), value)) {
        assert(is_ObjNativeFn(value) && "string builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(obj_val(this), as_ObjNativeFn(value), gc);
        value = obj_val(boundMethod);
        return true_val;
    }
    return false_val;
}

Value ObjString::getByIndex(Value k, Value &v)
{
    if (!is_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    if (index < 0 || index >= length) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    char a = C_str_ref()[index];
    v = obj_val(newObjString(a, gc));
    return true_val;
}

Value ObjString::createIter(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

void ObjString::blacken() {}

ObjString *newObjString(const String &str, GC *gc)
{
    const size_t length = str.length();
    uint32_t hash = hashString(str.c_str(), length);
    if (auto interned = gc->getStr(str.c_str(), length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str.c_str(), length, hash, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(const char *str, GC *gc)
{
    const size_t length = strlen(str);
    uint32_t hash = hashString(str, length);
    if (auto interned = gc->getStr(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str, length, hash, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(char *str, size_t length, GC *gc)
{
    uint32_t hash = hashString(str, length);
    if (auto interned = gc->getStr(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str, length, hash, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(char ch, GC *gc)
{
    constexpr size_t length = 1;
    uint32_t hash = hashString(&ch, length);
    if (auto interned = gc->getStr(&ch, length, hash); interned != nullptr) {
        return interned;
    }
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[0] = ch;
    newStr[length] = '\0';
    auto obj = gc->allocate_object<ObjString>(newStr, length, hash, true, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjStringFromRaw(char *str, size_t length, GC *gc)
{
    uint32_t hash = hashString(str, length);
    if (auto interned = gc->getStr(str, length, hash); interned != nullptr) {
        gc->free_array<char>(str, length + 1);
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str, length, hash, true, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *concatenateString(const ObjString *a, const ObjString *b, GC *gc)
{
    const size_t length = a->length + b->length;
    if (length < a->length) {
        return nullptr;
    }
    char *newStr = gc->allocate_array<char>(length + 1);
    memcpy(newStr, a->C_str_ref(), a->length);
    memcpy(newStr + a->length, b->C_str_ref(), b->length);
    newStr[length] = '\0';
    uint32_t hash = hashString(newStr, length);
    if (auto interned = gc->getStr(newStr, length, hash); interned != nullptr) {
        gc->free_array<char>(newStr, length + 1);
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(newStr, length, hash, true, gc);
    gc->insertStr(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

} // namespace aria
