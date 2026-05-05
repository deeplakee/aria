#include "compile/byteCodeGenerator.h"
#include "compile/ast.h"
#include "compile/functionContext.h"
#include "object/objFunction.h"
#include "object/objString.h"

#include <cmath>
#include <cstring>

namespace aria {

static Map<TokenType, opCode> tokenToBinaryOpCode = {
    {TokenType::PLUS, opCode::ADD},
    {TokenType::MINUS, opCode::SUBTRACT},
    {TokenType::STAR, opCode::MULTIPLY},
    {TokenType::SLASH, opCode::DIVIDE},
    {TokenType::PERCENT, opCode::MOD},
    {TokenType::PLUS_EQUAL, opCode::ADD},
    {TokenType::MINUS_EQUAL, opCode::SUBTRACT},
    {TokenType::STAR_EQUAL, opCode::MULTIPLY},
    {TokenType::SLASH_EQUAL, opCode::DIVIDE},
    {TokenType::PERCENT_EQUAL, opCode::MOD},
    {TokenType::EQUAL_EQUAL, opCode::EQUAL},
    {TokenType::NOT_EQUAL, opCode::NOT_EQUAL},
    {TokenType::GREATER, opCode::GREATER},
    {TokenType::GREATER_EQUAL, opCode::GREATER_EQUAL},
    {TokenType::LESS, opCode::LESS},
    {TokenType::LESS_EQUAL, opCode::LESS_EQUAL},
    {TokenType::PLUS_PLUS, opCode::ADD},
    {TokenType::MINUS_MINUS, opCode::SUBTRACT},
};

static bool isSuperVarNode(const ASTNode *node)
{
    if (auto var = dynamic_cast<const VarNode *>(node)) {
        return var->tag == VarTag::SUPER;
    }
    return false;
}

static void checkAssignFlag(ASTNode *node)
{
    if (node->asLvalue) {
        node->asLvalue = false;
        throw ariaInvalidAssignException(ErrorCode::SEMANTIC_INVALID_ASSIGNMENT, "Invalid assign.");
    }
}

static void checkDefine(const FunctionContext *context, const Token &nameToken)
{
    if (context->isDefined(nameToken.text)) {
        String msg = semanticError(
            "Variable '{}' already declared in this scope.\n{}",
            nameToken.text,
            nameToken.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_DUPLICATE_DECL, msg);
    }
}

static void declareLocalVariable(FunctionContext *context, const Token &nameToken)
{
    if (!context->addLocal(nameToken.text)) {
        fatalError(
            ErrorCode::INTERNAL_BYTECODE_GEN_FAIL,
            "Too many local variables have been declared within the current scope.\n{}",
            nameToken.posInfo());
    }
}

ByteCodeGenerator::ByteCodeGenerator(
    const String &_moduleName, const String &_moduleLocation, ValueHashTable *_globals, GC *_gc)
    : context{new FunctionContext{_moduleName, _moduleLocation, _globals, _gc}}
{}

ByteCodeGenerator::~ByteCodeGenerator()
{
    delete context;
}

ObjFunction *ByteCodeGenerator::generateCode(const UniquePtr<ASTNode> &root)
{
    root->accept(*this);
    return context->currentFunction();
}

void ByteCodeGenerator::visitProgramNode(ProgramNode *node)
{
    for (const auto &decl : node->decls) {
        decl->accept(*this);
    }
    Chunk *chunk = context->chunk;
    chunk->emitFunEndRet(chunk->lineOfLastCode());
}

FunctionContext *ByteCodeGenerator::createLocalFunctionContext(
    const FunDeclNode *node, FunctionType type) const
{
    return new FunctionContext{
        type,
        context,
        node->funNameToken.text,
        static_cast<int>(node->params.size()),
        node->acceptsVarargs};
}

void ByteCodeGenerator::defineFunction(
    ObjFunction *fun, const Token &funNameToken, uint32_t line) const
{
    Chunk *chunk = context->chunk;

    chunk->emitOpValue(opCode::LOAD_CONST, NanBox::fromObj(fun), line);
    if (context->scopeDepth > 0) {
        checkDefine(context, funNameToken);
        declareLocalVariable(context, funNameToken);
        context->finalizeLocal();
    } else {
        chunk->emitOpValue(opCode::DEF_GLOBAL, NanBox::fromObj(fun->name), line);
    }
}

void ByteCodeGenerator::defineParam(const Token &paramNameToken) const
{
    if (context->isDefined(paramNameToken.text)) {
        String msg = semanticError("The parameter has been used before.\n{}", paramNameToken.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_DUPLICATE_DECL, msg);
    }
    declareLocalVariable(context, paramNameToken);
    context->finalizeLocal();
}

void ByteCodeGenerator::emitClosure(Chunk *chunk, ObjFunction *fun, uint32_t line)
{
    fun->initUpvalues();
    if (fun->upvalueCount != 0) {
        chunk->emitOpValue(opCode::CLOSURE, NanBox::fromObj(fun), line);
        for (int i = 0; i < fun->upvalueCount; i++) {
            chunk->emitByte(context->upvalues[i].isLocal ? 1 : 0, line);
            chunk->emitWord(context->upvalues[i].index, line);
        }
    }
}

void ByteCodeGenerator::visitFunDeclNode(FunDeclNode *node)
{
    FunctionContext *innerCtx = createLocalFunctionContext(node, FunctionType::FUNCTION);
    ObjFunction *fun = innerCtx->currentFunction();
    Chunk* outerCtxChunk = context->chunk;
    defineFunction(fun, node->funNameToken, node->endLine);
    context = innerCtx;

    // Call beginScope() then all parameters will be local variables
    context->beginScope();
    for (auto &param : node->params) {
        defineParam(param);
    }

    node->body->accept(*this);
    context->chunk->emitFunEndRet(context->chunk->lineOfLastCode());
    emitClosure(outerCtxChunk, fun, node->endLine);

#ifdef DEBUG_PRINT_COMPILED_CODE
    fun->chunk->disassemble(fun->toString());
#endif

    context = context->enclosing;
    delete innerCtx;
}

void ByteCodeGenerator::genInheritCode(const Token &superClassNameToken)
{
    UniquePtr<ASTNode> loadSuperKlassNode = std::make_unique<VarNode>(superClassNameToken);
    loadSuperKlassNode->accept(*this);
    context->chunk->emitOp(opCode::INHERIT);
    context->currentClass->hasSuperClass = true;
}

ObjString *ByteCodeGenerator::genMethodCode(ASTNode *node)
{
    Chunk *chunk = context->chunk;
    auto *method = dynamic_cast<FunDeclNode *>(node);
    auto name = method->funNameToken;
    auto params = method->params;
    auto endLine = method->endLine;

    auto isInit = (name.text == "init");
    auto methodType = isInit ? FunctionType::INIT_METHOD : FunctionType::METHOD;
    FunctionContext *innerCtx = createLocalFunctionContext(method, methodType);
    context = innerCtx;
    ObjFunction *function = context->fun;
    chunk->emitOpValue(opCode::LOAD_CONST, NanBox::fromObj(function), chunk->lineOfLastCode());

    context->beginScope();

    for (auto &param : params) {
        defineParam(param);
    }

    method->body->accept(*this);

    context->chunk->emitFunEndRet(endLine, isInit);

    emitClosure(chunk, function, endLine);

#ifdef DEBUG_PRINT_COMPILED_CODE
    function->chunk->disassemble(name.text);
#endif

    context = context->enclosing;
    delete innerCtx;
    return function->name;
}

void ByteCodeGenerator::defineAndLoadClass(const Token &className)
{
    Chunk *chunk = context->chunk;
    auto classNameObj = newObjString(className.text, context->gc);
    if (context->scopeDepth > 0) {
        checkDefine(context, className);
        declareLocalVariable(context, className);
        context->finalizeLocal();
        int localSlot = context->findLocalVariable(className.text);
        chunk->emitOpArg16(
            opCode::LOAD_LOCAL, static_cast<uint16_t>(localSlot), chunk->lineOfLastCode());
    } else {
        chunk->emitOpValue(opCode::DEF_GLOBAL, NanBox::fromObj(classNameObj), chunk->lineOfLastCode());
        chunk->emitOpValue(
            opCode::LOAD_GLOBAL, NanBox::fromObj(classNameObj), chunk->lineOfLastCode());
    }
}

void ByteCodeGenerator::visitClassDeclNode(ClassDeclNode *node)
{
    Chunk *chunk = context->chunk;
    auto token_name = node->nameToken;
    auto token_super_name = node->superNameToken;
    ObjString *className = newObjString(token_name.text, context->gc);
    chunk->emitOpValue(opCode::MAKE_CLASS, NanBox::fromObj(className), token_name.line);

    auto thisClass = new ClassContext{context->currentClass, false};
    context->currentClass = thisClass;

    if (token_name.text != token_super_name.text) {
        genInheritCode(token_super_name);
    }

    defineAndLoadClass(token_name);

    for (const auto &method : node->methods) {
        ASTNode *rawMethodNodePtr = method.get();
        ObjString *methodName = genMethodCode(rawMethodNodePtr);
        if (methodName->length == 4 && memcmp(methodName->C_str_ref(), "init", 4) == 0) {
            chunk->emitOp(opCode::MAKE_INIT_METHOD);
        } else {
            chunk->emitOpValue(
                opCode::MAKE_METHOD, NanBox::fromObj(methodName), chunk->lineOfLastCode());
        }
    }

    chunk->emitOp(opCode::POP);

    context->currentClass = context->currentClass->enclosing;
    delete thisClass;
}

void ByteCodeGenerator::defineLocalVar(const VarDeclNode *node, int index)
{
    Token token = node->names[index];

    checkDefine(context, token);

    declareLocalVariable(context, token);
    node->exprs[index]->accept(*this);
    context->finalizeLocal();
}

void ByteCodeGenerator::defineGlobalVar(const VarDeclNode *node, int index)
{
    Token token = node->names[index];
    const String &varName = token.text;
    const auto varLine = token.line;

    node->exprs[index]->accept(*this);

    const Value name_obj = NanBox::fromObj(newObjString(varName, context->gc));
    context->chunk->emitOpValue(opCode::DEF_GLOBAL, name_obj, varLine);
}

void ByteCodeGenerator::visitVarDeclNode(VarDeclNode *node)
{
    if (context->scopeDepth < 0) {
        String msg = format("Illegal scope for variable declaration.\n{}", node->names[0].posInfo());
        throw ariaCompilingException(ErrorCode::INTERNAL_ILLEGAL_SCOPE, msg);
    }

    for (auto i = 0; i < node->names.size(); ++i) {
        if (context->scopeDepth == 0) {
            defineGlobalVar(node, i);
        } else {
            defineLocalVar(node, i);
        }
    }
}
void ByteCodeGenerator::visitBlockNode(BlockNode *node)
{
    context->beginScope();
    for (const auto &decl : node->decls) {
        decl->accept(*this);
    }
    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, node->endLine);
}

