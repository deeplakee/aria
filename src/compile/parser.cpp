#include "compile/parser.h"
#include "chunk/code.h"
#include "error/ariaException.h"
#include "error/error.h"

namespace aria {

void Parser::checkEOF()
{
    if (atEnd()) {
        String msg = parseError(tokens.back(), "unexpected end of code");
        throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_EOF, msg);
    }
}

Token Parser::advance()
{
    checkEOF();
    current++;
    return tokens[current - 1];
}

Token Parser::peek(int n)
{
    return tokens[current + n];
}

bool Parser::atEnd()
{
    return current + 1 >= tokens.size();
}

bool Parser::match(TokenType t)
{
    if (atEnd()) {
        return false;
    }
    if (tokens[current].type == t) {
        ++current;
        return true;
    }
    return false;
}

bool Parser::check(TokenType t)
{
    if (tokens[current].type == t) {
        return true;
    }
    return false;
}

void Parser::consume(TokenType t, StringView msg)
{
    if (match(t)) {
        return;
    }
    if (atEnd()) {
        String end_msg = syntaxError("reached end of code, and {}\n{}", msg, tokens.back().info());
        throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_EOF, end_msg);
    }
    auto err_msg = parseError(tokens[current], msg);
    throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_TOKEN, err_msg);
}

TokenType Parser::currentType()
{
    return tokens[current].type;
}

UniquePtr<ASTNode> Parser::parse()
{
    return parseProgram();
}

void Parser::synchronize()
{
    panic = false;
    if (tokens[current - 1].type == TokenType::SEMICOLON) {
        return;
    }
    auto t = tokens[current].type;
    while (t != TokenType::CODE_EOF) {
        switch (t) {
        case TokenType::SEMICOLON:
            current++;
            return;
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::BREAK:
        case TokenType::CONTINUE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default:
            current++;
            t = tokens[current].type;
        }
    }
}

// program        → declaration*
UniquePtr<ASTNode> Parser::parseProgram()
{
    List<UniquePtr<ASTNode>> declarations;
    while (!check(TokenType::CODE_EOF)) {
        declarations.emplace_back(parseDeclaration());
    }
    return std::make_unique<ProgramNode>(std::move(declarations));
}

// declaration    → funDecl
//                | classDecl
//                | varDecl
//                | statement
UniquePtr<ASTNode> Parser::parseDeclaration()
{
    UniquePtr<ASTNode> declaration = nullptr;
    try {
        if (match(TokenType::FUN)) {
            declaration = parseFunDecl();
        } else if (match(TokenType::CLASS)) {
            declaration = parseClassDecl();
        } else if (match(TokenType::VAR)) {
            declaration = parseVarDecl();
        } else {
            declaration = parseStatement();
        }
    } catch (const ariaCompilingException &e) {
        error(e.what());
    }
    if (panic) {
        synchronize();
    }
    return declaration;
}

