#ifndef ARIA_OBJSTRING_H
#define ARIA_OBJSTRING_H

#include "object/object.h"

namespace aria {

class ValueHashTable;

class ObjString : public Obj
{
public:
    ObjString() = delete;

    ObjString(char *_chars, size_t _length, uint32_t _hash, bool _ownChars, GC *_gc);

    ObjString(const char *_chars, size_t _length, uint32_t _hash, GC *_gc);

    ~ObjString() override;

    using Obj::toString;

    using Obj::representation;

    String toString(ValueStack *printStack) override;

    String representation(ValueStack *printStack) override;

    size_t objSize() override { return sizeof(ObjString); }

    Value getByField(ObjString *name, Value &value) override;

    Value getByIndex(Value k, Value &v) override;

    Value createIter(GC *gc) override;

    void blacken() override;

    char *C_str() { return isLong ? longChars : shortChars; }

    const char *C_str_ref() const { return isLong ? longChars : shortChars; }

    static constexpr auto SHORT_CAPACITY = 15;

    bool isLong;
    union {
        char shortChars[SHORT_CAPACITY + 1];
        char *longChars;
    };
    size_t length;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool isObjString(Value value)
{
    return isObjType(value, ObjType::STRING);
}

inline ObjString *asObjString(Value value)
{
#ifdef DEBUG_MODE
    return dynamic_cast<ObjString *>(NanBox::toObj(value));
#else
    return static_cast<ObjString *>(NanBox::toObj(value));
#endif
}

inline char *asCString(Value value)
{
    return asObjString(value)->C_str();
}

// construct from ref
ObjString *newObjString(const String &str, GC *gc);

// construct from ref
ObjString *newObjString(const char *str, GC *gc);

// construct from ref
ObjString *newObjString(char *str, size_t length, GC *gc);

// construct from ref
ObjString *newObjString(char ch, GC *gc);

ObjString *concatenateString(const ObjString *a, const ObjString *b, GC *gc);

} // namespace aria

#endif //ARIA_OBJSTRING_H
