#include <gtest/gtest.h>

#include "src/compile/token.h"
#include "util/util.h"

#include <sstream>

TEST(ComplieTokenTest, TokenTest)
{
    const char* C_fileName;
    auto fileName = std::make_shared<char []>(10);
    strncpy(fileName.get(), C_fileName,strlen(C_fileName));
    auto t = aria::Token{aria::TokenType::COLON,":", fileName, 11, 22};
    EXPECT_EQ(t.line, 11);
    EXPECT_EQ(t.column, 22);
    EXPECT_EQ(t.type, aria::TokenType::COLON);

    EXPECT_EQ(aria::Token::str2Type("nil"), aria::TokenType::NIL);
    EXPECT_EQ(aria::Token::str2Type("aaa"), aria::TokenType::IDENTIFIER);

    std::ostringstream oss;
    aria::print(oss, "{}", t.toString());
    EXPECT_STREQ(oss.str().c_str(), "<11 22 COLON ':'>");
}