// funDecl        → "fun" function
// function       → identifier "(" ( identifier ( "," identifier )* ( "," varargs )? | varargs )? ")" block
// varargs        → "..." identifier
UniquePtr<ASTNode> Parser::parseFunDecl()
{
    consume(TokenType::IDENTIFIER, "Expect function name");
    Token tk_fun_name = lastToken();
    List<Token> params;
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    bool acceptsVarargs = false;
    int arity = 0;
    while (!check(TokenType::RIGHT_PAREN)) {
        if (arity >= 255) {
            reportParseError(tokens[current], "Can't have more than 255 parameters.");
        }
        if (match(TokenType::ELLIPSIS)) {
            acceptsVarargs = true;
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(tokens[current - 1]);
            break;
        }
        consume(TokenType::IDENTIFIER, "Expect parameter name.");
        arity++;
        params.push_back(tokens[current - 1]);
        if (!match(TokenType::COMMA)) {
            break;
        }
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{{' before function body.");
    UniquePtr<ASTNode> body = parseBlock();
    auto endLine = lastToken().line;

    return std::make_unique<FunDeclNode>(
        tk_fun_name, std::move(params), std::move(body), acceptsVarargs, endLine);
}

// classDecl      → "class" identifier ( ":" identifier )?  "{" function* "}"
UniquePtr<ASTNode> Parser::parseClassDecl()
{
    consume(TokenType::IDENTIFIER, "Expect class name");
    Token tk_name = lastToken();
    Token tk_super_name = tk_name;
    if (match(TokenType::COLON)) {
        consume(TokenType::IDENTIFIER, "Expect super class name");
        tk_super_name = lastToken();
        if (tk_super_name.text == tk_name.text) {
            reportParseError(tk_name, "A class can't inherit from itself.");
        }
    }

    consume(TokenType::LEFT_BRACE, "Expect '{{' before class body.");

    List<UniquePtr<ASTNode>> methods;

    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::CODE_EOF)) {
        methods.emplace_back(parseFunDecl());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");

    return std::make_unique<ClassDeclNode>(
        std::move(tk_name), std::move(tk_super_name), std::move(methods));
}

// varDecl        → "var" identifier ( "=" expression )? ( "," identifier ( "=" expression )? )* ";"
UniquePtr<ASTNode> Parser::parseVarDecl()
{
    List<Token> names;
    List<UniquePtr<ASTNode>> exprs;

    for (;;) {
        consume(TokenType::IDENTIFIER, "Expect a variable name");

        names.push_back(lastToken());

        // read initializer
        auto &tk = tokens[current - 1];
        if (match(TokenType::EQUAL)) {
            exprs.emplace_back(parseExpression());
        } else {
            auto tmp_nil = Token{TokenType::NIL, tk.filename, tk.line, tk.column};
            exprs.emplace_back(std::make_unique<NilNode>(std::move(tmp_nil)));
        }

        // read next variable declare or ';'
        if (match(TokenType::COMMA)) {
            // nothing to do
        } else if (match(TokenType::SEMICOLON)) {
            break;
        } else {
            reportParseError(tokens[current], "Expected ',' or ';' here");
            break;
        }
    }

    return std::make_unique<VarDeclNode>(std::move(names), std::move(exprs));
}

// statement      → printStmt
//                | ifStmt
//                | whileStmt
//                | forStmt
//                | breakStmt
//                | continueStmt
//                | returnStmt
//                | importStmt
//                | tryCatchStmt
//                | throwStmt
//                | block
//                | exprStmt
UniquePtr<ASTNode> Parser::parseStatement()
{
    if (match(TokenType::PRINT)) {
        return parsePrintStmt();
    }
    if (match(TokenType::IF)) {
        return parseIfStmt();
    }
    if (match(TokenType::WHILE)) {
        return parseWhileStmt();
    }
    if (match(TokenType::FOR)) {
        return parseForStmt();
    }
    if (match(TokenType::BREAK)) {
        return parseBreakStmt();
    }
    if (match(TokenType::CONTINUE)) {
        return parseContinueStmt();
    }
    if (match(TokenType::RETURN)) {
        return parseReturnStmt();
    }
    if (match(TokenType::IMPORT)) {
        return parseImportStmt();
    }
    if (match(TokenType::TRY)) {
        return parseTryCatchStmt();
    }
    if (match(TokenType::THROW)) {
        return parseThrowStmt();
    }
    if (match(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }
    return parseExprStmt();
}

// printStmt      → "print" expression ";"
UniquePtr<ASTNode> Parser::parsePrintStmt()
{
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<PrintStmtNode>(std::move(expr));
}

// ifStmt         → "if" "(" expression ")" statement ( "else" statement )?
UniquePtr<ASTNode> Parser::parseIfStmt()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    UniquePtr<ASTNode> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    UniquePtr<ASTNode> body = parseStatement();
    UniquePtr<ASTNode> elseBody = nullptr;
    if (match(TokenType::ELSE)) {
        elseBody = parseStatement();
    }
    return std::make_unique<IfStmtNode>(std::move(condition), std::move(body), std::move(elseBody));
}

// whileStmt      → "while" "(" expression ")" statement
UniquePtr<ASTNode> Parser::parseWhileStmt()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    UniquePtr<ASTNode> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    UniquePtr<ASTNode> body = parseStatement();
    return std::make_unique<WhileStmtNode>(std::move(condition), std::move(body));
}

// forStmt        → "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement
//                | forInStmt
UniquePtr<ASTNode> Parser::parseForStmt()
{
    UniquePtr<ASTNode> varInit = nullptr;
    UniquePtr<ASTNode> condition = nullptr;
    UniquePtr<ASTNode> increment = nullptr;

    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    if (peek(0).type == TokenType::IDENTIFIER && peek(1).type == TokenType::IN) {
        return parseForInStmt();
    }

    if (match(TokenType::SEMICOLON)) {
        // No initializer.
    } else if (match(TokenType::VAR)) {
        varInit = parseVarDecl();
    } else {
        varInit = parseExprStmt();
    }

    if (!match(TokenType::SEMICOLON)) {
        condition = parseExpression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    }

    if (!match(TokenType::RIGHT_PAREN)) {
        increment = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
    }

    UniquePtr<ASTNode> body = parseStatement();
    return std::make_unique<ForStmtNode>(
        std::move(varInit), std::move(condition), std::move(increment), std::move(body));
}

// forInStmt      → "for" "(" identifier "in" expression ")" statement
UniquePtr<ASTNode> Parser::parseForInStmt()
{
    Token iterName = advance(); // eat iterName
    advance();                  // eat "in"
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for expression.");
    UniquePtr<ASTNode> body = parseStatement();
    return std::make_unique<ForInStmtNode>(iterName, std::move(expr), std::move(body));
}

// breakStmt      → "break" ";"
UniquePtr<ASTNode> Parser::parseBreakStmt()
{
    Token token_break = lastToken();
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'.");
    return std::make_unique<BreakStmtNode>(token_break);
}

// continueStmt   → "continue" ";"
UniquePtr<ASTNode> Parser::parseContinueStmt()
{
    Token token_continue = lastToken();
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'.");
    return std::make_unique<ContinueStmtNode>(token_continue);
}

// returnStmt     → "return" expression? ";"
UniquePtr<ASTNode> Parser::parseReturnStmt()
{
    const auto &tk = tokens[current - 1];
    UniquePtr<ASTNode> expr = nullptr;
    if (match(TokenType::SEMICOLON)) {
        auto tmp_nil = Token{TokenType::NIL, tk.filename, tk.line, tk.column};
        expr = std::make_unique<NilNode>(tmp_nil);
        return std::make_unique<ReturnStmtNode>(tk, std::move(expr));
    }
    expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmtNode>(tk, std::move(expr));
}

// importStmt     → "import" string "as" identifier ";"
UniquePtr<ASTNode> Parser::parseImportStmt()
{
    Token token_import = lastToken();
    consume(TokenType::STRING, "Expected a string of module name/path  after 'import'.");
    Token module = lastToken();
    Token name = module;
    consume(TokenType::AS, "Expect 'as' after module name.");
    consume(TokenType::IDENTIFIER, "Expect a new module name.");
    name = lastToken();
    consume(TokenType::SEMICOLON, "Expected ';' after module name.");
    return std::make_unique<ImportStmtNode>(token_import, module, name);
}

// tryCatchStmt   → "try" block "catch" "(" IDENTIFIER ")" block
UniquePtr<ASTNode> Parser::parseTryCatchStmt()
{
    Token token_try = lastToken();
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'try'.");
    UniquePtr<ASTNode> tryBody = parseBlock();
    consume(TokenType::CATCH, "Expect 'catch' after try body.");
    Token token_catch = lastToken();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'catch'.");
    consume(TokenType::IDENTIFIER, "Expect a exception name after '('.");
    Token exceptionName = lastToken();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after exception name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' after ')'.");
    UniquePtr<ASTNode> catchBody = parseBlock();

    return std::make_unique<TryCatchStmtNode>(
        std::move(tryBody), std::move(catchBody), token_try, token_catch, exceptionName);
}

