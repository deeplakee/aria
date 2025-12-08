#ifndef ARIA_AST_H
#define ARIA_AST_H

#include "compile/astVisitor.h"
#include "compile/token.h"
#include "error/error.h"
#include "util/util.h"
#include <utility>

namespace aria {
class FunctionContext;

inline constexpr const char *ADDED_INDENT = "    ";

struct ASTNode
{
    virtual ~ASTNode() = default;

    virtual void display(String indent) { println("{}BaseNode", indent); }

    virtual void accept(AstVisitor &visitor) = 0;

    void display() { display(""); }

    bool asLvalue = false;
};

struct ProgramNode : ASTNode
{
    List<UniquePtr<ASTNode>> decls;

    explicit ProgramNode(List<UniquePtr<ASTNode>> _decls)
        : decls{std::move(_decls)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct FunDeclNode : ASTNode
{
    Token funNameToken;
    List<Token> params;
    UniquePtr<ASTNode> body;
    bool acceptsVarargs;
    uint32_t endLine;

    FunDeclNode(
        Token _funNameToken,
        List<Token> _params,
        UniquePtr<ASTNode> _body,
        bool _acceptsVarargs,
        uint32_t _endLine)
        : funNameToken{std::move(_funNameToken)}
        , params{std::move(_params)}
        , body{std::move(_body)}
        , acceptsVarargs{_acceptsVarargs}
        , endLine{_endLine}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ClassDeclNode : ASTNode
{
    Token nameToken;
    Token superNameToken;
    List<UniquePtr<ASTNode>> methods;

    ClassDeclNode(Token _nameToken, Token _superNameToken, List<UniquePtr<ASTNode>> _methods)
        : nameToken{std::move(_nameToken)}
        , superNameToken{std::move(_superNameToken)}
        , methods{std::move(_methods)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct VarDeclNode : ASTNode
{
    List<Token> names;
    List<UniquePtr<ASTNode>> exprs;

    VarDeclNode(List<Token> _names, List<UniquePtr<ASTNode>> _exprs)
        : names{std::move(_names)}
        , exprs{std::move(_exprs)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct BlockNode : ASTNode
{
    List<UniquePtr<ASTNode>> decls;
    uint32_t endLine;
    BlockNode(List<UniquePtr<ASTNode>> _decls, uint32_t _endLine)
        : decls{std::move(_decls)}
        , endLine{_endLine}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct IfStmtNode : ASTNode
{
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> body;
    UniquePtr<ASTNode> elseBody;

    IfStmtNode(UniquePtr<ASTNode> _condition, UniquePtr<ASTNode> _body, UniquePtr<ASTNode> _elseBody)
        : condition{std::move(_condition)}
        , body{std::move(_body)}
        , elseBody{std::move(_elseBody)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct WhileStmtNode : ASTNode
{
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> body;

    WhileStmtNode(UniquePtr<ASTNode> _condition, UniquePtr<ASTNode> _body)
        : condition{std::move(_condition)}
        , body{std::move(_body)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ForStmtNode : ASTNode
{
    UniquePtr<ASTNode> varInit;
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> increment;
    UniquePtr<ASTNode> body;

    ForStmtNode(
        UniquePtr<ASTNode> _varInit,
        UniquePtr<ASTNode> _condition,
        UniquePtr<ASTNode> _increment,
        UniquePtr<ASTNode> _body)
        : varInit{std::move(_varInit)}
        , condition{std::move(_condition)}
        , increment{std::move(_increment)}
        , body{std::move(_body)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ForInStmtNode : ASTNode
{
    UniquePtr<ASTNode> expr;
    UniquePtr<ASTNode> body;
    Token iterNameToken;

    ForInStmtNode(Token _iterNameToken, UniquePtr<ASTNode> _expr, UniquePtr<ASTNode> _body)
        : expr{std::move(_expr)}
        , body{std::move(_body)}
        , iterNameToken{std::move(_iterNameToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct BreakStmtNode : ASTNode
{
    Token breakToken;
    explicit BreakStmtNode(Token _breakToken)
        : breakToken{std::move(_breakToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ContinueStmtNode : ASTNode
{
    Token continueToken;
    explicit ContinueStmtNode(Token _continueToken)
        : continueToken{std::move(_continueToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ReturnStmtNode : ASTNode
{
    Token returnToken;
    UniquePtr<ASTNode> expr;

    explicit ReturnStmtNode(Token _returnToken, UniquePtr<ASTNode> _expr)
        : returnToken{std::move(_returnToken)}
        , expr{std::move(_expr)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ImportStmtNode : ASTNode
{
    Token importToken;
    Token moduleToken;
    Token nameToken;

    ImportStmtNode(Token _importToken, Token _moduleToken, Token _nameToken)
        : importToken{std::move(_importToken)}
        , moduleToken{std::move(_moduleToken)}
        , nameToken{std::move(_nameToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct TryCatchStmtNode : ASTNode
{
    UniquePtr<ASTNode> tryBody;
    UniquePtr<ASTNode> catchBody;
    Token tryToken;
    Token catchToken;
    Token errToken;

    TryCatchStmtNode(
        UniquePtr<ASTNode> _tryBody,
        UniquePtr<ASTNode> _catchBody,
        Token _tryToken,
        Token _catchToken,
        Token _errToken)
        : tryBody{std::move(_tryBody)}
        , catchBody{std::move(_catchBody)}
        , tryToken{std::move(_tryToken)}
        , catchToken{std::move(_catchToken)}
        , errToken{std::move(_errToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ThrowStmtNode : ASTNode
{
    Token throwToken;
    UniquePtr<ASTNode> e;

    explicit ThrowStmtNode(Token _throwToken, UniquePtr<ASTNode> _e)
        : throwToken{std::move(_throwToken)}
        , e{std::move(_e)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct PrintStmtNode : ASTNode
{
    UniquePtr<ASTNode> expr;

    explicit PrintStmtNode(UniquePtr<ASTNode> _expr)
        : expr{std::move(_expr)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ExprStmtNode : ASTNode
{
    UniquePtr<ASTNode> expr;

    explicit ExprStmtNode(UniquePtr<ASTNode> _expr)
        : expr(std::move(_expr))
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct AssignExprNode : ASTNode
{
    UniquePtr<ASTNode> lhs;
    UniquePtr<ASTNode> rhs;
    Token opToken;

    AssignExprNode(UniquePtr<ASTNode> _lhs, Token _opToken, UniquePtr<ASTNode> _rhs)
        : lhs{std::move(_lhs)}
        , rhs{std::move(_rhs)}
        , opToken{std::move(_opToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct IncExprNode : ASTNode
{
    UniquePtr<ASTNode> operand;
    Token opToken;

    IncExprNode(UniquePtr<ASTNode> _operand, Token _opToken)
        : operand{std::move(_operand)}
        , opToken{std::move(_opToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct BinaryExprNode : ASTNode
{
    UniquePtr<ASTNode> lhs;
    UniquePtr<ASTNode> rhs;
    Token opToken;

    BinaryExprNode(UniquePtr<ASTNode> _lhs, Token _opToken, UniquePtr<ASTNode> _rhs)
        : lhs(std::move(_lhs))
        , rhs(std::move(_rhs))
        , opToken(std::move(_opToken))
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct UnaryExprNode : ASTNode
{
    UniquePtr<ASTNode> operand;
    Token opToken;

    UnaryExprNode(UniquePtr<ASTNode> _operand, Token _opToken)
        : operand{std::move(_operand)}
        , opToken{std::move(_opToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct CallExprNode : ASTNode
{
    UniquePtr<ASTNode> callee;
    List<UniquePtr<ASTNode>> args;

    CallExprNode(UniquePtr<ASTNode> _callee, List<UniquePtr<ASTNode>> _args)
        : callee{std::move(_callee)}
        , args{std::move(_args)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct FieldExprNode : ASTNode
{
    UniquePtr<ASTNode> receiver;
    Token fieldNameToken;

    FieldExprNode(UniquePtr<ASTNode> _receiver, Token _fieldNameToken)
        : receiver{std::move(_receiver)}
        , fieldNameToken{std::move(_fieldNameToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct IndexExprNode : ASTNode
{
    UniquePtr<ASTNode> receiver;
    UniquePtr<ASTNode> index;

    IndexExprNode(UniquePtr<ASTNode> _receiver, UniquePtr<ASTNode> _index)
        : receiver{std::move(_receiver)}
        , index{std::move(_index)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ListExprNode : ASTNode
{
    List<UniquePtr<ASTNode>> list;

    explicit ListExprNode(List<UniquePtr<ASTNode>> _list)
        : list{std::move(_list)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct MapExprNode : ASTNode
{
    List<UniquePtr<ASTNode>> pairs;

    explicit MapExprNode(List<UniquePtr<ASTNode>> _pairs)
        : pairs{std::move(_pairs)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct PairNode : ASTNode
{
    UniquePtr<ASTNode> key;
    UniquePtr<ASTNode> value;

    PairNode(UniquePtr<ASTNode> _key, UniquePtr<ASTNode> _value)
        : key{std::move(_key)}
        , value{std::move(_value)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

enum class VarTag : uint8_t { VAR, THIS, SUPER };

struct VarNode : ASTNode
{
    Token varNameToken;
    VarTag tag;

    explicit VarNode(Token _varNameToken)
        : varNameToken{std::move(_varNameToken)}
        , tag{VarTag::VAR}
    {}

    VarNode(Token _varNameToken, VarTag _tag)
        : varNameToken{std::move(_varNameToken)}
        , tag{_tag}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct NumberNode : ASTNode
{
    Token numToken;

    explicit NumberNode(Token _numToken)
        : numToken{std::move(_numToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct StringNode : ASTNode
{
    Token strToken;

    explicit StringNode(Token _strToken)
        : strToken{std::move(_strToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct TrueNode : ASTNode
{
    Token trueToken;

    explicit TrueNode(Token _trueToken)
        : trueToken{std::move(_trueToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct FalseNode : ASTNode
{
    Token falseToken;

    explicit FalseNode(Token _falseToken)
        : falseToken{std::move(_falseToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct NilNode : ASTNode
{
    Token nilToken;

    explicit NilNode(Token _nilToken)
        : nilToken{std::move(_nilToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ErrorNode : ASTNode
{
    Token errToken;

    explicit ErrorNode(Token _errToken)
        : errToken{std::move(_errToken)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

} // namespace aria

#endif //ARIA_AST_H
