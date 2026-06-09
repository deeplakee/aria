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

ObjString::ObjString(char *chars, size_t length, uint32_t hash, bool own_chars, GC *gc)
    : Obj{ObjType::STRING, hash, gc}
    , length_{length}
{
    if (length_ <= SHORT_CAPACITY) {
        is_long_ = false;
        memcpy(short_chars_, chars, length_);
        short_chars_[length_] = '\0';
        if (own_chars) {
            gc_->free_array<char>(chars, length_ + 1);
        }
    } else {
        is_long_ = true;
        if (own_chars) {
            long_chars_ = chars;
        } else {
            long_chars_ = gc_->allocate_array<char>(length_ + 1);
            memcpy(long_chars_, chars, length_);
            long_chars_[length_] = '\0';
        }
    }
}

ObjString::ObjString(const char *chars, size_t length, uint32_t hash, GC *gc)
    : Obj{ObjType::STRING, hash, gc}
    , length_{length}
{
    if (length_ <= SHORT_CAPACITY) {
        is_long_ = false;
        memcpy(short_chars_, chars, length_);
        short_chars_[length_] = '\0';
    } else {
        is_long_ = true;
        long_chars_ = gc_->allocate_array<char>(length_ + 1);
        memcpy(long_chars_, chars, length_);
        long_chars_[length_] = '\0';
    }
}

ObjString::~ObjString()
{
    if (is_long_ && long_chars_ != nullptr) {
        gc_->free_array<char>(long_chars_, length_ + 1);
    }
}

String ObjString::to_string()
{
    return String{c_str()};
}

String ObjString::representation()
{
    return String{format("'{}'", c_str())};
}

Value ObjString::get_by_field(ObjString *name, Value &value)
{
    if (gc_->string_methods_->get(NanBox::fromObj(name), value)) {
        assert(is_obj_native_fn(value) && "string builtin method is nativeFn");
        auto boundMethod = new_ObjBoundMethod(NanBox::fromObj(this), as_obj_native_fn(value), gc_);
        value = NanBox::fromObj(boundMethod);
        return NanBox::TrueValue;
    }
    return NanBox::FalseValue;
}

Value ObjString::get_by_index(Value k, Value &v)
{
    if (!NanBox::isNumber(k)) {
        return new_exception(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    int index = static_cast<int>(NanBox::toNumber(k));
    if (index != NanBox::toNumber(k)) {
        return new_exception(ErrorCode::RUNTIME_TYPE_ERROR, "index of string must be a integer");
    }
    if (index < 0 || index >= length_) {
        return new_exception(ErrorCode::RUNTIME_OUT_OF_BOUNDS, "index out of range");
    }
    char a = c_str()[index];
    v = NanBox::fromObj(new_ObjString(a, gc_));
    return NanBox::TrueValue;
}

Value ObjString::create_iter(GC *gc)
{
    return NanBox::fromObj(new_ObjIterator(this, gc));
}

void ObjString::blacken() {}

ObjString *new_ObjString(const String &str, GC *gc)
{
    const size_t length = str.length();
    uint32_t hash = hash_string(str.c_str(), length);
    if (auto interned = gc->find_interned_string(str.c_str(), length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str.c_str(), length, hash, gc);
    gc->intern_string(obj);
    log_obj_allocation(obj);
    return obj;
}

ObjString *new_ObjString(const char *str, GC *gc)
{
    const size_t length = strlen(str);
    uint32_t hash = hash_string(str, length);
    if (auto interned = gc->find_interned_string(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str, length, hash, gc);
    gc->intern_string(obj);
    log_obj_allocation(obj);
    return obj;
}

ObjString *new_ObjString(char *str, size_t length, GC *gc)
{
    uint32_t hash = hash_string(str, length);
    if (auto interned = gc->find_interned_string(str, length, hash); interned != nullptr) {
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(str, length, hash, gc);
    gc->intern_string(obj);
    log_obj_allocation(obj);
    return obj;
}

ObjString *new_ObjString(char ch, GC *gc)
{
    constexpr size_t length = 1;
    uint32_t hash = hash_string(&ch, length);
    if (auto interned = gc->find_interned_string(&ch, length, hash); interned != nullptr) {
        return interned;
    }
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[0] = ch;
    newStr[length] = '\0';
    auto obj = gc->allocate_object<ObjString>(newStr, length, hash, true, gc);
    gc->intern_string(obj);
    log_obj_allocation(obj);
    return obj;
}

ObjString *concatenate_string(const ObjString *a, const ObjString *b, GC *gc)
{
    const size_t length = a->length_ + b->length_;
    if (length < a->length_) {
        return nullptr;
    }
    uint32_t hash = hash_string(b->c_str(), b->length_, a->hash_);
    bool useGCBuffer = length < GC::k_gc_buffer_size;
    char *dest = useGCBuffer ? gc->string_op_buffer_ : gc->allocate_array<char>(length + 1);
    memcpy(dest, a->c_str(), a->length_);
    memcpy(dest + a->length_, b->c_str(), b->length_ + 1); // copy '\0' from b
    if (auto interned = gc->find_interned_string(dest, length, hash); interned != nullptr) {
        if (!useGCBuffer) {
            gc->free_array<char>(dest, length + 1);
        }
        return interned;
    }
    auto obj = gc->allocate_object<ObjString>(dest, length, hash, !useGCBuffer, gc);
    gc->intern_string(obj);
    log_obj_allocation(obj);
    return obj;
}

} // namespace aria