// throwStmt      → "throw" expression ";"
UniquePtr<ASTNode> Parser::parseThrowStmt()
{
    const auto &tk = tokens[current - 1];
    UniquePtr<ASTNode> e = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ThrowStmtNode>(tk, std::move(e));
}

// block          → "{" declaration* "}"
UniquePtr<ASTNode> Parser::parseBlock()
{
    List<UniquePtr<ASTNode>> declarations;
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::CODE_EOF)) {
        declarations.emplace_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    auto endLine = lastToken().line;
    return std::make_unique<BlockNode>(std::move(declarations), endLine);
}

// exprStmt       → expression ";"
UniquePtr<ASTNode> Parser::parseExprStmt()
{
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmtNode>(std::move(expr));
}

UniquePtr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

// assignment     → logic_or ( ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" ) assignment )?
//                | logic_or "++"
//                | logic_or "--"
UniquePtr<ASTNode> Parser::parseAssignment()
{
    auto expr = parseLogicOr();
    for (;;) {
        if (check(TokenType::EQUAL) || check(TokenType::PLUS_EQUAL) || check(TokenType::MINUS_EQUAL)
            || check(TokenType::STAR_EQUAL) || check(TokenType::SLASH_EQUAL)
            || check(TokenType::PERCENT_EQUAL)) {
            Token op = advance();
            expr = std::make_unique<AssignExprNode>(std::move(expr), op, parseAssignment());
        } else if (check(TokenType::PLUS_PLUS) || check(TokenType::MINUS_MINUS)) {
            Token op = advance();
            return std::make_unique<IncExprNode>(std::move(expr), op);
        } else {
            return expr;
        }
    }
}

