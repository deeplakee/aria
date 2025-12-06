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
        shortChars[length] = '\0';
        if (_ownChars) {
            gc->freeArray<char>(_chars, length + 1);
        }
    } else {
        isLong = true;
        if (_ownChars) {
            longChars = _chars;
        } else {
            longChars = gc->allocateArray<char>(length + 1);
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
        shortChars[length] = '\0';
    } else {
        isLong = true;
        longChars = gc->allocateArray<char>(length + 1);
        memcpy(longChars, _chars, length);
        longChars[length] = '\0';
    }
}

ObjString::~ObjString()
{
    if (isLong && longChars != nullptr) {
        gc->freeArray<char>(longChars, length + 1);
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
    if (gc->stringMethods->get(NanBox::fromObj(name), value)) {
        assert(isObjNativeFn(value) && "string builtin method is nativeFn");
        auto boundMethod = newObjBoundMethod(NanBox::fromObj(this), asObjNativeFn(value), gc);
        value = NanBox::fromObj(boundMethod);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjString::getByIndex(Value k, Value &v)
{
    if (!NanBox::isNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return genException(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    if (index < 0 || index >= length) {
        return genException(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    char a = C_str_ref()[index];
    v = NanBox::fromObj(newObjString(a, gc));
    return NanBox::TrueValue;
}

Value ObjString::createIter(GC *gc)
{
    return NanBox::fromObj(newObjIterator(this, gc));
}

void ObjString::blacken() {}

ObjString *newObjString(const String &str, GC *gc)
{
    const size_t length = str.length();
    uint32_t hash = hashString(str.c_str(), length);
    if (auto interned = gc->findInternedString(str.c_str(), length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocateObject<ObjString>(str.c_str(), length, hash, gc);
    gc->internString(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(const char *str, GC *gc)
{
    const size_t length = strlen(str);
    uint32_t hash = hashString(str, length);
    if (auto interned = gc->findInternedString(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocateObject<ObjString>(str, length, hash, gc);
    gc->internString(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(char *str, size_t length, GC *gc)
{
    uint32_t hash = hashString(str, length);
    if (auto interned = gc->findInternedString(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocateObject<ObjString>(str, length, hash, gc);
    gc->internString(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

ObjString *newObjString(char ch, GC *gc)
{
    constexpr size_t length = 1;
    uint32_t hash = hashString(&ch, length);
    if (auto interned = gc->findInternedString(&ch, length, hash); interned != nullptr) {
        return interned;
    }
    char *newStr = gc->allocateArray<char>(length + 1);
    newStr[0] = ch;
    newStr[length] = '\0';
    auto obj = gc->allocateObject<ObjString>(newStr, length, hash, true, gc);
    gc->internString(obj);
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
    uint32_t hash = hashString(b->C_str_ref(), b->length, a->hash);
    bool useGCBuffer = length < GC::GC_BUFFER_SIZE;
    char *dest = useGCBuffer ? gc->stringOpBuffer : gc->allocateArray<char>(length + 1);
    memcpy(dest, a->C_str_ref(), a->length);
    memcpy(dest + a->length, b->C_str_ref(), b->length + 1); // copy '\0' from b
    if (auto interned = gc->findInternedString(dest, length, hash); interned != nullptr) {
        if (!useGCBuffer) {
            gc->freeArray<char>(dest, length + 1);
        }
        return interned;
    }
    auto obj = gc->allocateObject<ObjString>(dest, length, hash, !useGCBuffer, gc);
    gc->internString(obj);
#ifdef DEBUG_LOG_GC
    println("{:p} allocate {} bytes (object STRING)", toVoidPtr(obj), sizeof(ObjString));
#endif
    return obj;
}

} // namespace aria