void ByteCodeGenerator::visitIfStmtNode(IfStmtNode *node)
{
    Chunk *chunk = context->chunk;
    node->condition->accept(*this);
    auto falseJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lineOfLastCode());
    node->body->accept(*this);
    if (node->elseBody != nullptr) {
        auto endJump = chunk->emitJump(opCode::JUMP_BWD, chunk->lineOfLastCode());
        chunk->patchJump(falseJump);
        node->elseBody->accept(*this);
        chunk->patchJump(endJump);
    } else {
        chunk->patchJump(falseJump);
    }
}

void ByteCodeGenerator::setupLoopContext()
{
    context->loopDepths.emplace(context->scopeDepth);
    context->loopBreaks.emplace();
    context->loopContinues.emplace();
}

void ByteCodeGenerator::teardownLoopContext()
{
    context->loopDepths.pop();
    context->loopBreaks.pop();
    context->loopContinues.pop();
}

void ByteCodeGenerator::patchLoopControlJumps(uint32_t continueDest)
{
    Chunk *chunk = context->chunk;
    // backpatch undefined jumps which are in the break statements and the continue statements
    for (auto &breaks = context->loopBreaks.top(); const auto breakJump : breaks) {
        chunk->patchJump(breakJump);
    }
    for (auto &continues = context->loopContinues.top(); const auto continueJump : continues) {
        chunk->patchJump(continueJump, continueDest);
    }
}

