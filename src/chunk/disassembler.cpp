#include "chunk/disassembler.h"
#include "chunk/chunk.h"
#include "object/objFunction.h"
#include "util/util.h"

namespace aria {

static uint16_t getU16data(const uint8_t *code, uint32_t offset)
{
    return static_cast<uint16_t>(code[offset]) | (static_cast<uint16_t>(code[offset + 1]) << 8);
}

void Disassembler::disassembleChunk(const Chunk *chunk, StringView name)
{
    println("  ======== {:^10} ========", name);

    for (uint32_t offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
        if (offset == -1) {
            break;
        }
    }
    println("  ======== {:^10} ========", "chunk end");
}

uint32_t Disassembler::disassembleInstruction(
    const Chunk *chunk, uint32_t offset, bool alwaysPrintLine)
{
    print("{:06} ", offset);
    if (alwaysPrintLine) {
        print("{:6} ", chunk->lines[offset]);
    } else {
        if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
            print("     | ");
        } else {
            print("{:6} ", chunk->lines[offset]);
        }
    }

    switch (auto instruction = static_cast<opCode>((*chunk)[offset])) {
    case opCode::LOAD_CONST:
        return constantInstruction(chunk, "LOAD_CONST", offset);
    case opCode::LOAD_NIL:
        return simpleInstruction("LOAD_NIL", offset);
    case opCode::LOAD_TRUE:
        return simpleInstruction("LOAD_TRUE", offset);
    case opCode::LOAD_FALSE:
        return simpleInstruction("LOAD_FALSE", offset);
    case opCode::LOAD_LOCAL:
        return threeBytesInstruction(chunk, "LOAD_LOCAL", offset);
    case opCode::STORE_LOCAL:
        return threeBytesInstruction(chunk, "STORE_LOCAL", offset);
    case opCode::LOAD_UPVALUE:
        return threeBytesInstruction(chunk, "LOAD_UPVALUE", offset);
    case opCode::STORE_UPVALUE:
        return threeBytesInstruction(chunk, "STORE_UPVALUE", offset);
    case opCode::CLOSE_UPVALUE:
        return simpleInstruction("CLOSE_UPVALUE", offset);
    case opCode::DEF_GLOBAL:
        return constantInstruction(chunk, "DEF_GLOBAL", offset);
    case opCode::LOAD_GLOBAL:
        return constantInstruction(chunk, "LOAD_GLOBAL", offset);
    case opCode::STORE_GLOBAL:
        return constantInstruction(chunk, "STORE_GLOBAL", offset);
    case opCode::LOAD_FIELD:
        return constantInstruction(chunk, "LOAD_FIELD", offset);
    case opCode::STORE_FIELD:
        return constantInstruction(chunk, "STORE_FIELD", offset);
    case opCode::LOAD_SUBSCR:
        return simpleInstruction("LOAD_SUBSCR", offset);
    case opCode::STORE_SUBSCR:
        return simpleInstruction("STORE_SUBSCR", offset);
    case opCode::EQUAL:
        return simpleInstruction("EQUAL", offset);
    case opCode::NOT_EQUAL:
        return simpleInstruction("NOT_EQUAL", offset);
    case opCode::GREATER:
        return simpleInstruction("GREATER", offset);
    case opCode::GREATER_EQUAL:
        return simpleInstruction("GREATER_EQUAL", offset);
    case opCode::LESS:
        return simpleInstruction("LESS", offset);
    case opCode::LESS_EQUAL:
        return simpleInstruction("LESS_EQUAL", offset);
    case opCode::ADD:
        return simpleInstruction("ADD", offset);
    case opCode::SUBTRACT:
        return simpleInstruction("SUBTRACT", offset);
    case opCode::MULTIPLY:
        return simpleInstruction("MULTIPLY", offset);
    case opCode::DIVIDE:
        return simpleInstruction("DIVIDE", offset);
    case opCode::MOD:
        return simpleInstruction("MOD", offset);
    case opCode::NOT:
        return simpleInstruction("NOT", offset);
    case opCode::NEGATE:
        return simpleInstruction("NEGATE", offset);
    case opCode::POP:
        return simpleInstruction("POP", offset);
    case opCode::POP_N:
        return twoBytesInstruction(chunk, "POP_N", offset);
    case opCode::PRINT:
        return simpleInstruction("PRINT", offset);
    case opCode::NOP:
        return simpleInstruction("NOP", offset);
    case opCode::JUMP_FWD:
        return jumpInstruction(chunk, "JUMP_FWD", offset, -1);
    case opCode::JUMP_BWD:
        return jumpInstruction(chunk, "JUMP_BWD", offset, 1);
    case opCode::JUMP_TRUE:
        return jumpInstruction(chunk, "JUMP_TRUE", offset, 1);
    case opCode::JUMP_TRUE_NOPOP:
        return jumpInstruction(chunk, "JUMP_TRUE_NOPOP", offset, 1);
    case opCode::JUMP_FALSE:
        return jumpInstruction(chunk, "JUMP_FALSE", offset, 1);
    case opCode::JUMP_FALSE_NOPOP:
        return jumpInstruction(chunk, "JUMP_FALSE_NOPOP", offset, 1);
    case opCode::CALL:
        return twoBytesInstruction(chunk, "CALL", offset);
    case opCode::CLOSURE:
        return closureInstruction(chunk, offset);
    case opCode::MAKE_CLASS:
        return constantInstruction(chunk, "MAKE_CLASS", offset);
    case opCode::INHERIT:
        return simpleInstruction("INHERIT", offset);
    case opCode::MAKE_METHOD:
        return constantInstruction(chunk, "MAKE_METHOD", offset);
    case opCode::MAKE_INIT_METHOD:
        return simpleInstruction("MAKE_INIT_METHOD", offset);
    case opCode::LOAD_SUPER_METHOD:
        return constantInstruction(chunk, "LOAD_SUPER_METHOD", offset);
    case opCode::MAKE_LIST:
        return threeBytesInstruction(chunk, "MAKE_LIST", offset);
    case opCode::MAKE_MAP:
        return threeBytesInstruction(chunk, "MAKE_MAP", offset);
    case opCode::IMPORT:
        return constantInstruction(chunk, "IMPORT", offset);
    case opCode::GET_ITER:
        return simpleInstruction("GET_ITER", offset);
    case opCode::ITER_HAS_NEXT:
        return simpleInstruction("ITER_HAS_NEXT", offset);
    case opCode::ITER_GET_NEXT:
        return simpleInstruction("ITER_GET_NEXT", offset);
    case opCode::SETUP_EXCEPT:
        return jumpInstruction(chunk, "SETUP_EXCEPT", offset, 1);
    case opCode::END_EXCEPT:
        return simpleInstruction("END_EXCEPT", offset);
    case opCode::THROW:
        return simpleInstruction("THROW", offset);
    case opCode::RETURN:
        return simpleInstruction("RETURN", offset);
    default:
        println("Unknown opcode {:02x}", static_cast<uint8_t>(instruction));
        return -1;
    }
}

