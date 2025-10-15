#include "compile/byteCodeGenerator.h"
#include "compile/ast.h"
#include "compile/functionContext.h"
#include "object/objFunction.h"
#include "object/objString.h"

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
        return var->tag == VarTag::_super;
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

static void checkDefine(const FunctionContext *context, const Token &tk_name)
{
    if (context->isDefined(tk_name.text)) {
        String msg = semanticError(
            "Variable '{}' already declared in this scope.\n{}", tk_name.text, tk_name.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_DUPLICATE_DECL, msg);
    }
}

static void declareLocalVariable(FunctionContext *context, const Token &tk_name)
{
    if (!context->addLocal(tk_name.text)) {
        fatalError(
            ErrorCode::INTERNAL_BYTECODE_GEN_FAIL,
            "Too many local variables have been declared within the current scope.\n{}",
            tk_name.posInfo());
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
    chunk->emitFunEndRet(chunk->lastOpLine());
}

FunctionContext *ByteCodeGenerator::createLocalFunctionContext(
    const FunDeclNode *node, FunctionType type) const
{
    return new FunctionContext{
        type,
        context,
        node->token_name.text,
        static_cast<int>(node->params.size()),
        node->acceptsVarargs};
}

void ByteCodeGenerator::defineFunction(FunDeclNode *node, FunctionContext *innerCtx) const
{
    Chunk *chunk = context->chunk;
    auto endLine = node->endLine;
    ObjFunction *fun = innerCtx->fun;

    chunk->emitOpData(opCode::LOAD_CONST, obj_val(fun), endLine);
    if (context->scopeDepth > 0) {
        checkDefine(context, node->token_name);
        declareLocalVariable(context, node->token_name);
        context->finalizeLocal();
    } else {
        chunk->emitOpData(opCode::DEF_GLOBAL, obj_val(fun->name), endLine);
    }
}

void ByteCodeGenerator::defineParam(const Token &param) const
{
    if (context->isDefined(param.text)) {
        String msg = semanticError("The parameter has been used before.\n{}", param.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_DUPLICATE_DECL, msg);
    }
    declareLocalVariable(context, param);
    context->finalizeLocal();
}

void ByteCodeGenerator::emitClosure(Chunk *chunk, ObjFunction *fun, uint32_t line)
{
    fun->initUpvalues(context->gc);
    if (fun->upvalueCount != 0) {
        chunk->emitOpData(opCode::CLOSURE, obj_val(fun), line);
        for (int i = 0; i < fun->upvalueCount; i++) {
            chunk->emitByte(context->upvalues[i].isLocal ? 1 : 0, line);
            chunk->emitWord(context->upvalues[i].index, line);
        }
    }
}

void ByteCodeGenerator::visitFunDeclNode(FunDeclNode *node)
{
    Chunk *chunk = context->chunk;
    auto endLine = node->endLine;
    auto *innerCtx = createLocalFunctionContext(node, FunctionType::FUNCTION);
    auto fun = innerCtx->fun;

    defineFunction(node, innerCtx);

    context = innerCtx;
    context->beginScope();

    for (auto &param : node->params) {
        defineParam(param);
    }

    node->body->accept(*this);
    context->chunk->emitFunEndRet(context->chunk->lastOpLine());
    emitClosure(chunk, fun, endLine);

#ifdef DEBUG_PRINT_COMPILED_CODE
    fun->chunk->disassemble(fun->toString());
#endif

    context = context->enclosing;
    delete innerCtx;
}

void ByteCodeGenerator::genInheritCode(const Token &super_name)
{
    UniquePtr<ASTNode> loadSuperKlassNode = std::make_unique<VarNode>(super_name);
    loadSuperKlassNode->accept(*this);
    context->chunk->emitOp(opCode::INHERIT);
    context->currentClass->hasSuperClass = true;
}

ObjString *ByteCodeGenerator::genMethodCode(ASTNode *node)
{
    Chunk *chunk = context->chunk;
    auto *method = dynamic_cast<FunDeclNode *>(node);
    auto name = method->token_name;
    auto params = method->params;
    auto endLine = method->endLine;

    auto isInit = (name.text == "init");
    auto methodType = isInit ? FunctionType::INIT_METHOD : FunctionType::METHOD;
    FunctionContext *innerCtx = createLocalFunctionContext(method, methodType);
    context = innerCtx;
    ObjFunction *function = context->fun;
    chunk->emitOpData(opCode::LOAD_CONST, obj_val(function), chunk->lastOpLine());

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
        chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(localSlot), chunk->lastOpLine());
    } else {
        chunk->emitOpData(opCode::DEF_GLOBAL, obj_val(classNameObj), chunk->lastOpLine());
        chunk->emitOpData(opCode::LOAD_GLOBAL, obj_val(classNameObj), chunk->lastOpLine());
    }
}

