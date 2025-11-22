#ifndef ARIA_ERRORCODE_H
#define ARIA_ERRORCODE_H

#include "common.h"

namespace aria {

enum class ErrorCode : uint8_t {
    OK,

    // ========== SYNTAX ERROR ==========
    SYNTAX_UNEXPECTED_TOKEN,    // 意外的符号
    SYNTAX_MISSING_TOKEN,       // 缺失的符号（例如缺少分号）
    SYNTAX_INVALID_EXPRESSION,  // 表达式无效
    SYNTAX_UNTERMINATED_STRING, // 字符串未闭合
    SYNTAX_UNEXPECTED_EOF,      // 已到达输入末尾 (unexpected end of file/input)
    SYNTAX_UNKNOWN,             // 未知语法错误

    // ========== SEMANTIC ERROR ==========
    SEMANTIC_UNDEFINED_VARIABLE,  // 使用了未声明的变量
    SEMANTIC_TYPE_MISMATCH,       // 类型不匹配
    SEMANTIC_UNDEFINED_FUNCTION,  // 调用未定义的函数
    SEMANTIC_ARGUMENT_MISMATCH,   // 函数参数不匹配
    SEMANTIC_DUPLICATE_DECL,      // 重复声明
    SEMANTIC_LITERAL_OVERFLOW,    // 数字字面量超出语言可表示范围
    SEMANTIC_INVALID_ASSIGNMENT,  // 无效的赋值对象（如给常量赋值或给非左值赋值）
    SEMANTIC_UNKNOWN_OPERATOR,    // 未知的操作符
    SEMANTIC_INVALID_CONTINUE,    // continue 不在循环中
    SEMANTIC_INVALID_BREAK,       // break 不在循环中
    SEMANTIC_INVALID_RETURN,      // 顶层 return
    SEMANTIC_INVALID_INIT_RETURN, // 构造器里 return 带值
    SEMANTIC_INVALID_SUPER,       // 无效使用 super
    SEMANTIC_INVALID_THIS,        // 无效使用 this
    SEMANTIC_UNKNOWN,             // 未知语义错误

    // ========== RUNTIME ERROR ==========
    RUNTIME_STACK_OVERFLOW,      // 栈溢出
    RUNTIME_STACK_UNDERFLOW,     // 栈下溢（弹空栈）
    RUNTIME_NULL_REFERENCE,      // 空引用/空指针
    RUNTIME_DIVISION_BY_ZERO,    // 除零错误
    RUNTIME_MODULO_BY_ZERO,      // 模零错误
    RUNTIME_OUT_OF_BOUNDS,       // 下标越界
    RUNTIME_INVALID_INSTRUCTION, // 无效的指令
    RUNTIME_EXISTED_VARIABLE,    // 重复定义的变量
    RUNTIME_UNDEFINED_VARIABLE,  // 未定义的变量
    RUNTIME_TYPE_ERROR,          // 运行时类型错误
    RUNTIME_MODULE_INIT_ERROR,   // 模块初始化错误
    RUNTIME_INVALID_CALL,        // 无法调用的对象（例如试图调用非函数）
    RUNTIME_MISMATCH_ARG_COUNT,  // 不匹配的参数个数
    RUNTIME_INVALID_FIELD_OP,    // 无效的属性访问
    RUNTIME_IMPORT_FAILED,       // 模块导入失败
    RUNTIME_INVALID_INDEX_OP,    // 无效的索引访问
    RUNTIME_INVALID_FRAME,       // 无效的栈帧
    RUNTIME_INVALID_STATE,       // 无效的执行快照
    RUNTIME_UNCAUGHT_EXCEPTION,  // 未捕获的错误/异常
    RUNTIME_UNKNOWN,             // 未知运行时错误

    // ========== INTERNAL ERROR ==========
    INTERNAL_INVALID_AST,       // AST 节点非法
    INTERNAL_BYTECODE_GEN_FAIL, // 字节码生成失败
    INTERNAL_UNSUPPORTED_NODE,  // 不支持的 AST 节点
    INTERNAL_ILLEGAL_SCOPE,     // 无效的作用域
    INTERNAL_NULL_POINTER,      // 意外的空指针
    INTERNAL_STACK_CORRUPTION,  // 调用栈损坏
    INTERNAL_UNKNOWN,           // 未知内部错误

    // ========== RESOURCE ERROR ==========
    RESOURCE_CHUNK_OVERFLOW,      // 单个字节码块过大
    RESOURCE_JUMP_OVERFLOW,       // 跳转偏移量过大
    RESOURCE_LINE_OVERFLOW,       // 源码行号溢出
    RESOURCE_MEMORY_EXHAUSTED,    // 内存分配失败
    RESOURCE_STRING_POOL_FULL,    // 字符串池已满
    RESOURCE_VARIABLE_OVERFLOW,   // 变量过多
    RESOURCE_LIST_OVERFLOW,       // 列表过大
    RESOURCE_LIST_CONSTRUCT_FAIL, // 列表创建失败
    RESOURCE_MAP_CONSTRUCT_FAIL,  // 哈希表创建失败
    RESOURCE_MAP_OVERFLOW,        // 哈希表过大
    RESOURCE_STRING_OVERFLOW,     // 字符串过大
};

}

#endif //ARIA_ERRORCODE_H
