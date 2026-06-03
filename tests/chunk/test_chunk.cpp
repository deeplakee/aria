#include <gtest/gtest.h>

#include "tests/gc/gc_init.h"

#include "src/chunk/chunk.h"

using namespace aria;

class ChunkTest : public GCFixture
{
};

// emit_byte 和读取
TEST_F(ChunkTest, EmitByte)
{
    Chunk chunk{gc};
    chunk.emit_byte(0x42, 1);
    chunk.emit_byte(0xFF, 1);

    EXPECT_EQ(chunk.count_, 2);
    EXPECT_EQ(chunk[0], 0x42);
    EXPECT_EQ(chunk[1], 0xFF);
}

// emit_word (16 位，小端序)
TEST_F(ChunkTest, EmitWord)
{
    Chunk chunk{gc};
    chunk.emit_word(0x1234, 1);

    EXPECT_EQ(chunk.count_, 2);
    EXPECT_EQ(chunk[0], 0x34);  // 小端序：低字节在前
    EXPECT_EQ(chunk[1], 0x12);
}

// emit_op
TEST_F(ChunkTest, EmitOp)
{
    Chunk chunk{gc};
    chunk.emit_op(opCode::RETURN, 1);

    EXPECT_EQ(chunk.count_, 1);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::RETURN));
}

// emit_op_arg8
TEST_F(ChunkTest, EmitOpArg8)
{
    Chunk chunk{gc};
    chunk.emit_op_arg8(opCode::LOAD_LOCAL, 42, 1);

    EXPECT_EQ(chunk.count_, 2);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::LOAD_LOCAL));
    EXPECT_EQ(chunk[1], 42);
}

// emit_op_arg16
TEST_F(ChunkTest, EmitOpArg16)
{
    Chunk chunk{gc};
    chunk.emit_op_arg16(opCode::JUMP_FWD, 300, 1);

    EXPECT_EQ(chunk.count_, 3);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::JUMP_FWD));
    // 300 = 0x012C，小端序
    EXPECT_EQ(chunk[1], 0x2C);
    EXPECT_EQ(chunk[2], 0x01);
}

// emit_op_value 和常量池
TEST_F(ChunkTest, EmitOpValue)
{
    Chunk chunk{gc};
    chunk.emit_op_value(opCode::LOAD_CONST, NanBox::fromNumber(3.14), 1);

    EXPECT_EQ(chunk.count_, 3);  // opcode + 2 bytes index
    EXPECT_EQ(chunk.consts_.size(), 1);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(chunk.consts_[0]), 3.14);
}

// 多个常量
TEST_F(ChunkTest, MultipleConstants)
{
    Chunk chunk{gc};
    chunk.emit_op_value(opCode::LOAD_CONST, NanBox::fromNumber(1), 1);
    chunk.emit_op_value(opCode::LOAD_CONST, NanBox::fromNumber(2), 1);
    chunk.emit_op_value(opCode::LOAD_CONST, NanBox::fromNumber(3), 1);

    EXPECT_EQ(chunk.consts_.size(), 3);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(chunk.consts_[0]), 1.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(chunk.consts_[1]), 2.0);
    EXPECT_DOUBLE_EQ(NanBox::toNumber(chunk.consts_[2]), 3.0);
}

// emit_jump 返回 offset 字节的位置
TEST_F(ChunkTest, EmitJump)
{
    Chunk chunk{gc};
    // emit_jump: [opcode, 0xFF, 0xFF] 占 3 字节
    uint32_t jumpPos = chunk.emit_jump(opCode::JUMP_FWD, 1);

    EXPECT_EQ(chunk.count_, 3);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::JUMP_FWD));
    EXPECT_EQ(chunk[1], 0xFF);  // placeholder
    EXPECT_EQ(chunk[2], 0xFF);  // placeholder
    // jumpPos 是 offset 第一个字节的位置
    EXPECT_EQ(jumpPos, 1);
}

// patch_jump 填充跳转偏移（单参数版本）
TEST_F(ChunkTest, PatchJump)
{
    Chunk chunk{gc};
    // [JUMP_FWD, 0xFF, 0xFF] at 0,1,2
    uint32_t jumpPos = chunk.emit_jump(opCode::JUMP_FWD, 1);
    // [RETURN] at 3
    chunk.emit_op(opCode::RETURN, 1);

    // patch_jump(offset_pos) 计算 offset = count_ - (patch_pos + 2) = 4 - 3 = 1
    chunk.patch_jump(jumpPos);

    uint16_t offset = chunk[jumpPos] | (chunk[jumpPos + 1] << 8);
    EXPECT_EQ(offset, 1);
    // opcode 保持不变
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::JUMP_FWD));
}

// line_of_last_code
TEST_F(ChunkTest, LineOfLastCode)
{
    Chunk chunk{gc};
    chunk.emit_byte(0x01, 10);
    chunk.emit_byte(0x02, 20);
    chunk.emit_byte(0x03, 30);

    EXPECT_EQ(chunk.line_of_last_code(), 30);
}

// emit_pop_n 单字节
TEST_F(ChunkTest, EmitPopN_SingleByte)
{
    Chunk chunk{gc};
    // count=1 → emit POP
    chunk.emit_pop_n(1, 1);
    EXPECT_EQ(chunk.count_, 1);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::POP));
}

// emit_pop_n 多字节
TEST_F(ChunkTest, EmitPopN_MultiByte)
{
    Chunk chunk{gc};
    // count=5 → emit POP_N + 5
    chunk.emit_pop_n(5, 1);
    EXPECT_EQ(chunk.count_, 2);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::POP_N));
    EXPECT_EQ(chunk[1], 5);
}

// emit_fun_end_ret（非 init）
TEST_F(ChunkTest, EmitFunEndRet)
{
    Chunk chunk{gc};
    chunk.emit_fun_end_ret(1, false);

    EXPECT_EQ(chunk.count_, 2);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::LOAD_NIL));
    EXPECT_EQ(chunk[1], static_cast<uint8_t>(opCode::RETURN));
}

// emit_fun_end_ret（init method）
TEST_F(ChunkTest, EmitFunEndRetInit)
{
    Chunk chunk{gc};
    chunk.emit_fun_end_ret(1, true);

    // LOAD_LOCAL(1) + arg16(2) + RETURN(1) = 4 字节
    EXPECT_EQ(chunk.count_, 4);
    EXPECT_EQ(chunk[0], static_cast<uint8_t>(opCode::LOAD_LOCAL));
    EXPECT_EQ(chunk[1], 0);  // arg low byte
    EXPECT_EQ(chunk[2], 0);  // arg high byte
    EXPECT_EQ(chunk[3], static_cast<uint8_t>(opCode::RETURN));
}
