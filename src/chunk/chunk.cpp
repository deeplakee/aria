#include "chunk/chunk.h"
#include "chunk/disassembler.h"
#include "error/error.h"
#include "value/valueHashTable.h"

namespace aria {

Chunk::Chunk(GC *gc)
    : gc_{gc}
    , count_{0}
    , capacity_{0}
    , codes_{nullptr}
    , lines_(nullptr)
    , consts_{gc}
    , globals_{new ValueHashTable{gc}}
    , globals_manageable_{false}
{}

Chunk::Chunk(ValueHashTable *globals, GC *gc)
    : gc_{gc}
    , count_{0}
    , capacity_{0}
    , codes_{nullptr}
    , lines_(nullptr)
    , consts_{gc}
    , globals_{globals}
    , globals_manageable_{false}
{}

Chunk::~Chunk()
{
    gc_->free_array<uint8_t>(codes_, capacity_);
    gc_->free_array<uint32_t>(lines_, capacity_);
    if (globals_manageable_) {
        delete globals_;
    }
}

uint8_t Chunk::operator[](uint32_t offset) const
{
    return codes_[offset];
}

void Chunk::emit_byte(uint8_t byte, uint32_t line)
{
    if (capacity_ < count_ + 1) {
        uint64_t new_capacity = GC::grow_capacity(capacity_);
        if (new_capacity > UINT32_MAX) {
            fatal_error(ErrorCode::RESOURCE_CHUNK_OVERFLOW, "Too many codes in one chunk");
        }
        codes_ = gc_->resize_array<uint8_t>(codes_, capacity_, new_capacity);
        lines_ = gc_->resize_array<uint32_t>(lines_, capacity_, new_capacity);
        capacity_ = static_cast<uint32_t>(new_capacity);
    }
    codes_[count_] = byte;
    lines_[count_] = line;
    ++count_;
}

void Chunk::emit_byte(uint8_t byte)
{
    emit_byte(byte, line_of_last_code());
}

void Chunk::emit_word(uint16_t word, uint32_t line)
{
    auto [low, high] = split_word(word);
    emit_byte(low, line);
    emit_byte(high, line);
}

void Chunk::emit_word(uint16_t word)
{
    auto [low, high] = split_word(word);
    emit_byte(low);
    emit_byte(high);
}

void Chunk::emit_op(opCode op)
{
    emit_byte(static_cast<uint8_t>(op), line_of_last_code());
}

void Chunk::emit_op(opCode op, uint32_t line)
{
    return emit_byte(static_cast<uint8_t>(op), line);
}

void Chunk::emit_op_arg8(opCode op, uint8_t arg, uint32_t line)
{
    emit_op(op, line);
    emit_byte(arg, line);
}

void Chunk::emit_op_arg16(opCode op, uint16_t arg, uint32_t line)
{
    emit_op(op, line);
    emit_word(arg, line);
}

void Chunk::emit_op_value(opCode op, Value value, uint32_t line)
{
    gc_->push_temp_root(value);
    consts_.push(value);
    gc_->pop_temp_root(1);
    uint32_t index = consts_.size() - 1;
    if (index > UINT16_MAX) {
        fatal_error(ErrorCode::RESOURCE_CHUNK_OVERFLOW, "Too many constants in one chunk.");
    }
    emit_op(op, line);
    emit_word(static_cast<uint16_t>(index), line);
}

uint32_t Chunk::emit_jump(opCode jump_op, uint32_t line)
{
    emit_op(jump_op, line);
    emit_word(0xFFFF, line);
    return count_ - 2;
}

void Chunk::emit_pop_n(uint32_t count, uint32_t line)
{
    while (count > 0) {
        if (count == 1) {
            emit_op(opCode::POP, line);
            break;
        }
        uint8_t n = (count < UINT8_MAX) ? static_cast<uint8_t>(count) : UINT8_MAX;
        emit_op_arg8(opCode::POP_N, n, line);
        count -= n;
    }
}

void Chunk::emit_scope_cleanup(const List<opCode> &ops, uint32_t line)
{
    uint32_t pop_count = 0;
    for (const auto op : ops) {
        if (op == opCode::POP) {
            pop_count++;
        } else if (op == opCode::CLOSE_UPVALUE) {
            if (pop_count > 0) {
                emit_pop_n(pop_count, line);
                pop_count = 0;
            }
            emit_op(opCode::CLOSE_UPVALUE, line);
        } else {
            throw ariaCompilingException(
                ErrorCode::SEMANTIC_UNKNOWN_OPERATOR, "Unexpected opcode in scope cleanup");
        }
    }
    if (pop_count > 0) {
        emit_pop_n(pop_count, line);
    }
}

void Chunk::emit_fun_end_ret(uint32_t line, bool is_init_method)
{
    if (is_init_method) {
        emit_op_arg16(opCode::LOAD_LOCAL, 0, line);
    } else {
        emit_op(opCode::LOAD_NIL, line);
    }
    emit_op(opCode::RETURN, line);
}

void Chunk::patch_jump(uint32_t src_pos, uint32_t dest_pos)
{
    uint32_t offset = 0;
    if (dest_pos < src_pos) {
        // Jump forward
        rewrite_op(opCode::JUMP_FWD, src_pos - 1);
        offset = src_pos + 2 - dest_pos;
    } else {
        // Jump backward
        rewrite_op(opCode::JUMP_BWD, src_pos - 1);
        offset = dest_pos - (src_pos + 2);
    }
    if (offset > UINT16_MAX) {
        fatal_error(ErrorCode::RESOURCE_JUMP_OVERFLOW, "Too much code to jump over.");
    }
    rewrite_word(offset, src_pos);
}

void Chunk::patch_jump(uint32_t patch_pos)
{
    uint32_t offset = count_ - (patch_pos + 2);
    if (offset > UINT16_MAX) {
        fatal_error(ErrorCode::RESOURCE_JUMP_OVERFLOW, "Too much code to jump over.");
    }
    rewrite_word(offset, patch_pos);
}

void Chunk::disassemble(StringView name) const
{
    Disassembler::disassembleChunk(this, name);
}

uint32_t Chunk::line_of_last_code() const
{
    if (count_ == 0) {
        return 0;
    }
    return lines_[count_ - 1];
}

void Chunk::rewrite_byte(uint8_t byte, uint32_t index)
{
    codes_[index] = byte;
}

void Chunk::rewrite_word(uint16_t word, uint32_t index)
{
    auto [low, high] = split_word(word);
    rewrite_byte(low, index);
    rewrite_byte(high, index + 1);
}

void Chunk::rewrite_op(opCode op, uint32_t index)
{
    rewrite_byte(static_cast<uint8_t>(op), index);
}

} // namespace aria