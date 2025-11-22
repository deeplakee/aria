#ifndef ARIA_CODE_H
#define ARIA_CODE_H

#include "common.h"

namespace aria {

enum class opCode : uint8_t {
    // HALT = 0,
    // Data loading and storage
    LOAD_CONST = 1,
    LOAD_NIL,
    LOAD_TRUE,
    LOAD_FALSE,

    LOAD_LOCAL,
    STORE_LOCAL,

    LOAD_UPVALUE,
    STORE_UPVALUE,
    CLOSE_UPVALUE,

    DEF_GLOBAL,
    LOAD_GLOBAL,
    STORE_GLOBAL,

    LOAD_FIELD,
    STORE_FIELD,

    LOAD_SUBSCR,
    STORE_SUBSCR,

    // Arithmetic and logical operations
    EQUAL,
    NOT_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MOD,
    NOT,
    NEGATE,

    // Stack operations
    POP,
    POP_N,

    // Output/Debug
    PRINT,
    NOP,

    // Control flow (jump and branch)
    JUMP_FWD,
    JUMP_BWD,
    JUMP_TRUE,
    JUMP_TRUE_NOPOP,
    JUMP_FALSE,
    JUMP_FALSE_NOPOP,

    // Functions and Closures
    CALL,
    CLOSURE,

    // Classes and Objects
    MAKE_CLASS,
    INHERIT,
    MAKE_METHOD,
    MAKE_INIT_METHOD,
    INVOKE_METHOD,
    LOAD_SUPER_METHOD,

    // Container operations
    MAKE_LIST,
    MAKE_MAP,

    // Module import
    IMPORT,

    // iterator
    GET_ITER,
    ITER_HAS_NEXT,
    ITER_GET_NEXT,

    // exception handling
    SETUP_EXCEPT,
    END_EXCEPT,
    THROW,

    // return
    RETURN,
};

}

#endif //ARIA_CODE_H