void ByteCodeGenerator::visitClassDeclNode(ClassDeclNode *node)
{
    Chunk *chunk = context->chunk;
    auto token_name = node->token_name;
    auto token_super_name = node->token_super_name;
    ObjString *className = newObjString(token_name.text, context->gc);
    chunk->emitOpData(opCode::MAKE_CLASS, obj_val(className), token_name.line);

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
            chunk->emitOpData(opCode::MAKE_METHOD, obj_val(methodName), chunk->lastOpLine());
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

    const Value name_obj = obj_val(newObjString(varName, context->gc));
    context->chunk->emitOpData(opCode::DEF_GLOBAL, name_obj, varLine);
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
    auto falseJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lastOpLine());
    node->body->accept(*this);
    if (node->elseBody != nullptr) {
        auto endJump = chunk->emitJump(opCode::JUMP_BWD, chunk->lastOpLine());
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
    auto exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lastOpLine());
    node->body->accept(*this);
    auto loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lastOpLine());
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
        exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lastOpLine());
    }

    node->body->accept(*this);

    auto incrementStart = chunk->count;
    if (node->increment != nullptr) {
        node->increment->accept(*this);
        chunk->emitOp(opCode::POP);
    }

    uint32_t loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lastOpLine());
    chunk->patchJump(loopEnd, loopStart);
    if (exitJump != -1) {
        chunk->patchJump(exitJump);
    }

    patchLoopControlJumps(incrementStart);

    teardownLoopContext();

    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lastOpLine());
}

void ByteCodeGenerator::visitForInStmtNode(ForInStmtNode *node)
{
    Token tk_loop_var_name = node->token_name;
    Token tk_iter_name = node->token_name;
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
    chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(iterSlot), chunk->lastOpLine());
    chunk->emitOp(opCode::ITER_HAS_NEXT);
    uint32_t exitJump = chunk->emitJump(opCode::JUMP_FALSE, chunk->lastOpLine());

    // store loopVar
    int loopVarSlot = context->findLocalVariable(tk_loop_var_name.text);
    chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(iterSlot), chunk->lastOpLine());
    chunk->emitOp(opCode::ITER_GET_NEXT);
    chunk->emitOpArg16(opCode::STORE_LOCAL, static_cast<uint16_t>(loopVarSlot), chunk->lastOpLine());
    chunk->emitOp(opCode::POP);

    node->body->accept(*this);

    uint32_t incrementStart = chunk->count;

    uint32_t loopEnd = chunk->emitJump(opCode::JUMP_FWD, chunk->lastOpLine());
    chunk->patchJump(loopEnd, loopStart);

    chunk->patchJump(exitJump);

    patchLoopControlJumps(incrementStart);

    teardownLoopContext();

    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lastOpLine());
}

void ByteCodeGenerator::visitBreakStmtNode(BreakStmtNode *node)
{
    Chunk *chunk = context->chunk;
    if (context->loopDepths.empty()) {
        String msg
            = semanticError("Break statement should inside a loop.\n{}", node->token_break.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_BREAK, msg);
    }
    int popCount = context->popLocalsOnControlFlow();
    chunk->emitPopN(popCount, node->token_break.line);

    auto breakJump = chunk->emitJump(opCode::JUMP_BWD, node->token_break.line);
    context->loopBreaks.top().push_back(breakJump);
}

void ByteCodeGenerator::visitContinueStmtNode(ContinueStmtNode *node)
{
    Chunk *chunk = context->chunk;
    if (context->loopDepths.empty()) {
        String msg = semanticError(
            "Continue statement should inside a loop.\n{}", node->token_continue.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_CONTINUE, msg);
    }
    int popCount = context->popLocalsOnControlFlow();
    chunk->emitPopN(popCount, node->token_continue.line);

    // opCode::JUMP_BWD may be modified
    // because in forStmt the continue jumps backward,but in whileStmt the continue jumps forward.
    auto continueJump = chunk->emitJump(opCode::JUMP_BWD, node->token_continue.line);
    context->loopContinues.top().push_back(continueJump);
}

void ByteCodeGenerator::visitReturnStmtNode(ReturnStmtNode *node)
{
    if (context->fun->type == FunctionType::SCRIPT) {
        String msg
            = semanticError("Can't return from top-level code.\n{}", node->token_return.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_RETURN, msg};
    }
    if (context->fun->type == FunctionType::INIT_METHOD) {
        String msg = semanticError(
            "Can't return a value from an initializer.\n{}", node->token_return.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_RETURN, msg};
    }
    node->expr->accept(*this);
    context->chunk->emitOp(opCode::RETURN);
}

