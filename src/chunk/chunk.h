#ifndef ARIA_CHUNK_H
#define ARIA_CHUNK_H

#include "chunk/code.h"
#include "memory/gc.h"
#include "value/valueArray.h"

namespace aria {

class ValueHashTable;

class Chunk
{
public:
    explicit Chunk(GC *gc);

    Chunk(ValueHashTable *globals, GC *gc);

    ~Chunk();

    uint8_t operator[](uint32_t offset) const;

    void emit_byte(uint8_t byte, uint32_t line);

    void emit_byte(uint8_t byte);

    void emit_word(uint16_t word, uint32_t line);

    void emit_word(uint16_t word);

    void emit_op(opCode op);

    void emit_op(opCode op, uint32_t line);

    void emit_op_arg8(opCode op, uint8_t arg, uint32_t line);

    void emit_op_arg16(opCode op, uint16_t arg, uint32_t line);

    void emit_op_value(opCode op, Value value, uint32_t line);

    // Write a jump instruction with a placeholder (2 bytes)
    // return the offset position (for subsequent backfilling)
    uint32_t emit_jump(opCode jump_op, uint32_t line);

    void emit_pop_n(uint32_t count, uint32_t line);

    void emit_scope_cleanup(const List<opCode> &ops, uint32_t line);

    void emit_fun_end_ret(uint32_t line, bool is_init_method = false);

    void patch_jump(uint32_t src_pos, uint32_t dest_pos);

    // patch forward position
    // equals patch_jump(patch_pos, current)
    void patch_jump(uint32_t patch_pos);

    void disassemble(StringView name) const;

    [[nodiscard]] uint32_t line_of_last_code() const;

    GC *gc_;
    uint32_t count_;
    uint32_t capacity_;
    uint8_t *codes_;
    uint32_t *lines_;
    ValueArray consts_;
    ValueHashTable *globals_;
    bool globals_manageable_;

private:
    void rewrite_op(opCode op, uint32_t index);

    void rewrite_byte(uint8_t byte, uint32_t index);

    void rewrite_word(uint16_t word, uint32_t index);
};

} // namespace aria

#endif //ARIA_CHUNK_H
