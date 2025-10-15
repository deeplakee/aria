#include <gtest/gtest.h>

#include "src/compile/lexer.h"
#include "util/util.h"

#include <sstream>

TEST(ComplieLexerTest, LexerTest)
{
    aria::String str = "var a = 1;\n"
                       "println(a);\n"
                       "    # 1 = a;\n"
                       "var b = 3 * a;";

    const char *result = "<1 1 VAR 'var'>\n"
                         "<1 5 IDENTIFIER 'a'>\n"
                         "<1 7 EQUAL '='>\n"
                         "<1 9 NUMBER '1'>\n"
                         "<1 10 SEMICOLON ';'>\n"
                         "<2 1 IDENTIFIER 'println'>\n"
                         "<2 8 LEFT_PAREN '('>\n"
                         "<2 9 IDENTIFIER 'a'>\n"
                         "<2 10 RIGHT_PAREN ')'>\n"
                         "<2 11 SEMICOLON ';'>\n"
                         "<4 1 VAR 'var'>\n"
                         "<4 5 IDENTIFIER 'b'>\n"
                         "<4 7 EQUAL '='>\n"
                         "<4 9 NUMBER '3'>\n"
                         "<4 11 STAR '*'>\n"
                         "<4 13 IDENTIFIER 'a'>\n"
                         "<4 14 SEMICOLON ';'>\n"
                         "<4 15 EOF 'EOF'>\n";

    aria::Lexer lexer = aria::Lexer{"Repl", str};
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens.size(), 18);
    EXPECT_FALSE(lexer.hadError());

    std::ostringstream oss;
    for (const auto &token : tokens) {
        aria::println(oss, token.toString());
    }
    EXPECT_STREQ(oss.str().c_str(), result);
}