void ByteCodeGenerator::visitImportStmtNode(ImportStmtNode *node)
{
    Chunk *chunk = context->chunk;
    ObjString *module = newObjString(node->token_module.text, context->gc);
    Token tk_name = node->token_name;
    uint32_t line = chunk->lastOpLine();
    chunk->emitOpData(opCode::IMPORT, obj_val(module), node->token_import.line);

    if (context->scopeDepth > 0) {
        checkDefine(context, tk_name);
        declareLocalVariable(context, tk_name);
        context->finalizeLocal();
    } else {
        ObjString *name = newObjString(tk_name.text, context->gc);
        chunk->emitOpData(opCode::DEF_GLOBAL, obj_val(name), line);
    }
}

void ByteCodeGenerator::visitTryCatchStmtNode(TryCatchStmtNode *node)
{
    Chunk *chunk = context->chunk;

    uint32_t begin = chunk->emitJump(opCode::SETUP_EXCEPT, node->token_try.line);
    node->tryBody->accept(*this);
    chunk->emitOp(opCode::END_EXCEPT);

    uint32_t exitJump = chunk->emitJump(opCode::JUMP_BWD, node->token_catch.line);
    chunk->patchJump(begin);

    context->beginScope();
    declareLocalVariable(context, node->token_err);
    context->finalizeLocal();
    node->catchBody->accept(*this);
    auto ops = context->endScope();
    context->chunk->emitScopeCleanup(ops, chunk->lastOpLine());
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
    auto &op = node->op;
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
        String msg = semanticError("Invalid assignment target.\n{}", node->op.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_ASSIGNMENT, msg);
    }
}

void ByteCodeGenerator::visitIncExprNode(IncExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    auto &expr = node->expr;
    Token op = node->op;
    expr->accept(*this);
    chunk->emitOpData(opCode::LOAD_CONST, number_val(1), op.line);
    chunk->emitOp(tokenToBinaryOpCode[op.type], op.line);
    try {
        expr->asLvalue = true;
        expr->accept(*this);
        expr->asLvalue = false;
    } catch ([[maybe_unused]] const ariaInvalidAssignException &e) {
        String msg = semanticError("Invalid assignment target.\n{}", node->op.posInfo());
        throw ariaCompilingException(ErrorCode::SEMANTIC_INVALID_ASSIGNMENT, msg);
    }
}

void ByteCodeGenerator::visitBinaryExprNode(BinaryExprNode *node)
{
    checkAssignFlag(node);
    auto chunk = context->chunk;
    auto t = node->op.type;
    const auto line = node->op.line;
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
    const auto t = node->op.type;
    const auto line = node->op.line;
    node->operand->accept(*this);
    if (t == TokenType::NOT) {
        chunk->emitOp(opCode::NOT, line);
    } else if (t == TokenType::MINUS) {
        chunk->emitOp(opCode::NEGATE, line);
    } else {
        String msg = semanticError("Invalid unary operator.\n{}", node->op.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN_OPERATOR, msg);
    }
}

void ByteCodeGenerator::visitCallExprNode(CallExprNode *node)
{
    checkAssignFlag(node);
    Chunk *chunk = context->chunk;
    node->calledExpr->accept(*this);
    for (const auto &arg : node->args) {
        arg->accept(*this);
    }
    chunk->emitOp(opCode::CALL);
    chunk->emitByte(static_cast<uint8_t>(node->args.size()));
}

void ByteCodeGenerator::genLoadFieldNodeCode(FieldExprNode *node)
{
    Chunk *chunk = context->chunk;
    Token tk_fieldName = node->token_field_name;

    node->receiver->accept(*this);

    Value name = obj_val(newObjString(tk_fieldName.text, context->gc));

    if (isSuperVarNode(node->receiver.get())) {
        chunk->emitOpData(opCode::LOAD_SUPER_METHOD, name, tk_fieldName.line);
        return;
    }

    chunk->emitOpData(opCode::LOAD_FIELD, name, tk_fieldName.line);
}