void ByteCodeGenerator::visitWhileStmtNode(WhileStmtNode *node)
{
    Chunk *chunk = context->chunk;
    setupLoopContext();

    auto loopStart = chunk->count;
    node->condition->accept(*this);
    auto exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lineOfLastCode());
    node->body->accept(*this);
    auto loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lineOfLastCode());
    chunk->patchJump(loopEnd, loopStart);
    chunk->patchJump(exitJump);

    patchLoopControlJumps(loopStart);

    teardownLoopContext();
}

void ByteCodeGenerator::visitForStmtNode(ForStmtNode *node)
{
    Chunk *chunk = context->chunk;
    context->beginScope();

    if (node->varInit != nullptr) {
        node->varInit->accept(*this);
    }

    setupLoopContext();

    auto loopStart = chunk->count;

    int64_t exitJump = -1;
    if (node->condition != nullptr) {
        node->condition->accept(*this);
        exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lineOfLastCode());
    }

    node->body->accept(*this);

    auto incrementStart = chunk->count;
    if (node->increment != nullptr) {
        node->increment->accept(*this);
        chunk->emitOp(opCode::POP);
    }

    uint32_t loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lineOfLastCode());
    chunk->patchJump(loopEnd, loopStart);
    if (exitJump != -1) {
        chunk->patchJump(exitJump);
    }

    patchLoopControlJumps(incrementStart);

    teardownLoopContext();

    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lineOfLastCode());
}