// logic_or       → logic_and ( ( "or" | "||" ) logic_and )*
UniquePtr<ASTNode> Parser::parseLogicOr()
{
    auto lhs = parseLogicAnd();
    for (;;) {
        if (currentType() == TokenType::OR) {
            auto op = advance();
            auto rhs = parseLogicAnd();
            lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
        } else {
            return lhs;
        }
    }
}

// logic_and      → equality ( ( "and" | "&&" ) equality )*
UniquePtr<ASTNode> Parser::parseLogicAnd()
{
    auto lhs = parseEquality();
    for (;;) {
        if (currentType() == TokenType::AND) {
            auto op = advance();
            auto rhs = parseEquality();
            lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
        } else {
            return lhs;
        }
    }
}

// equality       → comparison ( ( "!=" | "==" ) comparison )*
UniquePtr<ASTNode> Parser::parseEquality()
{
    auto lhs = parseComparison();
    for (;;) {
        TokenType t = currentType();
        if (t != TokenType::NOT_EQUAL && t != TokenType::EQUAL_EQUAL) {
            return lhs;
        }
        auto op = advance();
        auto rhs = parseComparison();
        lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
    }
}

// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )*
UniquePtr<ASTNode> Parser::parseComparison()
{
    auto lhs = parseTerm();
    for (;;) {
        TokenType t = currentType();
        if (t != TokenType::GREATER && t != TokenType::GREATER_EQUAL && t != TokenType::LESS
            && t != TokenType::LESS_EQUAL) {
            return lhs;
        }
        auto op = advance();
        auto rhs = parseTerm();
        lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
    }
}

// term           → factor ( ( "-" | "+" ) factor )*
UniquePtr<ASTNode> Parser::parseTerm()
{
    auto lhs = parseFactor();
    for (;;) {
        TokenType t = currentType();
        if (t != TokenType::MINUS && t != TokenType::PLUS) {
            return lhs;
        }
        auto op = advance();
        auto rhs = parseFactor();
        lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
    }
}

// factor         → unary ( ( "/" | "*" | "%" ) unary )*
UniquePtr<ASTNode> Parser::parseFactor()
{
    auto lhs = parseUnary();
    for (;;) {
        TokenType t = currentType();
        if (t != TokenType::SLASH && t != TokenType::STAR && t != TokenType::PERCENT) {
            return lhs;
        }
        auto op = advance();
        auto rhs = parseUnary();
        lhs = std::make_unique<BinaryExprNode>(std::move(lhs), op, std::move(rhs));
    }
}

// unary          → ( "-" | "!" | "not" ) unary
//                | value
UniquePtr<ASTNode> Parser::parseUnary()
{
    TokenType t = currentType();
    if (t != TokenType::NOT && t != TokenType::MINUS) {
        return parseValue();
    }
    auto op = advance();
    auto expr = parseUnary();
    return std::make_unique<UnaryExprNode>(std::move(expr), op);
}

// value         → primary ( args | "." identifier | "[" expression "]" )*
UniquePtr<ASTNode> Parser::parseValue()
{
    auto lhs = parsePrimary();
    for (;;) {
        if (match(TokenType::LEFT_PAREN)) {
            lhs = parseArgs(std::move(lhs));
        } else if (match(TokenType::DOT)) {
            consume(TokenType::IDENTIFIER, "Expected field name.");
            lhs = std::make_unique<FieldExprNode>(std::move(lhs), lastToken());
        } else if (match(TokenType::LEFT_BRACKET)) {
            auto expr = parseExpression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after index.");
            lhs = std::make_unique<IndexExprNode>(std::move(lhs), std::move(expr));
        } else {
            return lhs;
        }
    }
}

