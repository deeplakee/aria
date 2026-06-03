#ifndef ARIA_OBJSTRING_H
#define ARIA_OBJSTRING_H

#include "object/object.h"

namespace aria {

class ValueHashTable;

class ObjString : public Obj
{
public:
    ObjString() = delete;

    ObjString(char *chars, size_t length, uint32_t hash, bool own_chars, GC *gc);

    ObjString(const char *chars, size_t length, uint32_t hash, GC *gc);

    ~ObjString() override;

    String to_string() override;

    String representation() override;

    size_t obj_size() override { return sizeof(ObjString); }

    Value get_by_field(ObjString *name, Value &value) override;

    Value get_by_index(Value k, Value &v) override;

    Value create_iter(GC *gc) override;

    void blacken() override;

    char *c_str() { return is_long_ ? long_chars_ : short_chars_; }

    const char *c_str() const { return is_long_ ? long_chars_ : short_chars_; }

    static constexpr auto SHORT_CAPACITY = 15;

    bool is_long_;
    union {
        char short_chars_[SHORT_CAPACITY + 1];
        char *long_chars_;
    };
    size_t length_;

    static void init(GC *_gc, ValueHashTable *builtins);
};

inline bool is_obj_string(Value value)
{
    return is_obj_type(value, ObjType::STRING);
}

inline ObjString *as_obj_string(Value value)
{
    return as_Obj<ObjString>(value);
}

inline char *as_c_string(Value value)
{
    return as_obj_string(value)->c_str();
}

// construct from ref
ObjString *new_ObjString(const String &str, GC *gc);

// construct from ref
ObjString *new_ObjString(const char *str, GC *gc);

// construct from ref
ObjString *new_ObjString(char *str, size_t length, GC *gc);

// construct from ref
ObjString *new_ObjString(char ch, GC *gc);

ObjString *concatenate_string(const ObjString *a, const ObjString *b, GC *gc);

} // namespace aria

#endif //ARIA_OBJSTRING_H
