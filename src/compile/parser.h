#ifndef PARSER_H
#define PARSER_H

#include "compile/ast.h"
#include "compile/token.h"

namespace aria {

class Parser
{
public:
    Parser() = delete;

    explicit Parser(List<Token> _tokens)
        : current{0}
        , tokens{std::move(_tokens)}
        , err_flag{false}
        , panic{false}
    {}

    ~Parser() = default;

    UniquePtr<ASTNode> parse();

    bool hasError() const { return err_flag; }

private:
    uint64_t current;
    List<Token> tokens;
    bool err_flag;
    bool panic;

    void checkEOF();

    Token advance();

    Token peek(int n = 0);

    Token lastToken() { return tokens[current - 1]; }

    bool atEnd();

    bool match(TokenType t);

    bool check(TokenType t);

    void consume(TokenType t, StringView msg);

    TokenType currentType();

    String parseError(const Token &errorToken, StringView msg)
    {
        err_flag = true;
        panic = true;
        return syntaxError("{}\n{}", msg, errorToken.info());
    }

    void reportParseError(const Token &errorToken, StringView msg)
    {
        error(parseError(errorToken, msg));
    }

    void synchronize();

    UniquePtr<ASTNode> parseProgram();

    UniquePtr<ASTNode> parseDeclaration();

    UniquePtr<ASTNode> parseFunDecl();

    UniquePtr<ASTNode> parseClassDecl();

    UniquePtr<ASTNode> parseVarDecl();

    UniquePtr<ASTNode> parseStatement();

    UniquePtr<ASTNode> parsePrintStmt();

    UniquePtr<ASTNode> parseIfStmt();

    UniquePtr<ASTNode> parseWhileStmt();

    UniquePtr<ASTNode> parseForStmt();

    UniquePtr<ASTNode> parseForInStmt();

    UniquePtr<ASTNode> parseBreakStmt();

    UniquePtr<ASTNode> parseContinueStmt();

    UniquePtr<ASTNode> parseReturnStmt();

    UniquePtr<ASTNode> parseImportStmt();

    UniquePtr<ASTNode> parseTryCatchStmt();

    UniquePtr<ASTNode> parseThrowStmt();

    UniquePtr<ASTNode> parseBlock();

    UniquePtr<ASTNode> parseExprStmt();

public:
    UniquePtr<ASTNode> parseExpression();

private:
    UniquePtr<ASTNode> parseAssignment();

    UniquePtr<ASTNode> parseLogicOr();

    UniquePtr<ASTNode> parseLogicAnd();

    UniquePtr<ASTNode> parseEquality();

    UniquePtr<ASTNode> parseComparison();

    UniquePtr<ASTNode> parseTerm();

    UniquePtr<ASTNode> parseFactor();

    UniquePtr<ASTNode> parseUnary();

    UniquePtr<ASTNode> parseValue();

    UniquePtr<ASTNode> parseArgs(UniquePtr<ASTNode> called);

    UniquePtr<ASTNode> parsePrimary();

    UniquePtr<ASTNode> parseParenExpr();

    UniquePtr<ASTNode> parseListExpr();

    UniquePtr<ASTNode> parseMapExpr();

    UniquePtr<ASTNode> parsePairExpr();
};

} // namespace aria

#endif //PARSER_H