void ByteCodeGenerator::visitForInStmtNode(ForInStmtNode *node)
{
    Token tk_loop_var_name = node->iterNameToken;
    Token tk_iter_name = node->iterNameToken;
    tk_iter_name.text = "__" + tk_iter_name.text + "__ITER__";

    Chunk *chunk = context->chunk;
    context->beginScope();

    // init iterator and loopVar
    declareLocalVariable(context, tk_iter_name);
    node->expr->accept(*this);
    chunk->emitOp(opCode::GET_ITER);
    context->finalizeLocal();
    declareLocalVariable(context, tk_loop_var_name);
    chunk->emitOp(opCode::LOAD_NIL);
    context->finalizeLocal();

    setupLoopContext();

    uint32_t loopStart = chunk->count;

    // get iterator.hasNext
    int iterSlot = context->findLocalVariable(tk_iter_name.text);
    chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(iterSlot), chunk->lineOfLastCode());
    chunk->emitOp(opCode::ITER_HAS_NEXT);
    uint32_t exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lineOfLastCode());

    // store loopVar
    int loopVarSlot = context->findLocalVariable(tk_loop_var_name.text);
    chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(iterSlot), chunk->lineOfLastCode());
    chunk->emitOp(opCode::ITER_GET_NEXT);
    chunk->emitOpArg16(
        opCode::STORE_LOCAL, static_cast<uint16_t>(loopVarSlot), chunk->lineOfLastCode());
    chunk->emitOp(opCode::POP);

    node->body->accept(*this);

    uint32_t incrementStart = chunk->count;

    uint32_t loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lineOfLastCode());
    chunk->patchJump(loopEnd, loopStart);

    chunk->patchJump(exitJump);

    patchLoopControlJumps(incrementStart);

    teardownLoopContext();

    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lineOfLastCode());
}

