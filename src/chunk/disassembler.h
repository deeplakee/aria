#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "chunk/code.h"

namespace aria {

class Chunk;

class Disassembler
{
public:
    static void disassembleChunk(const Chunk *chunk, StringView name);

    static uint32_t disassembleInstruction(const Chunk *chunk, uint32_t offset);

    static uint32_t simpleInstruction(const char *name, uint32_t offset);

    static uint32_t constantInstruction(const Chunk *chunk, String name, uint32_t offset);

    static uint32_t twoBytesInstruction(const Chunk *chunk, String name, uint32_t offset);

    static uint32_t threeBytesInstruction(const Chunk *chunk, String name, uint32_t offset);

    static uint32_t jumpInstruction(const Chunk *chunk, String name, uint32_t offset, int sign);

    static uint32_t closureInstruction(const Chunk *chunk, uint32_t offset);
};

} // namespace aria

#endif //DISASSEMBLER_H
