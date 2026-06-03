#include <gtest/gtest.h>

#include "src/compile/lexer.h"

using namespace aria;

// 空输入
TEST(LexerTest2, EmptyInput)
{
    Lexer lexer{""};
    auto tokens = lexer.tokenize();
    // 空输入至少产生一个 CODE_EOF
    EXPECT_GE(tokens.size(), 1);
    EXPECT_FALSE(lexer.had_error());
}

// 所有运算符
TEST(LexerTest2, AllOperators)
{
    Lexer lexer{"+ - * / % ++ -- += -= *= /= == != > < >= <= = ! . , ; : ( ) { } [ ] ..."};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());
    // 去掉 CODE_EOF，数一下运算符数量
    int opCount = 0;
    for (const auto &t : tokens) {
        if (t.type != TokenType::CODE_EOF) opCount++;
    }
    EXPECT_EQ(opCount, 30);
}

// 所有关键字
TEST(LexerTest2, AllKeywords)
{
    Lexer lexer{"and as break catch class continue else false for fun if in import nil not or "
                "print return super this throw true try var while"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    int kwCount = 0;
    for (const auto &t : tokens) {
        if (t.type != TokenType::CODE_EOF) {
            EXPECT_NE(t.type, TokenType::IDENTIFIER);  // 不应有标识符
            kwCount++;
        }
    }
    EXPECT_EQ(kwCount, 25);
}

// 字符串字面量
TEST(LexerTest2, StringLiterals)
{
    Lexer lexer{"\"hello\" \"world\" \"\" \"escape\\n\""};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    int strCount = 0;
    for (const auto &t : tokens) {
        if (t.type == TokenType::STRING) strCount++;
    }
    EXPECT_EQ(strCount, 4);
}

// 数字字面量
TEST(LexerTest2, NumberLiterals)
{
    Lexer lexer{"0 42 3.14 100.0 0.5"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    int numCount = 0;
    for (const auto &t : tokens) {
        if (t.type == TokenType::NUMBER) numCount++;
    }
    EXPECT_EQ(numCount, 5);
}

// 标识符
TEST(LexerTest2, Identifiers)
{
    Lexer lexer{"foo bar_baz _x camelCase UPPER_CASE x123"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    int idCount = 0;
    for (const auto &t : tokens) {
        if (t.type == TokenType::IDENTIFIER) idCount++;
    }
    EXPECT_EQ(idCount, 6);
}

// 注释被跳过
TEST(LexerTest2, CommentsSkipped)
{
    Lexer lexer{"var x = 1 // this is a comment\nvar y = 2"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    // 不应产生注释类型的 token
    for (const auto &t : tokens) {
        // 注释被跳过，所有 token 都是有效类型
        EXPECT_NE(t.type, TokenType::ERROR);
    }
}

// 多行跟踪
TEST(LexerTest2, LineTracking)
{
    Lexer lexer{"a\nb\nc"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    // 第一个标识符在第 1 行
    EXPECT_EQ(tokens[0].line, 1);
    // 第二个在第 2 行
    EXPECT_EQ(tokens[1].line, 2);
    // 第三个在第 3 行
    EXPECT_EQ(tokens[2].line, 3);
}

// 单字符和双字符运算符区分
TEST(LexerTest2, SingleVsDoubleCharOps)
{
    Lexer lexer{"< <= > >= = == ! != .. ..."};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    EXPECT_EQ(tokens[0].type, TokenType::LESS);
    EXPECT_EQ(tokens[1].type, TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[2].type, TokenType::GREATER);
    EXPECT_EQ(tokens[3].type, TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[4].type, TokenType::EQUAL);
    EXPECT_EQ(tokens[5].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[6].type, TokenType::NOT);
    EXPECT_EQ(tokens[7].type, TokenType::NOT_EQUAL);
}

// 混合表达式
TEST(LexerTest2, MixedExpression)
{
    Lexer lexer{"var result = (a + b) * 2;"};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());

    EXPECT_EQ(tokens[0].type, TokenType::VAR);
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].type, TokenType::EQUAL);
    EXPECT_EQ(tokens[3].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[4].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[5].type, TokenType::PLUS);
    EXPECT_EQ(tokens[6].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[7].type, TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokens[8].type, TokenType::STAR);
    EXPECT_EQ(tokens[9].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[10].type, TokenType::SEMICOLON);
}

// 只有空白
TEST(LexerTest2, OnlyWhitespace)
{
    Lexer lexer{"   \t\n  "};
    auto tokens = lexer.tokenize();
    EXPECT_FALSE(lexer.had_error());
    // 只有 CODE_EOF
    EXPECT_EQ(tokens.size(), 1);
}
