#include <gtest/gtest.h>

#include "src/util/util.h"

using namespace aria;

TEST(UtilTest, IsDigitTest) {
    // 测试数字字符
    EXPECT_TRUE(is_digit('0'));
    EXPECT_TRUE(is_digit('5'));
    EXPECT_TRUE(is_digit('9'));

    // 测试非数字字符
    EXPECT_FALSE(is_digit('a'));
    EXPECT_FALSE(is_digit('Z'));
    EXPECT_FALSE(is_digit('_'));
    EXPECT_FALSE(is_digit(' '));
    EXPECT_FALSE(is_digit('\0'));
    EXPECT_FALSE(is_digit('@'));
}

TEST(UtilTest, IsAlphaTest) {
    // 测试字母字符
    EXPECT_TRUE(is_alpha('a'));
    EXPECT_TRUE(is_alpha('z'));
    EXPECT_TRUE(is_alpha('A'));
    EXPECT_TRUE(is_alpha('Z'));
    EXPECT_TRUE(is_alpha('_'));  // 特殊处理下划线

    // 测试非字母字符
    EXPECT_FALSE(is_alpha('0'));
    EXPECT_FALSE(is_alpha('9'));
    EXPECT_FALSE(is_alpha(' '));
    EXPECT_FALSE(is_alpha('@'));
    EXPECT_FALSE(is_alpha('\0'));
}

TEST(UtilTest, IsAlphaNumTest) {
    // 测试字母数字字符
    EXPECT_TRUE(is_alpha_num('a'));
    EXPECT_TRUE(is_alpha_num('z'));
    EXPECT_TRUE(is_alpha_num('A'));
    EXPECT_TRUE(is_alpha_num('Z'));
    EXPECT_TRUE(is_alpha_num('0'));
    EXPECT_TRUE(is_alpha_num('9'));

    // 测试非字母数字字符
    EXPECT_FALSE(is_alpha_num('_'));
    EXPECT_FALSE(is_alpha_num(' '));
    EXPECT_FALSE(is_alpha_num('@'));
    EXPECT_FALSE(is_alpha_num('\0'));
}

TEST(UtilTest, SplitWordTest) {
    // 测试分割16位字
    uint16_t word = 0xABCD;
    auto result = split_word(word);
    EXPECT_EQ(result[0], 0xCD);
    EXPECT_EQ(result[1], 0xAB);

    word = 0x1234;
    result = split_word(word);
    EXPECT_EQ(result[0], 0x34);
    EXPECT_EQ(result[1], 0x12);

    word = 0x00FF;
    result = split_word(word);
    EXPECT_EQ(result[0], 0xFF);
    EXPECT_EQ(result[1], 0x00);
}

TEST(UtilTest, SplitDWordTest) {
    // 测试分割32位字
    uint32_t dword = 0xABCD1234;
    auto result = split_dword(dword);
    EXPECT_EQ(result[0], 0x34);
    EXPECT_EQ(result[1], 0x12);
    EXPECT_EQ(result[2], 0xCD);
    EXPECT_EQ(result[3], 0xAB);

    dword = 0x01020304;
    result = split_dword(dword);
    EXPECT_EQ(result[0], 0x04);
    EXPECT_EQ(result[1], 0x03);
    EXPECT_EQ(result[2], 0x02);
    EXPECT_EQ(result[3], 0x01);

    dword = 0xFF000000;
    result = split_dword(dword);
    EXPECT_EQ(result[0], 0x00);
    EXPECT_EQ(result[1], 0x00);
    EXPECT_EQ(result[2], 0x00);
    EXPECT_EQ(result[3], 0xFF);
}

TEST(UtilTest, IsZeroTest) {
    // 测试接近零的值
    EXPECT_TRUE(is_zero(0.0));
    EXPECT_TRUE(is_zero(1e-11));
    EXPECT_TRUE(is_zero(-1e-11));

    // 测试非零值
    EXPECT_FALSE(is_zero(1e-9));
    EXPECT_FALSE(is_zero(-1e-9));
    EXPECT_FALSE(is_zero(1.0));
    EXPECT_FALSE(is_zero(-1.0));

    // 测试边界情况
    EXPECT_TRUE(is_zero(0.9999999999e-10));
    EXPECT_FALSE(is_zero(1.0000000001e-10));
}

TEST(UtilTest, NextPowerOf2Test) {
    // 这个函数最小返回16
    // 测试已经是2的幂的情况
    EXPECT_EQ(next_power_of_2(1), 16);
    EXPECT_EQ(next_power_of_2(2), 16);
    EXPECT_EQ(next_power_of_2(4), 16);
    EXPECT_EQ(next_power_of_2(1024), 1024);

    // 测试不是2的幂的情况
    EXPECT_EQ(next_power_of_2(3), 16);
    EXPECT_EQ(next_power_of_2(5), 16);
    EXPECT_EQ(next_power_of_2(15), 16);
    EXPECT_EQ(next_power_of_2(31), 32);
    EXPECT_EQ(next_power_of_2(1023), 1024);

    // 测试边界情况
    EXPECT_EQ(next_power_of_2(0), 16);  // 0的特殊处理
    EXPECT_EQ(next_power_of_2(0x7FFFFFFF), 0x80000000);
}

TEST(UtilTest,EscapeBracesTest) {
    // 1. 普通字符串（无花括号）
    EXPECT_EQ(escape_braces("Hello, World!"), "Hello, World!");

    // 2. 包含 { 的字符串
    EXPECT_EQ(escape_braces("This is a { test"), "This is a {{ test");

    // 3. 包含 } 的字符串
    EXPECT_EQ(escape_braces("This is a } test"), "This is a }} test");

    // 4. 同时包含 { 和 } 的字符串
    EXPECT_EQ(escape_braces("This { is a } test"), "This {{ is a }} test");

    // 5. 空字符串
    EXPECT_EQ(escape_braces(""), "");

    // 6. 连续多个 { 或 }
    EXPECT_EQ(escape_braces("{{{}}}"), "{{{{{{}}}}}}");
}