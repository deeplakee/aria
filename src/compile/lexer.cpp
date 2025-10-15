#include "compile/lexer.h"
#include "error/ariaException.h"
#include "error/error.h"
#include "util/util.h"

#include <utility>

namespace aria {

List<Token> Lexer::tokenize()
{
    List<Token> tokens;
    while (true) {
        tokens.push_back(scanToken());
        if (tokens.back().type == TokenType::CODE_EOF) {
            break;
        }
    }
    return tokens;
}

Token Lexer::scanToken()
{
    skipWhitespace();
    start = current;

    if (isAtEnd()) {
        return makeToken(TokenType::CODE_EOF, "EOF");
    }

    char c = advance();
    if (isAlpha(c)) {
        return makeIdentifier();
    }
    if (isDigit(c)) {
        return makeNumber();
    }

    switch (c) {
    case '(':
        return makeToken(TokenType::LEFT_PAREN, "(");
    case ')':
        return makeToken(TokenType::RIGHT_PAREN, ")");
    case '[':
        return makeToken(TokenType::LEFT_BRACKET, "[");
    case ']':
        return makeToken(TokenType::RIGHT_BRACKET, "]");
    case '{':
        return makeToken(TokenType::LEFT_BRACE, "{");
    case '}':
        return makeToken(TokenType::RIGHT_BRACE, "}");
    case ';':
        return makeToken(TokenType::SEMICOLON, ";");
    case ',':
        return makeToken(TokenType::COMMA, ",");
    case ':':
        return makeToken(TokenType::COLON, ":");
    case '.': {
        if (isDigit(peek())) {
            return makeDecimal();
        }
        if (peek() == '.' && peekNext() == '.') {
            advance();
            advance();
            return makeToken(TokenType::ELLIPSIS, "...");
        }
        return makeToken(TokenType::DOT, ".");
    }
    case '-': {
        if (match('-')) {
            return makeToken(TokenType::MINUS_MINUS, "--");
        }
        if (match('=')) {
            return makeToken(TokenType::MINUS_EQUAL, "-=");
        }
        // == != < <= > >= = { ( [ += -= *= /= %=
        if (last() == '=' || last() == '>' || last() == '<' || last() == '(' || last() == '['
            || last() == '{') {
            if (isDigit(peek())) {
                return makeNumber();
            }
            if (match('.')) {
                return makeDecimal();
            }
        }
        return makeToken(TokenType::MINUS, "-");
    }
    case '+': {
        if (match('+')) {
            return makeToken(TokenType::PLUS_PLUS, "++");
        }
        if (match('=')) {
            return makeToken(TokenType::PLUS_EQUAL, "+=");
        }
        return makeToken(TokenType::PLUS, "+");
    }
    case '/': {
        if (match('=')) {
            return makeToken(TokenType::SLASH_EQUAL, "/=");
        }
        return makeToken(TokenType::SLASH, "/");
    }
    case '*': {
        if (match('=')) {
            return makeToken(TokenType::STAR_EQUAL, "*=");
        }
        return makeToken(TokenType::STAR, "*");
    }
    case '%': {
        if (match('=')) {
            return makeToken(TokenType::PERCENT_EQUAL, "%=");
        }
        return makeToken(TokenType::PERCENT, "%");
    }
    case '!': {
        if (match('=')) {
            return makeToken(TokenType::NOT_EQUAL, "!=");
        }
        return makeToken(TokenType::NOT, "!");
    }
    case '=': {
        if (match('=')) {
            return makeToken(TokenType::EQUAL_EQUAL, "==");
        }
        return makeToken(TokenType::EQUAL, "=");
    }
    case '<': {
        if (match('=')) {
            return makeToken(TokenType::LESS_EQUAL, "<=");
        }
        return makeToken(TokenType::LESS, "<");
    }
    case '>': {
        if (match('=')) {
            return makeToken(TokenType::GREATER_EQUAL, ">=");
        }
        return makeToken(TokenType::GREATER, ">");
    }
    case '|':
        if (match('|')) {
            return makeToken(TokenType::OR, "||");
        }
        break;
    case '&':
        if (match('&')) {
            return makeToken(TokenType::AND, "&&");
        }
        break;
    case '"':
        return makeString('"');
    case '\'':
        return makeString('\'');
    default:
        break;
    }
    String msg = lexerError(String{c}, "UnexpectedToken");
    throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_TOKEN, msg);
}

Token Lexer::makeNumber()
{
    while (isDigit(peek())) {
        advance();
    }

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek()))
            advance();
    }
    return makeToken(TokenType::NUMBER, source.substr(start, current - start));
}

Token Lexer::makeDecimal()
{
    while (isDigit(peek())) {
        advance();
    }
    return makeToken(TokenType::NUMBER, source.substr(start, current - start));
}

Token Lexer::makeString(char front)
{
    std::string value;

    while (!isAtEnd() && peek() != front) {
        if (peek() == '\n') {
            String str = source.substr(start, current - start - 1);
            String msg = lexerError(str, "UnterminatedString");
            throw ariaCompilingException(ErrorCode::SYNTAX_UNTERMINATED_STRING, msg);
        }

        // Handle escape character
        if (peek() == '\\') {
            advance(); // Skip the backslash
            switch (char escaped = peek()) {
            case 'n':
                value += '\n'; // Newline escape
                break;
            case 't':
                value += '\t'; // Tab escape
                break;
            case '\\':
                value += '\\'; // Backslash escape
                break;
            case '\"':
                value += '\"'; // Double quote escape
                break;
            default:
                // If we encounter an unsupported escape sequence, just add the backslash and next char
                value += '\\';
                value += escaped;
                break;
            }
        } else {
            value += peek(); // Regular character
        }

        advance();
    }

    if (isAtEnd()) {
        String str = source.substr(start, current - start);
        String msg = lexerError(str, "UnterminatedString");
        throw ariaCompilingException(ErrorCode::SYNTAX_UNTERMINATED_STRING, msg);
    }

    // The closing quote
    advance();
    return makeToken(TokenType::STRING, value);
}

Token Lexer::makeIdentifier()
{
    while (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }
    String text = source.substr(start, current - start);
    return makeToken(Token::str2Type(text), text);
}

void Lexer::skipWhitespace()
{
    for (;;) {
        switch (peek()) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            advance();
            if (line == UINT32_MAX) {
                fatalError(ErrorCode::RESOURCE_LINE_OVERFLOW, "Source file line count overflow");
            }
            line++;
            column = 1;
            break;
        case '#':
            advance();
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
            break;
        default:
            return;
        }
    }
}

} // namespace aria
