#include <gtest/gtest.h>

#include "compile/lexer.h"
#include "src/compile/parser.h"
#include "util/util.h"

#include <sstream>

TEST(ComplieParserTest, ParseExpr1Test)
{
    aria::String source = R"(-a+b*(c-100) >= !("hello,world	"%(c!=d)))";

    aria::String result = "BinaryExprNode\n"
                          "    ├── op:\n"
                          "    │       GREATER_EQUAL\n"
                          "    ├── lhs:\n"
                          "    │       BinaryExprNode\n"
                          "    │           ├── op:\n"
                          "    │           │       PLUS\n"
                          "    │           ├── lhs:\n"
                          "    │           │       UnaryExprNode\n"
                          "    │           │           ├── op:\n"
                          "    │           │           │       MINUS\n"
                          "    │           │           └── operand:\n"
                          "    │           │                   VarNode:a\n"
                          "    │           └── rhs:\n"
                          "    │                   BinaryExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       STAR\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:b\n"
                          "    │                       └── rhs:\n"
                          "    │                               BinaryExprNode\n"
                          "    │                                   ├── op:\n"
                          "    │                                   │       MINUS\n"
                          "    │                                   ├── lhs:\n"
                          "    │                                   │       VarNode:c\n"
                          "    │                                   └── rhs:\n"
                          "    │                                           NumberNode:100\n"
                          "    └── rhs:\n"
                          "            UnaryExprNode\n"
                          "                ├── op:\n"
                          "                │       NOT\n"
                          "                └── operand:\n"
                          "                        BinaryExprNode\n"
                          "                            ├── op:\n"
                          "                            │       MOD\n"
                          "                            ├── lhs:\n"
                          "                            │       StringNode:hello,world\t\n"
                          "                            └── rhs:\n"
                          "                                    BinaryExprNode\n"
                          "                                        ├── op:\n"
                          "                                        │       NOT_EQUAL\n"
                          "                                        ├── lhs:\n"
                          "                                        │       VarNode:c\n"
                          "                                        └── rhs:\n"
                          "                                                VarNode:d\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parseExpression();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST(ComplieParserTest, ParseExpr2Test)
{
    aria::String source = "a-b-c";
    aria::String result = "BinaryExprNode\n"
                          "    ├── op:\n"
                          "    │       MINUS\n"
                          "    ├── lhs:\n"
                          "    │       BinaryExprNode\n"
                          "    │           ├── op:\n"
                          "    │           │       MINUS\n"
                          "    │           ├── lhs:\n"
                          "    │           │       VarNode:a\n"
                          "    │           └── rhs:\n"
                          "    │                   VarNode:b\n"
                          "    └── rhs:\n"
                          "            VarNode:c\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parseExpression();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST(ComplieParserTest, ParseExpr3Test)
{
    aria::String source = R"(a && b || c / ("hello" + false) == true and true)";
    aria::String result
        = "BinaryExprNode\n"
          "    ├── op:\n"
          "    │       OR\n"
          "    ├── lhs:\n"
          "    │       BinaryExprNode\n"
          "    │           ├── op:\n"
          "    │           │       AND\n"
          "    │           ├── lhs:\n"
          "    │           │       VarNode:a\n"
          "    │           └── rhs:\n"
          "    │                   VarNode:b\n"
          "    └── rhs:\n"
          "            BinaryExprNode\n"
          "                ├── op:\n"
          "                │       AND\n"
          "                ├── lhs:\n"
          "                │       BinaryExprNode\n"
          "                │           ├── op:\n"
          "                │           │       EQUAL_EQUAL\n"
          "                │           ├── lhs:\n"
          "                │           │       BinaryExprNode\n"
          "                │           │           ├── op:\n"
          "                │           │           │       SLASH\n"
          "                │           │           ├── lhs:\n"
          "                │           │           │       VarNode:c\n"
          "                │           │           └── rhs:\n"
          "                │           │                   BinaryExprNode\n"
          "                │           │                       ├── op:\n"
          "                │           │                       │       PLUS\n"
          "                │           │                       ├── lhs:\n"
          "                │           │                       │       StringNode:hello\n"
          "                │           │                       └── rhs:\n"
          "                │           │                               FalseNode\n"
          "                │           └── rhs:\n"
          "                │                   TrueNode\n"
          "                └── rhs:\n"
          "                        TrueNode\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parseExpression();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST(ComplieParserTest, ParseProgram4Test)
{
    aria::String source = R"(1 * ( 2 + 3 ) and true || "Hello World!";print 1+2*4-40/5%6;)";
    aria::String result
        = "ProgramNode\n"
          "    ├── decl:\n"
          "    │       ExprStmtNode\n"
          "    │           └── expr:\n"
          "    │                   BinaryExprNode\n"
          "    │                       ├── op:\n"
          "    │                       │       OR\n"
          "    │                       ├── lhs:\n"
          "    │                       │       BinaryExprNode\n"
          "    │                       │           ├── op:\n"
          "    │                       │           │       AND\n"
          "    │                       │           ├── lhs:\n"
          "    │                       │           │       BinaryExprNode\n"
          "    │                       │           │           ├── op:\n"
          "    │                       │           │           │       STAR\n"
          "    │                       │           │           ├── lhs:\n"
          "    │                       │           │           │       NumberNode:1\n"
          "    │                       │           │           └── rhs:\n"
          "    │                       │           │                   BinaryExprNode\n"
          "    │                       │           │                       ├── op:\n"
          "    │                       │           │                       │       PLUS\n"
          "    │                       │           │                       ├── lhs:\n"
          "    │                       │           │                       │       NumberNode:2\n"
          "    │                       │           │                       └── rhs:\n"
          "    │                       │           │                               NumberNode:3\n"
          "    │                       │           └── rhs:\n"
          "    │                       │                   TrueNode\n"
          "    │                       └── rhs:\n"
          "    │                               StringNode:Hello World!\n"
          "    └── decl:\n"
          "            PrintStmtNode\n"
          "                └── expr:\n"
          "                        BinaryExprNode\n"
          "                            ├── op:\n"
          "                            │       MINUS\n"
          "                            ├── lhs:\n"
          "                            │       BinaryExprNode\n"
          "                            │           ├── op:\n"
          "                            │           │       PLUS\n"
          "                            │           ├── lhs:\n"
          "                            │           │       NumberNode:1\n"
          "                            │           └── rhs:\n"
          "                            │                   BinaryExprNode\n"
          "                            │                       ├── op:\n"
          "                            │                       │       STAR\n"
          "                            │                       ├── lhs:\n"
          "                            │                       │       NumberNode:2\n"
          "                            │                       └── rhs:\n"
          "                            │                               NumberNode:4\n"
          "                            └── rhs:\n"
          "                                    BinaryExprNode\n"
          "                                        ├── op:\n"
          "                                        │       MOD\n"
          "                                        ├── lhs:\n"
          "                                        │       BinaryExprNode\n"
          "                                        │           ├── op:\n"
          "                                        │           │       SLASH\n"
          "                                        │           ├── lhs:\n"
          "                                        │           │       NumberNode:40\n"
          "                                        │           └── rhs:\n"
          "                                        │                   NumberNode:5\n"
          "                                        └── rhs:\n"
          "                                                NumberNode:6\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST(ComplieParserTest, ParseProgram5Test)
{
    aria::String source = R"(
1 * ( 2 + 3 ) and true || "Hello World!";
print 1+2*4-40/5%6;
{
    print 1+2;
    print 3*4;
    print 4/5;
    print 5%6;
    print true;
}
)";
    aria::String result
        = "ProgramNode\n"
          "    ├── decl:\n"
          "    │       ExprStmtNode\n"
          "    │           └── expr:\n"
          "    │                   BinaryExprNode\n"
          "    │                       ├── op:\n"
          "    │                       │       OR\n"
          "    │                       ├── lhs:\n"
          "    │                       │       BinaryExprNode\n"
          "    │                       │           ├── op:\n"
          "    │                       │           │       AND\n"
          "    │                       │           ├── lhs:\n"
          "    │                       │           │       BinaryExprNode\n"
          "    │                       │           │           ├── op:\n"
          "    │                       │           │           │       STAR\n"
          "    │                       │           │           ├── lhs:\n"
          "    │                       │           │           │       NumberNode:1\n"
          "    │                       │           │           └── rhs:\n"
          "    │                       │           │                   BinaryExprNode\n"
          "    │                       │           │                       ├── op:\n"
          "    │                       │           │                       │       PLUS\n"
          "    │                       │           │                       ├── lhs:\n"
          "    │                       │           │                       │       NumberNode:2\n"
          "    │                       │           │                       └── rhs:\n"
          "    │                       │           │                               NumberNode:3\n"
          "    │                       │           └── rhs:\n"
          "    │                       │                   TrueNode\n"
          "    │                       └── rhs:\n"
          "    │                               StringNode:Hello World!\n"
          "    ├── decl:\n"
          "    │       PrintStmtNode\n"
          "    │           └── expr:\n"
          "    │                   BinaryExprNode\n"
          "    │                       ├── op:\n"
          "    │                       │       MINUS\n"
          "    │                       ├── lhs:\n"
          "    │                       │       BinaryExprNode\n"
          "    │                       │           ├── op:\n"
          "    │                       │           │       PLUS\n"
          "    │                       │           ├── lhs:\n"
          "    │                       │           │       NumberNode:1\n"
          "    │                       │           └── rhs:\n"
          "    │                       │                   BinaryExprNode\n"
          "    │                       │                       ├── op:\n"
          "    │                       │                       │       STAR\n"
          "    │                       │                       ├── lhs:\n"
          "    │                       │                       │       NumberNode:2\n"
          "    │                       │                       └── rhs:\n"
          "    │                       │                               NumberNode:4\n"
          "    │                       └── rhs:\n"
          "    │                               BinaryExprNode\n"
          "    │                                   ├── op:\n"
          "    │                                   │       MOD\n"
          "    │                                   ├── lhs:\n"
          "    │                                   │       BinaryExprNode\n"
          "    │                                   │           ├── op:\n"
          "    │                                   │           │       SLASH\n"
          "    │                                   │           ├── lhs:\n"
          "    │                                   │           │       NumberNode:40\n"
          "    │                                   │           └── rhs:\n"
          "    │                                   │                   NumberNode:5\n"
          "    │                                   └── rhs:\n"
          "    │                                           NumberNode:6\n"
          "    └── decl:\n"
          "            BlockNode\n"
          "                ├── decl:\n"
          "                │       PrintStmtNode\n"
          "                │           └── expr:\n"
          "                │                   BinaryExprNode\n"
          "                │                       ├── op:\n"
          "                │                       │       PLUS\n"
          "                │                       ├── lhs:\n"
          "                │                       │       NumberNode:1\n"
          "                │                       └── rhs:\n"
          "                │                               NumberNode:2\n"
          "                ├── decl:\n"
          "                │       PrintStmtNode\n"
          "                │           └── expr:\n"
          "                │                   BinaryExprNode\n"
          "                │                       ├── op:\n"
          "                │                       │       STAR\n"
          "                │                       ├── lhs:\n"
          "                │                       │       NumberNode:3\n"
          "                │                       └── rhs:\n"
          "                │                               NumberNode:4\n"
          "                ├── decl:\n"
          "                │       PrintStmtNode\n"
          "                │           └── expr:\n"
          "                │                   BinaryExprNode\n"
          "                │                       ├── op:\n"
          "                │                       │       SLASH\n"
          "                │                       ├── lhs:\n"
          "                │                       │       NumberNode:4\n"
          "                │                       └── rhs:\n"
          "                │                               NumberNode:5\n"
          "                ├── decl:\n"
          "                │       PrintStmtNode\n"
          "                │           └── expr:\n"
          "                │                   BinaryExprNode\n"
          "                │                       ├── op:\n"
          "                │                       │       MOD\n"
          "                │                       ├── lhs:\n"
          "                │                       │       NumberNode:5\n"
          "                │                       └── rhs:\n"
          "                │                               NumberNode:6\n"
          "                └── decl:\n"
          "                        PrintStmtNode\n"
          "                            └── expr:\n"
          "                                    TrueNode\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}

TEST(ComplieParserTest, ParseProgram6Test)
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
    aria::String result = "ProgramNode\n"
                          "    ├── decl:\n"
                          "    │       VarDeclNode\n"
                          "    │           └── a:\n"
                          "    │                   StringNode:world\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:100\n"
                          "    ├── decl:\n"
                          "    │       IfStmtNode\n"
                          "    │           ├── condition:\n"
                          "    │           │       BinaryExprNode\n"
                          "    │           │           ├── op:\n"
                          "    │           │           │       EQUAL_EQUAL\n"
                          "    │           │           ├── lhs:\n"
                          "    │           │           │       VarNode:a\n"
                          "    │           │           └── rhs:\n"
                          "    │           │                   NumberNode:10\n"
                          "    │           ├── body:\n"
                          "    │           │       BlockNode\n"
                          "    │           │           └── decl:\n"
                          "    │           │                   PrintStmtNode\n"
                          "    │           │                       └── expr:\n"
                          "    │           │                               StringNode:a is 10\n"
                          "    │           └── elseBody:\n"
                          "    │                   IfStmtNode\n"
                          "    │                       ├── condition:\n"
                          "    │                       │       BinaryExprNode\n"
                          "    │                       │           ├── op:\n"
                          "    │                       │           │       LESS\n"
                          "    │                       │           ├── lhs:\n"
                          "    │                       │           │       VarNode:a\n"
                          "    │                       │           └── rhs:\n"
                          "    │                       │                   NumberNode:10\n"
                          "    │                       ├── body:\n"
                          "    │                       │       BlockNode\n"
                          "    │                       │           └── decl:\n"
                          "    │                       │                   PrintStmtNode\n"
                          "    │                       │                       └── expr:\n"
                          "    │                       │                               "
                          "StringNode:a is less than 10\n"
                          "    │                       └── elseBody:\n"
                          "    │                               BlockNode\n"
                          "    │                                   └── decl:\n"
                          "    │                                           PrintStmtNode\n"
                          "    │                                               └── expr:\n"
                          "    │                                                       "
                          "StringNode:a is greater than 10\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       PLUS_EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:24\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       MINUS_EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:4\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       STAR_EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:2\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       SLASH_EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:6\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   AssignExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       PERCENT_EQUAL\n"
                          "    │                       ├── lhs:\n"
                          "    │                       │       VarNode:a\n"
                          "    │                       └── rhs:\n"
                          "    │                               NumberNode:13\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   IncExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       INCREMENT\n"
                          "    │                       └── operand:\n"
                          "    │                               VarNode:a\n"
                          "    ├── decl:\n"
                          "    │       ExprStmtNode\n"
                          "    │           └── expr:\n"
                          "    │                   IncExprNode\n"
                          "    │                       ├── op:\n"
                          "    │                       │       DECREMENT\n"
                          "    │                       └── operand:\n"
                          "    │                               VarNode:a\n"
                          "    └── decl:\n"
                          "            PrintStmtNode\n"
                          "                └── expr:\n"
                          "                        VarNode:a\n";

    auto lexer = aria::Lexer{source};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.hadError());
    auto parser = aria::Parser{tokens};
    auto ast = parser.parse();
    EXPECT_FALSE(parser.hasError());
    testing::internal::CaptureStdout();
    ast->display("");
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, result);
}