// one byte instruction
uint32_t Disassembler::simpleInstruction(const char *name, const uint32_t offset)
{
    println("{}", name);
    return offset + 1;
}

// three bytes instruction
uint32_t Disassembler::constantInstruction(const Chunk *chunk, String name, const uint32_t offset)
{
    uint16_t slot = getU16data(chunk->codes, offset + 1);
    if (name == "LOAD_CONST") {
        String constName = valueRepresentation(chunk->consts[slot]);
        println("{:<18} ({}) {}", name, slot, constName);
    } else {
        String constName = valueString(chunk->consts[slot]);
        println("{:<18} {}", name, constName);
    }

    return offset + 3;
}

// two bytes instruction
uint32_t Disassembler::twoBytesInstruction(const Chunk *chunk, String name, const uint32_t offset)
{
    const uint8_t n = (*chunk)[offset + 1];
    println("{:<18} {}", name, n);
    return offset + 2;
}

// three bytes instruction
uint32_t Disassembler::threeBytesInstruction(const Chunk *chunk, String name, const uint32_t offset)
{
    uint16_t slot = getU16data(chunk->codes, offset + 1);
    if (name == "LOAD_LOCAL" || name == "STORE_LOCAL") {
        println("{:<18} base+{}", name, slot);
    } else if (name == "LOAD_UPVALUE" || name == "STORE_UPVALUE") {
        println("{:<18} upvalue({})", name, slot);
    } else {
        println("{:<18} {}", name, slot);
    }
    return offset + 3;
}

