#include "chunk/chunk.h"
#include "chunk/disassembler.h"
#include "error/error.h"
#include "value/valueHashTable.h"

namespace aria {

Chunk::Chunk(GC *_gc)
    : gc{_gc}
    , count{0}
    , capacity{0}
    , codes{nullptr}
    , lines(nullptr)
    , consts{_gc}
    , globals{new ValueHashTable{_gc}}
    , globalsManageable{false}
{}

Chunk::Chunk(ValueHashTable *_globals, GC *_gc)
    : gc{_gc}
    , count{0}
    , capacity{0}
    , codes{nullptr}
    , lines(nullptr)
    , consts{_gc}
    , globals{_globals}
    , globalsManageable{false}
{}

Chunk::~Chunk()
{
    gc->free_array<uint8_t>(codes, capacity);
    gc->free_array<uint32_t>(lines, capacity);
    if (globalsManageable) {
        delete globals;
    }
}

uint8_t Chunk::operator[](uint32_t offset) const
{
    return codes[offset];
}

void Chunk::emitByte(uint8_t byte, uint32_t line)
{
    if (capacity < count + 1) {
        uint64_t newCapacity = GC::grow_capacity(capacity);
        if (newCapacity > UINT32_MAX) {
            fatalError(ErrorCode::RESOURCE_CHUNK_OVERFLOW, "Too many codes in one chunk");
        }
        codes = gc->grow_array<uint8_t>(codes, capacity, newCapacity);
        lines = gc->grow_array<uint32_t>(lines, capacity, newCapacity);
        capacity = static_cast<uint32_t>(newCapacity);
    }
    codes[count] = byte;
    lines[count] = line;
    ++count;
}

void Chunk::emitByte(uint8_t byte)
{
    emitByte(byte, lastOpLine());
}

void Chunk::emitWord(uint16_t word, uint32_t line)
{
    auto [low, high] = splitWord(word);
    emitByte(low, line);
    emitByte(high, line);
}

void Chunk::emitWord(uint16_t word)
{
    auto [low, high] = splitWord(word);
    emitByte(low);
    emitByte(high);
}

void Chunk::emitOp(opCode op)
{
    emitByte(static_cast<uint8_t>(op), lastOpLine());
}

void Chunk::emitOp(opCode op, uint32_t line)
{
    return emitByte(static_cast<uint8_t>(op), line);
}

void Chunk::emitOpArg8(opCode op, uint8_t arg, uint32_t line)
{
    emitOp(op, line);
    emitByte(arg, line);
}

void Chunk::emitOpArg16(opCode op, uint16_t arg, uint32_t line)
{
    emitOp(op, line);
    emitWord(arg, line);
}

void Chunk::emitOpData(opCode op, Value value, uint32_t line)
{
    gc->cache(value);
    consts.push(value);
    gc->releaseCache();
    uint32_t index = consts.size() - 1;
    if (index > UINT16_MAX) {
        fatalError(ErrorCode::RESOURCE_CHUNK_OVERFLOW, "Too many constants in one chunk.");
    }
    emitOp(op, line);
    emitWord(static_cast<uint16_t>(index), line);
}

uint32_t Chunk::emitJump(opCode jumpOp, uint32_t line)
{
    emitOp(jumpOp, line);
    emitWord(0xFFFF, line);
    return count - 2;
}

void Chunk::emitPopN(uint32_t count, uint32_t line)
{
    while (count > 0) {
        if (count == 1) {
            emitOp(opCode::POP, line);
            break;
        }
        uint8_t n = (count < UINT8_MAX) ? static_cast<uint8_t>(count) : UINT8_MAX;
        emitOpArg8(opCode::POP_N, n, line);
        count -= n;
    }
}

void Chunk::emitScopeCleanup(const List<opCode> &ops, uint32_t line)
{
    uint32_t popCount = 0;
    for (const auto op : ops) {
        if (op == opCode::POP) {
            popCount++;
        } else if (op == opCode::CLOSE_UPVALUE) {
            if (popCount > 0) {
                emitPopN(popCount, line);
                popCount = 0;
            }
            emitOp(opCode::CLOSE_UPVALUE, line);
        } else {
            throw ariaCompilingException(
                ErrorCode::SEMANTIC_UNKNOWN_OPERATOR, "Unexpected opcode in scope cleanup");
        }
    }
    if (popCount > 0) {
        emitPopN(popCount, line);
    }
}

void Chunk::emitFunEndRet(uint32_t line, bool isInitMethod)
{
    if (isInitMethod) {
        emitOpArg16(opCode::LOAD_LOCAL, 0, line);
    } else {
        emitOp(opCode::LOAD_NIL, line);
    }
    emitOp(opCode::RETURN, line);
}

void Chunk::patchJump(uint32_t srcPos, uint32_t destPos)
{
    uint32_t offset = 0;
    if (destPos < srcPos) {
        // Jump forward
        rewriteOp(opCode::JUMP_FWD, srcPos - 1);
        offset = srcPos + 2 - destPos;
    } else {
        // Jump backward
        rewriteOp(opCode::JUMP_BWD, srcPos - 1);
        offset = destPos - (srcPos + 2);
    }
    if (offset > UINT16_MAX) {
        fatalError(ErrorCode::RESOURCE_JUMP_OVERFLOW, "Too much code to jump over.");
    }
    rewriteWord(offset, srcPos);
}

void Chunk::patchJump(uint32_t patchPos)
{
    uint32_t offset = count - (patchPos + 2);
    if (offset > UINT16_MAX) {
        fatalError(ErrorCode::RESOURCE_JUMP_OVERFLOW, "Too much code to jump over.");
    }
    rewriteWord(offset, patchPos);
}

void Chunk::disassemble(StringView name) const
{
    Disassembler::disassembleChunk(this, name);
}

uint32_t Chunk::lastOpLine() const
{
    if (count == 0) {
        return 0;
    }
    return lines[count - 1];
}

void Chunk::rewriteByte(uint8_t byte, uint32_t index)
{
    codes[index] = byte;
}

void Chunk::rewriteWord(uint16_t word, uint32_t index)
{
    auto [low, high] = splitWord(word);
    rewriteByte(low, index);
    rewriteByte(high, index + 1);
}

void Chunk::rewriteOp(opCode op, uint32_t index)
{
    rewriteByte(static_cast<uint8_t>(op), index);
}

} // namespace aria