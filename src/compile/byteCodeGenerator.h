#ifndef ARIA_BYTECODEGENERATOR_H
#define ARIA_BYTECODEGENERATOR_H

#include "chunk/chunk.h"
#include "common.h"
#include "compile/astVisitor.h"
#include "compile/token.h"
#include "object/funDef.h"

namespace aria {

class FunctionContext;
class GC;
class ObjFunction;

class ByteCodeGenerator : public AstVisitor
{
public:
    ByteCodeGenerator(
        const String &_moduleName, const String &_moduleLocation, ValueHashTable *_globals, GC *_gc);

    ~ByteCodeGenerator() override;

    ObjFunction *generateCode(const UniquePtr<ASTNode> &root);

private:
    FunctionContext *context;

    void genInheritCode(const Token &super_name);

    ObjString *genMethodCode(ASTNode *node);

    void defineAndLoadClass(const Token &className);

    void defineLocalVar(const VarDeclNode *node, int index);

    void defineGlobalVar(const VarDeclNode *node, int index);

    void genLoadFieldNodeCode(FieldExprNode *node);

    void genStoreFieldNodeCode(FieldExprNode *node);

    void genLoadIndexNodeCode(IndexExprNode *node);

    void genStoreIndexNodeCode(IndexExprNode *node);

    void genStoreVarNodeCode(const VarNode *node) const;

    void genLoadVarNodeCode(const VarNode *node) const;

    FunctionContext *createLocalFunctionContext(const FunDeclNode *node, FunctionType type) const;

    void defineFunction(FunDeclNode *node, FunctionContext *innerCtx) const;

    void defineParam(const Token &param) const;

    void emitClosure(Chunk *chunk, ObjFunction *fun, uint32_t line);

    void setupLoopContext();

    void teardownLoopContext();

    void patchLoopControlJumps(uint32_t continueDest);

    void visitProgramNode(ProgramNode *node) override;

    void visitFunDeclNode(FunDeclNode *node) override;

    void visitClassDeclNode(ClassDeclNode *node) override;

    void visitVarDeclNode(VarDeclNode *node) override;

    void visitBlockNode(BlockNode *node) override;

    void visitIfStmtNode(IfStmtNode *node) override;

    void visitWhileStmtNode(WhileStmtNode *node) override;

    void visitForStmtNode(ForStmtNode *node) override;

    void visitForInStmtNode(ForInStmtNode *node) override;

    void visitBreakStmtNode(BreakStmtNode *node) override;

    void visitContinueStmtNode(ContinueStmtNode *node) override;

    void visitReturnStmtNode(ReturnStmtNode *node) override;

    void visitImportStmtNode(ImportStmtNode *node) override;

    void visitTryCatchStmtNode(TryCatchStmtNode *node) override;

    void visitThrowStmtNode(ThrowStmtNode *node) override;

    void visitPrintStmtNode(PrintStmtNode *node) override;

    void visitExprStmtNode(ExprStmtNode *node) override;

    void visitAssignExprNode(AssignExprNode *node) override;

    void visitIncExprNode(IncExprNode *node) override;

    void visitBinaryExprNode(BinaryExprNode *node) override;

    void visitUnaryExprNode(UnaryExprNode *node) override;

    void visitCallExprNode(CallExprNode *node) override;

    void visitFieldExprNode(FieldExprNode *node) override;

    void visitIndexExprNode(IndexExprNode *node) override;

    void visitListExprNode(ListExprNode *node) override;

    void visitMapExprNode(MapExprNode *node) override;

    void visitPairNode(PairNode *node) override;

    void visitVarNode(VarNode *node) override;

    void visitNumberNode(NumberNode *node) override;

    void visitStringNode(StringNode *node) override;

    void visitTrueNode(TrueNode *node) override;

    void visitFalseNode(FalseNode *node) override;

    void visitNilNode(NilNode *node) override;

    void visitErrorNode(ErrorNode *node) override;
};

} // namespace aria

#endif //ARIA_BYTECODEGENERATOR_H
