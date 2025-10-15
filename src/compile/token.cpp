#include "compile/token.h"
#include "util/util.h"

namespace aria {

void printTokenList(const List<Token> &tokens)
{
    for (const Token &token : tokens) {
        println(token.toString());
    }
}

Map<String, TokenType> Token::keywords = {
    {"and", TokenType::AND},           /**< "and" 关键字对应的标记类型。 */
    {"as", TokenType::AS},             /**< "as" 关键字对应的标记类型。 */
    {"break", TokenType::BREAK},       /**< "break" 关键字对应的标记类型。 */
    {"catch", TokenType::CATCH},       /**< "catch" 关键字对应的标记类型。 */
    {"class", TokenType::CLASS},       /**< "class" 关键字对应的标记类型。 */
    {"continue", TokenType::CONTINUE}, /**< "continue" 关键字对应的标记类型。 */
    {"else", TokenType::ELSE},         /**< "else" 关键字对应的标记类型。 */
    {"false", TokenType::FALSE},       /**< "false" 关键字对应的标记类型。 */
    {"for", TokenType::FOR},           /**< "for" 关键字对应的标记类型。 */
    {"fun", TokenType::FUN},           /**< "function" 关键字对应的标记类型。 */
    {"if", TokenType::IF},             /**< "if" 关键字对应的标记类型。 */
    {"in", TokenType::IN},             /**< "if" 关键字对应的标记类型。 */
    {"import", TokenType::IMPORT},     /**< "import" 关键字对应的标记类型。 */
    {"nil", TokenType::NIL},           /**< "nil" 关键字对应的标记类型。 */
    {"not", TokenType::NOT},           /**< "not" 关键字对应的标记类型。 */
    {"or", TokenType::OR},             /**< "or" 关键字对应的标记类型。 */
    {"print", TokenType::PRINT},       /**< "print" 关键字对应的标记类型。 */
    {"return", TokenType::RETURN},     /**< "return" 关键字对应的标记类型。 */
    {"super", TokenType::SUPER},       /**< "super" 关键字对应的标记类型。 */
    {"this", TokenType::THIS},         /**< "this" 关键字对应的标记类型。 */
    {"throw", TokenType::THROW},       /**< "throw" 关键字对应的标记类型。 */
    {"true", TokenType::TRUE},         /**< "true" 关键字对应的标记类型。 */
    {"try", TokenType::TRY},           /**< "try" 关键字对应的标记类型。 */
    {"var", TokenType::VAR},           /**< "var" 关键字对应的标记类型。 */
    {"while", TokenType::WHILE},       /**< "while" 关键字对应的标记类型。 */
};

const char *Token::tokenText[] = {

    "LEFT_PAREN",    ///< 左括号 (，用于表示代码中的分组或函数调用。
    "RIGHT_PAREN",   ///< 右括号 )，用于表示代码中的分组或函数调用。
    "LEFT_BRACE",    ///< 左花括号 {，用于表示代码中的代码块的开始。
    "RIGHT_BRACE",   ///< 右花括号 }，用于表示代码中的代码块的结束。
    "LEFT_BRACKET",  ///< 左方括号 [，用于表示数组下标访问或其他用途。
    "RIGHT_BRACKET", ///< 右方括号 ]，用于表示数组下标访问或其他用途。
    "COMMA",         ///< 逗号 ,，用于分隔函数参数或数组元素等。
    "DOT",           ///< 点 .，用于访问对象的属性或方法。
    "COLON",         ///< 冒号 :, 用于类继承和哈希表初始化
    "MINUS",         ///< 减号 -，用于表示减法操作。
    "PLUS",          ///< 加号 +，用于表示加法操作。
    "SEMICOLON",     ///< 分号 ;，用于表示语句的结束。
    "SLASH",         ///< 斜杠 /，用于表示除法操作。
    "STAR",          ///< 星号 *，用于表示乘法操作。
    "MOD",           ///< 百分号 %，用于表示取模操作。
    "ELLIPSIS",            ///< 省略号 ...，用于表示不定参数个数。
    "INCREMENT",     ///< 双加号 ++，用于表示自增操作。
    "DECREMENT",     ///< 双减号 --，用于表示自减操作。

    "NOT_EQUAL",     ///< 感叹号等号 !=，用于表示不等于比较操作。
    "EQUAL",         ///< 单个等号 =，用于赋值操作。
    "EQUAL_EQUAL",   ///< 双等号 ==，用于相等比较操作。
    "GREATER",       ///< 大于号 >，用于大于比较操作。
    "GREATER_EQUAL", ///< 大于等于号 >=，用于大于等于比较操作。
    "LESS",          ///< 小于号 <，用于小于比较操作。
    "LESS_EQUAL",    ///< 小于等于号 <=，用于小于等于比较操作。
    "PLUS_EQUAL",          ///< 加等于号 +=，用于加法操作
    "MINUS_EQUAL",         ///< 减等于号 -=，用于减法操作
    "STAR_EQUAL",          ///< 乘等于号 *=，用于乘法操作
    "SLASH_EQUAL",         ///< 除等于号 /=，用于除法操作
    "PERCENT_EQUAL",       ///< 模等于号 %=，用于取模操作

    "IDENTIFIER", ///< 标识符，用于表示变量名、函数名等
    "STRING",     ///< 字符串字面量，表示一串文本。
    "NUMBER",     ///< 数字字面量，表示数字。

    // Keywords
    "AND",      ///< and &&
    "AS",       ///< as
    "BREAK",    ///< break
    "CATCH",    ///< catch
    "CLASS",    ///< class
    "CONTINUE", ///< continue
    "ELSE",     ///< else
    "FALSE",    ///< false
    "FOR",      ///< for
    "FUN",      ///< fun
    "IF",       ///< if
    "IN",       ///< in
    "IMPORT",   ///< import
    "NIL",      ///< nil
    "NOT",      ///< not !
    "OR",       ///< or ||
    "PRINT",    ///< print
    "RETURN",   ///< return
    "SUPER",    ///< super
    "THIS",     ///< this
    "THROW",    ///< throw
    "TRUE",     ///< true
    "TRY",      ///< try
    "VAR",      ///< var
    "WHILE",    ///< while

    "ERROR",
    "EOF" ///< EOF,表示已经到达代码文件的末尾。
};

} // namespace aria
