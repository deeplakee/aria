#include "compile/compiler.h"
#include "compile/byteCodeGenerator.h"
#include "compile/functionContext.h"
#include "compile/lexer.h"
#include "compile/parser.h"
#include "compile/token.h"
#include "error/ariaException.h"
#include "memory/gc.h"
#include "object/objFunction.h"
#include "object/objString.h"

#include <utility>

namespace aria {

ObjFunction *Compiler::compile(String sourceLocation, String source, GC *gc, ValueHashTable *globals)
{
    return compile(std::move(sourceLocation), "anonymous", std::move(source), gc, globals);
}

ObjFunction *Compiler::compile(
    String moduleLocation, String moduleName, String source, GC *gc, ValueHashTable *globals)
{
    try {
#ifdef DEBUG_PRINT_SRC_CODE
        std::cout << source << std::endl;
#endif
        Lexer lexer{moduleLocation, std::move(source)};
        auto tokens = lexer.tokenize();
        if (lexer.hadError()) {
            return nullptr;
        }
        Parser parser{tokens};
        auto ast = parser.parse();
        if (parser.hasError()) {
            return nullptr;
        }
#ifdef DEBUG_PRINT_CODE_AST
        ast->display();
#endif

        auto generator = ByteCodeGenerator{moduleName, moduleLocation, globals, gc};
        auto fn = generator.generateCode(ast);

#ifdef DEBUG_PRINT_COMPILED_CODE
        fn->chunk->disassemble(moduleName);
#endif
        return fn;
    } catch (const ariaException &e) {
        error(e.what());
    }
    return nullptr;
}

} // namespace aria