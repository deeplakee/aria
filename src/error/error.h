#ifndef ARIA_ERROR_H
#define ARIA_ERROR_H

#include "common.h"
#include "error/ErrorCode.h"
#include "error/ariaException.h"
#include "util/util.h"
#include <source_location>

namespace aria {

template<typename... Args>
void error(const String &fmt, Args &&...args)
{
    println(std::cerr, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] void fatalError(ErrorCode code, const String &fmt, Args &&...args)
{
    auto msg = "[fatalError] " + std::vformat(fmt, std::make_format_args(args...));
    error(msg);
    throw ariaException(code, msg);
}

template<typename... Args>
String syntaxError(const String &fmt, Args &&...args)
{
    return "[SyntaxError]" + std::vformat(fmt, std::make_format_args(args...));
}

template<typename... Args>
String semanticError(const String &fmt, Args &&...args)
{
    return "[SemanticError]" + std::vformat(fmt, std::make_format_args(args...));
}

template<typename... Args>
String runtimeError(const String &fmt, Args &&...args)
{
    return "[RuntimeError]" + std::vformat(fmt, std::make_format_args(args...));
}

} // namespace aria

#endif //ARIA_ERROR_H
