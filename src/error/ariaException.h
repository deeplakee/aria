#ifndef ARIA_ARIAEXCEPTION_H
#define ARIA_ARIAEXCEPTION_H

#include "common.h"
#include "error/ErrorCode.h"
#include <exception>

namespace aria {

class ariaException : public std::exception
{
public:
    explicit ariaException(ErrorCode _code, String _msg)
        : code{_code}
        , msg{std::move(_msg)}
    {}

    const char *what() const noexcept override { return msg.c_str(); }

    ErrorCode code;
    String msg;
};

class ariaCompilingException : public ariaException
{
public:
    explicit ariaCompilingException(ErrorCode _code, String _msg)
        : ariaException{_code, std::move(_msg)}
    {}
};

class ariaRuntimeException : public ariaException
{
public:
    explicit ariaRuntimeException(ErrorCode _code, String _msg)
        : ariaException{_code, std::move(_msg)}
    {}
};

class ariaInvalidAssignException : public ariaException
{
public:
    explicit ariaInvalidAssignException(ErrorCode _code, String _msg)
        : ariaException{_code, std::move(_msg)}
    {}
};

} // namespace aria

#endif //ARIA_ARIAEXCEPTION_H