void ByteCodeGenerator::visitBreakStmtNode(BreakStmtNode *node)
{
    Chunk *chunk = context->chunk;
    if (context->loopDepths.empty()) {
        String msg
            = semanticError("Break statement should inside a loop.\n{}", node->breakToken.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_BREAK, msg);
    }
    int popCount = context->popLocalsOnControlFlow();
    chunk->emitPopN(popCount, node->breakToken.line);

    auto breakJump = chunk->emitJump(opCode::JUMP_BWD, node->breakToken.line);
    context->loopBreaks.top().push_back(breakJump);
}

void ByteCodeGenerator::visitContinueStmtNode(ContinueStmtNode *node)
{
    Chunk *chunk = context->chunk;
    if (context->loopDepths.empty()) {
        String msg = semanticError(
            "Continue statement should inside a loop.\n{}", node->continueToken.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_CONTINUE, msg);
    }
    int popCount = context->popLocalsOnControlFlow();
    chunk->emitPopN(popCount, node->continueToken.line);

    // opCode::JUMP_BWD may be modified
    // because in forStmt the continue jumps backward,but in whileStmt the continue jumps forward.
    auto continueJump = chunk->emitJump(opCode::JUMP_BWD, node->continueToken.line);
    context->loopContinues.top().push_back(continueJump);
}

void ByteCodeGenerator::visitReturnStmtNode(ReturnStmtNode *node)
{
    if (context->fun->type == FunctionType::SCRIPT) {
        String msg
            = semanticError("Can't return from top-level code.\n{}", node->returnToken.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_RETURN, msg};
    }
    if (context->fun->type == FunctionType::INIT_METHOD) {
        String msg = semanticError(
            "Can't return a value from an initializer.\n{}", node->returnToken.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_RETURN, msg};
    }
    node->expr->accept(*this);
    context->chunk->emitOp(opCode::RETURN);
}

void ByteCodeGenerator::visitImportStmtNode(ImportStmtNode *node)
{
    Chunk *chunk = context->chunk;
    ObjString *module = newObjString(node->moduleToken.text, context->gc);
    Token tk_name = node->nameToken;
    uint32_t line = chunk->lineOfLastCode();
    chunk->emitOpValue(opCode::IMPORT, NanBox::fromObj(module), node->importToken.line);

    if (context->scopeDepth > 0) {
        checkDefine(context, tk_name);
        declareLocalVariable(context, tk_name);
        context->finalizeLocal();
    } else {
        ObjString *name = newObjString(tk_name.text, context->gc);
        chunk->emitOpValue(opCode::DEF_GLOBAL, NanBox::fromObj(name), line);
    }
}

void ByteCodeGenerator::visitTryCatchStmtNode(TryCatchStmtNode *node)
{
    Chunk *chunk = context->chunk;

    uint32_t begin = chunk->emitJump(opCode::SETUP_EXCEPT, node->tryToken.line);
    node->tryBody->accept(*this);
    chunk->emitOp(opCode::END_EXCEPT);

    uint32_t exitJump = chunk->emitJump(opCode::JUMP_BWD, node->catchToken.line);
    chunk->patchJump(begin);

    context->beginScope();
    declareLocalVariable(context, node->errToken);
    context->finalizeLocal();
    node->catchBody->accept(*this);
    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lineOfLastCode());
    chunk->emitOp(opCode::END_EXCEPT);
    chunk->patchJump(exitJump);
}

void ByteCodeGenerator::visitThrowStmtNode(ThrowStmtNode *node)
{
    node->e->accept(*this);
    context->chunk->emitOp(opCode::THROW);
}

void ByteCodeGenerator::visitPrintStmtNode(PrintStmtNode *node)
{
    node->expr->accept(*this);
    context->chunk->emitOp(opCode::PRINT);
}

