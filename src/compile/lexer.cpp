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
        tokens.push_back(scan_token());
        if (tokens.back().type == TokenType::CODE_EOF) {
            break;
        }
    }
    return tokens;
}

Token Lexer::scan_token()
{
    skip_whitespace();
    start = current;

    if (is_at_end()) {
        return make_token(TokenType::CODE_EOF, "EOF");
    }

    char c = advance();
    if (is_alpha(c)) {
        return make_identifier();
    }
    if (is_digit(c)) {
        return make_number();
    }

    switch (c) {
    case '(':
        return make_token(TokenType::LEFT_PAREN, "(");
    case ')':
        return make_token(TokenType::RIGHT_PAREN, ")");
    case '[':
        return make_token(TokenType::LEFT_BRACKET, "[");
    case ']':
        return make_token(TokenType::RIGHT_BRACKET, "]");
    case '{':
        return make_token(TokenType::LEFT_BRACE, "{");
    case '}':
        return make_token(TokenType::RIGHT_BRACE, "}");
    case ';':
        return make_token(TokenType::SEMICOLON, ";");
    case ',':
        return make_token(TokenType::COMMA, ",");
    case ':':
        return make_token(TokenType::COLON, ":");
    case '.': {
        if (is_digit(peek())) {
            return make_decimal();
        }
        if (peek() == '.' && peek_next() == '.') {
            advance();
            advance();
            return make_token(TokenType::ELLIPSIS, "...");
        }
        return make_token(TokenType::DOT, ".");
    }
    case '-': {
        if (match('-')) {
            return make_token(TokenType::MINUS_MINUS, "--");
        }
        if (match('=')) {
            return make_token(TokenType::MINUS_EQUAL, "-=");
        }
        // == != < <= > >= = { ( [ += -= *= /= %=
        if (last() == '=' || last() == '>' || last() == '<' || last() == '(' || last() == '['
            || last() == '{') {
            if (is_digit(peek())) {
                return make_number();
            }
            if (match('.')) {
                return make_decimal();
            }
        }
        return make_token(TokenType::MINUS, "-");
    }
    case '+': {
        if (match('+')) {
            return make_token(TokenType::PLUS_PLUS, "++");
        }
        if (match('=')) {
            return make_token(TokenType::PLUS_EQUAL, "+=");
        }
        return make_token(TokenType::PLUS, "+");
    }
    case '/': {
        if (match('=')) {
            return make_token(TokenType::SLASH_EQUAL, "/=");
        }
        return make_token(TokenType::SLASH, "/");
    }
    case '*': {
        if (match('=')) {
            return make_token(TokenType::STAR_EQUAL, "*=");
        }
        return make_token(TokenType::STAR, "*");
    }
    case '%': {
        if (match('=')) {
            return make_token(TokenType::PERCENT_EQUAL, "%=");
        }
        return make_token(TokenType::PERCENT, "%");
    }
    case '!': {
        if (match('=')) {
            return make_token(TokenType::NOT_EQUAL, "!=");
        }
        return make_token(TokenType::NOT, "!");
    }
    case '=': {
        if (match('=')) {
            return make_token(TokenType::EQUAL_EQUAL, "==");
        }
        return make_token(TokenType::EQUAL, "=");
    }
    case '<': {
        if (match('=')) {
            return make_token(TokenType::LESS_EQUAL, "<=");
        }
        return make_token(TokenType::LESS, "<");
    }
    case '>': {
        if (match('=')) {
            return make_token(TokenType::GREATER_EQUAL, ">=");
        }
        return make_token(TokenType::GREATER, ">");
    }
    case '|':
        if (match('|')) {
            return make_token(TokenType::OR, "||");
        }
        break;
    case '&':
        if (match('&')) {
            return make_token(TokenType::AND, "&&");
        }
        break;
    case '"':
        return make_string('"');
    case '\'':
        return make_string('\'');
    default:
        break;
    }
    String msg = lexer_error(String{c}, "UnexpectedToken");
    throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_TOKEN, msg);
}

Token Lexer::make_number()
{
    while (is_digit(peek())) {
        advance();
    }

    if (peek() == '.' && is_digit(peek_next())) {
        advance();
        while (is_digit(peek()))
            advance();
    }
    return make_token(TokenType::NUMBER, source.substr(start, current - start));
}

Token Lexer::make_decimal()
{
    while (is_digit(peek())) {
        advance();
    }
    return make_token(TokenType::NUMBER, source.substr(start, current - start));
}

Token Lexer::make_string(char front)
{
    std::string value;

    while (!is_at_end() && peek() != front) {
        if (peek() == '\n') {
            String str = source.substr(start, current - start - 1);
            String msg = lexer_error(str, "UnterminatedString");
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

    if (is_at_end()) {
        String str = source.substr(start, current - start);
        String msg = lexer_error(str, "UnterminatedString");
        throw ariaCompilingException(ErrorCode::SYNTAX_UNTERMINATED_STRING, msg);
    }

    // The closing quote
    advance();
    return make_token(TokenType::STRING, value);
}

Token Lexer::make_identifier()
{
    while (is_alpha(peek()) || is_digit(peek())) {
        advance();
    }
    String text = source.substr(start, current - start);
    return make_token(Token::str2Type(text), text);
}

void Lexer::skip_whitespace()
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
                fatal_error(ErrorCode::RESOURCE_LINE_OVERFLOW, "Source file line count overflow");
            }
            line++;
            column = 1;
            break;
        case '#':
            advance();
            while (peek() != '\n' && !is_at_end()) {
                advance();
            }
            break;
        default:
            return;
        }
    }
}

} // namespace aria
