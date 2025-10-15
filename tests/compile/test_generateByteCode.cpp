#include <gtest/gtest.h>

#include "compile/byteCodeGenerator.h"
#include "compile/functionContext.h"
#include "compile/lexer.h"
#include "memory/gc.h"
#include "object/objFunction.h"
#include "src/compile/parser.h"
#include "tests/gc/gc_init.h"
#include "util/util.h"

TEST_F(CompileGenByteCodeTest, GenByteCode1Test)
{
    aria::String source = R"(true and 1 * ( 2 + 3 ) || "Hello World!")";
    aria::String result = "  ========  example1  ========\n"
                          "000000      1 LOAD_TRUE\n"
                          "000001      | JUMP_FALSE_NOPOP   1 -> 16\n"
                          "000004      | POP\n"
                          "000005      | LOAD_CONST         (0) 1\n"
                          "000008      | LOAD_CONST         (1) 2\n"
                          "000011      | LOAD_CONST         (2) 3\n"
                          "000014      | ADD\n"
                          "000015      | MULTIPLY\n"
                          "000016      | JUMP_TRUE_NOPOP    16 -> 23\n"
                          "000019      | POP\n"
                          "000020      | LOAD_CONST         (3) 'Hello World!'\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parseExpression();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example1");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST_F(CompileGenByteCodeTest, GenByteCode2Test)
{
    aria::String source = R"(1 * ( 2 + 3 ) and true || "Hello World!";print 1+2*4-40/5%6;)";
    aria::String result = "  ========  example2  ========\n"
                          "000000      1 LOAD_CONST         (0) 1\n"
                          "000003      | LOAD_CONST         (1) 2\n"
                          "000006      | LOAD_CONST         (2) 3\n"
                          "000009      | ADD\n"
                          "000010      | MULTIPLY\n"
                          "000011      | JUMP_FALSE_NOPOP   11 -> 16\n"
                          "000014      | POP\n"
                          "000015      | LOAD_TRUE\n"
                          "000016      | JUMP_TRUE_NOPOP    16 -> 23\n"
                          "000019      | POP\n"
                          "000020      | LOAD_CONST         (3) 'Hello World!'\n"
                          "000023      | POP\n"
                          "000024      | LOAD_CONST         (4) 1\n"
                          "000027      | LOAD_CONST         (5) 2\n"
                          "000030      | LOAD_CONST         (6) 4\n"
                          "000033      | MULTIPLY\n"
                          "000034      | ADD\n"
                          "000035      | LOAD_CONST         (7) 40\n"
                          "000038      | LOAD_CONST         (8) 5\n"
                          "000041      | DIVIDE\n"
                          "000042      | LOAD_CONST         (9) 6\n"
                          "000045      | MOD\n"
                          "000046      | SUBTRACT\n"
                          "000047      | PRINT\n"
                          "000048      | LOAD_NIL\n"
                          "000049      | RETURN\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example2");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST_F(CompileGenByteCodeTest, GenByteCode3Test)
{
    aria::String source = R"(
    print "hello";
{
    var a = 1, b = 2;
    a = a + b * 2;
    print a;
}
    var a = "world";
    a = 100;
    print a;
)";

    aria::String result = "  ========  example3  ========\n"
                          "000000      2 LOAD_CONST         (0) 'hello'\n"
                          "000003      | PRINT\n"
                          "000004      4 LOAD_CONST         (1) 1\n"
                          "000007      | LOAD_CONST         (2) 2\n"
                          "000010      5 LOAD_LOCAL         base+1\n"
                          "000013      | LOAD_LOCAL         base+2\n"
                          "000016      | LOAD_CONST         (3) 2\n"
                          "000019      | MULTIPLY\n"
                          "000020      | ADD\n"
                          "000021      | STORE_LOCAL        base+1\n"
                          "000024      | POP\n"
                          "000025      6 LOAD_LOCAL         base+1\n"
                          "000028      | PRINT\n"
                          "000029      7 POP_N              2\n"
                          "000031      8 LOAD_CONST         (4) 'world'\n"
                          "000034      | DEF_GLOBAL         a\n"
                          "000037      9 LOAD_CONST         (6) 100\n"
                          "000040      | STORE_GLOBAL       a\n"
                          "000043      | POP\n"
                          "000044     10 LOAD_GLOBAL        a\n"
                          "000047      | PRINT\n"
                          "000048      | LOAD_NIL\n"
                          "000049      | RETURN\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example3");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST_F(CompileGenByteCodeTest, GenByteCode4Test)
{
    aria::String source = R"(
    var a = "world";
    a = 100;
    if(a == 10) {
        print "a is 10";
    } else if(a < 10){
        print "a is less than 10";
    } else {
        print "a is greater than 10";
    }
    a += 24;
    a -= 4;
    a *= 2;
    a /= 6;
    a %= 13;
    # 1 = a;
    a++;
    a--;
    print a;
)";

    aria::String result = "  ========  example4  ========\n"
                          "000000      2 LOAD_CONST         (0) 'world'\n"
                          "000003      | DEF_GLOBAL         a\n"
                          "000006      3 LOAD_CONST         (2) 100\n"
                          "000009      | STORE_GLOBAL       a\n"
                          "000012      | POP\n"
                          "000013      4 LOAD_GLOBAL        a\n"
                          "000016      | LOAD_CONST         (5) 10\n"
                          "000019      | EQUAL\n"
                          "000020      | JUMP_FALSE         20 -> 30\n"
                          "000023      5 LOAD_CONST         (6) 'a is 10'\n"
                          "000026      | PRINT\n"
                          "000027      | JUMP_BWD           27 -> 51\n"
                          "000030      6 LOAD_GLOBAL        a\n"
                          "000033      | LOAD_CONST         (8) 10\n"
                          "000036      | LESS\n"
                          "000037      | JUMP_FALSE         37 -> 47\n"
                          "000040      7 LOAD_CONST         (9) 'a is less than 10'\n"
                          "000043      | PRINT\n"
                          "000044      | JUMP_BWD           44 -> 51\n"
                          "000047      9 LOAD_CONST         (10) 'a is greater than 10'\n"
                          "000050      | PRINT\n"
                          "000051     11 LOAD_GLOBAL        a\n"
                          "000054      | LOAD_CONST         (12) 24\n"
                          "000057      | ADD\n"
                          "000058      | STORE_GLOBAL       a\n"
                          "000061      | POP\n"
                          "000062     12 LOAD_GLOBAL        a\n"
                          "000065      | LOAD_CONST         (15) 4\n"
                          "000068      | SUBTRACT\n"
                          "000069      | STORE_GLOBAL       a\n"
                          "000072      | POP\n"
                          "000073     13 LOAD_GLOBAL        a\n"
                          "000076      | LOAD_CONST         (18) 2\n"
                          "000079      | MULTIPLY\n"
                          "000080      | STORE_GLOBAL       a\n"
                          "000083      | POP\n"
                          "000084     14 LOAD_GLOBAL        a\n"
                          "000087      | LOAD_CONST         (21) 6\n"
                          "000090      | DIVIDE\n"
                          "000091      | STORE_GLOBAL       a\n"
                          "000094      | POP\n"
                          "000095     15 LOAD_GLOBAL        a\n"
                          "000098      | LOAD_CONST         (24) 13\n"
                          "000101      | MOD\n"
                          "000102      | STORE_GLOBAL       a\n"
                          "000105      | POP\n"
                          "000106     17 LOAD_GLOBAL        a\n"
                          "000109      | LOAD_CONST         (27) 1\n"
                          "000112      | ADD\n"
                          "000113      | STORE_GLOBAL       a\n"
                          "000116      | POP\n"
                          "000117     18 LOAD_GLOBAL        a\n"
                          "000120      | LOAD_CONST         (30) 1\n"
                          "000123      | SUBTRACT\n"
                          "000124      | STORE_GLOBAL       a\n"
                          "000127      | POP\n"
                          "000128     19 LOAD_GLOBAL        a\n"
                          "000131      | PRINT\n"
                          "000132      | LOAD_NIL\n"
                          "000133      | RETURN\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example4");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST_F(CompileGenByteCodeTest, GenByteCode5Test)
{
    aria::String source = R"(
    var i = 0;
    while( i < 10){
        var tmp1 = 1, tmp2 = i;
        i++;
        if(i == 3){
            continue;
        }
        if(i == 7){
            break;
        }
        print i;
    }
)";

    aria::String result = "  ========  example5  ========\n"
                          "000000      2 LOAD_CONST         (0) 0\n"
                          "000003      | DEF_GLOBAL         i\n"
                          "000006      3 LOAD_GLOBAL        i\n"
                          "000009      | LOAD_CONST         (3) 10\n"
                          "000012      | LESS\n"
                          "000013      | JUMP_FALSE         13 -> 72\n"
                          "000016      4 LOAD_CONST         (4) 1\n"
                          "000019      | LOAD_GLOBAL        i\n"
                          "000022      5 LOAD_GLOBAL        i\n"
                          "000025      | LOAD_CONST         (7) 1\n"
                          "000028      | ADD\n"
                          "000029      | STORE_GLOBAL       i\n"
                          "000032      | POP\n"
                          "000033      6 LOAD_GLOBAL        i\n"
                          "000036      | LOAD_CONST         (10) 3\n"
                          "000039      | EQUAL\n"
                          "000040      | JUMP_FALSE         40 -> 48\n"
                          "000043      7 POP_N              2\n"
                          "000045      | JUMP_FWD           45 -> 6\n"
                          "000048      9 LOAD_GLOBAL        i\n"
                          "000051      | LOAD_CONST         (12) 7\n"
                          "000054      | EQUAL\n"
                          "000055      | JUMP_FALSE         55 -> 63\n"
                          "000058     10 POP_N              2\n"
                          "000060      | JUMP_BWD           60 -> 72\n"
                          "000063     12 LOAD_GLOBAL        i\n"
                          "000066      | PRINT\n"
                          "000067     13 POP_N              2\n"
                          "000069      | JUMP_FWD           69 -> 6\n"
                          "000072      | LOAD_NIL\n"
                          "000073      | RETURN\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example5");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST_F(CompileGenByteCodeTest, GenByteCode6Test)
{
    aria::String source = R"(
    for(var i = 0; i < 10; i++){
        var tmp1 = 1, tmp2 = i;
        if(i == 3){
            continue;
        }
        if(i == 7){
            break;
        }
        print i;
    }
)";

    aria::String result = "  ========  example6  ========\n"
                          "000000      2 LOAD_CONST         (0) 0\n"
                          "000003      | LOAD_LOCAL         base+1\n"
                          "000006      | LOAD_CONST         (1) 10\n"
                          "000009      | LESS\n"
                          "000010      | JUMP_FALSE         10 -> 69\n"
                          "000013      3 LOAD_CONST         (2) 1\n"
                          "000016      | LOAD_LOCAL         base+1\n"
                          "000019      4 LOAD_LOCAL         base+1\n"
                          "000022      | LOAD_CONST         (3) 3\n"
                          "000025      | EQUAL\n"
                          "000026      | JUMP_FALSE         26 -> 34\n"
                          "000029      5 POP_N              2\n"
                          "000031      | JUMP_BWD           31 -> 55\n"
                          "000034      7 LOAD_LOCAL         base+1\n"
                          "000037      | LOAD_CONST         (4) 7\n"
                          "000040      | EQUAL\n"
                          "000041      | JUMP_FALSE         41 -> 49\n"
                          "000044      8 POP_N              2\n"
                          "000046      | JUMP_BWD           46 -> 69\n"
                          "000049     10 LOAD_LOCAL         base+1\n"
                          "000052      | PRINT\n"
                          "000053     11 POP_N              2\n"
                          "000055      2 LOAD_LOCAL         base+1\n"
                          "000058      | LOAD_CONST         (5) 1\n"
                          "000061      | ADD\n"
                          "000062      | STORE_LOCAL        base+1\n"
                          "000065      | POP\n"
                          "000066      | JUMP_FWD           66 -> 3\n"
                          "000069      | POP\n"
                          "000070      | LOAD_NIL\n"
                          "000071      | RETURN\n"
                          "  ======== chunk end  ========\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    aria::ByteCodeGenerator generator = aria::ByteCodeGenerator{"anonymous", "script", nullptr, gc};
    auto fn = generator.generateCode(ast);
    testing::internal::CaptureStdout();

    fn->chunk->disassemble("example6");

    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}