void ByteCodeGenerator::visitExprStmtNode(ExprStmtNode *node)
{
    node->expr->accept(*this);
    context->chunk->emitOp(opCode::POP);
}

void ByteCodeGenerator::visitAssignExprNode(AssignExprNode *node)
{
    checkAssignFlag(node);
    auto &op = node->opToken;
    if (op.type != TokenType::EQUAL) {
        node->lhs->accept(*this);
    }
    node->rhs->accept(*this);
    if (op.type != TokenType::EQUAL) {
        context->chunk->emitOp(tokenToBinaryOpCode[op.type], op.line);
    }
    try {
        node->lhs->asLvalue = true;
        node->lhs->accept(*this);
        node->lhs->asLvalue = false;
    } catch ([[maybe_unused]] const ariaInvalidAssignException &e) {
        String msg = semanticError("Invalid assignment target.\n{}", node->opToken.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_ASSIGNMENT, msg);
    }
}

void ByteCodeGenerator::visitIncExprNode(IncExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    auto &expr = node->operand;
    Token op = node->opToken;
    expr->accept(*this);
    chunk->emitOpValue(opCode::LOAD_CONST, NanBox::fromNumber(1), op.line);
    chunk->emitOp(tokenToBinaryOpCode[op.type], op.line);
    try {
        expr->asLvalue = true;
        expr->accept(*this);
        expr->asLvalue = false;
    } catch ([[maybe_unused]] const ariaInvalidAssignException &e) {
        String msg = semanticError("Invalid assignment target.\n{}", node->opToken.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_ASSIGNMENT, msg);
    }
}

void ByteCodeGenerator::visitBinaryExprNode(BinaryExprNode *node)
{
    checkAssignFlag(node);
    auto chunk = context->chunk;
    auto t = node->opToken.type;
    const auto line = node->opToken.line;
    if (t == TokenType::AND || t == TokenType::OR) {
        node->lhs->accept(*this);
        auto jump = (t == TokenType::AND) ? opCode::JUMP_FALSE_NOPOP : opCode::JUMP_TRUE_NOPOP;
        auto endJump = chunk->emitJump(jump, line);
        chunk->emitOp(opCode::POP, line);
        node->rhs->accept(*this);
        chunk->patchJump(endJump);
    } else {
        node->lhs->accept(*this);
        node->rhs->accept(*this);
        context->chunk->emitOp(tokenToBinaryOpCode[t], line);
    }
}

void ByteCodeGenerator::visitUnaryExprNode(UnaryExprNode *node)
{
    checkAssignFlag(node);
    auto chunk = context->chunk;
    const auto t = node->opToken.type;
    const auto line = node->opToken.line;
    node->operand->accept(*this);
    if (t == TokenType::NOT) {
        chunk->emitOp(opCode::NOT, line);
    } else if (t == TokenType::MINUS) {
        chunk->emitOp(opCode::NEGATE, line);
    } else {
        String msg = semanticError("Invalid unary operator.\n{}", node->opToken.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN_OPERATOR, msg);
    }
}

void ByteCodeGenerator::visitCallExprNode(CallExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    node->callee->accept(*this);
    for (const auto &arg : node->args) {
        arg->accept(*this);
    }
    chunk->emitOp(opCode::CALL);
    chunk->emitByte(static_cast<uint8_t>(node->args.size()));
}

void ByteCodeGenerator::genLoadFieldNodeCode(FieldExprNode *node)
{
    Chunk *chunk = context->chunk;
    Token tk_fieldName = node->fieldNameToken;

    node->receiver->accept(*this);

    Value name = NanBox::fromObj(newObjString(tk_fieldName.text, context->gc));

    if (isSuperVarNode(node->receiver.get())) {
        chunk->emitOpValue(opCode::LOAD_SUPER_METHOD, name, tk_fieldName.line);
        return;
    }

    chunk->emitOpValue(opCode::LOAD_FIELD, name, tk_fieldName.line);
}

