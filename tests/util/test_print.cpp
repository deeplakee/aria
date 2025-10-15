#include <gtest/gtest.h>

#include "src/util/util.h"

#include <sstream>

TEST(PrintUtilTest, printAndprintln)
{
    std::ostringstream oss;
    aria::print(oss, "hello world");
    EXPECT_STREQ(oss.str().c_str(), "hello world");

    std::ostringstream oss1;
    aria::print(oss1, "hello {}\t\n","world");
    EXPECT_STREQ(oss1.str().c_str(), "hello world\t\n");

    std::ostringstream oss2;
    aria::println(oss2, "hello world");
    EXPECT_STREQ(oss2.str().c_str(), "hello world\n");

    std::ostringstream oss3;
    aria::println(oss3, "hello {} {}\t","world",22);
    EXPECT_STREQ(oss3.str().c_str(), "hello world 22\t\n");

    std::ostringstream oss4;
    aria::print(oss4, "");
    EXPECT_STREQ(oss4.str().c_str(), "");
}