#ifndef ARIA_LEXER_H
#define ARIA_LEXER_H

#include "compile/token.h"
#include "error/error.h"

namespace aria {

class Lexer
{
public:
    Lexer() = delete;

    Lexer(const String &_file, String _source)
        : file{std::make_shared<char[]>(_file.size() + 1)}
        , source{std::move(_source)}
        , current{0}
        , start{0}
        , line{1}
        , column{1}
        , err_flag{false}
    {
        std::ranges::copy(_file, file.get());
        file.get()[_file.size()] = '\0';
    }

    explicit Lexer(String _source)
        : Lexer{"Repl", std::move(_source)}
    {}

    ~Lexer() = default;

    List<Token> tokenize();

    Token scanToken();

    [[nodiscard]] uint32_t getLine() const { return line; }

    [[nodiscard]] uint32_t getColumn() const { return column; }

    [[nodiscard]] bool hadError() const { return err_flag; }

private:
    SharedPtr<char[]> file;
    String source;
    uint64_t current;
    uint64_t start;
    uint32_t line;
    uint32_t column;
    bool err_flag;

    char advance()
    {
        current++;
        column++;
        return source[current - 1];
    }

    [[nodiscard]] bool isAtEnd() const { return current >= source.length(); }

    [[nodiscard]] char peek() const { return isAtEnd() ? '\0' : source[current]; }

    [[nodiscard]] char last() const { return source[current - 1]; }

    [[nodiscard]] char peekNext() const
    {
        return (current + 1 >= source.length()) ? '\0' : source[current + 1];
    }

    [[nodiscard]] bool match(char expected)
    {
        if (isAtEnd() || source[current] != expected) {
            return false;
        }
        current++;
        column++;
        return true;
    }

    [[nodiscard]] Token makeToken(TokenType type) const
    {
        return Token{type, file, line, static_cast<uint32_t>(column - (current - start))};
    }

    [[nodiscard]] Token makeToken(TokenType type, String msg) const
    {
        return Token{
            type, std::move(msg), file, line, static_cast<uint32_t>(column - (current - start))};
    }

    [[nodiscard]] String fileInfo() { return format("{}:{}:{}", file.get(), line, column); }

    [[nodiscard]] String lexerError(String token, StringView msg)
    {
        return syntaxError("{}\n{} '{}'", msg, fileInfo(), token);
    }

    Token makeNumber();

    Token makeDecimal();

    Token makeString(char front);

    Token makeIdentifier();

    void skipWhitespace();
};

} // namespace aria

#endif //ARIA_LEXER_H