void ByteCodeGenerator::genStoreFieldNodeCode(FieldExprNode *node)
{
    if (isSuperVarNode(node->receiver.get())) {
        auto superNode = dynamic_cast<VarNode *>(node->receiver.get());
        String msg = semanticError(
            "Can't use 'super' as left value.\n{}", superNode->varNameToken.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
    }
    Chunk *chunk = context->chunk;
    Token tk_fieldName = node->fieldNameToken;

    node->receiver->accept(*this);

    Value name = NanBox::fromObj(newObjString(tk_fieldName.text, context->gc));
    chunk->emitOpValue(opCode::STORE_FIELD, name, tk_fieldName.line);
}

void ByteCodeGenerator::visitFieldExprNode(FieldExprNode *node)
{
    if (node->asLvalue) {
        genStoreFieldNodeCode(node);
    } else {
        genLoadFieldNodeCode(node);
    }
}

void ByteCodeGenerator::genLoadIndexNodeCode(IndexExprNode *node)
{
    node->receiver->accept(*this);
    node->index->accept(*this);
    context->chunk->emitOp(opCode::LOAD_SUBSCR);
}

void ByteCodeGenerator::genStoreIndexNodeCode(IndexExprNode *node)
{
    node->receiver->accept(*this);
    node->index->accept(*this);
    context->chunk->emitOp(opCode::STORE_SUBSCR);
}

void ByteCodeGenerator::visitIndexExprNode(IndexExprNode *node)
{
    if (node->asLvalue) {
        genStoreIndexNodeCode(node);
    } else {
        genLoadIndexNodeCode(node);
    }
}

void ByteCodeGenerator::visitListExprNode(ListExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    for (const auto &val : node->list) {
        val->accept(*this);
    }
    uint16_t listSize = node->list.size();
    chunk->emitOp(opCode::MAKE_LIST);
    chunk->emitWord(listSize);
}

void ByteCodeGenerator::visitMapExprNode(MapExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    for (const auto &pair : node->pairs) {
        pair->accept(*this);
    }
    uint16_t mapSize = node->pairs.size();
    chunk->emitOp(opCode::MAKE_MAP);
    chunk->emitWord(mapSize);
}

void ByteCodeGenerator::visitPairNode(PairNode *node)
{
    checkAssignFlag(node);
    node->key->accept(*this);
    node->value->accept(*this);
}

void ByteCodeGenerator::genStoreVarNodeCode(const VarNode *node) const
{
    if (node->tag == VarTag::SUPER) {
        String msg
            = semanticError("Can't use 'super' as left value.\n{}", node->varNameToken.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
    }
    if (node->tag == VarTag::THIS && context->currentClass == nullptr) {
        String msg
            = semanticError("Can't use 'this' outside of a class.\n{}", node->varNameToken.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_THIS, msg};
    }
    Chunk *chunk = context->chunk;
    String varName = node->varNameToken.text;
    uint32_t line = node->varNameToken.line;
    int localOffset = context->findLocalVariable(varName);
    if (localOffset >= 0) {
        chunk->emitOpArg16(opCode::STORE_LOCAL, static_cast<uint16_t>(localOffset), line);
        return;
    }
    if (localOffset == -1) {
        String msg = format(
            "Can't read local variable in its own initializer.\n{}", node->varNameToken.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_UNDEFINED_VARIABLE, msg};
    }
    if (localOffset == -2) {
        int upvalueSlot = context->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            chunk->emitOpArg16(opCode::STORE_UPVALUE, static_cast<uint16_t>(upvalueSlot), line);
            return;
        }
        if (upvalueSlot == -1) {
            Value name = NanBox::fromObj(newObjString(varName, chunk->gc));
            chunk->emitOpValue(opCode::STORE_GLOBAL, name, line);
            return;
        }
        if (upvalueSlot == -2) {
            fatalError(
                ErrorCode::RESOURCE_VARIABLE_OVERFLOW,
                "Too many closure variables in function.\n{}",
                node->varNameToken.info());
        }
    }
    fatalError(ErrorCode::INTERNAL_UNKNOWN, "Unknown error in StoreVarNode::generateByteCode.");
}

void ByteCodeGenerator::genLoadVarNodeCode(const VarNode *node) const
{
    Chunk *chunk = context->chunk;
    uint32_t line = node->varNameToken.line;
    String varName = node->varNameToken.text;
    if (node->tag == VarTag::SUPER) {
        if (context->currentClass == nullptr) {
            String msg = semanticError(
                "Can't use 'super' outside of a class.\n{}", node->varNameToken.posInfo());
            throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
        }
        if (!context->currentClass->hasSuperClass) {
            String msg = semanticError(
                "Can't use 'super' in a class with no superclass.\n{}",
                node->varNameToken.posInfo());
            throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
        }
        context->chunk->emitOpArg16(opCode::LOAD_LOCAL, 0, line);
        return;
    }

    if (node->tag == VarTag::THIS && context->currentClass == nullptr) {
        String msg
            = semanticError("Can't use 'this' outside of a class.\n{}", node->varNameToken.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_THIS, msg};
    }

    int localOffset = context->findLocalVariable(varName);
    if (localOffset >= 0) {
        chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(localOffset), line);
        return;
    }
    if (localOffset == -1) {
        String msg = format(
            "Can't read local variable in its own initializer.\n{}", node->varNameToken.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_UNDEFINED_VARIABLE, msg};
    }
    if (localOffset == -2) {
        int upvalueSlot = context->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            chunk->emitOpArg16(opCode::LOAD_UPVALUE, static_cast<uint16_t>(upvalueSlot), line);
            return;
        }
        if (upvalueSlot == -1) {
            Value name = NanBox::fromObj(newObjString(varName, context->gc));
            chunk->emitOpValue(opCode::LOAD_GLOBAL, name, line);
            return;
        }
        if (upvalueSlot == -2) {
            fatalError(
                ErrorCode::RESOURCE_VARIABLE_OVERFLOW,
                "Too many closure variables in function.\n{}",
                node->varNameToken.info());
        }
    }
    fatalError(ErrorCode::INTERNAL_UNKNOWN, "Unknown error in LoadVarNode::generateByteCode.");
}

