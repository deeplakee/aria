#ifndef CHUNK_H
#define CHUNK_H

#include "chunk/code.h"
#include "memory/gc.h"
#include "value/valueArray.h"

namespace aria {

class ValueHashTable;

class Chunk
{
public:
    explicit Chunk(GC *_gc);

    Chunk(ValueHashTable *_globals, GC *_gc);

    ~Chunk();

    uint8_t operator[](uint32_t offset) const;

    void emitByte(uint8_t byte, uint32_t line);

    void emitByte(uint8_t byte);

    void emitWord(uint16_t word, uint32_t line);

    void emitWord(uint16_t word);

    void emitOp(opCode op);

    void emitOp(opCode op, uint32_t line);

    void emitOpArg8(opCode op, uint8_t arg, uint32_t line);

    void emitOpArg16(opCode op, uint16_t arg, uint32_t line);

    void emitOpData(opCode op, Value value, uint32_t line);

    // Write a jump instruction with a placeholder (2 bytes)
    // return the offset position (for subsequent backfilling)
    uint32_t emitJump(opCode jumpOp, uint32_t line);

    void emitPopN(uint32_t count, uint32_t line);

    void emitScopeCleanup(const List<opCode> &ops, uint32_t line);

    void emitFunEndRet(uint32_t line, bool isInitMethod = false);

    void patchJump(uint32_t srcPos, uint32_t destPos);

    // patch forward position
    // equals patchJump(patchPos, current)
    void patchJump(uint32_t patchPos);

    void disassemble(StringView name) const;

    [[nodiscard]] uint32_t lastOpLine() const;

    GC *gc;
    uint32_t count;
    uint32_t capacity;
    uint8_t *codes;
    uint32_t *lines;
    ValueArray consts;
    ValueHashTable *globals;
    bool globalsManageable;

private:
    void rewriteOp(opCode op, uint32_t index);

    void rewriteByte(uint8_t byte, uint32_t index);

    void rewriteWord(uint16_t word, uint32_t index);
};

} // namespace aria

#endif //CHUNK_H