// args         → "(" ( expression ("," expression )* )? ")"
UniquePtr<ASTNode> Parser::parseArgs(UniquePtr<ASTNode> called)
{
    int count = 0;
    List<UniquePtr<ASTNode>> args;
    while (!check(TokenType::RIGHT_PAREN)) {
        if (count >= UINT8_MAX) {
            reportParseError(tokens[current], "Can't have more than 255 arguments.");
        }
        args.push_back(parseExpression());
        count++;
        if (!match(TokenType::COMMA)) {
            break;
        }
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return std::make_unique<CallExprNode>(std::move(called), std::move(args));
}

// primary        → number
//                | string
//                | identifier
//                | "nil"
//                | "true"
//                | "false"
//                | "this"
//                | "super"
//                | parenExpr
//                | listExpr
//                | mapExpr
UniquePtr<ASTNode> Parser::parsePrimary()
{
    switch (Token tk = advance(); tk.type) {
    case TokenType::NUMBER: {
        return std::make_unique<NumberNode>(tk);
    }
    case TokenType::STRING: {
        return std::make_unique<StringNode>(tk);
    }
    case TokenType::IDENTIFIER: {
        return std::make_unique<VarNode>(tk);
    }
    case TokenType::NIL: {
        return std::make_unique<NilNode>(tk);
    }
    case TokenType::TRUE: {
        return std::make_unique<TrueNode>(tk);
    }
    case TokenType::FALSE: {
        return std::make_unique<FalseNode>(tk);
    }
    case TokenType::THIS: {
        return std::make_unique<VarNode>(tk, VarTag::_this);
    }
    case TokenType::SUPER: {
        return std::make_unique<VarNode>(tk, VarTag::_super);
    }
    case TokenType::LEFT_PAREN: {
        return parseParenExpr();
    }
    case TokenType::LEFT_BRACKET: {
        return parseListExpr();
    }
    case TokenType::LEFT_BRACE: {
        return parseMapExpr();
    }
    default: {
        String msg = parseError(tk, "parsing primary failed");
        throw ariaCompilingException(ErrorCode::SYNTAX_UNEXPECTED_TOKEN, msg);
    }
    }
}

// parenExpr    → "(" expression ")"
UniquePtr<ASTNode> Parser::parseParenExpr()
{
    auto expr = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
}

// listExpr     → "[" ( expression ("," expression )* )? "]"
UniquePtr<ASTNode> Parser::parseListExpr()
{
    int count = 0;
    List<UniquePtr<ASTNode>> list;
    while (!check(TokenType::RIGHT_BRACKET)) {
        if (count >= UINT16_MAX / 2) {
            reportParseError(
                tokens[current], "Can't have more than 32767 elements in a list initialization.");
        }
        list.push_back(parseExpression());
        count++;
        if (!match(TokenType::COMMA)) {
            break;
        }
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after elements.");
    return std::make_unique<ListExprNode>(std::move(list));
}

// mapExpr      → "{" ( pair ("," pair)* )? "}"
UniquePtr<ASTNode> Parser::parseMapExpr()
{
    int count = 0;
    List<UniquePtr<ASTNode>> pairs;
    while (!check(TokenType::RIGHT_BRACE)) {
        if (count >= (UINT16_MAX / 2 / 2)) {
            reportParseError(
                tokens[current],
                "Can't have more than 16383 key-value pairs in a map initialization.");
        }
        pairs.push_back(parsePairExpr());
        count++;
        if (!match(TokenType::COMMA)) {
            break;
        }
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}}' after key-value pairs.");
    return std::make_unique<MapExprNode>(std::move(pairs));
}

// pair         →  expression ":" expression
UniquePtr<ASTNode> Parser::parsePairExpr()
{
    auto k = parseExpression();
    consume(TokenType::COLON, "Expect ':' after key.");
    auto v = parseExpression();
    return std::make_unique<PairNode>(std::move(k), std::move(v));
}

} // namespace aria
