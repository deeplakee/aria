#ifndef ARIA_ASTVISITOR_H
#define ARIA_ASTVISITOR_H

namespace aria {

struct StringNode;
struct NumberNode;
struct ErrorNode;
struct NilNode;
struct FalseNode;
struct TrueNode;
struct VarNode;
struct PairNode;
struct MapExprNode;
struct ListExprNode;
struct IndexExprNode;
struct FieldExprNode;
struct CallExprNode;
struct UnaryExprNode;
struct BinaryExprNode;
struct IncExprNode;
struct AssignExprNode;
struct ExprStmtNode;
struct PrintStmtNode;
struct ThrowStmtNode;
struct TryCatchStmtNode;
struct ImportStmtNode;
struct ReturnStmtNode;
struct ContinueStmtNode;
struct BreakStmtNode;
struct ForInStmtNode;
struct ForStmtNode;
struct WhileStmtNode;
struct IfStmtNode;
struct BlockNode;
struct VarDeclNode;
struct ClassDeclNode;
struct FunDeclNode;
struct ProgramNode;
struct ASTNode;

class AstVisitor
{
public:
    virtual void visitProgramNode(ProgramNode *node) = 0;

    virtual void visitFunDeclNode(FunDeclNode *node) = 0;

    virtual void visitClassDeclNode(ClassDeclNode *node) = 0;

    virtual void visitVarDeclNode(VarDeclNode *node) = 0;

    virtual void visitBlockNode(BlockNode *node) = 0;

    virtual void visitIfStmtNode(IfStmtNode *node) = 0;

    virtual void visitWhileStmtNode(WhileStmtNode *node) = 0;

    virtual void visitForStmtNode(ForStmtNode *node) = 0;

    virtual void visitForInStmtNode(ForInStmtNode *node) = 0;

    virtual void visitBreakStmtNode(BreakStmtNode *node) = 0;

    virtual void visitContinueStmtNode(ContinueStmtNode *node) = 0;

    virtual void visitReturnStmtNode(ReturnStmtNode *node) = 0;

    virtual void visitImportStmtNode(ImportStmtNode *node) = 0;

    virtual void visitTryCatchStmtNode(TryCatchStmtNode *node) = 0;

    virtual void visitThrowStmtNode(ThrowStmtNode *node) = 0;

    virtual void visitPrintStmtNode(PrintStmtNode *node) = 0;

    virtual void visitExprStmtNode(ExprStmtNode *node) = 0;

    virtual void visitAssignExprNode(AssignExprNode *node) = 0;

    virtual void visitIncExprNode(IncExprNode *node) = 0;

    virtual void visitBinaryExprNode(BinaryExprNode *node) = 0;

    virtual void visitUnaryExprNode(UnaryExprNode *node) = 0;

    virtual void visitCallExprNode(CallExprNode *node) = 0;

    virtual void visitFieldExprNode(FieldExprNode *node) = 0;

    virtual void visitIndexExprNode(IndexExprNode *node) = 0;

    virtual void visitListExprNode(ListExprNode *node) = 0;

    virtual void visitMapExprNode(MapExprNode *node) = 0;

    virtual void visitPairNode(PairNode *node) = 0;

    virtual void visitNumberNode(NumberNode *node) = 0;

    virtual void visitStringNode(StringNode *node) = 0;

    virtual void visitVarNode(VarNode *node) = 0;

    virtual void visitTrueNode(TrueNode *node) = 0;

    virtual void visitFalseNode(FalseNode *node) = 0;

    virtual void visitNilNode(NilNode *node) = 0;

    virtual void visitErrorNode(ErrorNode *node) = 0;

    virtual ~AstVisitor() = default;
};

} // namespace aria

#endif //ARIA_ASTVISITOR_H
