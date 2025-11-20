#include <gtest/gtest.h>

#include "src/util/util.h"

using namespace aria;

TEST(UtilTest, IsDigitTest) {
    // 测试数字字符
    EXPECT_TRUE(isDigit('0'));
    EXPECT_TRUE(isDigit('5'));
    EXPECT_TRUE(isDigit('9'));

    // 测试非数字字符
    EXPECT_FALSE(isDigit('a'));
    EXPECT_FALSE(isDigit('Z'));
    EXPECT_FALSE(isDigit('_'));
    EXPECT_FALSE(isDigit(' '));
    EXPECT_FALSE(isDigit('\0'));
    EXPECT_FALSE(isDigit('@'));
}

TEST(UtilTest, IsAlphaTest) {
    // 测试字母字符
    EXPECT_TRUE(isAlpha('a'));
    EXPECT_TRUE(isAlpha('z'));
    EXPECT_TRUE(isAlpha('A'));
    EXPECT_TRUE(isAlpha('Z'));
    EXPECT_TRUE(isAlpha('_'));  // 特殊处理下划线

    // 测试非字母字符
    EXPECT_FALSE(isAlpha('0'));
    EXPECT_FALSE(isAlpha('9'));
    EXPECT_FALSE(isAlpha(' '));
    EXPECT_FALSE(isAlpha('@'));
    EXPECT_FALSE(isAlpha('\0'));
}

TEST(UtilTest, IsAlphaNumTest) {
    // 测试字母数字字符
    EXPECT_TRUE(isAlphaNum('a'));
    EXPECT_TRUE(isAlphaNum('z'));
    EXPECT_TRUE(isAlphaNum('A'));
    EXPECT_TRUE(isAlphaNum('Z'));
    EXPECT_TRUE(isAlphaNum('0'));
    EXPECT_TRUE(isAlphaNum('9'));

    // 测试非字母数字字符
    EXPECT_FALSE(isAlphaNum('_'));
    EXPECT_FALSE(isAlphaNum(' '));
    EXPECT_FALSE(isAlphaNum('@'));
    EXPECT_FALSE(isAlphaNum('\0'));
}

TEST(UtilTest, SplitWordTest) {
    // 测试分割16位字
    uint16_t word = 0xABCD;
    auto result = splitWord(word);
    EXPECT_EQ(result[0], 0xCD);
    EXPECT_EQ(result[1], 0xAB);

    word = 0x1234;
    result = splitWord(word);
    EXPECT_EQ(result[0], 0x34);
    EXPECT_EQ(result[1], 0x12);

    word = 0x00FF;
    result = splitWord(word);
    EXPECT_EQ(result[0], 0xFF);
    EXPECT_EQ(result[1], 0x00);
}

TEST(UtilTest, SplitDWordTest) {
    // 测试分割32位字
    uint32_t dword = 0xABCD1234;
    auto result = splitDWord(dword);
    EXPECT_EQ(result[0], 0x34);
    EXPECT_EQ(result[1], 0x12);
    EXPECT_EQ(result[2], 0xCD);
    EXPECT_EQ(result[3], 0xAB);

    dword = 0x01020304;
    result = splitDWord(dword);
    EXPECT_EQ(result[0], 0x04);
    EXPECT_EQ(result[1], 0x03);
    EXPECT_EQ(result[2], 0x02);
    EXPECT_EQ(result[3], 0x01);

    dword = 0xFF000000;
    result = splitDWord(dword);
    EXPECT_EQ(result[0], 0x00);
    EXPECT_EQ(result[1], 0x00);
    EXPECT_EQ(result[2], 0x00);
    EXPECT_EQ(result[3], 0xFF);
}

TEST(UtilTest, IsZeroTest) {
    // 测试接近零的值
    EXPECT_TRUE(isZero(0.0));
    EXPECT_TRUE(isZero(1e-11));
    EXPECT_TRUE(isZero(-1e-11));

    // 测试非零值
    EXPECT_FALSE(isZero(1e-9));
    EXPECT_FALSE(isZero(-1e-9));
    EXPECT_FALSE(isZero(1.0));
    EXPECT_FALSE(isZero(-1.0));

    // 测试边界情况
    EXPECT_TRUE(isZero(0.9999999999e-10));
    EXPECT_FALSE(isZero(1.0000000001e-10));
}

TEST(UtilTest, NextPowerOf2Test) {
    // 这个函数最小返回16
    // 测试已经是2的幂的情况
    EXPECT_EQ(nextPowerOf2(1), 16);
    EXPECT_EQ(nextPowerOf2(2), 16);
    EXPECT_EQ(nextPowerOf2(4), 16);
    EXPECT_EQ(nextPowerOf2(1024), 1024);

    // 测试不是2的幂的情况
    EXPECT_EQ(nextPowerOf2(3), 16);
    EXPECT_EQ(nextPowerOf2(5), 16);
    EXPECT_EQ(nextPowerOf2(15), 16);
    EXPECT_EQ(nextPowerOf2(31), 32);
    EXPECT_EQ(nextPowerOf2(1023), 1024);

    // 测试边界情况
    EXPECT_EQ(nextPowerOf2(0), 16);  // 0的特殊处理
    EXPECT_EQ(nextPowerOf2(0x7FFFFFFF), 0x80000000);
}

TEST(UtilTest,EscapeBracesTest) {
    // 1. 普通字符串（无花括号）
    EXPECT_EQ(escapeBraces("Hello, World!"), "Hello, World!");

    // 2. 包含 { 的字符串
    EXPECT_EQ(escapeBraces("This is a { test"), "This is a {{ test");

    // 3. 包含 } 的字符串
    EXPECT_EQ(escapeBraces("This is a } test"), "This is a }} test");

    // 4. 同时包含 { 和 } 的字符串
    EXPECT_EQ(escapeBraces("This { is a } test"), "This {{ is a }} test");

    // 5. 空字符串
    EXPECT_EQ(escapeBraces(""), "");

    // 6. 连续多个 { 或 }
    EXPECT_EQ(escapeBraces("{{{}}}"), "{{{{{{}}}}}}");
}