void ByteCodeGenerator::genStoreFieldNodeCode(FieldExprNode *node)
{
    if (isSuperVarNode(node->receiver.get())) {
        auto superNode = dynamic_cast<VarNode *>(node->receiver.get());
        String msg = semanticError("Can't use 'super' as left value.\n{}", superNode->var.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
    }
    Chunk *chunk = context->chunk;
    Token tk_fieldName = node->token_field_name;

    node->receiver->accept(*this);

    Value name = obj_val(newObjString(tk_fieldName.text, context->gc));
    chunk->emitOpData(opCode::STORE_FIELD, name, tk_fieldName.line);
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
    if (node->tag == VarTag::_super) {
        String msg = semanticError("Can't use 'super' as left value.\n{}", node->var.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
    }
    if (node->tag == VarTag::_this && context->currentClass == nullptr) {
        String msg = semanticError("Can't use 'this' outside of a class.\n{}", node->var.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_THIS, msg};
    }
    Chunk *chunk = context->chunk;
    String varName = node->var.text;
    uint32_t line = node->var.line;
    int localOffset = context->findLocalVariable(varName);
    if (localOffset >= 0) {
        chunk->emitOpArg16(opCode::STORE_LOCAL, static_cast<uint16_t>(localOffset), line);
        return;
    }
    if (localOffset == -1) {
        String msg
            = format("Can't read local variable in its own initializer.\n{}", node->var.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_UNDEFINED_VARIABLE, msg};
    }
    if (localOffset == -2) {
        int upvalueSlot = context->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            chunk->emitOpArg16(opCode::STORE_UPVALUE, static_cast<uint16_t>(upvalueSlot), line);
            return;
        }
        if (upvalueSlot == -1) {
            Value name = obj_val(newObjString(varName, chunk->gc));
            chunk->emitOpData(opCode::STORE_GLOBAL, name, line);
            return;
        }
        if (upvalueSlot == -2) {
            fatalError(
                ErrorCode::RESOURCE_VARIABLE_OVERFLOW,
                "Too many closure variables in function.\n{}",
                node->var.info());
        }
    }
    fatalError(ErrorCode::INTERNAL_UNKNOWN, "Unknown error in StoreVarNode::generateByteCode.");
}

void ByteCodeGenerator::genLoadVarNodeCode(const VarNode *node) const
{
    Chunk *chunk = context->chunk;
    uint32_t line = node->var.line;
    String varName = node->var.text;
    if (node->tag == VarTag::_super) {
        if (context->currentClass == nullptr) {
            String msg
                = semanticError("Can't use 'super' outside of a class.\n{}", node->var.posInfo());
            throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
        }
        if (!context->currentClass->hasSuperClass) {
            String msg = semanticError(
                "Can't use 'super' in a class with no superclass.\n{}", node->var.posInfo());
            throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_SUPER, msg};
        }
        context->chunk->emitOpArg16(opCode::LOAD_LOCAL, 0, line);
        return;
    }

    if (node->tag == VarTag::_this && context->currentClass == nullptr) {
        String msg = semanticError("Can't use 'this' outside of a class.\n{}", node->var.posInfo());
        throw ariaCompilingException{ErrorCode::SEMANTIC_INVALID_THIS, msg};
    }

    int localOffset = context->findLocalVariable(varName);
    if (localOffset >= 0) {
        chunk->emitOpArg16(opCode::LOAD_LOCAL, static_cast<uint16_t>(localOffset), line);
        return;
    }
    if (localOffset == -1) {
        String msg
            = format("Can't read local variable in its own initializer.\n{}", node->var.info());
        throw ariaCompilingException{ErrorCode::SEMANTIC_UNDEFINED_VARIABLE, msg};
    }
    if (localOffset == -2) {
        int upvalueSlot = context->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            chunk->emitOpArg16(opCode::LOAD_UPVALUE, static_cast<uint16_t>(upvalueSlot), line);
            return;
        }
        if (upvalueSlot == -1) {
            Value name = obj_val(newObjString(varName, context->gc));
            chunk->emitOpData(opCode::LOAD_GLOBAL, name, line);
            return;
        }
        if (upvalueSlot == -2) {
            fatalError(
                ErrorCode::RESOURCE_VARIABLE_OVERFLOW,
                "Too many closure variables in function.\n{}",
                node->var.info());
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
    auto &num = node->num;
    try {
        value = std::stod(num.text);
    } catch ([[maybe_unused]] const std::invalid_argument &e) {
        String msg = semanticError("Invalid number.\n{}", num.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN, msg);
    } catch ([[maybe_unused]] const std::out_of_range &e) {
        String msg = semanticError("Number out of range.\n{}", num.info());
        throw ariaCompilingException(ErrorCode::SEMANTIC_LITERAL_OVERFLOW, msg);
    }
    context->chunk->emitOpData(opCode::LOAD_CONST, number_val(value), num.line);
}

void ByteCodeGenerator::visitStringNode(StringNode *node)
{
    checkAssignFlag(node);
    auto &str = node->str;
    Value strObj = obj_val(newObjString(str.text, context->gc));
    context->chunk->emitOpData(opCode::LOAD_CONST, strObj, str.line);
}

void ByteCodeGenerator::visitTrueNode(TrueNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_TRUE, node->token_true.line);
}

void ByteCodeGenerator::visitFalseNode(FalseNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_FALSE, node->token_false.line);
}

void ByteCodeGenerator::visitNilNode(NilNode *node)
{
    checkAssignFlag(node);
    context->chunk->emitOp(opCode::LOAD_NIL, node->token_nil.line);
}

void ByteCodeGenerator::visitErrorNode(ErrorNode *node)
{
    throw ariaCompilingException(ErrorCode::SEMANTIC_UNKNOWN, node->err.text);
}

} // namespace aria