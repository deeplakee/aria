#ifndef ARIA_AST_H
#define ARIA_AST_H

#include "compile/astVisitor.h"
#include "compile/token.h"
#include "error/error.h"
#include "util/util.h"
#include <utility>

namespace aria {
class FunctionContext;

constexpr const char *add_indent = "    ";

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
    Token token_name;
    List<Token> params;
    UniquePtr<ASTNode> body;
    bool acceptsVarargs;
    uint32_t endLine;

    FunDeclNode(
        Token _token_name,
        List<Token> _params,
        UniquePtr<ASTNode> _body,
        bool _acceptsVarargs,
        uint32_t _endLine)
        : token_name{std::move(_token_name)}
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
    Token token_name;
    Token token_super_name;
    List<UniquePtr<ASTNode>> methods;

    ClassDeclNode(Token _token_name, Token _token_super_name, List<UniquePtr<ASTNode>> _methods)
        : token_name{std::move(_token_name)}
        , token_super_name{std::move(_token_super_name)}
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
    Token token_name;

    ForInStmtNode(Token _token_name, UniquePtr<ASTNode> _expr, UniquePtr<ASTNode> _body)
        : expr{std::move(_expr)}
        , body{std::move(_body)}
        , token_name{std::move(_token_name)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct BreakStmtNode : ASTNode
{
    Token token_break;
    explicit BreakStmtNode(Token _token_break)
        : token_break{std::move(_token_break)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ContinueStmtNode : ASTNode
{
    Token token_continue;
    explicit ContinueStmtNode(Token _token_continue)
        : token_continue{std::move(_token_continue)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ReturnStmtNode : ASTNode
{
    Token token_return;
    UniquePtr<ASTNode> expr;

    explicit ReturnStmtNode(Token _token_return, UniquePtr<ASTNode> _expr)
        : token_return{std::move(_token_return)}
        , expr{std::move(_expr)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ImportStmtNode : ASTNode
{
    Token token_import;
    Token token_module;
    Token token_name;

    ImportStmtNode(Token _token_import, Token _token_module, Token _token_name)
        : token_import{std::move(_token_import)}
        , token_module{std::move(_token_module)}
        , token_name{std::move(_token_name)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct TryCatchStmtNode : ASTNode
{
    UniquePtr<ASTNode> tryBody;
    UniquePtr<ASTNode> catchBody;
    Token token_try;
    Token token_catch;
    Token token_err;

    TryCatchStmtNode(
        UniquePtr<ASTNode> _tryBody,
        UniquePtr<ASTNode> _catchBody,
        Token _token_try,
        Token _token_catch,
        Token _token_err)
        : tryBody{std::move(_tryBody)}
        , catchBody{std::move(_catchBody)}
        , token_try{std::move(_token_try)}
        , token_catch{std::move(_token_catch)}
        , token_err{std::move(_token_err)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ThrowStmtNode : ASTNode
{
    Token token_throw;
    UniquePtr<ASTNode> e;

    explicit ThrowStmtNode(Token _token_throw, UniquePtr<ASTNode> _e)
        : token_throw{std::move(_token_throw)}
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
    Token op;

    AssignExprNode(UniquePtr<ASTNode> _lhs, Token _op, UniquePtr<ASTNode> _rhs)
        : lhs{std::move(_lhs)}
        , rhs{std::move(_rhs)}
        , op{std::move(_op)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct IncExprNode : ASTNode
{
    UniquePtr<ASTNode> expr;
    Token op;

    IncExprNode(UniquePtr<ASTNode> _expr, Token _op)
        : expr{std::move(_expr)}
        , op{std::move(_op)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct BinaryExprNode : ASTNode
{
    UniquePtr<ASTNode> lhs;
    UniquePtr<ASTNode> rhs;
    Token op;

    BinaryExprNode(UniquePtr<ASTNode> _lhs, Token _op, UniquePtr<ASTNode> _rhs)
        : lhs(std::move(_lhs))
        , rhs(std::move(_rhs))
        , op(std::move(_op))
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct UnaryExprNode : ASTNode
{
    UniquePtr<ASTNode> operand;
    Token op;

    UnaryExprNode(UniquePtr<ASTNode> _operand, Token _op)
        : operand{std::move(_operand)}
        , op{std::move(_op)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct CallExprNode : ASTNode
{
    UniquePtr<ASTNode> calledExpr;
    List<UniquePtr<ASTNode>> args;

    CallExprNode(UniquePtr<ASTNode> _calledExpr, List<UniquePtr<ASTNode>> _args)
        : calledExpr{std::move(_calledExpr)}
        , args{std::move(_args)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct FieldExprNode : ASTNode
{
    UniquePtr<ASTNode> receiver;
    Token token_field_name;

    FieldExprNode(UniquePtr<ASTNode> _receiver, Token _token_field_name)
        : receiver{std::move(_receiver)}
        , token_field_name{std::move(_token_field_name)}
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

enum class VarTag : uint8_t { _var, _this, _super };

struct VarNode : ASTNode
{
    Token var;
    VarTag tag;

    explicit VarNode(Token _var)
        : var{std::move(_var)}
        , tag{VarTag::_var}
    {}

    VarNode(Token _var, VarTag _tag)
        : var{std::move(_var)}
        , tag{_tag}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct NumberNode : ASTNode
{
    Token num;

    explicit NumberNode(Token _num)
        : num{std::move(_num)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct StringNode : ASTNode
{
    Token str;

    explicit StringNode(Token _str)
        : str{std::move(_str)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct TrueNode : ASTNode
{
    Token token_true;

    explicit TrueNode(Token _token_true)
        : token_true{std::move(_token_true)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct FalseNode : ASTNode
{
    Token token_false;

    explicit FalseNode(Token _token_false)
        : token_false{std::move(_token_false)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct NilNode : ASTNode
{
    Token token_nil;

    explicit NilNode(Token _token_nil)
        : token_nil{std::move(_token_nil)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

struct ErrorNode : ASTNode
{
    Token err;

    explicit ErrorNode(Token _err)
        : err{std::move(_err)}
    {}

    void accept(AstVisitor &visitor) override;

    void display(String indent) override;
};

} // namespace aria

#endif //ARIA_AST_H