// three bytes instruction
uint32_t Disassembler::jumpInstruction(
    const Chunk *chunk, String name, const uint32_t offset, const int sign)
{
    uint16_t jump = getU16data(chunk->codes, offset + 1);
    println("{:<18} {} -> {}", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

//  1+3k bytes instruction
uint32_t Disassembler::closureInstruction(const Chunk *chunk, const uint32_t offset)
{
    uint32_t _offset = offset;
    uint16_t funIndex = getU16data(chunk->codes, _offset + 1);
    _offset += 3;
    ObjFunction *function = asObjFunction(chunk->consts[funIndex]);
    print("{:<18} {}\n", "CLOSURE", function->toString());
    for (int j = 0; j < function->upvalueCount; j++) {
        auto isLocal = (*chunk)[_offset++];
        String varType = isLocal ? "local" : "upvalue";
        uint16_t index = getU16data(chunk->codes, _offset);
        _offset += 2;
        println("{:06}      |{:<20}{}({})", _offset - 3, " ——————", varType, index);
    }
    return _offset;
}

uint32_t Disassembler::readInstruction(const Chunk *chunk, const uint32_t offset)
{
    if (offset >= chunk->count) {
        return 0;
    }
    switch (auto instruction = static_cast<opCode>((*chunk)[offset])) {
    case opCode::LOAD_CONST:
        return offset + 3;
    case opCode::LOAD_NIL:
        return offset + 1;
    case opCode::LOAD_TRUE:
        return offset + 1;
    case opCode::LOAD_FALSE:
        return offset + 1;
    case opCode::LOAD_LOCAL:
        return offset + 3;
    case opCode::STORE_LOCAL:
        return offset + 3;
    case opCode::LOAD_UPVALUE:
        return offset + 3;
    case opCode::STORE_UPVALUE:
        return offset + 3;
    case opCode::CLOSE_UPVALUE:
        return offset + 1;
    case opCode::DEF_GLOBAL:
        return offset + 3;
    case opCode::LOAD_GLOBAL:
        return offset + 3;
    case opCode::STORE_GLOBAL:
        return offset + 3;
    case opCode::LOAD_FIELD:
        return offset + 3;
    case opCode::STORE_FIELD:
        return offset + 3;
    case opCode::LOAD_SUBSCR:
        return offset + 1;
    case opCode::STORE_SUBSCR:
        return offset + 1;
    case opCode::EQUAL:
        return offset + 1;
    case opCode::NOT_EQUAL:
        return offset + 1;
    case opCode::GREATER:
        return offset + 1;
    case opCode::GREATER_EQUAL:
        return offset + 1;
    case opCode::LESS:
        return offset + 1;
    case opCode::LESS_EQUAL:
        return offset + 1;
    case opCode::ADD:
        return offset + 1;
    case opCode::SUBTRACT:
        return offset + 1;
    case opCode::MULTIPLY:
        return offset + 1;
    case opCode::DIVIDE:
        return offset + 1;
    case opCode::MOD:
        return offset + 1;
    case opCode::NOT:
        return offset + 1;
    case opCode::NEGATE:
        return offset + 1;
    case opCode::POP:
        return offset + 1;
    case opCode::POP_N:
        return offset + 2;
    case opCode::PRINT:
        return offset + 1;
    case opCode::NOP:
        return offset + 1;
    case opCode::JUMP_FWD:
        return offset + 3;
    case opCode::JUMP_BWD:
        return offset + 3;
    case opCode::JUMP_TRUE:
        return offset + 3;
    case opCode::JUMP_TRUE_NOPOP:
        return offset + 3;
    case opCode::JUMP_FALSE:
        return offset + 3;
    case opCode::JUMP_FALSE_NOPOP:
        return offset + 3;
    case opCode::CALL:
        return offset + 2;
    case opCode::CLOSURE: {
        uint32_t _offset = offset;
        uint16_t funIndex = getU16data(chunk->codes, _offset + 1);
        _offset += 3;
        ObjFunction *function = asObjFunction(chunk->consts[funIndex]);
        _offset += 2 * function->upvalueCount;
        return _offset;
    }
    case opCode::MAKE_CLASS:
        return offset + 3;
    case opCode::INHERIT:
        return offset + 1;
    case opCode::MAKE_METHOD:
        return offset + 3;
    case opCode::MAKE_INIT_METHOD:
        return offset + 1;
    case opCode::LOAD_SUPER_METHOD:
        return offset + 3;
    case opCode::MAKE_LIST:
        return offset + 3;
    case opCode::MAKE_MAP:
        return offset + 3;
    case opCode::IMPORT:
        return offset + 3;
    case opCode::GET_ITER:
        return offset + 1;
    case opCode::ITER_HAS_NEXT:
        return offset + 1;
    case opCode::ITER_GET_NEXT:
        return offset + 1;
    case opCode::SETUP_EXCEPT:
        return offset + 3;
    case opCode::END_EXCEPT:
        return offset + 1;
    case opCode::THROW:
        return offset + 1;
    case opCode::RETURN:
        return offset + 1;
    default:
        return offset + 1;
    }
}

} // namespace aria