void ByteCodeGenerator::visitVarNode(VarNode *node)
{
    if (node->asLvalue) {
        genStoreVarNodeCode(node);
    } else {
        genLoadVarNodeCode(node);
    }
}

void ByteCodeGenerator::visitNumberNode(NumberNode *node)
{
    checkAssignFlag(node);
    double value = INFINITY;
    auto &num = node->numToken;
    try {
        value = std::stod(num.text);
    } catch ([[maybe_unused]] const std::invalid_argument &e) {
        String msg = semanticError("Invalid number.\n{}", num.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN, msg);
    } catch ([[maybe_unused]] const std::out_of_range &e) {
        String msg = semanticError("Number out of range.\n{}", num.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_LITERAL_OVERFLOW, msg);
    }
    context->chunk->emitOpValue(opCode::LOAD_CONST, NanBox::fromNumber(value), num.line);
}

void ByteCodeGenerator::visitStringNode(StringNode *node)
{
    checkAssignFlag(node);
    auto &str = node->strToken;
    Value strObj = NanBox::fromObj(newObjString(str.text, context->gc));
    context->chunk->emitOpValue(opCode::LOAD_CONST, strObj, str.line);
}

void ByteCodeGenerator::visitTrueNode(TrueNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_TRUE, node->trueToken.line);
}

void ByteCodeGenerator::visitFalseNode(FalseNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_FALSE, node->falseToken.line);
}

void ByteCodeGenerator::visitNilNode(NilNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_NIL, node->nilToken.line);
}

void ByteCodeGenerator::visitErrorNode(ErrorNode *node)
{
    throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN, node->errToken.text);
}

} // namespace aria